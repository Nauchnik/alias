#include "base_ls.h"
#include <sstream>

base_local_search::base_local_search() :
	graph_file_name(""),
	cnf_name(""),
	cpu_cores(1),
	time_limit(DEFAULT_TIME_LIMIT),
	skipped_points_count(0),
	interrupted_points_count(0)
{
	start_t = chrono::high_resolution_clock::now();
	srand(time(NULL));
}

void base_local_search::loadVars()
{
	if (pcs_name != "")
		vars = readVarsFromPcs(pcs_name); // get search space from a given pcs-file
	else
		vars = getAllCnfVars(cnf_name);

	if (!vars.size()) {
		cerr << "*** search space is empty" << endl;
		exit(-1);
	}

	cout << "search space variables number " << vars.size() << endl;
}

// Parse a dimacs CNF formula from a given file and
// save its clauses into a given vector.
vector<unsigned> base_local_search::getAllCnfVars(const string filename)
{
	int vars_count = 0;
	ifstream ifile(filename.c_str());
	if (!ifile.is_open()) {
		cerr << "Error : failed to open " << filename << endl;
		exit(-1);
	}

	string str;

	while (getline(ifile, str)) {
		if ((!str.size()) || (str[0] == 'c'))
			continue; // skip empty and comment strings
		stringstream sstream;
		sstream << str;
		if (str[0] == 'p') {
			cout << "try to get vars from string " << str << endl;
			sstream >> str >> str >> vars_count;
		}
	}

	if (vars_count <= 0) {
		cerr << "*** incorrect vars_count " << vars_count << endl;
		exit(-1);
	}

	ifile.close();

	vector<unsigned> vars_vec;
	for (int i = 0; i < vars_count; i++)
		vars_vec.push_back(i + 1);

	return vars_vec;
}

vector<unsigned> base_local_search::readVarsFromPcs(string pcs_name)
{
	ifstream pcs_file(pcs_name);
	if (!pcs_file.is_open()) {
		cerr << "error while opening " << pcs_name;
		exit(-1);
	}

	string str;
	unsigned var;
	vector<unsigned> vars_vec;
	while (getline(pcs_file, str)) {
		if (str.size() <= 2)
			continue;
		size_t pos = str.find(' ');
		if (pos != string::npos)
			str = str.substr(1, pos - 1);
		bool isDigitVal = true;
		for (auto &x : str)
			if (!isdigit(x)) {
				isDigitVal = false;
				break;
			}
		if (!isDigitVal)
			continue;
		stringstream sstream;
		sstream << str;
		sstream >> var;
		vars_vec.push_back(var);
	}

	pcs_file.close();

	return vars_vec;
}

void base_local_search::setGraphFileName()
{
	string cnf_name_short, solver_name_short;
	size_t found1 = cnf_name.find_last_of("/");
	size_t found2 = cnf_name.find_last_of(".");
	if (found1 != string::npos)
		cnf_name_short = cnf_name.substr(found1+1, found2-found1-1);
	else
		cnf_name_short = cnf_name;
	found1 = solver_name.find_last_of("/");
	if (found1 != string::npos)
		solver_name_short = solver_name.substr(found1 + 1, solver_name.size() - found1 - 1);
	else
		solver_name_short = solver_name;
	cout << "cnf_name_short " << cnf_name_short << endl;
	graph_file_name = "alias_" + solver_name_short + "_" + cnf_name_short;
	cout << "graph_file_name " << graph_file_name << endl;
}

void base_local_search::writeToGraphFile(const string str) 
{
	graph_file.open(graph_file_name, ios_base::app);
	graph_file << str << endl;
	graph_file.close();
}

int base_local_search::getCpuCores()
{
	int cpu_cores = std::thread::hardware_concurrency();
	cout << "cpu_cores " << cpu_cores << endl;
	if (cpu_cores <= 0)
		exit(-1);
	return cpu_cores;
}

