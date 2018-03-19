/* Author: Oleg Zaikin, ISDCT SB RAS, Irkutsk */

#include <iostream>
#include "igbfs.h"

void writeUsage();

int main(int argc, char *argv[])
{
	if ( ( ( argc == 2 ) && (argv[1] == "--help") ) || (argc < 3) ) {
		writeUsage();
		return 1;
	}
	
	igbfs igbfs_obj;
	igbfs_obj.parseParams(argc, argv);
	igbfs_obj.init();
	igbfs_obj.processBackdoor();
	igbfs_obj.solveInstance();
	igbfs_obj.reportResult();
	
	return 0;
}

void writeUsage()
{
	cout << "USAGE: ./alias_ls [options] <cnf-file> <result-output-file>" << endl;
	cout << endl << "CORE OPTIONS:" << endl << endl;
	cout << "-solver    = <string>.                    [OBLIGATORY] IPASIR-based incremental SAT solver name" << endl;
	cout << "-script    = <string> (default: ALIAS.py) [OPTIONAL]   ALIAS script name" << endl;
	cout << "-pcs       = <string>.                    [OPTIONAL]   Name of a PCS (Parameter Configuration Space) file with a start point" << endl;
	cout << "-jump-lim  = <uint32> (default: 3)        [OPTIONAL]   Jumps from local minumums limit, set 0 if you want local search to be fast" << endl;
	cout << "-backdoor  = <string>                     [OPTIONAL]   Name of a file which contains a backdoor (numeration from 1)" << endl;
	cout << "--solve                                   [OPTIONAL]   Enable solving of a given instance by a found backdoor" << endl;
	cout << endl << "MAIN OPTIONS:" << endl << endl;
	cout << "-cpu-lim   = <double> (default: 3600)     [OPTIONAL]   CPU wall time limit" << endl;
	cout << "-verb      = <int32> [0..2] (default: 1)  [OPTIONAL]   Verbosity" << endl;
	cout << "--help     Print help message." << endl;
}