//
// Created by levigibson on 1/8/26.
//

#include "cube.h"
#include "search.h"
#include "pruning.h"
#include <ctime>

// extern "C"{

// void init_all(){
//     init_cube();
//     init_pruning();
//     init_search((char*)"./out.csv");
// }

// void init_solver(const char* scramble) {
//     // parse scramble, reset state
// }


// int solve_step() {
//     // run N nodes of search
//     // if a new solution is found:
//     //   store it in a buffer
//     //   return 1
//     // return 0 if nothing new yet
// }


// }

int main(int argc, char* argv[]){
    srand(time(NULL));
    init_cube();
    init_pruning();
    init_search((char*)"./out.csv");

    Cube cube = Cube();
    cube.parse_alg((char*)"R2 U2 R F' R' F U2 R2");
    find_algorithms(&cube);
    // Alg alg = Alg();
    // alg.from_cube(&cube);

    // for (int corner = 0; corner < 8; corner++){
    //     for (int orientation = 0; orientation < 3; orientation++){
    //         alg.rotate(corner, orientation);
    //         // exit(0);
    //     }
    // }

    // for (int corner = 0; corner < 8; corner++){
    //     for (int orientation = 0; orientation < 3; orientation++){
    //         memcpy(alg.moves, alg.all_angles[corner][orientation], sizeof(alg.moves));

    //         printf("Corner:%d Orientation: %d\n", corner, orientation);
    //         alg.print();
    //         printf("\n");
    //     }
    // }
    

    return 0;
}
