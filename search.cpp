//
// Created by levigibson on 1/8/26.
//

#include "search.h"
#ifdef WASM
#include <emscripten.h>
#endif

U64 EXTENDED_HASHES[1000000];
int num_states_extended = 0;
U64 SOLUTION_HASHES[1000000];
int algs_found = 0;
int nodes = 0;

Alg alg = Alg();

#ifdef WASM
EM_JS(void, update, (), {
    window.update();
});
#endif

int extend_search(int depth, Cube* cube){
        nodes++;
        int extended = 1;

    #ifdef WASM
    if (nodes % 100 == 0){
        update();
    }
    #endif

    assert(cube->ply < 60);

    U64 h = cube->hash();
    int distanceFromSolved = is_close_to_solved(h);

    if (!distanceFromSolved){
        return 0;
    }

    if (cube->is_solved()) {
        
        alg.from_cube(cube);

        if (!U64_scan(alg.hash(), SOLUTION_HASHES, algs_found)) {

            // if (algs_found % 1000 == 0) {
            //     printf("%d algs found\n", algs_found);
            // }

            SOLUTION_HASHES[algs_found] = alg.hash();
            algs_found++;
            assert(algs_found < 1000000);

            alg.send();
        }
    }

    if (depth <= 0)
        return 0;

    for (int move = 0; move <= F2; move++){
        //R is the value that gets subtracted from depth with iterating
        //It will almost always be 1 but sometimes it gets changed
        int RS = 1;

        if (!cube->is_repetition(move)) {

            cube->make_move(move);

            if (is_close_to_solved(cube->hash()) >= distanceFromSolved){
                cube->pop();
                continue;
            }

            int res = extend_search(depth - RS, cube);
            cube->pop();

            if (res)
                return 1;
        }
    }

    return 0;
}

int main_search(int depth, Cube* cube){
    nodes++;

    #ifdef WASM
    if (nodes % 100 == 0){
        update();
    }
    #endif

    assert(cube->ply < 60);

    U64 h = cube->hash();
    int distanceFromSolved = is_close_to_solved(h);

    if (distanceFromSolved){
        if (!U64_scan(h, EXTENDED_HASHES, num_states_extended)) {
            EXTENDED_HASHES[num_states_extended] = h;
            num_states_extended++;
            assert(num_states_extended < 1000000);

            return extend_search(PRUNING_DEPTH, cube);
        }
    }

    if (depth <= 0)
        return 0;

    for (int move = 0; move <= F2; move++){
        //R is the value that gets subtracted from depth with iterating
        //It will almost always be 1 but sometimes it gets changed
        int RS = 1;
        if (move == R2 || move == U2 || move == F2)
            RS = 2;

        if (!cube->is_repetition(move)) {

            //Don't count U moves towards the move total for the first move
            //Because it's a pre-auf
            if (algGeneratingMode && cube->ply == 0 && (move == U || move == UP || move == U2))
                RS = 0;
        
            cube->make_move(move);
            int res = main_search(depth - RS, cube);
            cube->pop();

            if (res)
                return 1;

            
        }
    }

    return 0;
}

int depthSearched;

Cube searchCube;

void start_search(char* scramble, int algGenerating){
    Cube cube_copy = Cube();
    cube_copy.parse_alg(scramble);
    cube_copy.reset_history();
    num_states_extended = 0;
    algGeneratingMode = algGenerating;
    memset(EXTENDED_HASHES, 0, sizeof(EXTENDED_HASHES));

    algs_found = 0;
    depthSearched = 0;

    // for (int depth = 0; depth < 4; depth++){
    //     printf("searching depth %d nodes %d\n", depth + PRUNING_DEPTH, nodes);
    //     int res = search(depth, 0, &cube_copy);
    //     depthSearched = depth;

    //     if (res)
    //         break;
    // }

    memcpy(&searchCube, &cube_copy, sizeof(Cube));
}

void step(){
    depthSearched++;
    printf("searching depth %d nodes %d\n", depthSearched + PRUNING_DEPTH, nodes);
    int res = main_search(depthSearched, &searchCube);
}
