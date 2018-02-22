CPP = mpicxx
CPPFLAGS = -O3 -std=c++0x

igbfs: igbfs.o
	${CPP} ${CPPFLAGS} igbfs.o -o igbfs

igbfs.o: igbfs.cpp
	${CPP} ${CPPFLAGS} igbfs.cpp -c

clean:
	rm -rf *.o
	rm igbfs
	clear
