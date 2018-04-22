/* Author: Oleg Zaikin, ISDCT SB RAS, Irkutsk */

#ifndef merged_ihc_h
#define merged_ihc_h

#include "base_ls.h"

class merged_ihc : public base_local_search
{
public:
	void mergeVaribales();
	void hillClimbing(const point start_point);
};

#endif