string base_local_search::getCmdOutput(const char* cmd) {
	std::string result = "";
#ifndef _WIN32
	char buffer[128];
	FILE* pipe = popen(cmd, "r");
	if (!pipe) throw std::runtime_error("popen() failed!");
	try {
		while (!feof(pipe)) {
			if (fgets(buffer, 128, pipe) != NULL)
				result += buffer;
		}
	}
	catch (...) {
		pclose(pipe);
		throw;
	}
	pclose(pipe);
#endif
	return result;
}

double base_local_search::timeFromStart()
{
	chrono::high_resolution_clock::time_point cur_t = chrono::high_resolution_clock::now();
	chrono::duration<double> time_span = chrono::duration_cast<chrono::duration<double>>(cur_t - start_t);
	return time_span.count();
}

bool base_local_search::isTimeExceeded()
{
	chrono::high_resolution_clock::time_point cur_t = chrono::high_resolution_clock::now();
	chrono::duration<double> time_span = chrono::duration_cast<chrono::duration<double>>(cur_t - start_t);
	if (time_span.count() >= time_limit) {
		cout << "*** time is up" << endl;
		return true;
	}
	return false;
}

bool base_local_search::isEstTooLong() // for simple instances
{
	if (global_record_point.estimation / cpu_cores <= timeFromStart()) {
		cout << "*** estimation / " << cpu_cores << " is less than elapsed time" << endl;
		return true;
	}
	return false;
}

bool base_local_search::solveInstance()
{
	cout << "solve an instance using record point" << endl;

	if (isTimeExceeded())
		return false;

	if (global_record_point.weight() > MAX_SOLVING_VARS) {
		cout << "*** stop, record point weight " << global_record_point.weight() << " > " << MAX_SOLVING_VARS << endl;
		return false;
	}

	string command_str = getScriptCommand(solver_script_name, global_record_point);

	cout << "command_str : " << command_str << endl;
	string out_str = getCmdOutput(command_str.c_str());
	cout << out_str << endl;
	cout << "solving done" << endl;

	return true;
}

string base_local_search::getScriptCommand(const string script_name, const point cur_point)
{
	string command_str = "/share/apps/python/3.6.4/bin/python3.6 " + script_name + " -cnf " + cnf_name +
		" -solver " + solver_name;

	stringstream sstream;
	sstream << global_record_point.estimation;
	command_str += " -bkv " + sstream.str();
	sstream.str(""); sstream.clear();

	for (unsigned i = 0; i < vars.size(); i++) {
		sstream << vars[i];
		command_str += " -v" + sstream.str() + " '" + (cur_point.value[i] == true ? '1' : '0') + "'";
		sstream.str(""); sstream.clear();
	}

	return command_str;
}

void base_local_search::reportFinalEstimation()
{
	cout << "final point weight : " << global_record_point.weight() << endl;
	cout << "final runtime estimation on 1 CPU core : " << global_record_point.estimation << endl;
	cout << "final runtime estimation on " << cpu_cores << " CPU cores : " << global_record_point.estimation / cpu_cores << endl;
	printGlobalRecordPoint();
	cout << "total local search time " << timeFromStart() << endl;

	cout << "skipped points : " << skipped_points_count << endl;
	cout << "checked points : " << checked_points.size() << endl;
	cout << "interrupted points : " << interrupted_points_count << endl;
}

void base_local_search::init()
{
	loadVars();
	setGraphFileName();
}

void base_local_search::calculateEstimation(point &cur_point)
{
	string command_str = getScriptCommand(estimation_script_name, cur_point);

	//cout << "command_str : " << command_str << endl;
	string out_str = getCmdOutput(command_str.c_str());
	string bef_str = "SUCCESS, 0, 0, ";
	size_t pos1 = out_str.find(bef_str);
	if (pos1 != string::npos) {
		size_t pos2 = pos1 + bef_str.size();
		out_str = out_str.substr(pos2, out_str.size() - pos2);
		//cout << "output : " << out_str << endl;
		stringstream sstream;
		sstream << out_str;
		sstream >> cur_point.estimation;
	}
}