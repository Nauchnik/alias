/* Author: Oleg Zaikin, ISDCT SB RAS, Irkutsk */

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <stdexcept>
#include <chrono>
#include <stdio.h>
#include <algorithm>
#include <thread>

using namespace std;

const unsigned INITIAL_JUMP_STEP = 10;
const unsigned MAX_SOLVING_VARS = 32;
const unsigned MIN_VARS_JUMP_FROM = 40;
const unsigned MAX_ITERATIONS = 2;
const double DEFAULT_TIME = 3600;

struct point
{
	vector<bool> value;
	double estimation;
	unsigned weight() {
		unsigned result = 0;
		for (auto x : value) result += x ? 1 : 0;
		return result;
	}
	bool operator==(const point& p) const { return value == p.value; }
};

struct local_search_data
{
	vector<unsigned> vars;
	vector<point> checked_points;
	unsigned skipped_points_count;
	unsigned interrupted_points_count;
	unsigned vars_decr_times;
	unsigned jump_step;
	point local_record_point;
	point global_record_point;
	point before_jump_point;
	bool is_jump_mode;
	string graph_file_name;
	fstream graph_file;

	bool isChecked(const point cur_point) {
		return (find(checked_points.begin(), checked_points.end(), cur_point) != checked_points.end());
	}
	void backJump() {
		cout << "* backjumping" << endl;
		local_record_point.estimation = 1e+308;
		//cout << "* forget global record" << endl;
		jump_step = INITIAL_JUMP_STEP;
		vars_decr_times = 0;
		local_record_point = before_jump_point;
		cout << "back to the point with weight " << local_record_point.weight() << " : " << endl;
		printPoint(before_jump_point);
		is_jump_mode = false; // jump only once
		cout << "* is_jump_mode " << is_jump_mode << endl;
	}
	void printPoint(const point p)
	{
		for (unsigned i = 0; i < p.value.size(); i++)
			if (p.value[i])
				cout << vars[i] << " ";
		cout << endl;
	}
	void printGlobalRecordPoint()
	{
		printPoint(global_record_point);
	}
	void writeToGraphFile(const string str)
	{
		graph_file.open(graph_file_name, ios_base::app);
		graph_file << str << endl;
		graph_file.close();
	}
	point permutateRecordPoint()
	{
		cout << "* permutate record point" << endl;
		point mod_point = global_record_point;
		unsigned changed_vals = 0;
		unsigned vars_to_change = mod_point.weight() / 3;
		cout << "vars_to_change " << vars_to_change << endl;
		for (unsigned i = 0; i < vars_to_change; i++) {
			unsigned rand_ind = rand() % mod_point.value.size();
			mod_point.value[i] = (mod_point.value[i] == true) ? false : true;
		}
		cout << "modified point with weight " << mod_point.weight() << " : " << endl;
		printPoint(mod_point);
		return mod_point;
	}
};

local_search_data local_s_d;
string cnf_name = "";
string solver_name = "";
string estimation_script_name = "";
string solver_script_name = "";
double time_limit = 0;
int cpu_cores = 0;
chrono::high_resolution_clock::time_point start_t;

vector<unsigned> readVarsFromPcs(string pcs_name);
vector<unsigned> getAllCnfVars(const string filename);
void iteratedGBFS(point start_point);
void GBFS(const point start_point);
void calculateEstimation(point &cur_point);
bool solveInstance();
bool isTimeExceeded();
bool isEstTooLong();
string getCmdOutput(const char* cmd);
string getScriptCommand(const string script_name, const point cur_point);
point jumpPoint(point cur_point);
void updateLocalRecord(point cur_point);
double timeFromStart();

