//
// Created by levigibson on 1/8/26.
//

#include "search.h"
#ifdef WASM
#include <emscripten.h>
#endif

U64 SOLUTION_HASHES[1000000];
int algs_found = 0;
int mainNodes = 0;
int pruningDepth = 0;

Alg alg = Alg();

#ifdef WASM
EM_JS(void, update, (), {
    window.update();
});
#endif

int solve_condition(Cube* cube){
    if (gegMode){
        return cube->is_htr();
    } else {
        return cube->is_solved();
    }
}

int distance_from_condition(U64 h){
    if (gegMode){
        return distance_from_htr(h);
    } else {
        return distance_from_solved(h);
    }
}

int main_search(int depth, Cube* cube){
    mainNodes++;

    #ifdef WASM
    if (mainNodes % 100 == 0){
        update();
    }
    #endif

    assert(cube->ply < 60);

    if (solve_condition(cube)) {
        
        alg.from_cube(cube);

        if (!U64_scan(alg.hash(), SOLUTION_HASHES, algs_found)) {

            if (algs_found % 100 == 0) {
                printf("%d algs found\n", algs_found);
            }

            SOLUTION_HASHES[algs_found] = alg.hash();
            algs_found++;
            assert(algs_found < 1000000);

            alg.send();
        }
    }

    U64 h = cube->hash();
    int distanceFromSolved = distance_from_condition(h);

    if (depth <= pruningDepth && distanceFromSolved > depth){
        return 0;
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

void start_search(char* scramble, int algGenerating, int geg){
    Cube cube_copy = Cube();
    cube_copy.parse_alg(scramble);
    cube_copy.reset_history();
    algGeneratingMode = algGenerating;
    gegMode = geg;
    
    fingertricks_found = 0;
    algs_found = 0;
    depthSearched = 0;
    mainNodes = 0;

    if (geg){
        pruningDepth = HTR_PRUNING_DEPTH;
    } else  {
        pruningDepth = PRUNING_DEPTH;
    }

    // for (int depth = 0; depth < 4; depth++){
    //     printf("searching depth %d mainNodes %d\n", depth + PRUNING_DEPTH, mainNodes);
    //     int res = search(depth, 0, &cube_copy);
    //     depthSearched = depth;

    //     if (res)
    //         break;
    // }

    memcpy(&searchCube, &cube_copy, sizeof(Cube));
}

void step(){
    depthSearched++;
    printf("searching depth %d mainNodes %d fingertricks %d\n", depthSearched + PRUNING_DEPTH, mainNodes, fingertricks_found);
    int res = main_search(depthSearched, &searchCube);
}
