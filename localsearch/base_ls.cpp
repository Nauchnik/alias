#include "base_ls.h"
#include <sstream>

base_local_search::base_local_search() :
	graph_file_name(""),
	cnf_name(""),
	solver_name(""),
	alias_script_name("ALIAS.py"),
	pcs_name(""),
	cpu_cores(1),
	cpu_lim(DEFAULT_TIME_LIMIT),
	skipped_points_count(0),
	interrupted_points_count(0),
	wall_time_solving(0),
	is_jump_mode(true),
	vars_decr_times(0),
	is_solve(false),
	jump_lim(DEFAULT_JUMP_LIM),
	result_output_name(""),
	script_out_str(""),
	backdoor_file_name(""),
	rand_from(0),
	rand_to(0),
	rand_points(0),
	verbosity(1)
{
	start_t = chrono::high_resolution_clock::now();
	srand(time(NULL));
	known_backdoor.value.resize(0);
	global_record_point.value.resize(0);
	local_record_point.estimation = HUGE_VAL;
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

void base_local_search::loadBackdoor()
{
	if (backdoor_file_name != "") {
		if (verbosity > 1)
			cout << "loadBackdoor()" << endl;
		ifstream ifile(backdoor_file_name);
		stringstream sstream;
		string str;
		getline(ifile, str);
		sstream << str;
		int val;
		known_backdoor.value.resize(vars.size());
		for (auto x : known_backdoor.value)
			x = false;
		cout << "known backdoor : " << endl;
		while (sstream >> val) {
			cout << val << " ";
			known_backdoor.value[val - 1] = true;
		}
		cout << endl;
		ifile.close();
	}
}

// Parse a dimacs CNF formula from a given file and
// save its clauses into a given vector.
vector<var> base_local_search::getAllCnfVars(const string filename)
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

	vector<var> vars_vec;
	var tmp_var;
	for (int i = 0; i < vars_count; i++) {
		tmp_var.value = i + 1;
		tmp_var.calculations = 0;
		tmp_var.global_records = 0;
		vars_vec.push_back(tmp_var);
	}

	return vars_vec;
}