int main(int argc, char *argv[])
{
	if (argc < 5) {
		cerr << "Usage: -cnf cnf_name -estimation_script script_name -solver_script script_name -pcs pcs_name -solver solver_name -time time_limit" << endl;
		exit(-1);
	}

	srand(time(NULL));

	string pcs_name = "";
	
	for (int i = 1; i < argc; i++) {
		string par_str = argv[i];
		if ((par_str == "-cnf") && (i+1 < argc))
			cnf_name = argv[i+1];
		else if ((par_str == "-pcs") && (i+1 < argc))
			pcs_name = argv[i+1];
		else if ((par_str == "-estimation_script") && (i+1 < argc))
			estimation_script_name = argv[i + 1];
		else if ((par_str == "-solver_script") && (i + 1 < argc))
			solver_script_name = argv[i + 1];
		else if ((par_str == "-solver") && (i+1 < argc))
			solver_name = argv[i + 1];
		else if ((par_str == "-time") && (i+1 < argc))
			time_limit = atof(argv[i+1]);
	}
	
	local_s_d.graph_file_name = "alias_records";
	size_t found = cnf_name.find_last_of("/");
	if (found != string::npos) // if full path, get its base part
		local_s_d.graph_file_name = cnf_name.substr(0, found) + "/" +local_s_d.graph_file_name;
	
	cout << "graph_file_name " << local_s_d.graph_file_name << endl;
	
	if (time_limit == 0) {
		time_limit = DEFAULT_TIME;
		cout << "use default time limit " << DEFAULT_TIME << endl;
	}
	
	cout << "cnf_name " << cnf_name << endl;
	cout << "pcs_name " << pcs_name << endl;
	cout << "solver_name " << solver_name << endl;
	cout << "estimation_script_name " << estimation_script_name << endl;
	cout << "solver_script_name " << solver_script_name << endl;
	cout << "time_limit " << time_limit << endl;
	
	if (pcs_name != "")
		local_s_d.vars = readVarsFromPcs(pcs_name); // get search space from a given pcs-file
	else
		local_s_d.vars = getAllCnfVars(cnf_name);

	if (!local_s_d.vars.size()) {
		cerr << "*** search space is empty" << endl;
		exit(-1);
	}
	
	cout << "search space variables number " << local_s_d.vars.size() << endl;
	
	point start_point;
	start_point.value.resize(local_s_d.vars.size());
	cout << "start point : " << endl;
	for (auto x : start_point.value)
		x = true;
	local_s_d.printPoint(start_point);
	
	start_t = chrono::high_resolution_clock::now();

	// init parameters values
	local_s_d.skipped_points_count = 0;
	local_s_d.interrupted_points_count = 0;
	local_s_d.is_jump_mode = true;

	cpu_cores = std::thread::hardware_concurrency();
	cout << "cpu_cores " << cpu_cores << endl;
	if (cpu_cores <= 0)
		exit(-1);
	
	iteratedGBFS(start_point);
	
	cout << "final point weight : " << local_s_d.global_record_point.weight() << endl;
	cout << "final runtime estimation on 1 CPU core : " << local_s_d.global_record_point.estimation << endl;
	cout << "final runtime estimation on " << cpu_cores << " CPU cores : " << local_s_d.global_record_point.estimation / cpu_cores << endl;
	local_s_d.printGlobalRecordPoint();
	cout << "total local search time " << timeFromStart() << endl;
	
	cout << "skipped points : " << local_s_d.skipped_points_count << endl;
	cout << "checked points : " << local_s_d.checked_points.size() << endl;
	cout << "interrupted points : " << local_s_d.interrupted_points_count << endl;
	
	if (!isTimeExceeded())
		solveInstance();

	cout << "*** final total time " << timeFromStart() << endl;
	
	return 0;
}

// Parse a dimacs CNF formula from a given file and
// save its clauses into a given vector.
vector<unsigned> getAllCnfVars(const string filename)
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

vector<unsigned> readVarsFromPcs(string pcs_name)
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
			str = str.substr(1,pos-1);
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

