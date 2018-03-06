#include "core/Solver.h"
#include "core/Dimacs.h"
#include <zlib.h>

#include <vector>
#include <random>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>

#include "core/json.hpp"


using json = nlohmann::json;

namespace Minisat {

//static std::vector<int> gen_random_point(int n);


std::vector<int> gen_random_point(int n) {
	std::vector<int> res(n);
	std::random_device rd;     // only used once to initialise (seed) engine
	std::mt19937 rng(rd());    // random-number engine used (Mersenne-Twister in this case)
	std::uniform_int_distribution<> uni(0, 1); // guaranteed unbiased

	for (int i = 0; i < n; i++) {
		res[i] = uni(rng);
	}
	return res;
}

std::string inttostr(int number)
{
	std:: stringstream ss;//create a stringstream

	ss << number;//add number to the stream

	return ss.str();//return a string with the contents of the stream
}

/*
	bool gen_valid_assumptions_rc2(std::vector<int> d_set, std::vector<int> diapason_start,
	uint64_t diapason_size, uint64_t number_of_assumptions, uint64_t& total_count, std::vector<std::vector<int>> & vector_of_assumptions);
*/			

static void print_assumptions_to_file(std::vector<std::vector<std::vector<int>>> VA, std::vector<uint64_t> t_count, 
	std::vector<std::vector<int>> SP, double proportion, std::string output_file, bool separate_diapasons) {
	
	int max_size = 10000;
	std::stringstream p;
	std::ofstream out;
	
	std::string outfn = output_file;

	bool sep = false;

	if ((separate_diapasons == true) && (VA.size() > 1))
		sep = true;
	
	if (sep==false) {
		out.open(outfn);
		std::cout << outfn << "\n";
	}
	//outfn = output_file + ".0";	
	//std::cout<<"Proportion: " << proportion << "\n";
	
	int d = 0;
	for (size_t i = 0; i < VA.size(); i++) {
		if (sep==true) {
			outfn = output_file + ".d" + inttostr(i);
			std::cout << outfn <<" " << VA[i].size() << "\n";
			out.open(outfn);
		}

		p << "c Diapason " << i << ": ";
		for (size_t j = 0; j < SP[i].size(); j++) {
			p << SP[i][j] << " ";		
		}
		p << ". Valid assumptions: " << t_count[i] << "\n";
		for (size_t j = 0; j < VA[i].size(); j++) {
			for (size_t k = 0; k < VA[i][j].size(); k++) {
				p << VA[i][j][k] << " ";
				d++;
			}
			p << "0\n";
			if (d > max_size) {
				out << p.rdbuf();
				p.clear();
				d = 0;
			}
		}
		if (sep) {
			if (d != 0) {
				out << p.rdbuf();
				p.clear();
				d = 0;
			}
			out.close();
		}
	}
	out << p.rdbuf();
	//std::cout << "Stringstream generated\n";
	
	out.close();
	
}

static void dump_assumptions_to_file(std::vector<std::vector<int>> assumptions, std::string output_file){
	if (assumptions.size()!=0){
		int max_size = 10000;
		std::stringstream p;
		std::ofstream out;
		out.open(output_file);
		int d = 0;
		for (size_t j = 0; j < assumptions.size(); j++) {
			for (size_t k = 0; k <assumptions[j].size(); k++) {
				p << assumptions[j][k] << " ";
				d++;
			}
			p << "0\n";
			if (d > max_size) {
				out << p.rdbuf();
				p.clear();
				d = 0;
			}
		}
		out << p.rdbuf();
		out.close();	
	}
}


void generate_sample(std::string json_file, std::string cnf_file, std::string output_file){
	static Solver* solver;

	//std::cout<<json_file<<"\n";
	std::ifstream json_in(json_file);
	json j;
	json_in >> j;	

	json_in.close();
	std::string mode = j["mode"];
	std::vector<int> decomposition_set = j["decomposition_set"];
	int dimension = decomposition_set.size();

/*	for (int i = 0; i< dec_set.size(); i++){
		std::cout<< dec_set[i]<<" ";
	}
	std::cout<<"\n";
*/
	bool multiple_diapasons = true;
	if (j.count("multiple_diapasons") > 0) {
		multiple_diapasons = j["multiple_diapasons"];
		//std::cout << "Block size set to " << block_size << "\n";
	}

	int number_of_diapasons = 0;
	if (j.count("number_of_diapasons") > 0) {
		number_of_diapasons = j["number_of_diapasons"];
	}

	int diapason_size = 0;
	if (j.count("diapason_size") > 0) {
		diapason_size = j["diapason_size"];
	}

	int number_of_assumptions = 0;
	if (j.count("number_of_assumptions") > 0) {
		number_of_assumptions = j["number_of_assumptions"];
		//std::cout << "Block size set to " << block_size << "\n";
	}

	std::vector<int> diapason_start;

	if (j.count("diapason_start") > 0) {
		std::vector<int>  tmp = j["diapason_start"];
		diapason_start = tmp;
	}

	std::vector<int> diapason_end;

	if (j.count("diapason_end") > 0) {
		std::vector<int>  tmp = j["diapason_end"];
		diapason_end = tmp;
	}
	
	int block_size = 0;
	if (j.count("block_size") > 0) {
		block_size = j["block_size"];
		//std::cout << "Block size set to " << block_size << "\n";
	}
	
	bool separate_diapasons = false;
	if (j.count("separate_diapasons") > 0) {
		separate_diapasons = j["separate_diapasons"];
		//std::cout << "Block size set to " << block_size << "\n";
	}
	int number_of_milestones=0;

	if (j.count("number_of_milestones") > 0) {
		number_of_milestones = j["number_of_milestones"];
		//std::cout << "Block size set to " << block_size << "\n";
	}
	int milestone_size=0;
	if (j.count("milestone_size") > 0) {
		milestone_size = j["milestone_size"];
		//std::cout << "Block size set to " << block_size << "\n";
	}

//preliminary check if the input is correct
	Solver S;
    double initial_time = cpuTime();

    solver = &S;
/*
	std::ifstream in(cnf_file.c_str(), std::ios::in);
	if (!in.is_open()){
		std::cout << "ERROR! Could not open file: " + cnf_file + "\n";
		exit(10);
	}
	

	Problem cnf;
	parse_DIMACS(in, cnf);
	in.close();
	S.addProblem(cnf);
*/
	
	gzFile in = gzopen(cnf_file.c_str(), "rb");
    if (in == NULL)
        printf("c ERROR! Could not open file: %s\n", cnf_file.c_str()), exit(1);
    
    parse_DIMACS(in, S);
    gzclose(in);
	
//	std::cout<<"Parsed cnf has "<<S.nVars() <<" variables and "<< S.nClauses()<<" clauses\n";

	std::vector<uint64_t> t_count;
	std::vector<std::vector<std::vector<int>>> results;
	//std::stringstream p;
	std::vector<std::vector<int>> diapason_startS;
	double proportion = 0;
	if (mode == "automatic"){		
		for (int i = 0; i < number_of_diapasons; i++){
			uint64_t current_count = 0;
			std::vector<int> current_start = gen_random_point(dimension);
			diapason_startS.push_back(current_start);	
			
			/*std::cout<<"Current start \n";
			for (int u = 0; u < current_start.size(); u++){
				std::cout<<current_start[u]<<" ";
			}
			std::cout<<"\n";
			*/

			std::vector<std::vector<int>> current_diapason_results;
			S.gen_valid_assumptions_rc2(decomposition_set,	current_start, diapason_size, number_of_assumptions,
			current_count, current_diapason_results);

            while (current_diapason_results.size()<number_of_assumptions){
                std::vector<int> tmp = gen_random_point(decomposition_set.size());
                std::vector<int> f(tmp);
                for (int u = 0; u < f.size(); u++){
                    if (f[u]==0){
                        f[u] = - decomposition_set[u];
                    }
                    else {
                        f[u] = decomposition_set[u];
                    }
                }
                current_diapason_results.push_back(f);
            }

			t_count.push_back(current_count);
			proportion += (double)current_count;
			results.push_back(current_diapason_results);
		}		
		proportion = proportion / (double) (number_of_diapasons*diapason_size);
		print_assumptions_to_file(results, t_count, diapason_startS, proportion, output_file, separate_diapasons);
	}
	if (mode == "whole"){
		//std::cout<<"Whole mode activated\n";
		
		uint64_t max_size = 1ULL << (dimension);
//		std::cout<<max_size<<"\n";
		std::vector<std::vector<int>> all_assumptions;
		uint64_t current_count=0;
/*
bool  Solver::gen_all_valid_assumptions_rc2(std::vector<int> d_set, uint64_t& total_count, std::vector<std::vector<int>> & vector_of_assumptions){
*/
		if (block_size > 0) {
			std::vector<int> t(dimension);
			std::vector<int> t_old(dimension);
			std::vector<int> t_res(dimension);
			for (size_t i = 0; i < t_res.size(); i++) {
				t_res[i] = 1;
			}
			int file_cnt = 0;
			uint64_t r_count = 0;
			bool ok = true;
			while (ok) {				
				S.gen_all_valid_assumptions_blocks_rc2(decomposition_set, t, block_size, current_count, all_assumptions);
				std::string tmp_fn = output_file + std::to_string(file_cnt);
				proportion = (double)(current_count - r_count) / (double)(block_size);
				//std::vector<std::vector<std::vector<int>>> TmpVA = { all_assumptions };
				//std::vector<uint64_t> TmpCNT = { current_count - r_count };
				//static void print_assumptions_to_file(std::vector<std::vector<std::vector<int>>> &VA, std::vector<uint64_t> &t_count, 
				//std::vector<std::vector<int>> SP, double proportion, std::string output_file)
				print_assumptions_to_file({ all_assumptions }, { current_count - r_count } , { t_old }, proportion , tmp_fn, separate_diapasons);
				all_assumptions.clear();
				r_count = current_count;
				if (t_res == t) {
					ok = false;
				}
				t_old= t;
				file_cnt++;
			}		
		}
		else {
			S.gen_all_valid_assumptions_rc2(decomposition_set, current_count, all_assumptions);
			std::vector<int> tmp(dimension);
			print_assumptions_to_file({ all_assumptions }, { current_count }, {tmp}, (double)(current_count)/double(1ULL<<dimension) , output_file, separate_diapasons);
		}

		
		results.push_back(all_assumptions);		
		
		proportion = (double) all_assumptions.size() / (double)(1ULL<<dimension);
	}
	if (mode == "hybrid"){		
		uint64_t valid_count = 0;
		S.count_all_valid_assumptions_rc2(decomposition_set, valid_count);
		std::cout<<"Valid points: "<<valid_count<<"\n";
		while ((number_of_diapasons * number_of_assumptions)>(double)0.1*(double)valid_count){
			number_of_assumptions = number_of_assumptions / 2;
		}

		std::vector<int> numbers;
		uint64_t t_p = valid_count/number_of_diapasons;
		

		std::random_device rd;     // only used once to initialise (seed) engine
		std::mt19937 rng(rd());    // random-number engine used (Mersenne-Twister in this case)
		std::uniform_int_distribution<> uni(0, t_p-number_of_assumptions); // guaranteed unbiased
			
		for (int i = 0; i <number_of_diapasons; i++){
			int s_p = t_p*i + uni(rng);
			for (int u = 0; u<number_of_assumptions; u++){
				numbers.push_back(s_p+u);
			} 			
		}
		
		//debug
/*
		std::cout<<"number_of_assumptions "<<number_of_assumptions<<"\n";
		std::cout<<"number_of_diapasons "<<number_of_diapasons<<"\n";
		std::cout<<"Numbers:\n";
		for (int i =0; i <numbers.size();i++){
			std::cout<<numbers[i]<<" ";
		}
		std::cout<<"\n";*/
		//debug		

		std::vector<std::vector<int>> assumptions;	
		uint64_t tmp=0;
		S.gen_all_valid_assumptions_rc2_numbers(decomposition_set, tmp, numbers, assumptions);
		//	bool gen_all_valid_assumptions_rc2_numbers(std::vector<int> d_set, uint64_t& total_count, std::vector<int> &numbers, std::vector<std::vector<int>> & vector_of_assumptions);
		
		for (int i = 0; i < number_of_diapasons; i++){
			uint64_t current_count = 0;			
			diapason_startS.push_back(assumptions[i*number_of_assumptions]);
			
			std::vector<std::vector<int>> current_diapason_results;	
			for (int u = 0; u <number_of_assumptions; u++){
				current_diapason_results.push_back(assumptions[i*number_of_assumptions +u]);
			}
 
			t_count.push_back(t_p);
			results.push_back(current_diapason_results);
		}		
		proportion = 0;
		print_assumptions_to_file(results, t_count, diapason_startS, proportion, output_file, separate_diapasons);
	}
	if (mode == "milestones"){		
		uint64_t valid_count = 0;
		S.count_all_valid_assumptions_rc2(decomposition_set, valid_count);
		std::cout<<"Valid points: "<<valid_count<<"\n";
		
		std::vector<int> milestones(number_of_milestones);
		for (int u=0; u < number_of_milestones; u++){
			milestones[u]= (valid_count*u)/number_of_milestones;
		}		
		//debug
		for (int u=0; u < number_of_milestones; u++){
			std::cout << milestones[u] << " ";
		}		
		std::cout<<"\n";
		//debug
		
		std::vector<std::vector<int>> ms_assumptions;	

		uint64_t tmp=0;
		S.gen_all_valid_assumptions_rc2_numbers(decomposition_set, tmp, milestones, ms_assumptions);
		ms_assumptions.push_back(milestones);

		//	bool gen_all_valid_assumptions_rc2_numbers(std::vector<int> d_set, uint64_t& total_count, std::vector<int> &numbers, std::vector<std::vector<int>> & vector_of_assumptions);
		dump_assumptions_to_file(ms_assumptions, output_file);
		std::cout<<output_file<<"\n";
	}
	if (mode == "solving"){
		uint64_t current_count = 0;
		bool ok = true;
		int file_cnt = 0;
		std::vector<std::vector<int>> all_assumptions;
		while (ok) {				

			if (milestone_size<block_size){
				block_size = milestone_size;
			}				

			S.gen_all_valid_assumptions_blocks_rc2(decomposition_set, diapason_start, block_size, current_count, all_assumptions);

			milestone_size = milestone_size - block_size;
			if (milestone_size==0){
				ok=false;
			}
			std::string tmp_fn = output_file + std::to_string(file_cnt);

			dump_assumptions_to_file(all_assumptions, tmp_fn);
			std::cout<<tmp_fn<<" "<<all_assumptions.size()<<"\n";	
			all_assumptions.clear();
			
	
			file_cnt++;
		}			

	}

	if (mode == "manual_whole"){
		uint64_t current_count = 0;
		bool ok = true;
		int file_cnt = 0;
		std::vector<std::vector<int>> all_assumptions;

//bool Solver::gen_diapason_whole(std::vector<int> d_set, std::vector<int> &diapason_start, std::vector<int> &diapason_end,
//		uint64_t block_size, uint64_t& total_count, std::vector<std::vector<int>> & vector_of_assumptions){
		while (ok) {	

/*
			std::cout<<"Diapason_start:\n";	
			for (int i =0; i < diapason_start.size();i++){
				std::cout<<diapason_start[i]<<" ";
			}
			std::cout<<"\n"<<"Diapason_end:\n";
			for (int i =0; i < diapason_end.size();i++){
 				std::cout<<diapason_end[i]<<" ";
			}			
			std::cout<<"\n";
			std::cout<<"Ds>de:"<< (diapason_start>diapason_end) <<"\n";
	*/		

							
			ok = S.gen_diapason_whole(decomposition_set, diapason_start, diapason_end, block_size, current_count, all_assumptions);
			
			if (diapason_start >= diapason_end){
				ok = false;			
			}
			if (all_assumptions.size()!=0) {
				std::string tmp_fn = output_file + std::to_string(file_cnt);

				dump_assumptions_to_file(all_assumptions, tmp_fn);
				std::cout<<tmp_fn<<" "<<all_assumptions.size()<<"\n";	
				all_assumptions.clear();
				file_cnt++;
			}
		}			
	}

	std::cout<<"Done!\n";

}
}
