/* Author: Oleg Zaikin, ISDCT SB RAS, Irkutsk */

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
	void print(const vector<unsigned> vars);
	
	bool operator==(const point& p) const { return value == p.value; }
};

inline unsigned point::weight() {
	unsigned result = 0;
	for (auto x : value) result += x ? 1 : 0;
	return result;
}

inline void point::print(const vector<unsigned> vars) {
	for (unsigned i = 0; i < value.size(); i++)
		if (value[i]) cout << vars[i] << " ";
	cout << endl;
}

#endif

