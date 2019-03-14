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
	igbfs_obj.findBackdoor();
	igbfs_obj.solveInstance();
	igbfs_obj.reportResult();
	
	return 0;
}

void writeUsage()
{
	cout << "USAGE: ./alias_ls [options] <cnf-file> <result-output-file> \n \n";
	cout << "CORE OPTIONS: \n \n";
	cout << "-solver      = <string>.                    [OBLIGATORY] IPASIR-based incremental SAT solver name \n";
	cout << "-script      = <string> (default: ALIAS.py) [OPTIONAL]   ALIAS script name \n";
	cout << "-pcs         = <string>.                    [OPTIONAL]   Name of a PCS (Parameter Configuration Space) file with a start point \n";
	cout << "-backdoor    = <string>                     [OPTIONAL]   Name of a file which contains a backdoor (numeration from 1) \n";
	cout << "-jump-lim    = <uint32> (default: 3)        [OPTIONAL]   Jumps from local minumums limit, set 0 if you want local search to be fast \n";
	cout << "--nojump                                    [OPTIONAL]   Turn off start jumping mode \n";
	cout << "--solve                                     [OPTIONAL]   Enable solving of a given instance by a found backdoor \n";
	cout << "-rand-from   = <uint32>                     [OPTIONAL]   Random search for backdoors - left bound of variables number \n";
	cout << "-rand-to     = <uint32>                     [OPTIONAL]   Random search for backdoors - right bound of variables number \n";
	cout << "-rand-points = <uint32>                     [OPTIONAL]   Random search for backdoors for each (rand_to - rand_from + 1) variables \n";
	cout << "\n" << "MAIN OPTIONS: \n \n";
	cout << "-cpu-lim     = <double> (default: 3600)     [OPTIONAL]   CPU wall time limit \n";
	cout << "-verb        = <int32> [0..2] (default: 1)  [OPTIONAL]   Verbosity \n";
	cout << "--help       Print help message. \n";
}