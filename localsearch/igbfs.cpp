#include "igbfs.h"

#include <iostream>
#include <sstream>
#include <algorithm>
#include <iterator>

bool compareByCalculations(const var &a, const var &b)
{
	return a.calculations < b.calculations;
}

bool compareByRecords(const var &a, const var &b)
{
	return a.global_records < b.global_records;
}

bool compareByVarValue(const var &a, const var &b)
{
	return a.value < b.value;
}

void igbfs::backJump() {
	cout << "* backjumping" << endl;
	local_record_point.estimation = 1e+308;
	//cout << "* forget global record" << endl;
	jump_step = INITIAL_JUMP_STEP;
	vars_decr_times = 0;
	local_record_point = before_jump_point;
	cout << "back to the point with weight " << local_record_point.weight() << " : " << endl;
	cout << before_jump_point.getStr(vars);
	is_jump_mode = false; // jump only once
	cout << "* is_jump_mode " << is_jump_mode << endl;
}

point igbfs::permutateRecordPoint()
{
	cout << "* permutate record point" << endl;
	vector<var> global_record_vars_vec, extra_vars;
	for (unsigned i = 0; i < vars.size(); i++)
		if (global_record_point.value[i])
			global_record_vars_vec.push_back(vars[i]);
		else 
			extra_vars.push_back(vars[i]);
	// get additional vars with low calculations
	vector<var> add_calc_vars = extra_vars;
	sort(add_calc_vars.begin(), add_calc_vars.end(), compareByCalculations);
	cout << "add_calc_vars sorted by calculations : " << endl;
	for (auto x : add_calc_vars)
		cout << x.calculations << " ";
	cout << endl << "get first " << ADD_VARS_CALC << " of them" << endl;
	add_calc_vars.resize(ADD_VARS_CALC);
	// exclude best calc vars from extra vars
	vector<var> extra_vars_wout_add_calc;
	vector<var>::iterator it;
	for (auto x : extra_vars) {
		it = find(add_calc_vars.begin(), add_calc_vars.end(), x);
		if (it == add_calc_vars.end())
			extra_vars_wout_add_calc.push_back(x);
	}
	cout << "extra_vars_wout_add_calc size " << extra_vars_wout_add_calc.size() << endl;
	// get additional vars with high global records
	vector<var> add_records_vars = extra_vars_wout_add_calc;
	sort(add_records_vars.begin(), add_records_vars.end(), compareByRecords);
	reverse(begin(add_records_vars), end(add_records_vars));
	cout << "add_records_vars sorted (reverse) by records : " << endl;
	for (auto x : add_records_vars)
		cout << x.global_records << " ";
	cout << endl << "get first " << ADD_VARS_RECORDS << " of them" << endl;
	add_records_vars.resize(ADD_VARS_RECORDS);
	// get vars with low records to exclude
	/*sort(global_record_vars_vec.begin(), global_record_vars_vec.end(), compareByRecords);
	cout << "current global record sorted by records : " << endl;
	for (auto x : global_record_vars_vec)
		cout << x.global_records << " ";
	cout << endl << "remove first " << REM_VARS_RECORDS << " of them" << endl;
	reverse(begin(global_record_vars_vec), end(global_record_vars_vec));
	global_record_vars_vec.resize(global_record_vars_vec.size() - REM_VARS_RECORDS);
	sort(global_record_vars_vec.begin(), global_record_vars_vec.end(), compareByRecords);
	cout << "current global record after remove, sorted by records : " << endl;
	for (auto x : global_record_vars_vec)
		cout << x.global_records << " ";*/
	// add vars
	cout << endl << "add additional vars" << endl;
	for (auto x : add_calc_vars)
		global_record_vars_vec.push_back(x);
	for (auto x : add_records_vars)
		global_record_vars_vec.push_back(x);
	sort(global_record_vars_vec.begin(), global_record_vars_vec.end(), compareByVarValue);
	cout << "new point : " << endl;
	for (auto x : global_record_vars_vec)
		cout << x.value << " ";
	cout << endl;
	
	point mod_point;
	mod_point.value.resize(vars.size());
	for (unsigned i = 0; i < vars.size(); i++)
		mod_point.value[i] = false;
	for (unsigned i = 0; i < global_record_vars_vec.size(); i++)
		mod_point.value[global_record_vars_vec[i].value - 1] = true;
	cout << "mod_point : " << endl;
	for (auto x : mod_point.value)
		cout << x << " ";
	cout << endl;
	
	if (mod_point.weight() - ADD_VARS_CALC - ADD_VARS_RECORDS != global_record_point.weight()) {
		cerr << "mod_point weight " << mod_point.weight() << endl;
		cerr << " global_record_point weight " << global_record_point.weight() << endl;
		exit(-1);
	}
	
	while (isChecked(mod_point)) {
		mod_point = randPermutateRecordPoint(); // get random point until an unchecked one will be found
	}

	cout << "modified point with weight " << mod_point.weight() << " : " << endl;
	cout << mod_point.getStr(vars);
	return mod_point;
}

