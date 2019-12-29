/* Author: Oleg Zaikin, ISDCT SB RAS, Irkutsk */

#ifndef base_ls_h
#define base_ls_h

#include <vector>
#include <fstream>
#include <string>
#include <thread>
#include <stdexcept>
#include <iostream>
#include <chrono>
#include <algorithm>
#include <iterator>
#include <set>
#include <cmath>
#include <random>
#include <unordered_map>
#include "point.h"

const double DEFAULT_TIME_LIMIT = 3600;
const unsigned MAX_SOLVING_VARS = 35;
const unsigned DEFAULT_JUMP_LIM = 3;
const unsigned MIN_VARS_JUMP = 100;
const double MAX_OBJ_FUNC_VALUE = 1e100;

#define SOLVE 1
#define ESTIMATE 2

using namespace std;

class base_local_search
{
public:
	base_local_search();

	unsigned opt_alg;

	int getCpuCores();
	string getCmdOutput(const char* cmd);
	double timeFromStart();
	bool isTimeExceeded();

	void parseParams(const int argc, char *argv[]);
	void init();
	void loadVars();
	void loadBackdoor();
	point pointFromUintVec(vector<unsigned> var_vec);
	vector<unsigned> uintVecFromPoint(point p);
	void coutUintVec(vector<unsigned> vec);
	vector<var> getAllCnfVars(const string filename);
	vector<var> readVarsFromPcs(string pcs_name);
	bool isEstTooLong();
	void writeToGraphFile(const string str);
	void setGraphFileName();
	bool solveInstance();
	void reportResult();
	void calculateEstimation(point &cur_point);
	bool isChecked(point p);
	void clearInterruptedChecked();
	string getScriptCommand(const int mode, const point cur_point);
	bool isKnownBackdoor();
	int getVarPos(const int val);
	void printGlobalRecordPoint();
	string printUintVector(vector<unsigned>);
	
	int total_func_calculations;
	int total_skipped_func_calculations;

protected:
	vector<var> vars;
	unordered_map<string, double> checked_points;
	unsigned skipped_points_count;
	unsigned interrupted_points_count;
	point global_record_point;
	point local_record_point;
	string cnf_name;
	string pcs_name;
	string graph_file_name;
	fstream graph_file;
	string alias_script_name;
	string solver_name;
	int cpu_cores;
	double cpu_lim;
	double wall_time_solving;
	int verbosity;
	point before_jump_point;
	bool is_random_search;
	unsigned vars_decr_times;
	chrono::high_resolution_clock::time_point start_t;
	bool is_solve;
	string result_output_name;
	string script_out_str;
	string backdoor_file_name;
	point known_backdoor;
	random_device rd;
	// iteretedHCVJ parameters
	unsigned jump_lim;
	unsigned jump_step;
	bool is_jump_mode;
	unsigned time_limit_per_task; // limit in seconds for ALIAS.py
};

inline int base_local_search::getVarPos(const int val)
{
	int pos = -1;
	for (int i = 0; i < vars.size(); i++)
		if (vars[i].value == val) {
			pos = i;
			break;
		}
	if (pos == -1) {
		cerr << "pos == -1" << endl;
		exit(-1);
	}
	return pos;
}

#endif


