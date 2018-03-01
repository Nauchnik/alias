#include "igbfs.h"

#include <iostream>
#include <sstream>
#include <algorithm>

void igbfs::backJump() {
	cout << "* backjumping" << endl;
	local_record_point.estimation = 1e+308;
	//cout << "* forget global record" << endl;
	jump_step = INITIAL_JUMP_STEP;
	vars_decr_times = 0;
	local_record_point = before_jump_point;
	cout << "back to the point with weight " << local_record_point.weight() << " : " << endl;
	before_jump_point.print(vars);
	is_jump_mode = false; // jump only once
	cout << "* is_jump_mode " << is_jump_mode << endl;
}

point igbfs::permutateRecordPoint()
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
	mod_point.print(vars);
	return mod_point;
}

void igbfs::iteratedGBFS()
{
	point start_point;
	start_point.value.resize(vars.size());
	for (auto x : start_point.value)
		x = true;
	cout << "start point : " << endl;
	start_point.print(vars);

	unsigned iteration_count = 0;
	global_record_point = start_point;
	global_record_point.estimation = 1e+308;
	stringstream sstream;
	cpu_cores = getCpuCores();
	sstream << "vars est-1-core est-" << cpu_cores << "-cores elapsed";
	writeToGraphFile(sstream.str());
	sstream.str(""); sstream.clear();
	for (;;) {
		cout << endl << "*** GBFS iteration # " << iteration_count << endl;
		if (iteration_count > 0) {
			stringstream sstream;
			sstream << "--- GBFS iteration # " << iteration_count;
			writeToGraphFile(sstream.str());
			sstream.str(""); sstream.clear();
		}
		GBFS(start_point);
		iteration_count++;
		if (isTimeExceeded() || isEstTooLong() || (iteration_count > MAX_ITERATIONS))
		{
			cout << "*** interrupt the search" << endl;
			break;
		}
		start_point = permutateRecordPoint();
	}
	sstream << "--- start solving, time " << timeFromStart();
	writeToGraphFile(sstream.str());
	sstream.str(""); sstream.clear();
	
	reportFinalEstimation();
}

void igbfs::GBFS(const point start_point)
{
	bool is_record_updated;
	vars_decr_times = 0;
	jump_step = INITIAL_JUMP_STEP;
	local_record_point = start_point;
	local_record_point.estimation = 1e+308;

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
				cout << "skip point with estimation " << cur_point.estimation << endl;
				interrupted_points_count++;
				if ( is_jump_mode ) {
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
			else {
				if (i >= 0) // return initial value
					cur_point.value[changing_vars[i]] = local_record_point.value[changing_vars[i]]; 
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
		cout << "** new global_record_esimation " << global_record_point.estimation << 
			    " with weight " << global_record_point.weight() << endl;
		printGlobalRecordPoint();
		stringstream sstream;
		//"vars est-1-core est-" << cpu_cores << "-cores elapsed" << endl;
		sstream << global_record_point.weight() << " " << global_record_point.estimation << " " <<
			global_record_point.estimation / cpu_cores << " " << timeFromStart();
		writeToGraphFile(sstream.str());
		if (global_record_point.weight() <= prev_global_record_point.weight())
			vars_decr_times++;
		else {
			vars_decr_times = 0;
			if (is_jump_mode) {
				is_jump_mode = false;
				cout << "** is_jump_mode " << is_jump_mode << endl;
			}
		}
	}
	else {
		cout << "* new local_record_estimation " << local_record_point.estimation <<
			" with weight " << local_record_point.weight() << endl;
	}
	cout << "time " << timeFromStart() << endl;
	
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
	cout << "jump_step " << jump_step << endl;
	if (cur_point.value.size() > 10) {
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
	}
	cout << "jump point weight " << jump_point.weight() << endl;
	jump_point.print(vars);
	
	return jump_point;
}
