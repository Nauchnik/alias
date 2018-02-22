#ifndef igbfs_h
#define igbfs_h

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

class igbfs
{
private:
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

	inline bool isChecked(const point cur_point) {
		return (find(checked_points.begin(), checked_points.end(), cur_point) != checked_points.end());
	}

	void backJump();
	void getGraphFileName();

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

#endif


