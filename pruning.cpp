//
// Created by levigibson on 1/8/26.
//

#include "pruning.h"
#include <iostream>

typedef struct {
    U64 key;
    int length;
} Key;

Key CLOSE_SOLVE_KEYS[62360];
int close_solved_keys_found = 0;

int is_close_to_solved(U64 h){
    int left = 0;
    int right = close_solved_keys_found - 1;

    while (left <= right) {
        int mid = left + ((right - left) >> 1);
        U64 val = CLOSE_SOLVE_KEYS[mid].key;

        if (val == h) {
            return CLOSE_SOLVE_KEYS[mid].length;
        } else if (val < h) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }

    return 0;
}

int is_key_already_found(U64 h, int ply){
    for (int i = 0; i < close_solved_keys_found; i++){
        if (h == CLOSE_SOLVE_KEYS[i].key){
            CLOSE_SOLVE_KEYS[i].length = std::min(ply, CLOSE_SOLVE_KEYS[i].length);
            return 1;
        }
    }
    return 0;
}

void close_solve_search(int depth, int ply, Cube* cube){
    U64 h = cube->hash();
    
    int found_key = is_key_already_found(h, ply);

    if (!found_key){
        CLOSE_SOLVE_KEYS[close_solved_keys_found].key = h;
        CLOSE_SOLVE_KEYS[close_solved_keys_found].length = ply;
        close_solved_keys_found++;
    }

    if (depth == 0)
        return;

    for (int move = 0; move <= F2; move++){
        if (cube->is_repetition(move))
            continue;
        cube->make_move(move);
        close_solve_search(depth - 1, ply+1, cube);
        cube->pop();
    }
}

static int cmp_key(const void *a, const void *b) {
    U64 x = ((const Key *)a)->key;
    U64 y = ((const Key *)b)->key;
    return (x > y) - (x < y);  // avoids overflow
}

void sort_close_solve_keys() {
    qsort(CLOSE_SOLVE_KEYS, close_solved_keys_found, sizeof(Key), cmp_key);
}

void init_pruning(){
    //62360
    Cube cube = Cube();
    for (int depth = 0; depth <= PRUNING_DEPTH; depth++){
        close_solve_search(PRUNING_DEPTH, 1, &cube);
    }
    sort_close_solve_keys();
    printf("Found %d hashable solutions\n", close_solved_keys_found);
//    printf("Found %d hashable HTRs\n", close_htr_keys_found);
}
