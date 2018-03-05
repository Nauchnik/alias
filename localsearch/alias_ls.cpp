/* Author: Oleg Zaikin, ISDCT SB RAS, Irkutsk */

#include <iostream>

#include "igbfs.h"

int main(int argc, char *argv[])
{
	if (argc < 5) {
		cerr << "Usage: -cnf cnf_name -script script_name -pcs pcs_name -solver solver_name -time time_limit -verb verb_val" << endl;
		exit(-1);
	}

	igbfs igbfs_obj;

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
		else if ((par_str == "-time") && (i+1 < argc))
			igbfs_obj.time_limit = atof(argv[i+1]);
		else if ((par_str == "-verb") && (i+1 < argc))
			igbfs_obj.verbosity = atoi(argv[i + 1]);
	}
	
	igbfs_obj.init();
	igbfs_obj.iteratedGBFS();
	igbfs_obj.solveInstance();

	cout << "*** final total time " << igbfs_obj.timeFromStart() << endl;
	
	return 0;
}