CPP = mpicxx
CPPFLAGS = -O3 -std=c++0x

igbfs: base_ls.o igbfs.o alias_ls.o
	${CPP} ${CPPFLAGS} base_ls.o igbfs.o alias_ls.o -o alias_ls

alias_ls.o: alias_ls.cpp
	${CPP} ${CPPFLAGS} alias_ls.cpp -c

igbfs.o: igbfs.cpp
	${CPP} ${CPPFLAGS} igbfs.cpp -c

base_ls.o: base_ls.cpp
	${CPP} ${CPPFLAGS} base_ls.cpp -c

clean:
	rm -rf *.o
	rm alias_ls
	clear