point igbfs::randPermutateRecordPoint()
{
	cout << "permutateRecordPoint()" << endl;
	point mod_point = global_record_point;
	unsigned changed_vals = 0;
	unsigned vars_to_change = mod_point.weight() / 3;
	cout << "vars_to_change " << vars_to_change << endl;
	for (unsigned i = 0; i < vars_to_change; i++) {
		unsigned rand_ind = rand() % mod_point.value.size();
		mod_point.value[i] = (mod_point.value[i] == true) ? false : true;
	}
	return mod_point;
}

void igbfs::iteratedGBFS()
{
	point start_point;
	start_point.value.resize(vars.size());
	for (auto x : start_point.value)
		x = true;
	cout << "start point : " << endl;
	cout << start_point.getStr(vars);

	unsigned jumps_count = 0;
	global_record_point = start_point;
	global_record_point.estimation = 1e+308;
	stringstream sstream;
	cpu_cores = getCpuCores();
	sstream << "vars est-1-core est-" << cpu_cores << "-cores elapsed";
	writeToGraphFile(sstream.str());
	sstream.str(""); sstream.clear();
	for (;;) {
		cout << endl << "*** GBFS iteration # " << jumps_count << endl;
		if (jumps_count > 0) {
			stringstream sstream;
			sstream << "--- GBFS iteration # " << jumps_count;
			writeToGraphFile(sstream.str());
			sstream.str(""); sstream.clear();
		}
		GBFS(start_point);
		jumps_count++;
		if (isTimeExceeded() || isEstTooLong() || (jumps_count > jump_lim)) {
			cout << "*** interrupt the search" << endl;
			break;
		}
		start_point = permutateRecordPoint();
	}
	sstream << "--- start solving, time " << timeFromStart();
	writeToGraphFile(sstream.str());
	sstream.str(""); sstream.clear();

	cout << "final point weight : " << global_record_point.weight() << endl;
	cout << "final runtime estimation on 1 CPU core : " << global_record_point.estimation << endl;
	cout << "final runtime estimation on " << cpu_cores << " CPU cores : " << global_record_point.estimation / cpu_cores << endl;
	cout << "final backdoor : " << endl;
	printGlobalRecordPoint();
	cout << "total local search time " << timeFromStart() << endl;

	cout << "skipped points : " << skipped_points_count << endl;
	cout << "checked points : " << checked_points.size() << endl;
	cout << "interrupted points : " << interrupted_points_count << endl;
}

