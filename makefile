all:
	make sha1.o
	make base64.o
	make ws.o
	make test
sha1.o: sha1.cpp
	clang++ sha1.cpp -c -osha1.o -std=c++17
base64.o: base64.cpp
	clang++ base64.cpp -c -obase64.o -std=c++17
ws.o: ws.cpp
	clang++ ws.cpp -c -ows.o -std=c++17
test: test.cpp
	clang++ test.cpp -otest
check-syntax:
	clang++ -fsyntax-only sha1.cpp -std=c++17
	clang++ -fsyntax-only base64.cpp -std=c++17
	clang++ -fsyntax-only ws.cpp -std=c++17
	clang++ -fsyntax-only test.cpp -std=c++17;
