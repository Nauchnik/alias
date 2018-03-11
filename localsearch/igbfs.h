/* Author: Oleg Zaikin, ISDCT SB RAS, Irkutsk */

#ifndef igbfs_h
#define igbfs_h

#include "base_ls.h"

using namespace std;

const unsigned INITIAL_JUMP_STEP = 10;
const unsigned MIN_VARS_JUMP_FROM = 30;
const unsigned MAX_ITERATIONS = 2;
const unsigned ADD_VARS_CALC = 7;
const unsigned ADD_VARS_RECORDS = 8;
const unsigned REM_VARS_RECORDS = 5;

class igbfs : public base_local_search
{
public:
	void backJump();
	point permutateRecordPoint();
	point randPermutateRecordPoint();
	void GBFS(const point start_point);
	void updateLocalRecord(point cur_point);
	point jumpPoint(point cur_point);
	void iteratedGBFS();
};

#endif


