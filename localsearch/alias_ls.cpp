/* Author: Oleg Zaikin, ISDCT SB RAS, Irkutsk */

#include <iostream>
#include "igbfs.h"

void writeUsage();

int main(int argc, char *argv[])
{
	if ( argc < 9 ) {
		writeUsage();
		exit(-1);
	}
	
	igbfs igbfs_obj;
	bool is_solve = true;

	for (int i = 1; i < argc; i++) {
		string par_str = argv[i];
		if ((par_str == "-cnf") && (i+1 < argc))
			igbfs_obj.cnf_name = argv[i+1];
		else if ((par_str == "-pcs") && (i+1 < argc))
			igbfs_obj.pcs_name = argv[i+1];
		else if ((par_str == "-script") && (i + 1 < argc))
			igbfs_obj.alias_script_name = argv[i + 1];
		else if ((par_str == "-solver") && (i+1 < argc))
			igbfs_obj.solver_name = argv[i + 1];
		else if ((par_str == "-timelimit") && (i+1 < argc))
			igbfs_obj.time_limit = atof(argv[i+1]);
		else if ((par_str == "-verb") && (i+1 < argc))
			igbfs_obj.verbosity = atoi(argv[i + 1]);
		else if (par_str == "-e")
			is_solve = false;
	}
	
	igbfs_obj.init();
	igbfs_obj.iteratedGBFS();
	if (is_solve)
		igbfs_obj.solveInstance();
	cout << "*** final total time " << igbfs_obj.timeFromStart() << endl;
	
	return 0;
}

void writeUsage()
{
	cout << "Usage: -cnf file_name -script file_name -solver file_name [-timelimit seconds] [-pcs file_name] [-e] [-verb val]" << endl;
}