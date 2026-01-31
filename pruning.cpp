//
// Created by levigibson on 1/8/26.
//

#include "pruning.h"


U64 CLOSE_SOLVE_KEYS[62360];
int close_solved_keys_found = 0;

int is_close_to_solved(U64 h){
    int left = 0;
    int right = close_solved_keys_found - 1;

    while (left <= right) {
        int mid = left + ((right - left) >> 1);
        U64 val = CLOSE_SOLVE_KEYS[mid];

        if (val == h) {
            return 1;
        } else if (val < h) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }

    return 0;

//    for (int i = 0; i < close_solved_keys_found; i++){
//        if (h == CLOSE_SOLVE_KEYS[i]){
//            return 1;
//        }
//    }
//    return 0;
}

void close_solve_search(int depth, Cube* cube){
    U64 h = cube->hash();
    int found_key = 0;
    for (int i = 0; i < close_solved_keys_found; i++){
        if (h == CLOSE_SOLVE_KEYS[i])
            found_key = 1;
    }

    if (!found_key){
        CLOSE_SOLVE_KEYS[close_solved_keys_found] = h;
        close_solved_keys_found++;
    }

    if (depth == 0)
        return;

    for (int move = 0; move <= F2; move++){
        if (cube->is_repetition(move))
            continue;
        cube->make_move(move);
        close_solve_search(depth - 1, cube);
        cube->pop();
    }
}

static int cmp_u64(const void *a, const void *b) {
    U64 x = *(const U64 *)a;
    U64 y = *(const U64 *)b;
    return (x > y) - (x < y);  // avoids overflow
}

void sort_close_solve_keys() {
    qsort(CLOSE_SOLVE_KEYS, close_solved_keys_found, sizeof(U64), cmp_u64);
}

void init_pruning(){
    Cube cube = Cube();
    close_solve_search(PRUNING_DEPTH, &cube);
    sort_close_solve_keys();
    // printf("Found %d hashable solutions\n", close_solved_keys_found);
//    printf("Found %d hashable HTRs\n", close_htr_keys_found);
}
