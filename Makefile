call:
	g++ cube.cpp main.cpp search.cpp pruning.cpp -O3 -o potoo
gdb:
	g++ cube.cpp main.cpp search.cpp pruning.cpp -O0 -fsanitize=address -fno-omit-frame-pointer -g
clean:
	rm a.out
	rm gco