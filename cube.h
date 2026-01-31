//
// Created by levigibson on 1/8/26.
//

#ifndef GCO_CUBE_H
#define GCO_CUBE_H

#include <cstdint>
#include <cstdio>
#include <cassert>
#include <cstdlib>
#include <cstring>

#define is_qt(move) move < R2 ? 1 : 0
#define U64 uint64_t

#define copy_cube() Cube tmp_cube = *cube
#define paste_cube() *cube = tmp_cube

//Moves
enum {
    R, U, F, RP, UP, FP, R2, U2, F2
};



//Corner indicies
//DBL is rarely used
enum {
    UBL, UBR, UFR, UFL, DFL, DFR, DBR, DBL
};


//Axis
enum {
    UD, RL, FB
};

//Orientation of corners values
// NT = No Twist (Oriented) = 0
// RT = Right Twist = 1
// LT = Left Twist = 2
enum {
    NT, RT, LT
};

void init_cube();
void print_u64(U64 x);
int U64_scan(U64 x, U64* arr, int length);

// Epic 2x2 cube class specialized for genning GCO algs
//(Lots of HTR functions)
class Cube {
public:
    //Corners! These are represented with binary
    //DBL is fixed, so there are only 7 pieces.
    // The only allowed moves are {R U F}
    uint8_t corners[7];

    Cube();

    //Set cube to solved state, reset history
    void reset();

    //Make Move
    //Don't touch the change_history variable plz and thank you
    void make_move(int move, int change_history = 1);

    //Pass normal cubing alg as string
    //Do all the moves on the cube
    void parse_alg(char* alg);

    //Is the cube solved? Idk this finds out
    int is_solved();

    //Is the cube in HTR? Idk this finds out
    int is_htr();

    int is_repetition(int move);

    //clear history and set random state
    void set_random_state();

    U64 hash();

    //Corner Binary tools
    static void print_corner_binary(uint8_t corner);
    static inline uint8_t encode_corner(uint8_t position, uint8_t orientation);
    static inline uint8_t corner_index(uint8_t corner);
    static inline uint8_t corner_orientation(uint8_t corner);
    void print_corners();

    //Finds index of corner
    int find_corner(int corner);

    //History functions
    void reset_history();
    void update_history(int move);

    //print all moves done to cube since last reset
    void print_stack();

    //Undo last move
    void pop();

    //last move done
    int prevmove();

    //history of hashes
    U64 history[512];
    //How many moves have been done since the last reset
    int ply;
    //All moves performed
    int stack[512];

private:
    //Internal Functions used by make_move()
    void four_cycle(uint8_t i0, uint8_t i1, uint8_t i2, uint8_t i3);
    void two_cycle(uint8_t i0, uint8_t i1);
    void right_twist(int index);
    void left_twist(int index);
    void twist_R();
    void twist_F();

};

extern Cube POSSIBLE_HTR_STATES[];

// enum {
//     UBL_U, UBL_L, UBL_B,
//     UBR_U, UBR_R, UBR_B,
//     UFR_U, UFR_R, UFR_F,
//     UFL_U, UFL_L, UFL_F,
//     DBL_D, DBL_L, DBL_B,
//     DBR_D, DBR_R, DBR_B,
//     DFR_D, DFR_R, DFR_F,
//     DFL_D, DFL_L, DFL_F,//4
// }

class Alg{
public:
    int moves[64];
    int all_angles[8][3][64];
    //Indicies:
    //8192 combos to be safe
    //64 moves
    //Fingertrick type, regrip, handpos
    int all_fingertrick_combinations[8192*2][64][3];
    int fingertrick_combinations_found;
    int fingertrick_stack[64][3];

    int length;
    Alg();
    void append(int move);
    void from_cube(Cube* cube);
    void print();
    void send();
    void rotate(int corner, int orientation);

    int score();
    U64 hash();

private:
    void gen(int depth, int handpos);
    int score_fingertricks(int index);
};


#endif //GCO_CUBE_H
