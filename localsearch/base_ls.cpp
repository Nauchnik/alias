#include <iostream>
#include "base_ls.h"

base_local_search::base_local_search() :
	graph_file_name("alias_records")
{}


void base_local_search::getGraphFileName()
{
	size_t found = cnf_name.find_last_of("/");
	if (found != string::npos) // if full path, get its base part
		graph_file_name = cnf_name.substr(0, found) + "/" + graph_file_name;
	cout << "graph_file_name " << graph_file_name << endl;
}