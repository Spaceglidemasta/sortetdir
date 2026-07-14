out := bin\sortetdir.exe

flags := -std=c++17 -lm -Iinclude -O3


win: src/main.cpp
	g++ src/main.cpp -o $(out) $(flags)
	$(out)

unix: src/main.cpp
	g++ src/main.cpp -o $(out) $(flags)
	$(out)

cwin: src/main.cpp
	g++ src/main.cpp -o $(out)	 $(flags)

cunix: src/main.cpp
	g++ src/main.cpp -o $(out) $(flags)