win: src/main.cpp
	g++ src/main.cpp -o sortetdir.exe -std=c++17 -lm -Iinclude
	sortetdir.exe

unix: src/main.cpp
	g++ src/main.cpp -o sortetdir -std=c++17 -lm -Iinclude
	./sortetdir