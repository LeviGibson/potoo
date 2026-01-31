//
// Created by levigibson on 1/8/26.
//

#ifndef GCO_PRUNING_H
#define GCO_PRUNING_H

#include "cube.h"

//Make sure to change the array size when updating this
#define PRUNING_DEPTH 6

void init_pruning();
int is_close_to_solved(U64 h);

#endif //GCO_PRUNING_H
