#ifndef base_ls_h
#define base_ls_h

#include <vector>
#include <fstream>
#include <string>

#include "point.h"

using namespace std;

class base_local_search
{
public:
	base_local_search();
	vector<unsigned> vars;
	vector<point> checked_points;
	unsigned skipped_points_count;
	unsigned interrupted_points_count;
	point global_record_point;
	string cnf_name;
	string graph_file_name;
	fstream graph_file;

	void getGraphFileName();

	inline bool isChecked(const point cur_point) {
		return (find(checked_points.begin(), checked_points.end(), cur_point) != checked_points.end());
	}

	inline void printPoint(const point p) {
		for (unsigned i = 0; i < p.value.size(); i++)
			if (p.value[i]) cout << vars[i] << " ";
		cout << endl; }

	inline void printGlobalRecordPoint() { printPoint(global_record_point); }

	inline void writeToGraphFile(const string str) {
		graph_file.open(graph_file_name, ios_base::app);
		graph_file << str << endl;
		graph_file.close(); }
};

#endif


