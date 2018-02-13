/* Author: Oleg Zaikin, ISDCT SB RAS, Irkutsk */

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <ctime>
#include<cstdlib>

// link SAT solver via ipasir
extern "C" {
#include "ipasir.h"
}

using namespace std;

int result = 0;

void loadFormula(string filename, vector<vector<int> > &clauses, int &vars);
int terminator(void* state);
void learnCb(void* state, int* clause);
double cpuTime();

int main(int argc, char** argv) 
{
	cout << "[genipainterval] USAGE: ./prog dimacs.cnf assumptions [verbosity]" << endl;
	if (argc < 3)
		exit(-1);

	string filename = argv[1];
	vector<vector<int> > clauses;
	int vars = 0;
	loadFormula(filename, clauses, vars); // load clauses from a given CNF
	cout << "got " << clauses.size() << " clauses from the file " << filename << endl;
	
	filename = argv[2];
	vector<vector<int> > assumptions;
	loadFormula(filename, assumptions, vars); // load assumptions from a given file
	cout << "got " << assumptions.size() << " assumptions from the file " << filename << endl;

	cout << "got " << vars << " variables" << endl;
	
	int verbosity = 0;
	if ( argc > 3 )
		verbosity = atoi(argv[3]);
	
	if (verbosity > 0)
		cout << "solver : " << ipasir_signature() << endl;

	void *solver = ipasir_init();
	// set temination callback
	ipasir_set_terminate(solver, NULL, terminator);
	ipasir_set_learn(solver, NULL, 0, learnCb);
	
	if (verbosity > 0)
	    cout << "solver init done" << endl;
	
	// add clauses to a solver
	for (unsigned i = 0; i < clauses.size(); i++) {
		vector<int> clause = clauses[i];
		for (unsigned j = 0; j < clause.size(); j++)
			ipasir_add(solver, clause[j]);
		ipasir_add(solver, 0);
	}
	
	if (verbosity > 0)
	    cout << "CNF was loaded" << endl;
	  
	// for each assumption launch the solver and collect time
	double sum_time = 0;
	int res = 0;
	for (unsigned i = 0; i < assumptions.size(); i++) {
		vector<int> assumption = assumptions[i];
		for (unsigned j = 0; j < assumption.size(); j++) {
			ipasir_assume(solver, assumption[j]);
			if (verbosity > 0)
			    cout << assumption[j] << " ";
		}
		cout << endl;
		if (verbosity > 0)
		    cout << "assume done" << endl;
		if (verbosity > 0)
			cout << "before ipasir_solve() # " << i << endl;
		double cur_time = cpuTime();
		res = ipasir_solve(solver);
		cur_time = cpuTime() - cur_time;
		sum_time += cur_time;
		cout << "sum_time " << sum_time << endl;
		if (res == 10)
		    break;
		if (verbosity > 0)
			cout << "ipasir_solve() # " << i << ", result " << res << " , " << cur_time << " seconds" << endl;
	}
	
	if (res == 10) {
		cout << "[genipainterval] formula is satisfiable" << endl;
		cout << "[genipainterval] satisfying assignment : " << endl;
		for (int i = 1; i <= vars; i++)
			cout << ipasir_val(solver, i) << " ";
		cout << endl;
	}
	else 
	    cout << "[genipainterval] formula is unstatisfiable" << endl;
	
	cout << "[genipainterval] All done " << assumptions.size() << " " << sum_time << endl;
	
	//ipasir_release(solver);

	return 0;
}

// Parse a dimacs CNF formula from a given file and
// save its clauses into a given vector.
void loadFormula(string filename, vector<vector<int> > &clauses, int &vars)
{
	clauses.clear();
	ifstream ifile(filename.c_str());
	if (!ifile.is_open()) {
		cerr << "Error : failed to open " << filename << endl;
		exit(-1);
	}
	
	// read all string from a file
	string str;
	
	while (getline(ifile, str)) {
		if ((!str.size()) || (str[0] == 'c'))
			continue; // skip empty, comment and header strings
		stringstream sstream;
		sstream << str;
		if (str[0] == 'p') {
			cout << "try to get vars from string " << str << endl;
			sstream >> str >> str >> vars;
			continue;
		}
		
		int val;
		vector<int> clause;
		while (sstream >> val) // form a new clause
			if (val) clause.push_back(val);
		if (clause.size())
			clauses.push_back(clause); // add the formed clause to the array 
	}

	ifile.close();
}

int terminator(void* state) 
{
    return result;
}

void learnCb(void* state, int* clause) {}

double cpuTime()
{
	return ((double)clock())/CLOCKS_PER_SEC;
}