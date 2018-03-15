================================================================================
Description

modulAr tooL for fInding bAckdoors for Sat (ALIAS) - a customizable scalable tool that can employ incremental state-of-the-art SAT solvers and 
black-box optimization heuristics to search for backdoors for hard SAT instances. The found backdoor is then used to solve the corresponding 
instance by the same incremental solver. Thereby, ALIAS can be viewed as a tool for constructing backdoor-based divide-and-conquer parallel SAT 
solvers.

Folders:

solver - contains ALIAS.py and its settings file. ALIAS.py calculates a runtime estimation for a given backdoor. It also can solve a given instance 
using the backdoor.

localsearch - implementation of greedy best first search (GBFS) which traverse a search space in order to find a backdoor with low runtime estimation 

genipainterval - program for IPASIR. This program, given a CNF formula and a set of assumptions, processes the latter using some incremental way.

sampler - a minisat-based program which prepares data for calculating the runtime estimation for a given backdoor

ipasir - IPASIR sources (https://github.com/biotomas/ipasir) required to build incremental SAT solvers.

utils - scripts for launchsing ALIAS on SMAC and on supercomputers.

================================================================================
Required packages

python3.6+, g++, build-essential, git and unzip are required:

sudo apt-get install python3.6 g++ build-essential git unzip

================================================================================
Build

git clone --recurse-submodules https://github.com/Nauchnik/alias.git

cd alias

make

make command builds binaries for every IPASIR-compatable SAT solver from the folder alias/ipasir/sat/. By default, this folder contains sources for 
the picosat solver. In order to build any other IPASIR-compatable solver, one should copy its sources into the mentioned folder. Solvers from the SAT 
Competition 2017 Incremental track https://baldur.iti.kit.edu/sat-competition-2017/solvers/incremental/ can also  be used fot this purpose. 
For instance, to build IPASIR-based glucose4, one should:

- download the archive https://baldur.iti.kit.edu/sat-competition-2017/solvers/incremental/glucose-ipasir.zip

- unzip ./glucose-ipasir.zip -d ./glucose-ipasir/

- cp ./glucose-ipasir/sat/glucose4/ ./alias/ipasir/sat/

================================================================================
Launch

cd ./bin/

./alias_ls [options]

By default alias_ls is searching for a backdoor with good estimation. It can also solve a given instance if --solve is set.

Example #1 -- starting point is a whole set of CNF variables, picosat solver, time limit 100 seconds, don't solve a given instance using a backdoor
./alias_ls -cnf=../test/sgen6-1200-5-1.cnf -solver=genipainterval-picosat961 -script=ALIAS.py -cpu-lim=100

Example #2 -- starting point is a whole set of CNF variables, glucose4 solver, time limit 3600 seconds, solve a given instance using a backdoor

./alias_ls -cnf=../test/sgen6-1200-5-1.cnf -solver=genipainterval-glucose4 -script=ALIAS.py -cpu-lim=3600 --solve

Example #3 - starting point is the set of first 72 variables

./alias_ls -cnf=../test/ASG_72_keystream76_0.cnf -pcs=../test/first72vars.pcs -solver=genipainterval-picosat961 -script=ALIAS.py -cpu-lim=100

================================================================================
License

LICENSE generic license for parts of this software not explicitly covered by its own license restrictions, which can be found in the 
corresponding LICENSE or COPYRIGHT file in a sub directory or in a distribution package, such as in an included tar or zip file