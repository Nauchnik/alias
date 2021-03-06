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
	cout << "-opt-alg     = <unsigned> (default: 1+1)    [OPTIONAL]   Optimization algorithm. \n";
	cout << "	0: random search in the whole space" << endl;
	cout << "	1: random search reduce size by one" << endl;
	cout << "	2: simple hill climbing (add/remove)" << endl;
	cout << "	3: steepest ascent hill climbing (add/remove)" << endl;
	cout << "	4: tabu search (add/remove)" << endl;
	cout << "	5: (1+1)-EA" << endl;
	cout << "	6: iterated hill climbing with variables-based jump" << endl;
	cout << "	7: simple hill climbing (add/remove/partial-replace)" << endl;
	cout << "	8: (1+1)-EA with simple hill climbing (add/remove/partial-replace)" << endl;
	cout << "	9: simple hill climbing (add/remove/replace)" << endl;
	cout << "-backdoor    = <string>                     [OPTIONAL]   Name of a file which contains a backdoor (numeration from 1) \n";
	cout << "--solve                                     [OPTIONAL]   Enable solving of a given instance by a found (or a given) backdoor \n";
	cout << "\n" << "MAIN OPTIONS: \n \n";
	cout << "-cpu-lim     = <double> (default: 3600)     [OPTIONAL]   CPU wall time limit \n";
	cout << "-verb        = <int32> [0..2] (default: 1)  [OPTIONAL]   Verbosity \n";
	cout << "--help       Print help message. \n";
}