void iteratedGBFS(point start_point)
{
	unsigned iteration_count = 0;
	local_s_d.global_record_point = start_point;
	local_s_d.global_record_point.estimation = 1e+308;
	stringstream sstream;
	sstream << "vars est-1-core est-" << cpu_cores << "-cores elapsed";
	local_s_d.writeToGraphFile(sstream.str());
	sstream.str(""); sstream.clear();
	for (;;) {
		cout << endl << "*** GBFS iteration # " << iteration_count << endl;
		if (iteration_count > 0) {
			stringstream sstream;
			sstream << "--- GBFS iteration # " << iteration_count;
			local_s_d.writeToGraphFile(sstream.str());
			sstream.str(""); sstream.clear();
		}
		GBFS(start_point);
		iteration_count++;
		if (isTimeExceeded() || isEstTooLong() || (iteration_count > MAX_ITERATIONS))
		{
			cout << "*** interrupt the search" << endl;
			break;
		}
		start_point = local_s_d.permutateRecordPoint();
	}
	sstream << "--- start solving, time " << timeFromStart();
	local_s_d.writeToGraphFile(sstream.str());
	sstream.str(""); sstream.clear();
}

void GBFS(const point start_point)
{
	bool is_record_updated;
	local_s_d.vars_decr_times = 0;
	local_s_d.jump_step = INITIAL_JUMP_STEP;
	local_s_d.local_record_point = start_point;
	local_s_d.local_record_point.estimation = 1e+308;

	cout << "GBFS() start" << endl;
	
	for (;;) {
		is_record_updated = false;
		vector<unsigned> changing_vars;
		for (unsigned j = 0; j < local_s_d.local_record_point.value.size(); j++)
			changing_vars.push_back(j);
		random_shuffle(changing_vars.begin(), changing_vars.end());
		/*cout << "changing_vars : " << endl;
		for (auto x : changing_vars)
			cout << x << " ";
		cout << endl;*/
		int changing_vars_count = changing_vars.size();
		//cout << "new start point " << endl;
		for (int i = -1; i < changing_vars_count; i++) { // i == -1 is required to check a point itself
			if (isTimeExceeded() || isEstTooLong())
				break; // if time is up, then stop GBFS
			point cur_point = local_s_d.local_record_point;
			if (i >= 0)
				cur_point.value[changing_vars[i]] = local_s_d.local_record_point.value[changing_vars[i]] ? false : true;
			if  (local_s_d.isChecked(cur_point)) {
				local_s_d.skipped_points_count++;
				continue;
			}
			calculateEstimation(cur_point);
			local_s_d.checked_points.push_back(cur_point);

			if (cur_point.estimation <= 0) {
				cout << "skip point with estimation " << cur_point.estimation << endl;
				local_s_d.interrupted_points_count++;
				if ( local_s_d.is_jump_mode ) {
					local_s_d.backJump();
					is_record_updated = true;
					break;
				}
				else
					continue;
			}

			if (cur_point.estimation < local_s_d.local_record_point.estimation) {
				is_record_updated = true;
				updateLocalRecord(cur_point);
				break;
			}
			else {
				if (i >= 0) // return initial value
					cur_point.value[changing_vars[i]] = local_s_d.local_record_point.value[changing_vars[i]]; 
			}
		}

		if (isTimeExceeded() || isEstTooLong())
			break; // if time is up, then stop GBFS

		if (!is_record_updated) { // if a local minimum has been found, then stop GBFS
			cout << "a local minimum has been found, stop GBFS" << endl;
			break;
		}
	}

	cout << "GBFS() end" << endl << endl;
}

void calculateEstimation(point &cur_point)
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

string getScriptCommand(const string script_name, const point cur_point)
{
	string command_str = "/share/apps/python/3.6.4/bin/python3.6 " + script_name + " -cnf " + cnf_name +
		" -solver " + solver_name;
	
	stringstream sstream;
	sstream << local_s_d.global_record_point.estimation;
	command_str += " -bkv " + sstream.str();
	sstream.str(""); sstream.clear();

	for (unsigned i = 0; i < local_s_d.vars.size(); i++) {
		sstream << local_s_d.vars[i];
		command_str += " -v" + sstream.str() + " '" + (cur_point.value[i] == true ? '1' : '0') + "'";
		sstream.str(""); sstream.clear();
	}
	
	return command_str;
}

