//
// Created by levigibson on 1/8/26.
//

#include "search.h"

U64 EXTENDED_HASHES[1000000];
int num_states_extended = 0;
U64 SOLUTION_HASHES[1000000];
int algs_found = 0;
int nodes = 0;

FILE* outfile;

void init_search(char* fn){
    outfile = fopen(fn, "w");
}

Alg alg = Alg();

int search(int depth, int extended, Cube* cube){
    nodes++;

    assert(cube->ply < 60);

    U64 h = cube->hash();

    if (extended && cube->is_solved()) {
        
        alg.from_cube(cube);

        if (!U64_scan(alg.hash(), SOLUTION_HASHES, algs_found)) {

            if (algs_found % 1000 == 0) {
                printf("%d algs found\n", algs_found);
            }

            SOLUTION_HASHES[algs_found] = alg.hash();
            algs_found++;
            assert(algs_found < 1000000);

            alg.send(outfile);
        }
    }

    if (!extended && is_close_to_solved(cube->hash())){
        if (!U64_scan(h, EXTENDED_HASHES, num_states_extended)) {
            extended = 1;
            depth += PRUNING_DEPTH;
            EXTENDED_HASHES[num_states_extended] = h;
            num_states_extended++;
            assert(num_states_extended < 1000000);
        }
    }

    if (depth <= 0)
        return 0;

    for (int move = 0; move <= F2; move++){
        //R is the value that gets subtracted from depth with iterating
        //It will almost always be 1 but sometimes it gets changed
        int RS = 1;
        if (!extended && (move == R2 || move == U2 || move == F2))
            RS = 2;

        if (!cube->is_repetition(move)) {

            //Don't count U moves towards the move total for the first move
            //Because it's a pre-auf
            // if (cube->ply == 0 && (move == U || move == UP || move == U2))
            //     RS = 0;

            cube->make_move(move);
            int res = search(depth - RS, extended, cube);
            cube->pop();

            if (res)
                return 1;
        }
    }

    return 0;
}

void find_algorithms(Cube* cube){
    Cube cube_copy = *cube;
    cube_copy.reset_history();
    num_states_extended = 0;
    memset(EXTENDED_HASHES, 0, sizeof(EXTENDED_HASHES));

    algs_found = 0;

    for (int depth = 0; depth < 11; depth++){
        printf("searching depth %d nodes %d\n", depth + PRUNING_DEPTH, nodes);
        int res = search(depth, 0, &cube_copy);
    }
}