vector<var> base_local_search::readVarsFromPcs(string pcs_name)
{
	ifstream pcs_file(pcs_name);
	if (!pcs_file.is_open()) {
		cerr << "error while opening " << pcs_name;
		exit(-1);
	}

	string str;
	vector<var> vars_vec;
	var tmp_var;
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
		sstream >> tmp_var.value;
		tmp_var.calculations = 0;
		tmp_var.global_records = 0;
		vars_vec.push_back(tmp_var);
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
	if (time_span.count() >= cpu_lim) {
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

void base_local_search::reportResult()
{
	stringstream sstream;
	sstream << "Backdoor (numeration from 1):" << endl;
	sstream << global_record_point.getStr(vars);
	sstream << "Estimation for 1 CPU core : " << global_record_point.estimation << " seconds" << endl;
	sstream << "Estimation for " << cpu_cores << " CPU cores : " << global_record_point.estimation / cpu_cores << " seconds" << endl;
	if (is_solve) {
		sstream << alias_script_name << " output on the found backdoor : " << endl;
		sstream << script_out_str << endl;
	}
	sstream << "total wall time " << timeFromStart() << endl;
	cout << sstream.str();
	
	if (result_output_name != "") {
		ofstream ofile(result_output_name);
		ofile << sstream.str();
		ofile.close();
	}
}

bool strPrefix(const string init_str, const string prefix, string &res_str)
{
	size_t found = init_str.find(prefix);
	if (found != string::npos) {
		res_str = init_str.substr(found + prefix.length());
		return true;
	}
	return false;
}

void base_local_search::parseParams(const int argc, char *argv[])
{
	for (int i = 1; i < argc; i++) {
		string par_str = argv[i];
		string res_str;
		if (strPrefix(par_str, "-pcs=", res_str))
			pcs_name = res_str;
		else if (strPrefix(par_str, "-script=", res_str))
			alias_script_name = res_str;
		else if (strPrefix(par_str, "-solver=", res_str))
			solver_name = res_str;
		else if (strPrefix(par_str, "-cpu-lim=", res_str))
			istringstream(res_str) >> cpu_lim;
		else if (strPrefix(par_str, "-jump-lim", res_str))
			istringstream(res_str) >> jump_lim;
		else if (strPrefix(par_str, "-verb=", res_str))
			istringstream(res_str) >> verbosity;
		else if (strPrefix(par_str, "-backdoor=", res_str))
			istringstream(res_str) >> backdoor_file_name;
		else if (strPrefix(par_str, "-rand-from=", res_str))
			istringstream(res_str) >> rand_from;
		else if (strPrefix(par_str, "-rand-to=", res_str))
			istringstream(res_str) >> rand_to;
		else if (strPrefix(par_str, "-rand-points=", res_str))
			istringstream(res_str) >> rand_points;
		else if (par_str == "--solve")
			is_solve = true;
		else if (par_str == "--nojump")
			is_jump_mode = false;
		else if (cnf_name == "")
			cnf_name = par_str;
		else if (result_output_name == "")
			result_output_name = par_str;
	}

	if ((rand_from > 0) && (!rand_to))
		rand_to = rand_from;
	if ((rand_to > 0) && (!rand_from))
		rand_from = rand_to;
	if ((rand_to > 0) && (!rand_points))
		rand_points = 100;
	
	cout << "cnf name " << cnf_name << endl;
	cout << "pcs name " << pcs_name << endl;
	cout << "solver name " << solver_name << endl;
	cout << "alias script name " << alias_script_name << endl;
	cout << "cpu lim " << cpu_lim << endl;
	cout << "is_jump_mode " << is_jump_mode << endl;
	cout << "jump lim " << jump_lim << endl;
	cout << "is solve " << is_solve << endl;
	cout << "verbosity " << verbosity << endl;
	cout << "result_output_name " << result_output_name << endl;
	cout << "backdoor_file_name " << backdoor_file_name << endl;
	cout << "rand_to " << rand_to << endl;
	cout << "rand_from " << rand_from << endl;
	cout << "rand_points " << rand_points << endl;
	
	if (cnf_name == "") {
		cerr << "cnf name is empty" << endl;
		exit(-1);
	}
}

void base_local_search::init()
{
	loadVars();
	loadBackdoor();
	setGraphFileName();
	cpu_cores = getCpuCores();
}

void base_local_search::calculateEstimation(point &cur_point)
{
	string command_str = getScriptCommand(ESTIMATE, cur_point);
	if (verbosity > 1)
		cout << "command_str " << command_str << endl;
	
	string out_str = getCmdOutput(command_str.c_str());
	string bef_str = "SUCCESS, 0, 0, ";
	size_t pos1 = out_str.find(bef_str);
	if (pos1 != string::npos) {
		size_t pos2 = pos1 + bef_str.size();
		out_str = out_str.substr(pos2, out_str.size() - pos2);
		if (verbosity > 1)
			cout << "output : " << out_str << endl;
		stringstream sstream;
		sstream << out_str;
		sstream >> cur_point.estimation;
	}
	if (!is_jump_mode) {
		for (unsigned j = 0; j < cur_point.value.size(); j++)
			if (cur_point.value[j])
				vars[j].calculations++;
	}
}

bool base_local_search::solveInstance()
{
	if (isTimeExceeded() || (!is_solve))
		return false;

	cout << "solve the instance using a record point" << endl;

	wall_time_solving = cpu_lim - timeFromStart();
	if (verbosity > 0)
		cout << "solving wall time " << wall_time_solving << endl;
	
	if (wall_time_solving < (global_record_point.estimation / cpu_cores) / 10) {
		cout << "*** stop, wall_time_solving < (global_record_point.estimation / cpu_cores) / 10)" << endl;
		return false;
	}

	if (global_record_point.weight() > MAX_SOLVING_VARS) {
		cout << "*** stop, record point weight " << global_record_point.weight() << " > " << MAX_SOLVING_VARS << endl;
		return false;
	}

	string command_str = getScriptCommand(SOLVE, global_record_point);

	if (verbosity > 1)
		cout << "command_str : " << command_str << endl;
	script_out_str = getCmdOutput(command_str.c_str());
	cout << endl << alias_script_name << " output on the found backdoor : " << endl;
	cout << script_out_str << endl;
	
	return true;
}

string base_local_search::getScriptCommand(const int mode, const point cur_point)
{
	stringstream sstream;
	string command_str = "python3.6 " + alias_script_name;
	if (mode == SOLVE) {
		sstream << wall_time_solving;
		command_str += " -s -wtlimitsolve " + sstream.str();
		sstream.str(""); sstream.clear();
	}
	else
		command_str += " -e";

	sstream << cpu_cores;
	command_str += " -number_of_processes " + sstream.str();
	sstream.str(""); sstream.clear();
	command_str += " -cnf " + cnf_name + " -solver " + solver_name;

	if (!is_jump_mode) {
		sstream << local_record_point.estimation * 1.1;
		command_str += " -bkv " + sstream.str();
		sstream.str(""); sstream.clear();
	}

	for (unsigned i = 0; i < vars.size(); i++) {
		sstream << vars[i].value;
		command_str += " -v" + sstream.str() + " '" + (cur_point.value[i] == true ? '1' : '0') + "'";
		sstream.str(""); sstream.clear();
	}

	return command_str;
}

void base_local_search::printGlobalRecordPoint()
{
	cout << "final point weight : " << global_record_point.weight() << endl;
	cout << "final runtime estimation on 1 CPU core : " << global_record_point.estimation << endl;
	cout << "final runtime estimation on " << cpu_cores << " CPU cores : " << global_record_point.estimation / cpu_cores << endl;
	cout << "final backdoor : " << endl;
	cout << global_record_point.getStr(vars);
}

bool base_local_search::isKnownBackdoor()
{
	if (verbosity > 1)
		cout << "isKnownBackdoor()" << endl;
	return (known_backdoor.value.size() > 0) ? true : false;
}