void updateLocalRecord(point cur_point)
{
	local_s_d.local_record_point = cur_point;
	
	if (local_s_d.local_record_point.estimation < local_s_d.global_record_point.estimation) {
		point prev_global_record_point = local_s_d.global_record_point;
		local_s_d.global_record_point = local_s_d.local_record_point;
		cout << "** new global_record_esimation " << local_s_d.global_record_point.estimation << 
			    " with weight " << local_s_d.global_record_point.weight() << endl;
		local_s_d.printGlobalRecordPoint();
		stringstream sstream;
		//"vars est-1-core est-" << cpu_cores << "-cores elapsed" << endl;
		sstream << local_s_d.global_record_point.weight() << " " << local_s_d.global_record_point.estimation << " " <<
			local_s_d.global_record_point.estimation / cpu_cores << " " << timeFromStart();
		local_s_d.writeToGraphFile(sstream.str());
		if (local_s_d.global_record_point.weight() <= prev_global_record_point.weight())
			local_s_d.vars_decr_times++;
		else {
			local_s_d.vars_decr_times = 0;
			if (local_s_d.is_jump_mode) {
				local_s_d.is_jump_mode = false;
				cout << "** is_jump_mode " << local_s_d.is_jump_mode << endl;
			}
		}
	}
	else {
		cout << "* new local_record_estimation " << local_s_d.local_record_point.estimation <<
			" with weight " << local_s_d.local_record_point.weight() << endl;
	}
	cout << "time " << timeFromStart() << endl;
	
	if ( (local_s_d.is_jump_mode) && (cur_point.weight() > MIN_VARS_JUMP_FROM) && (local_s_d.vars_decr_times >= 4) )
		local_s_d.local_record_point = jumpPoint(cur_point);
	else
		local_s_d.local_record_point = cur_point;
}

string getCmdOutput(const char* cmd) {
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

double timeFromStart() 
{
	chrono::high_resolution_clock::time_point cur_t = chrono::high_resolution_clock::now();
	chrono::duration<double> time_span = chrono::duration_cast<chrono::duration<double>>(cur_t - start_t);
	return time_span.count();
}

bool isTimeExceeded()
{
	chrono::high_resolution_clock::time_point cur_t = chrono::high_resolution_clock::now();
	chrono::duration<double> time_span = chrono::duration_cast<chrono::duration<double>>(cur_t - start_t);
	if (time_span.count() >= time_limit) {
		cout << "*** time is up" << endl;
		return true;
	}
	return false;
}

bool isEstTooLong() // for simple instances
{
	if (local_s_d.global_record_point.estimation/cpu_cores <= timeFromStart()) {
		cout << "*** estimation / " << cpu_cores << " is less than elapsed time" << endl;
		return true;
	}
	return false;
}

point jumpPoint(point cur_point)
{
	local_s_d.local_record_point.estimation = 1e+308;
	//cout << "* forget global record" << endl;
	local_s_d.before_jump_point = cur_point;
	point jump_point = cur_point;
	cout << "local_s_d.jump_step " << local_s_d.jump_step << endl;
	if (cur_point.value.size() > 10) {
		unsigned changed_vals = 0;
		for (;;) {
			unsigned rand_ind = rand() % cur_point.value.size();
			if (jump_point.value[rand_ind]) {
				jump_point.value[rand_ind] = false;
				changed_vals++;
			}
			if (changed_vals == local_s_d.jump_step) {
				//local_s_d.jump_step += 10;
				break;
			}
		}
	}
	cout << "jump point weight " << jump_point.weight() << endl;
	local_s_d.printPoint(jump_point);
	
	return jump_point;
}

bool solveInstance()
{
	cout << "solve instance using record point" << endl;
	if (local_s_d.global_record_point.weight() > MAX_SOLVING_VARS) {
		cout << "*** stop, record point weight " << local_s_d.global_record_point.weight() << " > " << MAX_SOLVING_VARS << endl;
		return false;
	}

	string command_str = getScriptCommand(solver_script_name, local_s_d.global_record_point);

	cout << "command_str : " << command_str << endl;
	string out_str = getCmdOutput(command_str.c_str());
	cout << out_str << endl;
	cout << "solving done" << endl;

	return true;
}