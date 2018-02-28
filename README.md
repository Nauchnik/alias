modulAr tooL for fInding bAckdoors for Sat (ALIAS) - a customizable scalable tool that can employ incremental state-of-the-art SAT solvers and black-box optimization heuristics to search for backdoors for hard SAT instances. The found backdoor is then used to solve the corresponding instance by the same incremental solver. Thereby, ALIAS can be viewed as a tool for constructing backdoor-based divide-and-conquer parallel SAT solvers.

Directories:

localsearch - implementation of greedy best first search (GBFS) which traverse a search space in order to find a backdoor with low runtime estimation 

genipainterval - program that, given a CNF formula and a set of assumptions, processes the latter using some incremental way. To build it one needs the IPASIR API.

sampler - a minisat-based program which prepares data for calculating the runtime estimation for a given backdoor

smac - script for launching ALIAS by SMAC

solver - script which can calculates runtime estimation for a given backdoor and also solve a SAT instance using it

utils - some additional scripts

LICENSE generic license for parts of this software not explicitly covered by its own license restrictions, which can be found in the corresponding LICENSE or COPYRIGHT file in a sub directory or in a distribution package, such as in an included tar or zip file
