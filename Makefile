out := bin\sortetdir.exe

jsonpath := ^%appdata^%/sortetdir/config.json

flags := -std=c++17 -lm -Iinclude -O3


win: src/main.cpp
	g++ src/main.cpp -o $(out) $(flags) -D_JSONPATH=$(jsonpath)
	$(out)

winper: src/main.cpp
	g++ src/main.cpp -o $(out) $(flags) -D_PERSONAL_MODE -D_JSONPATH=$(jsonpath)
	$(out)

cwinper: src/main.cpp
	g++ src/main.cpp -o $(out) $(flags) -D_PERSONAL_MODE -D_JSONPATH=$(jsonpath)

unix: src/main.cpp
	g++ src/main.cpp -o $(out) $(flags) -D_JSONPATH=$(jsonpath)
	./$(out)

cwin: src/main.cpp
	g++ src/main.cpp -o $(out)	 $(flags) -D_JSONPATH=$(jsonpath)

cunix: src/main.cpp
	g++ src/main.cpp -o $(out) $(flags) -D_JSONPATH=$(jsonpath)