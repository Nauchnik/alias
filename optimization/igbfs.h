/* Author: Oleg Zaikin, ISDCT SB RAS, Irkutsk */

#ifndef igbfs_h
#define igbfs_h

#include "base_ls.h"

const unsigned INITIAL_JUMP_STEP = 10;
const unsigned MIN_VARS_JUMP_FROM = 30;
const unsigned ADD_VARS_CALC = 4;
const unsigned ADD_VARS_RECORDS = 4;
const unsigned REPLACE_VARS = 10;

class igbfs : public base_local_search
{
public:
	void backJump();
	point permutateRecordPoint();
	point randPermutateRecordPoint();
	//void GBFS(const point start_point);
	void updateLocalRecord(point cur_point, int neighbor_index = -1, int neighbor_size = -1);
	point jumpPoint(point cur_point);
	vector<point> neighbors(point p, vector<bool> &is_add_vars, int neigh_type = 0);
	bool processNeighborhood(vector<point> neighbors_points, point &neigh_center,
		vector<bool> is_add_vars, bool &is_break, bool is_add_remove_vars_req = false);
	void HCVJ(point start_point);
	void randSearchWholeSpace();
	void randSearchReduceOneVar();
	void simpleHillClimbing(int neigh_type = 0, point p = point());
	void simpleHillClimbingAddRemovePartialRaplace(point p = point());
	void steepestAscentHillClimbing();
	void tabuSearch();
	void iteratedHCVJ(); // iterated hill climbing with variables-based jump
	void onePlusOne(int fcalc_lim = -1, double time_lim = -1);
	void onePlusOneSimpleHillClimbing();
	int findBackdoor();
	point generateRandPoint(const unsigned point_var_count);
protected:
	unsigned get_diff_var(point p1, point p2);
};

#endif


