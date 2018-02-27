/* Author: Oleg Zaikin, ISDCT SB RAS, Irkutsk */

#ifndef igbfs_h
#define igbfs_h

#include "base_ls.h"

using namespace std;

const unsigned INITIAL_JUMP_STEP = 10;
const unsigned MIN_VARS_JUMP_FROM = 40;
const unsigned MAX_ITERATIONS = 2;

class igbfs : public base_local_search
{
public:
	igbfs();

	unsigned vars_decr_times;
	unsigned jump_step;
	point local_record_point;
	point before_jump_point;
	bool is_jump_mode;

	void backJump();
	point permutateRecordPoint();
	void iteratedGBFS(point start_point);
	void GBFS(const point start_point);
	void updateLocalRecord(point cur_point);
	point jumpPoint(point cur_point);
	void iteratedGBFS();
};

#endif


