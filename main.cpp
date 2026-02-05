//
// Created by levigibson on 1/8/26.
//

#include "cube.h"
#include "search.h"
#include "pruning.h"
#include <ctime>

#ifdef WASM
#include <emscripten.h>
#endif

extern "C"{

void init_all(){
    init_cube();
    init_pruning();
    
}

void start_solver(const char* scramble, bool algGenerating) {
    printf("Scramble Recieved: %s\n", scramble);
    printf("%d\n", algGenerating);
    start_search((char*)scramble, algGenerating);
}

void increase_depth(){
    step();
}

}

#ifdef EXE

int main(){
    srand(time(NULL));
    init_cube();
    init_pruning();

    // time_t seconds;
    // seconds = time(NULL);

    // start_solver((char*)"R2 U2 R F' R' F U2 R2", false);
    // for (int i = 0; i < 14-PRUNING_DEPTH; i++){
    //     increase_depth();
    //     printf("%d\n", i);
    // }


    // //77
    // printf("Time: %ld seconds\n", time(NULL)-seconds);

    Cube cube = Cube();
    cube.parse_alg((char*)"R' F R' F' R F' R' F' R U' F R");
    Alg* alg = (Alg*)malloc(sizeof(Alg));
    alg->from_cube(&cube);
    int score = alg->score();
    printf("%d\n", score);
    free(alg);
    
    
    // find_algorithms(&cube);
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

#endif
