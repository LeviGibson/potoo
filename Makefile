all:
	emcc *.cpp \
	-O3 \
	-DWASM \
	-s MODULARIZE=1 \
	-s EXPORT_ES6=1 \
	-s ENVIRONMENT=web \
	-s EXPORTED_FUNCTIONS='["_start_solver","_init_all","_malloc","_free","_increase_depth"]' \
	-s EXPORTED_RUNTIME_METHODS=stringToUTF8,lengthBytesUTF8 \
	-s INITIAL_MEMORY=64MB \
	-o potoo.js

exe:
	g++ *.cpp -DEXE -fsanitize=address -Wframe-larger-than=4096 -O3 -Wall -o potoo
prof:
	g++ *.cpp -g -O0 -DEXE -Wall -o potoo