void igbfs::GBFS(const point start_point)
{
	bool is_record_updated;
	vars_decr_times = 0;
	jump_step = INITIAL_JUMP_STEP;
	local_record_point = start_point;
	local_record_point.estimation = 1e+308;

	if (verbosity > 1)
		cout << "GBFS() start" << endl;
	
	for (;;) {
		is_record_updated = false;
		vector<unsigned> changing_vars;
		for (unsigned j = 0; j < local_record_point.value.size(); j++)
			changing_vars.push_back(j);
		random_shuffle(changing_vars.begin(), changing_vars.end());
		int changing_vars_count = changing_vars.size();
		//cout << "new start point " << endl;
		for (int i = -1; i < changing_vars_count; i++) { // i == -1 is required to check a point itself
			if (isTimeExceeded() || isEstTooLong())
				break; // if time is up, then stop GBFS
			point cur_point = local_record_point;
			if (i >= 0)
				cur_point.value[changing_vars[i]] = local_record_point.value[changing_vars[i]] ? false : true;
			if  (isChecked(cur_point)) {
				skipped_points_count++;
				continue;
			}
			calculateEstimation(cur_point);
			checked_points.push_back(cur_point);

			if (cur_point.estimation <= 0) {
				if (verbosity > 0)
					cout << "estimation for a point is worse than the current record, interrupt calculations" << endl;
				interrupted_points_count++;
				if (is_jump_mode) {
					backJump();
					is_record_updated = true;
					break;
				}
				else
					continue;
			}

			if (cur_point.estimation < local_record_point.estimation) {
				is_record_updated = true;
				updateLocalRecord(cur_point);
				break;
			}
			else if ( (is_jump_mode) && (i == -1) ) { // next point in jump mode is worse than previous
				cout << "backjumping cause of i == -1" << endl;
				backJump();
				is_record_updated = true;
				break;
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

void igbfs::updateLocalRecord(point cur_point)
{
	local_record_point = cur_point;
	
	if (local_record_point.estimation < global_record_point.estimation) {
		point prev_global_record_point = global_record_point;
		global_record_point = local_record_point;
		cout << "elapsed wall time : " << timeFromStart() << " sec | " << 
			    "record backdoor with " << global_record_point.weight() << " vars | " <<
			    " estimation " << global_record_point.estimation / cpu_cores << " sec on " << 
			    cpu_cores << " CPU cores" << endl;
		if (verbosity > 1)
			printGlobalRecordPoint();
		stringstream sstream;
		//"vars est-1-core est-" << cpu_cores << "-cores elapsed" << endl;
		sstream << global_record_point.weight() << " " << global_record_point.estimation << " " <<
			global_record_point.estimation / cpu_cores << " " << timeFromStart();
		writeToGraphFile(sstream.str());
		if (is_jump_mode) {
			if (global_record_point.weight() <= prev_global_record_point.weight())
				vars_decr_times++;
			else {
				vars_decr_times = 0;
				is_jump_mode = false;
				if (verbosity > 1)
					cout << "** is_jump_mode " << is_jump_mode << endl;
			}
		}
		if (global_record_point.weight() < MIN_VARS_JUMP_FROM)
			is_jump_mode = false;
		if (!is_jump_mode)
			for (unsigned j = 0; j < global_record_point.value.size(); j++)
				if (global_record_point.value[j])
					vars[j].global_records++;
		if (verbosity > 1) {
			cout << "* vars global records : ";
			for (auto x : vars)
				cout << x.global_records << " ";
			cout << endl;
			cout << "* vars calculations : ";
			for (auto x : vars)
				cout << x.calculations << " ";
			cout << endl;
		}
	}
	else {
		if (verbosity > 1)
			cout << "* new local_record_estimation " << local_record_point.estimation <<
				" with weight " << local_record_point.weight() << endl;
		stringstream sstream;
		//"vars est-1-core est-" << cpu_cores << "-cores elapsed" << endl;
		sstream << "\t" << local_record_point.weight() << " " << local_record_point.estimation << " " <<
			local_record_point.estimation / cpu_cores << " " << timeFromStart();
		writeToGraphFile(sstream.str());
	}
	
	if ( (is_jump_mode) && (cur_point.weight() > MIN_VARS_JUMP_FROM) && (vars_decr_times >= 4) )
		local_record_point = jumpPoint(cur_point);
	else
		local_record_point = cur_point;
}

point igbfs::jumpPoint(point cur_point)
{
	local_record_point.estimation = 1e+308;
	//cout << "* forget global record" << endl;
	before_jump_point = cur_point;
	point jump_point = cur_point;
	unsigned changed_vals = 0;
	for (;;) {
		unsigned rand_ind = rand() % cur_point.value.size();
		if (jump_point.value[rand_ind]) {
			jump_point.value[rand_ind] = false;
			changed_vals++;
		}
		if (changed_vals == jump_step) {
			//jump_step += 10;
			break;
		}
	}
	if (verbosity > 1) {
		cout << "jump_step " << jump_step << endl;
		cout << "jump point weight " << jump_point.weight() << endl;
		cout << jump_point.getStr(vars);
	}
	
	return jump_point;
}

void igbfs::processBackdoor()
{
	if (!isKnownBackdoor())
		iteratedGBFS();
	else if (!is_solve)
		estimateKnownBackdoor();
}