#include "igbfs.h"

#include <iostream>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <set>

bool compareByVarCalculations(const var &a, const var &b)
{
	return a.calculations < b.calculations;
}

bool compareByVarRecords(const var &a, const var &b)
{
	return a.global_records < b.global_records;
}

bool compareByVarValue(const var &a, const var &b)
{
	return a.value < b.value;
}

bool compareByVarRemObjVal(const var &a, const var &b)
{
	return a.obj_val_remove < b.obj_val_remove;
}

bool compareByVarAddObjVal(const var &a, const var &b)
{
	return a.obj_val_add < b.obj_val_add;
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
	cout << "point before mod (cur global min) : ";
	cout << global_record_point.getStr(vars);
	// get additional vars with low calculations
	vector<var> add_calc_vars = extra_vars;
	sort(add_calc_vars.begin(), add_calc_vars.end(), compareByVarCalculations);
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
	sort(add_records_vars.begin(), add_records_vars.end(), compareByVarRecords);
	reverse(begin(add_records_vars), end(add_records_vars));
	cout << "add_records_vars sorted (reverse) by records : " << endl;
	for (auto x : add_records_vars)
		cout << x.global_records << " ";
	cout << endl << "get first " << ADD_VARS_RECORDS << " of them" << endl;
	add_records_vars.resize(ADD_VARS_RECORDS);

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
	cout << mod_point.getStr(vars);
	
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

void igbfs::iteratedHCVJ()
{
	cout << "iterated hill climbing with variables-based jump \n";
	point start_point;
	start_point.value.resize(vars.size());
	for (auto x : start_point.value)
		x = true;
	cout << "start point : " << endl;
	cout << start_point.getStr(vars);

	global_record_point = start_point;
	global_record_point.estimation = HUGE_VAL;
	unsigned jumps_count = 0;
	for (;;) {
		cout << endl << "*** HCVJ iteration # " << jumps_count << endl;
		if (jumps_count > 0) {
			stringstream sstream;
			sstream << "--- HCVJ iteration # " << jumps_count;
			writeToGraphFile(sstream.str());
			sstream.str(""); sstream.clear();
		}
		HCVJ(start_point);
		jumps_count++;
		if (isTimeExceeded() || isEstTooLong() || (jumps_count > jump_lim)) {
			cout << "*** interrupt the search" << endl;
			writeToGraphFile("--- interrupt");
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

void igbfs::updateLocalRecord(point cur_point, int neighbor_index, int neighbor_size)
{
	local_record_point = cur_point;
	
	stringstream sstream;
	if (local_record_point.estimation < global_record_point.estimation) {
		point prev_global_record_point = global_record_point;
		global_record_point = local_record_point;
		cout << "elapsed wall time : " << timeFromStart() << " sec | " << 
			    "record backdoor with " << global_record_point.weight() << " vars | " <<
			    " estimation " << global_record_point.estimation / cpu_cores << " sec on " << 
			    cpu_cores << " CPU cores" << endl << flush;
		if (verbosity > 0)
			printGlobalRecordPoint();
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
		if ( (!is_jump_mode) && (!is_random_search) )
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
		sstream << global_record_point.weight() << " " << global_record_point.estimation << " " <<
			global_record_point.estimation / cpu_cores << " " << (unsigned)timeFromStart() << " " << 
			total_func_calculations << " " << total_skipped_func_calculations;
	}
	else {
		if (verbosity > 1)
			cout << "* new local_record_estimation " << local_record_point.estimation <<
				" with weight " << local_record_point.weight() << endl;
		// update of local minimum after, e.g. HCVJ() jump
		sstream << "\t" << local_record_point.weight() << " " << local_record_point.estimation << " " <<
			local_record_point.estimation / cpu_cores << " " << (unsigned)timeFromStart() << " " << 
			total_func_calculations << " " << total_skipped_func_calculations;
	}

	if ((neighbor_index >= 0) && (neighbor_size > 0))
	    sstream << " " << neighbor_index << "/" << neighbor_size;
	writeToGraphFile(sstream.str());
	
	if ( (is_jump_mode) && (cur_point.weight() > MIN_VARS_JUMP_FROM) && (vars_decr_times >= 4) )
		local_record_point = jumpPoint(cur_point);
}

point igbfs::jumpPoint(point cur_point)
{
	local_record_point.estimation = HUGE_VAL;
	//cout << "* forget global record" << endl;
	before_jump_point = cur_point;
	point jump_point = cur_point;
	unsigned changed_vals = 0;
	mt19937 mt(time(0));
	size_t cur_point_var_count = cur_point.value.size();
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

int igbfs::findBackdoor()
{
	if (isKnownBackdoor())
		return 1;

	// if the decomposition set is unknown and it is required to find it
	stringstream sstream;
	cpu_cores = getCpuCores();
	sstream << "vars est-1-core est-" << cpu_cores << "-cores elapsed-sec elapsed-fcalc skipped_fcalc neigh_index/neigh_size";
	writeToGraphFile(sstream.str());
	sstream.str(""); sstream.clear();
	switch (opt_alg) {
	case 0:
		randSearchWholeSpace();
		break;
	case 1:
		randSearchReduceOneVar();
		break;
	case 2:
		simpleHillClimbing(); // neighborhood = add/remove
		break;
	case 3:
		steepestAscentHillClimbing();
		break;
	case 4:
		tabuSearch();
		break;
	case 5:
		onePlusOne();
		break;
	case 6:
		iteratedHCVJ();
		break;
	case 7:
		simpleHillClimbingAddRemovePartialRaplace();
		break;
	case 8:
		onePlusOneSimpleHillClimbing();
		break;
	case 9:
		simpleHillClimbing(1); // neighborhood = add/remove/replace
		break;
	default:
		cout << "Unknown opt_alg, 1+1 was chosen\n";
		onePlusOne();
		break;
	}

	return 1;
}

point igbfs::generateRandPoint(const unsigned point_var_count)
{
	point p;
	size_t total_var_count = vars.size();
	p.value.resize(total_var_count);
	for (auto x : p.value)
		x = false;

	mt19937 mt(time(0));
	uniform_int_distribution<unsigned> dist(0, total_var_count - 1);

	set<unsigned> rand_set;
	for (;;) {
		unsigned val = dist(mt);
		rand_set.insert(val);
		if (rand_set.size() == point_var_count)
			break;
	}

	for (auto x : rand_set)
		p.value[x] = true;

	return p;
}

void igbfs::randSearchWholeSpace()
{
	cout << "randSearchWholeSpace()\n";
	is_jump_mode = false;
	is_random_search = true;
	
	mt19937 mt(time(0));
	size_t total_var_count = vars.size();
	uniform_int_distribution<unsigned> dist(1, total_var_count - 1);

	for (;;) {
		unsigned random_size;
		do {
			random_size = dist(mt);
		} while (random_size < 10); // TODO - remove after fixes in ALIAS.py
		point p = generateRandPoint(random_size);
		
		calculateEstimation(p);
		if (p.estimation <= 0)
			continue;
		if (p.estimation < global_record_point.estimation)
			updateLocalRecord(p);

		if (isTimeExceeded() || isEstTooLong()) {
			cout << "*** interrupt the search" << endl;
			writeToGraphFile("--- interrupt");
			break;
		}
	}
}

void igbfs::randSearchReduceOneVar()
{
	cout << "randSearchReduceOneVar()\n";
	is_jump_mode = false;
	is_random_search = true;
	
	// first, calculate on a start point - all variables
	point p;
	size_t total_var_count = vars.size();
	p.value.resize(total_var_count);
	for (auto x : p.value)
		x = true;
	calculateEstimation(p);
	updateLocalRecord(p);

	unsigned cur_set_size = total_var_count-1;
	for (;;) {
		point p = generateRandPoint(cur_set_size);

		calculateEstimation(p);
		if (p.estimation <= 0)
			continue;
		if (p.estimation < global_record_point.estimation) {
			updateLocalRecord(p);
			cur_set_size--;
			if (!cur_set_size) {
				cout << "exit: cur_set_size " << cur_set_size << endl;
				break;
			}
		}
		
		if (isTimeExceeded() || isEstTooLong()) {
			cout << "*** interrupt the search" << endl;
			writeToGraphFile("--- interrupt");
			break;
		}
	}
}

void igbfs::HCVJ(point start_point)
{
    bool is_record_updated;
    vars_decr_times = 0;
    jump_step = INITIAL_JUMP_STEP;
    local_record_point = start_point;
    local_record_point.estimation = HUGE_VAL;

	cout << "HCVJ() start from point : " << endl;
	cout << start_point.getStr(vars);
    
    bool is_break = false;
    for (;;) {
		is_record_updated = false;
		vector<unsigned> changing_vars;
		for (unsigned i = 0; i < local_record_point.value.size(); i++)
			changing_vars.push_back(i);
		random_shuffle(changing_vars.begin(), changing_vars.end());
		int changing_vars_count = changing_vars.size();
		for (int i = -1; i < changing_vars_count; i++) { // i == -1 is required to check a point itself
			cout << i << endl;
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
			//cout << "point : " << endl;
			//cout << cur_point.getStr(vars);
			calculateEstimation(cur_point);

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

		if (!is_record_updated) { // if a local minimum has been found, then stop HCVJ
			cout << "a local minimum has been found, stop HCVJ" << endl;
			break;
		}
    }

    cout << "HCVJ() end" << endl << endl;
}

vector<point> igbfs::neighbors(point neigh_center, vector<bool> &is_add_vars, int neigh_type)
{
	if (verbosity > 0) {
		cout << "start of neighbors()" << endl;
		cout << "neigh_center size : " << neigh_center.value.size() << endl;
	}

	vector<point> neighbors_points;
	// determine which variables can be added/removed from the given point
	is_add_vars.resize(vars.size());
	for (unsigned i = 0; i < neigh_center.value.size(); i++)
		is_add_vars[i] = !(neigh_center.value[i]);

	if (verbosity > 0) {
		cout << "is_add_vars size : " << is_add_vars.size() << endl;
		coutBoolVec(is_add_vars);
	}
	
	if (neigh_type <= 1) { // add add/remove points
		vector<point> remove_neighbors, add_neighbors;
		// add 'add/remove' points and shuffle them
		for (unsigned i = 0; i < neigh_center.value.size(); i++) {
			point new_p = neigh_center;
			if (neigh_center.value[i] == true) {
				new_p.value[i] = false;
				remove_neighbors.push_back(new_p);
			}
			else if (neigh_center.value[i] == false) {
				new_p.value[i] = true;
				add_neighbors.push_back(new_p);
			}
		}
		random_shuffle(remove_neighbors.begin(), remove_neighbors.end());
		random_shuffle(add_neighbors.begin(), add_neighbors.end());
		neighbors_points = remove_neighbors;
		for (auto x : add_neighbors)
			neighbors_points.push_back(x);
	}
	else if (neigh_type == 2) { // add/remove vars sorted by obj function's values
		vector<var> remove_vars_w_obj, remove_vars_no_obj, add_vars_w_obj, add_vars_no_obj;
		for (unsigned i = 0; i < is_add_vars.size(); i++) {
			var v = vars[i];
			if (is_add_vars[i]) {
				if (v.obj_val_add == MAX_OBJ_FUNC_VALUE) {
					v.obj_val_add = -1;
					add_vars_no_obj.push_back(v);
				}
				else
					add_vars_w_obj.push_back(v);
			}
			else {
				if (v.obj_val_remove == MAX_OBJ_FUNC_VALUE) {
					v.obj_val_remove = -1;
					remove_vars_no_obj.push_back(v);
				}
				else
					remove_vars_w_obj.push_back(v);
			}
		}
		vector<var> remove_vars, add_vars;
		random_shuffle(remove_vars_no_obj.begin(), remove_vars_no_obj.end());
		for (auto x : remove_vars_no_obj)
			remove_vars.push_back(x);
		sort(remove_vars_w_obj.begin(), remove_vars_w_obj.end(), compareByVarRemObjVal);
		for (auto x : remove_vars_w_obj)
			remove_vars.push_back(x);
		random_shuffle(add_vars_no_obj.begin(), add_vars_no_obj.end());
		for (auto x : add_vars_no_obj)
			add_vars.push_back(x);
		sort(add_vars_w_obj.begin(), add_vars_w_obj.end(), compareByVarAddObjVal);
		for (auto x : add_vars_w_obj)
			add_vars.push_back(x);

		if (verbosity > 0) {
			cout << "remove_vars size " << remove_vars.size() << endl;
			cout << "remove_vars : " << endl;
			for (auto v : remove_vars)
				cout << v.value << " : " << v.obj_val_remove << endl;
			cout << "add_vars size " << add_vars.size() << endl;
			cout << "add_vars : " << endl;
			for (auto v : add_vars)
				cout << v.value << " : " << v.obj_val_add << endl;
		}
		for (auto x : remove_vars) {
			int pos = getVarPos(x.value);
			point p = neigh_center;
			p.value[pos] = false;
			neighbors_points.push_back(p);
			/*if (verbosity > 0) {
				cout << "x.value : " << x.value << endl;
				cout << "pos : " << pos << endl;
				cout << "neigh_center[pos] : " << neigh_center.value[pos] << endl;
				cout << "p.value[pos] : " << p.value[pos] << endl;
				cout << "p.value  size " << p.value.size() << endl;
				cout << "p.value weight " << p.weight() << endl;
				cout << "neigh_center weight " << neigh_center.weight() << endl;
			}*/
		}
		if (add_vars.size() > 0) {
			for (auto y : add_vars) {
				int pos = getVarPos(y.value);
				point p = neigh_center;
				p.value[pos] = true;
				neighbors_points.push_back(p);
			}
		}
	}

	if (neigh_type == 1) { // add 'replace' points
		vector<point> replace_neighbors;
		if (verbosity > 0) {
			cout << "cur center point : " << endl;
			for (unsigned j = 0; j < neigh_center.value.size(); j++)
				cout << (int)neigh_center.value[j];
			cout << endl;
		}
		vector<int> p_var_indecies, notp_var_indecies;
		for (unsigned i = 0; i < neigh_center.value.size(); i++)
			if (neigh_center.value[i])
				p_var_indecies.push_back(i);
			else
				notp_var_indecies.push_back(i);
		if (verbosity > 0) {
			cout << "p_var_indecies size " << p_var_indecies.size() << endl;
			cout << "notp_var_indecies size " << notp_var_indecies.size() << endl;
			cout << "p_var_indecies : " << endl;
			for (auto x : p_var_indecies)
				cout << int(x) << " ";
			cout << endl;
			cout << "notp_var_indecies : " << endl;
			for (auto x : notp_var_indecies)
				cout << int(x) << " ";
			cout << endl;
		}
		int k = 0;
		if (verbosity > 0)
			cout << "first 10 replace points : " << endl;
		for (auto x : p_var_indecies) {
			for (auto y : notp_var_indecies) {
				point new_p = neigh_center;
				new_p.value[x] = false;
				new_p.value[y] = true;
				replace_neighbors.push_back(new_p);
				if (verbosity > 0) {
					if (k < 10) {
						for (unsigned j = 0; j < new_p.value.size(); j++)
							cout << (int)new_p.value[j];
						cout << endl;
						k++;
					}
				}
			}
		}
		cout << "neighbors size " << neighbors_points.size() << endl;
		random_shuffle(replace_neighbors.begin(), replace_neighbors.end());
		for (auto x : replace_neighbors)
			neighbors_points.push_back(x);
	}
	
	return neighbors_points;
}

unsigned igbfs::get_diff_var(point p1, point p2)
{
	vector<unsigned> vec1 = uintVecFromPoint(p1);
	vector<unsigned> vec2 = uintVecFromPoint(p2);
	vector<unsigned> diff_vec;
	if (vec1.size() > vec2.size()) {
		set_difference(vec1.begin(), vec1.end(),
			vec2.begin(), vec2.end(),
			inserter(diff_vec, diff_vec.begin()));
	}
	else {
		set_difference(vec2.begin(), vec2.end(),
			vec1.begin(), vec1.end(),
			inserter(diff_vec, diff_vec.begin()));
	}
	if (diff_vec.size() != 1) {
		cerr << "diff_vec size : " << diff_vec.size() << endl;
		exit(-1);
	}
	return diff_vec[0];
}

bool igbfs::processNeighborhood(vector<point> neighbors_points, point &neigh_center, 
								vector<bool> is_add_vars, bool &is_break, bool is_add_remove_vars_req)
{
	if (verbosity > 0)
		cout << "Start of processNeighborhood()" << endl;
	bool is_local_record_updated = false;
	is_break = false;
	vector<unsigned> center_uint_vec = uintVecFromPoint(neigh_center);
	int neighbor_index = -1;
	for (auto neighbor : neighbors_points) {
		neighbor_index++;
		calculateEstimation(neighbor);
		if (neighbor.estimation <= 0)
			continue;
		if (is_add_remove_vars_req) {
			unsigned var_val = get_diff_var(neighbor, neigh_center);
			int pos = getVarPos(var_val);
			double dval = -1;
			if (neighbor.estimation < MAX_OBJ_FUNC_VALUE)
				dval = neighbor.estimation / global_record_point.estimation;
			else
				dval = MAX_OBJ_FUNC_VALUE;
			if (verbosity > 0) {
				cout << "neighbor.weight : " << neighbor.weight() << endl;
				cout << "neighbor.estimation : " << neighbor.estimation << endl;
				cout << "global_record_point.estimation : " << global_record_point.estimation << endl;
				cout << "dval : " << dval << endl;
				cout << "var_val : " << var_val << endl;
				cout << "pos : " << pos << endl;
				cout << "is_add_vars[pos] : " << is_add_vars[pos] << endl;
			}
			if (is_add_vars[pos])
				vars[pos].obj_val_add = dval;
			else
				vars[pos].obj_val_remove = dval;
		}
		if (neighbor.estimation < global_record_point.estimation) {
			updateLocalRecord(neighbor, neighbor_index, neighbors_points.size());
			neigh_center = neighbor;
			is_local_record_updated = true;
			break;
		}
		if (isTimeExceeded() || isEstTooLong()) {
			cout << "*** interrupt the search" << endl;
			writeToGraphFile("--- interrupt");
			is_break = true;
			break;
		}
	}
	return is_local_record_updated;
}

void igbfs::simpleHillClimbing(int neigh_type, point p)
{
	cout << "simpleHillClimbing()\n";
	is_jump_mode = false;
	is_random_search = false;
	vector<var> add_remove_vars;
	
	point neigh_center;
	if (p.value.size() > 0) { // if a start point is given, use it
		neigh_center = p;
		cout << "start point is given : \n";
		vector<unsigned> vec = uintVecFromPoint(neigh_center);
		coutUintVec(vec);
	}
	else {
		// else use as the start point the set of all variables
		neigh_center.value.resize(vars.size());
		for (auto x : neigh_center.value)
			x = true;
	}
	if (!isChecked(neigh_center)) {
		calculateEstimation(neigh_center);
		updateLocalRecord(neigh_center);
	}
	
	bool is_break = false;
	for(;;) {
		bool is_local_record_updated = false;
		vector<bool> is_add_vars;
		vector<point> neighbors_points = neighbors(neigh_center, is_add_vars, neigh_type);
		int neighbor_index = -1;
		for (auto neighbor : neighbors_points) {
			neighbor_index++;
			calculateEstimation(neighbor);
			if (neighbor.estimation <= 0)
				continue;
			if (neighbor.estimation < global_record_point.estimation) {
				updateLocalRecord(neighbor, neighbor_index, neighbors_points.size());
				neigh_center = neighbor;
				is_local_record_updated = true;
				break;
			}
			if (isTimeExceeded() || isEstTooLong()) {
				cout << "*** interrupt the search" << endl;
				writeToGraphFile("--- interrupt");
				is_break = true;
				break;
			}
		}
		if (is_break)
			break;
		if (!is_local_record_updated) {
			cout << "*** interrupt the search: local minimum" << endl;
			writeToGraphFile("--- interrupt: local minimum");
			break; // local minimum
		}
	}
}

void igbfs::steepestAscentHillClimbing()
{
	cout << "steepestAscentHillClimbing()\n";
	is_jump_mode = false;
	is_random_search = false;

	// first, calculate on a start point - all variables
	point neigh_center;
	size_t total_var_count = vars.size();
	neigh_center.value.resize(total_var_count);
	for (auto x : neigh_center.value)
		x = true;
	calculateEstimation(neigh_center);
	updateLocalRecord(neigh_center);

	bool is_break = false;
	for (;;) {
		bool is_local_record_updated = false;
		vector<bool> is_add_vars;
		vector<point> neighbors_points = neighbors(neigh_center, is_add_vars);
		for (auto neighbor : neighbors_points) {
			calculateEstimation(neighbor);
			if (neighbor.estimation <= 0)
				continue;
			if (neighbor.estimation < global_record_point.estimation) {
				updateLocalRecord(neighbor);
				neigh_center = neighbor;
				is_local_record_updated = true;
			}

			if (isTimeExceeded() || isEstTooLong()) {
				cout << "*** interrupt the search" << endl;
				writeToGraphFile("--- interrupt");
				is_break = true;
				break;
			}
		}
		if (is_break)
			break;
		if (!is_local_record_updated) {
			cout << "*** interrupt the search: local minimum" << endl;
			writeToGraphFile("--- interrupt: local minimum");
			break; // local minimum
		}
	}
}

void igbfs::tabuSearch()
{
	cout << "tabuSearch()\n";
	is_jump_mode = false;
	is_random_search = false;

	// first, calculate on a start point - all variables
	point neigh_center;
	size_t total_var_count = vars.size();
	neigh_center.value.resize(total_var_count);
	for (auto x : neigh_center.value)
		x = true;
	calculateEstimation(neigh_center);
	updateLocalRecord(neigh_center);
	
	int max_tabu_list_size = 1000;
	vector<point> tabu_list;
	point best_point_in_neigh;
	bool is_break = false;
	vector<bool> is_add_vars;
	for (;;) {
		vector<point> neighbors_points = neighbors(neigh_center, is_add_vars);
		best_point_in_neigh.estimation = HUGE_VALF;
		for (auto neighbor : neighbors_points) {
			if (find(tabu_list.begin(), tabu_list.end(), neighbor) != tabu_list.end())
				continue;
			calculateEstimation(neighbor);
			if (neighbor.estimation <= 0)
				continue;
			if (neighbor.estimation < best_point_in_neigh.estimation)
				best_point_in_neigh = neighbor;

			if (isTimeExceeded() || isEstTooLong()) {
				cout << "*** interrupt the search" << endl;
				writeToGraphFile("--- interrupt");
				is_break = true;
				break;
			}
		}
		if (best_point_in_neigh.estimation == HUGE_VALF) {
			cout << "exit: local_min_point estimation " << best_point_in_neigh.estimation << endl;
			break;
		}
		if (is_break)
			break;
		tabu_list.push_back(best_point_in_neigh);
		if (tabu_list.size() > max_tabu_list_size)
			tabu_list.erase(tabu_list.begin());
		if (best_point_in_neigh.estimation < global_record_point.estimation)
			updateLocalRecord(best_point_in_neigh);
		neigh_center = best_point_in_neigh;
	}
}

void igbfs::onePlusOne(int fcalc_lim, double time_from_last_update, double time_lim)
{
	cout << "one_plus_one()\n";
	is_jump_mode = false;
	is_random_search = true;

	// first, calculate on a start point - all variables
	point neigh_center;
	size_t total_var_count = vars.size();
	neigh_center.value.resize(total_var_count);
	for (auto x : neigh_center.value)
		x = true;
	calculateEstimation(neigh_center);
	updateLocalRecord(neigh_center);

	mt19937 mt(time(0));
	uniform_real_distribution<double> dist(0, 1);
	double prob = 1 / (double)total_var_count;
	chrono::high_resolution_clock::time_point update_start_t = chrono::high_resolution_clock::now();
	chrono::high_resolution_clock::time_point start_t = chrono::high_resolution_clock::now();
	int update_fcalc = -1;

	for (;;) {
		point candidate_point = neigh_center;
		for (unsigned i=0; i<total_var_count; i++) {
			double val = dist(mt);
			if (val < prob)
				candidate_point.value[i] = (candidate_point.value[i] == true) ? false : true;
		}
		if (candidate_point.value == neigh_center.value)
			continue;
		calculateEstimation(candidate_point);
		if (candidate_point.estimation <= 0)
			continue;
		if (candidate_point.estimation < global_record_point.estimation) {
			updateLocalRecord(candidate_point);
			neigh_center = candidate_point;
			update_start_t = chrono::high_resolution_clock::now();
			update_fcalc = total_func_calculations;
		}

		if (isTimeExceeded() || isEstTooLong()) {
			cout << "*** interrupt the search" << endl;
			writeToGraphFile("--- interrupt");
			break;
		}
		chrono::high_resolution_clock::time_point cur_t = chrono::high_resolution_clock::now();
		chrono::duration<double> elapsed_from_last_update = chrono::duration_cast<chrono::duration<double>>(cur_t - update_start_t);
		chrono::duration<double> elapsed_from_start = chrono::duration_cast<chrono::duration<double>>(cur_t - start_t);
		if ( (update_fcalc > 0) && 
		     ( 
			 ((fcalc_lim > 0) && (total_func_calculations - update_fcalc >= fcalc_lim)) ||
			 ((time_from_last_update > 0) && (elapsed_from_last_update.count() >= time_from_last_update)) ||
			 ((time_lim > 0) && (elapsed_from_start.count() >= time_lim)) 
		     ) 
		   )
		{
			writeToGraphFile("--- interrupt (1+1)-EA due to limit");
			stringstream sstream;
			sstream << "\t" << global_record_point.weight() << " " << global_record_point.estimation << " " <<
				global_record_point.estimation / cpu_cores << " " << (unsigned)timeFromStart() << " " <<
				total_func_calculations << " " << total_skipped_func_calculations;
			writeToGraphFile(sstream.str());
			break;
		}
	}
}

void igbfs::simpleHillClimbingAddRemovePartialRaplace(point p)
{
	cout << "simpleHillClimbingAddRemovePartialRaplace()\n";
	is_jump_mode = false;
	is_random_search = false;
	vector<var> add_remove_vars;

	point neigh_center;
	if (p.value.size() > 0) { // if a start point is given, use it
		neigh_center = p;
		cout << "start point is given : \n";
		vector<unsigned> vec = uintVecFromPoint(neigh_center);
		coutUintVec(vec);
	}
	else {
		// else use as the start point the set of all variables
		neigh_center.value.resize(vars.size());
		for (auto x : neigh_center.value)
			x = true;
	}
	if (!isChecked(neigh_center)) {
		calculateEstimation(neigh_center);
		updateLocalRecord(neigh_center);
	}
	
	bool is_break = false;
	for (;;) {
		vector<bool> is_add_vars;
		vector<point> neighbors_points = neighbors(neigh_center, is_add_vars, 2); // add/remove first
		if (is_add_vars.size() != vars.size()) {
			cerr << "is_add_vars.size() != vars.size()" << endl;
			exit(-1);
		}
		if (verbosity > 0) {
			cout << "neighbors_points size : " << neighbors_points.size() << endl;
			cout << "is_add_vars : " << endl;
			coutBoolVec(is_add_vars);
		}
		
		bool is_local_record_updated = false;
		is_local_record_updated = processNeighborhood(neighbors_points, neigh_center, 
													  is_add_vars, is_break, true);
		if (is_break)
			break;
		if (is_local_record_updated)
			continue;
		
		cout << endl << "cur bkv : " << global_record_point.estimation << endl;
		vector<var> add_vars, remove_vars;
		bool is_add_var_inter = false;
		int inter_remove_var = 0;
		for (unsigned i=0; i<is_add_vars.size(); i++)
			if (is_add_vars[i]) {
				add_vars.push_back(vars[i]);
				if (vars[i].obj_val_add >= MAX_OBJ_FUNC_VALUE)
					is_add_var_inter = true;
			}
			else {
				remove_vars.push_back(vars[i]);
				if (vars[i].obj_val_remove >= MAX_OBJ_FUNC_VALUE)
					inter_remove_var++;
			}
		
		/*if ((is_add_var_inter) || (inter_remove_var == remove_vars.size())) {
			cout << "*** for all remove vars (or for at least one add var) the obj func was interrupted" << endl;
			cout << "is_add_var_inter : " << is_add_var_inter << endl;
			cout << "inter_remove_var : " << inter_remove_var << endl;
			cout << "remove_vars.size() : " << remove_vars.size() << endl;
			cout << "*** clear interrupted checked points" << endl;
			cout << "*** increase time_limit_per_task from " << time_limit_per_task << " to " << time_limit_per_task * 2 << endl;
			time_limit_per_task *= 2;
			stringstream sstream;
			sstream << time_limit_per_task;
			writeToGraphFile("--- time_limit_per_task : " + sstream.str());
			clearInterruptedChecked();
		}*/
		
		// remove vars with interrupted objective function first
		vector<var> remove_vars_w_obj_f_val, remove_vars_no_obj_f_val;
		for (auto &var : remove_vars) {
			if (var.obj_val_remove >= MAX_OBJ_FUNC_VALUE) {
				var.obj_val_remove = -1;
				remove_vars_no_obj_f_val.push_back(var);
			}
			else
				remove_vars_w_obj_f_val.push_back(var);
		}
		// sort remove vars by estimation
		if (remove_vars_w_obj_f_val.size() > 0)
			sort(remove_vars_w_obj_f_val.begin(), remove_vars_w_obj_f_val.end(), compareByVarRemObjVal);
		if (remove_vars_no_obj_f_val.size() > 0)
			random_shuffle(remove_vars_no_obj_f_val.begin(), remove_vars_no_obj_f_val.end());
		remove_vars.clear();
		for (auto v : remove_vars_no_obj_f_val)
			remove_vars.push_back(v);
		for (auto v : remove_vars_w_obj_f_val)
			remove_vars.push_back(v);
		vector<var> sorted_remove_vars_before_erasing = remove_vars;
		cout << endl << "sorted remove vars (interrupted first) : " << endl;
		for (auto v : remove_vars)
			cout << v.value << " " << " " << v.obj_val_remove << endl;
		cout << endl << "get first " << REPLACE_VARS << " of them" << endl;
		remove_vars.resize(REPLACE_VARS);
		// sort add vars by estimation
		sort(add_vars.begin(), add_vars.end(), compareByVarAddObjVal);
		vector<var> sorted_add_vars_before_erasing = add_vars;
		cout << endl << "sorted add vars : " << endl;
		for (auto v : add_vars)
			cout << v.value << " " << " " << v.obj_val_add << endl;
		cout << endl << "get first " << REPLACE_VARS << " of them" << endl;
		add_vars.resize(REPLACE_VARS);
		// remove vars with interrupted value of the objective function
		for (vector<var>::iterator it = add_vars.begin();
			it != add_vars.end();) 
		{
			if (it->obj_val_add >= MAX_OBJ_FUNC_VALUE)
				it = add_vars.erase(it);
			else
				++it;
		}
		cout << endl << "add vars after remove of interrupted : " << endl;
		for (auto v : add_vars)
			cout << v.value << " " << " " << v.obj_val_add << endl;

		// add REPLACE_VARS * REPLACE_VARS the most proper points at first
		neighbors_points.clear();
		if (add_vars.size() > 0) {
			for (auto x : remove_vars) {
				int pos1 = getVarPos(x.value);
				for (auto y : add_vars) {
					int pos2 = getVarPos(y.value);
					point p = neigh_center;
					p.value[pos1] = false;
					p.value[pos2] = true;
					neighbors_points.push_back(p);
				}
			}
			cout << "first " << REPLACE_VARS+1 << " neighbors : " << endl;
			for (unsigned j = 0; j < REPLACE_VARS+1; j++) {
				vector<unsigned> uvec = uintVecFromPoint(neighbors_points[j]);
				coutUintVec(uvec);
			}
		}
		// then add all remaining points
		vector<point> vec_p;
		for (auto x : sorted_remove_vars_before_erasing) {
			int pos1 = getVarPos(x.value);
			for (auto y : sorted_add_vars_before_erasing) {
				int pos2 = getVarPos(y.value);
				point p = neigh_center;
				p.value[pos1] = false;
				p.value[pos2] = true;
				vec_p.push_back(p);
			}
		}
		cout << sorted_remove_vars_before_erasing.size() << " remove vars in total" << endl;
		cout << sorted_add_vars_before_erasing.size() << " add vars in total" << endl;
		cout << vec_p.size() << " additional randomly shuffled points" << endl;
		cout << "neighbors_points size before additional : " << neighbors_points.size() << endl;
		random_shuffle(vec_p.begin(), vec_p.end()); // randomly shuffle additional points before adding
		for (auto p : vec_p) {
			if (find(neighbors_points.begin(), neighbors_points.end(), p) == neighbors_points.end())
				neighbors_points.push_back(p);
		}
		cout << "neighbors_points size after additional : " << neighbors_points.size() << endl;
		
		// process reduced replace neighborhood
		is_local_record_updated = false;
		if (neighbors_points.size() > 0) {
			is_local_record_updated = processNeighborhood(neighbors_points, neigh_center, is_add_vars, is_break);
			if (is_break)
				break;
		}
		
		if (!is_local_record_updated) {
			cout << "*** interrupt the search: local minimum" << endl;
			writeToGraphFile("--- interrupt: local minimum");
			break; // local minimum
		}
	}
}

void igbfs::onePlusOneSimpleHillClimbing()
{
	// max x function calculations or y seconds without updates of global min, or max total seconds
	onePlusOne(1000000, 1000000, 43200); 
	simpleHillClimbingAddRemovePartialRaplace(global_record_point);
}
