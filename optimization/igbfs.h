/* Author: Oleg Zaikin, ISDCT SB RAS, Irkutsk */

#ifndef igbfs_h
#define igbfs_h

#include "base_ls.h"

const unsigned INITIAL_JUMP_STEP = 10;
const unsigned MIN_VARS_JUMP_FROM = 30;
const unsigned ADD_VARS_CALC = 4;
const unsigned ADD_VARS_RECORDS = 4;
const unsigned REPLACE_VARS = 20;
//const unsigned REM_VARS_RECORDS = 5;

class igbfs : public base_local_search
{
public:
	void backJump();
	point permutateRecordPoint();
	point randPermutateRecordPoint();
	//void GBFS(const point start_point);
	void updateLocalRecord(point cur_point);
	point jumpPoint(point cur_point);
	vector<point> neighbors(point p, int neigh_type = 0);
	bool processNeghborhood(vector<point> neighbors_points, point &neigh_center,
		bool &is_break, vector<var> &add_remove_vars, bool is_add_remove_vars_req = false);
	void HCVJ(point start_point);
	void randSearchWholeSpace();
	void randSearchReduceOneVar();
	void simpleHillClimbing(point p = point(), int neigh_type = 0);
	void simpleHillClimbingAddRemovePartialRaplace(point p = point());
	void steepestAscentHillClimbing();
	void tabuSearch();
	void iteratedHCVJ(); // iterated hill climbing with variables-based jump
	void onePlusOne(int fcalc_lim = -1, double time_lim = -1);
	void onePlusOneSimpleHillClimbing();
	int findBackdoor();
	point generateRandPoint(const unsigned point_var_count);
};

#endif


