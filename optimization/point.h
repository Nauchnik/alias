/* Author: Oleg Zaikin, ISDCT SB RAS, Irkutsk */

#ifndef point_h
#define point_h

#include <vector>
#include <string>
#include <sstream>

using namespace std;

struct var
{
	unsigned value;
	unsigned global_records;
	unsigned calculations;
	double obj_val_remove;
	double obj_val_add;
	
	bool operator==(const var a)
	{
		return ((*this).value == a.value);
	}
};

class point
{
public:
	vector<bool> value;
	double estimation;
	
	unsigned weight();
	string getStr(const vector<var> vars);
	
	bool operator==(const point& p) const { return value == p.value; }
};

inline unsigned point::weight() {
	unsigned result = 0;
	for (auto x : value) 
		result += x ? 1 : 0;
	return result;
}

inline string point::getStr(const vector<var> vars) {
	stringstream sstream;
	for (unsigned i = 0; i < value.size(); i++)
		if (value[i]) sstream << vars[i].value << " ";
	sstream << endl;
	return sstream.str();
}

#endif

