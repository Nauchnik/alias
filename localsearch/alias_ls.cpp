/* Author: Oleg Zaikin, ISDCT SB RAS, Irkutsk */

#include <iostream>

#include "base_ls.h"

int main(int argc, char *argv[])
{
	if (argc < 5) {
		cerr << "Usage: -cnf cnf_name -estimation_script script_name -solver_script script_name -pcs pcs_name -solver solver_name -time time_limit" << endl;
		exit(-1);
	}

	base_local_search base_ls;

	/*
	string cnf_name = "";
	string solver_name = "";
	string estimation_script_name = "";
	string solver_script_name = "";
	double time_limit = 0;
	int cpu_cores = 0;
	chrono::high_resolution_clock::time_point start_t;

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

	cout << "*** final total time " << timeFromStart() << endl;*/
	
	return 0;
}