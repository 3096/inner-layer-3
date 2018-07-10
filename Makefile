main : bymltest.cpp
	g++ bymltest.cpp -o bymltest -std=c++11

win : bymltest.cpp
	g++ bymltest.cpp -o bymltest.exe -std=c++11 -static-libstdc++ -static-libgcc

clean:
	rm bymltest
