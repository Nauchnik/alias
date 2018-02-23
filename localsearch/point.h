#ifndef point_h
#define point_h

#include <vector>

using namespace std;

class point
{
public:
	vector<bool> value;
	double estimation;
	
	unsigned weight();
	
	bool operator==(const point& p) const { return value == p.value; }
};

inline unsigned point::weight() {
	unsigned result = 0;
	for (auto x : value) result += x ? 1 : 0;
	return result;
}

#endif

