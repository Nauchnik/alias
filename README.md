## DESCRIPTION

modulAr tooL for fInding bAckdoors for Sat (ALIAS) is a tool that can employ 
state-of-the-art incremental SAT solvers and black-box optimization heuristics 
to search for backdoors for hard SAT instances. The found backdoor can then be 
used to solve the corresponding instance by the same incremental solver. Thus, 
ALIAS can be viewed as a tool for constructing backdoor-based divide-and-conquer 
parallel SAT solvers.

## DIRECTORY OVERVIEW

solver/          contains ALIAS.py and its settings file. ALIAS.py takes as an 
                 input a SAT instance, incremental solver and a backdoor, and 
                 calculates a corresponding runtime estimation. It also can 
                 solve a given instance via provided backdoor and solver.

optimization/    implementation of discrete black-box optimization algorithms which 
                 traverse a search space in order to find a backdoor with low 
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

## REQUIRED PACKAGES

python3.6, g++ (version 5+), build-essential, git, unzip, zlib

> sudo apt-get install g++ build-essential git unzip python3.6 zlibc zlib1g zlib1g-dev

To install python3.6 on old Ubuntu versions (e.g., 14.04 or 16.04), please follow the instructions from this quide
https://askubuntu.com/questions/865554/how-do-i-install-python-3-6-using-apt-get

## HOW TO BUILD

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

## HOW TO LAUNCH

> cd ./bin/

> ./alias_ls [options] <cnf-file> <result-output-file>

By default alias_ls searches for a backdoor with good runtime estimation. It 
can also solve a given instance if the paramater --solve is set. 
A backdoor can be given by the parameter -backdoor.

If it is required to use an optimization algorithm to find a backdoor, 
a search space should be given as an input file in the PCS format.
PCS format is described here http://aclib.net/cssc2014/pcs-format.pdf
The file name should be preceded by -pcs

NB! Backdoors variables are numbered starting from 1.

In order to build a PCS file, one can use the script /alias/utils/generate_pcs.py
As an input it takes a text file that contains a list of Boolean variables from 
a search space. The script output is a file in the PCS format. For instance, 
a search space contains 4 Boolean varibles with numbers 1-4.
Then the input file of the script contains the following string:
1 2 3 4
The otuput file is as follows:
v1 {0,1}[1]
v2 {0,1}[1]
v3 {0,1}[1]
v4 {0,1}[1]

## HOW TO USE THE OBJECTIVE FUNCTION

One can employ any discrete black-box optimization algorithm to minimize the ALIAS's 
objective function in some search space. Hereinafter a backdoor corresponds to a point in a search space.
The instructions are as follows.
1. Instead of the C++ program alias_ls, the script /solver/ALIAS.py should be directly used.
2. Main parameters should be given to the script: -e, -cnf filename.
For instance, ./ALIAS.py -e -cnf ./ASG_72_keystream76_0.cnf
All paramters are described in /solver/README in detail.
3. A point from the search space should be given in the following format: -v[variable number] '0|1'.
Here -vx '1' means that a point contains the variable with the number x, '0' means otherwise.
Each variable from a point should be set as an independent parameter to the script.
For instance, the search space contains the following 72 Boolean variables: x1, x2, .., x72 and the CNF
ASG_72_keystream76_0.cnf is studied. Then, if one want to calculate the objective function value for the 
point {x2, x11, x29}, the script should be launched as follows:
./ALIAS.py -e -cnf ./ASG_72_keystream76_0.cnf -v2 '1' -v11 '1' -v29 '1'
4. The objective function value can be extracted from the script's output. In particular,
the value is a real number prceeded by the string 'SUCCESS, 0, 0, '
For instance, the script output 'SUCCESS, 0, 0, 345.34' means that the objective function value is 345.34.

Four hard optimization problems, which correspond to four SAT-based cryptanalysis problems of eSTREAM stream ciphers,
can be found in /benchmarks/eSTREAM/. Each problem is described by a pair <CNF file, PCS file>.
For instance, to launch the objective function on the optimization problem for the Mickey stream cipher,
two files should be used: Mickey_0.cnf and Mickey.pcs.

## EXAMPLES

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

## CITATION

ALIAS can be cited as follows:

```
@inproceedings{DBLP:conf/sat/KochemazovZ18,
  author    = {Stepan Kochemazov and
               Oleg Zaikin},
  title     = {{ALIAS:} {A} Modular Tool for Finding Backdoors for {SAT}},
  booktitle = {Theory and Applications of Satisfiability Testing - {SAT} 2018 - 21st
               International Conference, {SAT} 2018, Held as Part of the Federated
               Logic Conference, FloC 2018, Oxford, UK, July 9-12, 2018, Proceedings},
  pages     = {419--427},
  year      = {2018},
  crossref  = {DBLP:conf/sat/2018},
  url       = {https://doi.org/10.1007/978-3-319-94144-8\_25},
  doi       = {10.1007/978-3-319-94144-8\_25},
  timestamp = {Mon, 15 Oct 2018 13:22:26 +0200},
  biburl    = {https://dblp.org/rec/bib/conf/sat/KochemazovZ18},
  bibsource = {dblp computer science bibliography, https://dblp.org}
}
```
