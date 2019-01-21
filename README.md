================================================================================
DESCRIPTION

modulAr tooL for fInding bAckdoors for Sat (ALIAS) is a tool that can employ 
state-of-the-art incremental SAT solvers and black-box optimization heuristics 
to search for backdoors for hard SAT instances. The found backdoor can then be 
used to solve the corresponding instance by the same incremental solver. Thus, 
ALIAS can be viewed as a tool for constructing backdoor-based divide-and-conquer 
parallel SAT solvers.

================================================================================
DIRECTORY OVERVIEW

solver/          contains ALIAS.py and its settings file. ALIAS.py takes as an 
                 input a SAT instance, incremental solver and a backdoor, and 
                 calculates a corresponding runtime estimation. It also can 
                 solve a given instance via provided backdoor and solver.

localsearch/     implementation of greedy best first search (GBFS) which 
                 traverses a search space in order to find a backdoor with low 
                 runtime estimation 

genipainterval/  program for IPASIR. This program is used to build an incremental
                 SAT solver which, given a CNF formula and a set of assumptions, 
                 processes the latter in incremental way

sampler/         a cominisatps-based program which performs sampling, i.e. 
                 prepares the random data to compute the runtime estimation for 
                 a given backdoor

ipasir/          IPASIR sources (https://github.com/biotomas/ipasir) are 
                 required to build genipainterval
				 
utils/           scripts for launching ALIAS on SMAC and on supercomputers

test/            simple tests which can be used to make sure that everything is working 
		 as intended

benchmarks/      contains crafted and cryptographic instances which were
                 studied by ALIAS, as well as found backdoors

================================================================================
REQUIRED PACKAGES

python3.6, g++ (version 5+), build-essential, git, unzip, zlib

> sudo apt-get install g++ build-essential git unzip python3.6 zlibc zlib1g zlib1g-dev

To install python3.6 on old Ubuntu versions (e.g., 14.04 or 16.04), please follow the instructions from this quide
https://askubuntu.com/questions/865554/how-do-i-install-python-3-6-using-apt-get

================================================================================
HOW TO BUILD

> git clone --recurse-submodules https://github.com/Nauchnik/alias.git

> cd alias

> make

make command builds binaries for every IPASIR-compatible SAT solver from 
alias/ipasir/sat/. By default, this folder contains sources only for 
the picosat solver. In order to build any other IPASIR-compatable solver, 
one needs to copy its sources into the mentioned folder. Solvers from the SAT 
Competition 2017 Incremental track

https://baldur.iti.kit.edu/sat-competition-2017/solvers/incremental/ 
can be used for this purpose. 

For instance, to build IPASIR-based glucose4, one should:

- > wget https://baldur.iti.kit.edu/sat-competition-2017/solvers/incremental/glucose-ipasir.zip

- > unzip ./glucose-ipasir.zip -d ./glucose-ipasir/

- > cp -rf ./glucose-ipasir/sat/glucose4/ ./alias/ipasir/sat/

- > make clean

- > make

================================================================================
HOW TO LAUNCH

> cd ./bin/

> ./alias_ls [options] <cnf-file> <result-output-file>

By default alias_ls searches for a backdoor with good runtime estimation. It 
can also solve a given instance if the paramater --solve is set. 
A backdoor can be given by the parameter -backdoor.

NB! Backdoors variables are numbered starting from 1.

NB! In order to build a file which will specify a local search starting point, 
use /alias/utils/generate_pcs.py

PCS format is described here http://aclib.net/cssc2014/pcs-format.pdf

================================================================================
EXAMPLES

- Example 1 - starting point is a whole set of CNF variables, time limit 100 seconds, 
find backdoor and stop

> ./alias_ls ../test/sgen6-1200-5-1.cnf -solver=genipainterval-picosat961 -cpu-lim=100

- Example 2 - starting point is a whole set of CNF variables time limit 10000 seconds, 
find backdoor and use it to solve the SAT instance

> ./alias_ls ../test/sgen6-1200-5-1.cnf -solver=genipainterval-picosat961 -cpu-lim=10000 --solve

- Example 3 - starting point is a SUPBS (the first 72 variables), time limit 10000 seconds, 
solve a given instance using a found backdoor

> ./alias_ls ../test/ASG_72_keystream76_0.cnf -pcs=../test/first72vars.pcs -solver=genipainterval-picosat961 -cpu-lim=10000 --solve

- Example 4 - estimate runtime for an instance using given backdoor
> ./alias_ls ../test/sgen6-1200-5-1.cnf -solver=genipainterval-picosat961 -backdoor=../test/sgen6-1200-5-1.backdoor

- Example 5 - solve an instance using given backdoor, time limit 100000 seconds
> ./alias_ls ../test/ASG_72_keystream76_0.cnf -pcs=../test/first72vars.pcs -solver=genipainterval-picosat961 -backdoor=../test/ASG_72_keystream76_0.backdoor -cpu-lim=100000 --solve
