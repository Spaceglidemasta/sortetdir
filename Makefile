main: main.cpp
	g++ src/main.cpp -o main.exe -std=c++17 -lm -Iinclude
	main.exe