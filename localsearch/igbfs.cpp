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
	local_record_point.estimation = HUGE_VAL;
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
	vector<var> mod_vars, extra_vars;
	for (unsigned i = 0; i < vars.size(); i++)
		if (global_record_point.value[i])
			mod_vars.push_back(vars[i]);
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
		mod_vars.push_back(x);
	for (auto x : add_records_vars)
		mod_vars.push_back(x);
	sort(mod_vars.begin(), mod_vars.end(), compareByVarValue);
	cout << "vars of a new point : " << endl;
	for (auto x : mod_vars)
		cout << x.value << " ";
	cout << endl;
	
	point mod_point;
	mod_point.value.resize(vars.size());
	for (unsigned i = 0; i < vars.size(); i++)
		mod_point.value[i] = false;
	for (unsigned i = 0; i < mod_vars.size(); i++) {
		int pos = getVarPos(mod_vars[i].value);
		mod_point.value[pos] = true;
	}
	cout << "mod_point : " << endl;
	for (auto x : mod_point.value)
		cout << x << " ";
	cout << endl;
	
	if (mod_point.weight() - ADD_VARS_CALC - ADD_VARS_RECORDS != global_record_point.weight()) {
		cerr << "mod_point weight " << mod_point.weight() << endl;
		cerr << "global_record_point weight " << global_record_point.weight() << endl;
		exit(-1);
	}
	
	while (isChecked(mod_point)) {
		mod_point = generateRandPoint(global_record_point.weight() + ADD_VARS_CALC + ADD_VARS_RECORDS); // get random point until an unchecked one will be found
	}

	cout << "modified point with weight " << mod_point.weight() << " : " << endl;
	cout << mod_point.getStr(vars);
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
	global_record_point.estimation = HUGE_VAL;
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
			writeToGraphFile("--- interrupt\n");
			break;
		}
		start_point = permutateRecordPoint();
	}

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
	local_record_point.estimation = HUGE_VAL;

	if (verbosity > 1)
		cout << "GBFS() start" << endl;
	
	bool is_break = false;
	for (;;) {
		is_record_updated = false;
		vector<unsigned> changing_vars;
		for (unsigned i = 0; i < local_record_point.value.size(); i++)
			changing_vars.push_back(i);
		random_shuffle(changing_vars.begin(), changing_vars.end());
		int changing_vars_count = changing_vars.size();
		//cout << "new start point " << endl;
		for (int i = -1; i < changing_vars_count; i++) { // i == -1 is required to check a point itself
			if (isTimeExceeded() || isEstTooLong()) {
				is_break = true;
				break; // if time is up, then stop GBFS
			}
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
		
		if (is_break)
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
		if (verbosity > 0)
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
	local_record_point.estimation = HUGE_VAL;
	//cout << "* forget global record" << endl;
	before_jump_point = cur_point;
	point jump_point = cur_point;
	unsigned changed_vals = 0;
	mt19937 mt(rd());
	unsigned cur_point_var_count = cur_point.value.size();
	uniform_int_distribution<unsigned> dist(0, cur_point_var_count - 1);
	for (;;) {
		unsigned rand_ind = dist(mt);
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

void igbfs::findBackdoor()
{
	if ((!isKnownBackdoor()) && (rand_from > 0)) {
		randSearch();
	}
	else if (!isKnownBackdoor())
		iteratedGBFS();
	else if (isKnownBackdoor()) {
		calculateEstimation(known_backdoor);
		global_record_point = known_backdoor;
		printGlobalRecordPoint();
	}
}

void igbfs::randSearch()
{
	cout << "randSearch()\n";
	is_jump_mode = false;
	unsigned total_points_count = rand_points * (rand_to - rand_from + 1);
	cout << "total_points_count " << total_points_count << endl;

	bool is_break = false; 
	for (unsigned i=rand_to; i >= rand_from; i--) {
		cout << "generate " << rand_points << " points of " << i << " vars\n";
		for (unsigned j=0; j< rand_points; j++) {
			if (isTimeExceeded() || isEstTooLong()) {
				cout << "*** interrupt the search" << endl;
				writeToGraphFile("--- interrupt\n");
				is_break = true;
				break;
			}
			point p; 
			do {
				p = generateRandPoint(i);
			} while (isChecked(p));
			calculateEstimation(p);
			checked_points.push_back(p);
			cout << "checked " << checked_points.size() << " points out of " << total_points_count << endl;
			if (p.estimation <= 0)
				continue;
			if (p.estimation < global_record_point.estimation)
				updateLocalRecord(p);
		}
		if (is_break)
			break;
	}
}

point igbfs::generateRandPoint(const unsigned var_count)
{
	point p;
	unsigned total_var_count = vars.size();
	p.value.resize(total_var_count);
	for (auto x : p.value)
		x = false;

	mt19937 mt(rd());
	uniform_int_distribution<unsigned> dist(0, total_var_count-1);
	
	vector<unsigned> rand_vec;
	for (;;) {
		unsigned val = dist(mt);
		if (find(rand_vec.begin(), rand_vec.end(), val) == rand_vec.end())
			rand_vec.push_back(val);
		else
			continue;
		if (rand_vec.size() == var_count)
			break;
	}
	
	if (verbosity > 1) {
		cout << "rand_vec.size() " << rand_vec.size() << endl;
		for (auto x : rand_vec)
			cout << x << " ";
		cout << endl;
	}

	for (auto x : rand_vec)
		p.value[x] = true;

	return p;
}