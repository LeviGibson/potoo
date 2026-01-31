all:
	emcc *.cpp \
  -O3 \
  -s MODULARIZE=1 \
  -s EXPORT_ES6=1 \
  -s ENVIRONMENT=web \
  -s EXPORTED_FUNCTIONS='["_start_solver", "_init_all",'_malloc', '_free','_malloc','_increase_depth']' \
  -s MODULARIZE=1 -s EXPORT_ES6=1 \
  -sEXPORTED_RUNTIME_METHODS=stringToUTF8,lengthBytesUTF8 \
  -s INITIAL_MEMORY=64MB \
  -o potoo.js