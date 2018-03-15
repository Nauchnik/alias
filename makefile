all: 
	mkdir -p bin
	cd sampler; export MROOT=..; cd core; make rs; cp sampler_static ../../bin
	cd localsearch; make; cp alias_ls ../bin/
	chmod +x ./solver/ALIAS.py; cp ./solver/ALIAS.py ./bin/
	cp ./solver/alias_settings ./bin/
	chmod +x ./ipasir/scripts/mkall.sh; chmod +x ./ipasir/scripts/mkone.sh; chmod +x ./ipasir/scripts/mkcln.sh; 
	rm -rf ./ipasir/sat/minisat220/
	rm -rf ./ipasir/app/*
	cp -r ./genipainterval ./ipasir/app/ 
	cd ipasir; make all; cp ./bin/genipainterval* ../bin/
clean: 
	cd sampler/core/; export MROOT=..; make clean
	cd localsearch; make clean
	cd ipasir; make clean
	rm -rf bin/*
