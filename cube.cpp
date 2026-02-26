//
// Created by levigibson on 1/8/26.
//

#include "cube.h"
#include <iostream>
#ifdef WASM
#include <emscripten/emscripten.h>
#endif

int algGeneratingMode = 0;
int gegMode = 0;
int fingertricks_found;

int qt_cycles[6][4] = {
        {UFR, UBR, DBR, DFR},
        {UBR, UFR, UFL, UBL},
        {UFL, UFR, DFR, DFL},
        {DFR, DBR, UBR, UFR},
        {UBL, UFL, UFR, UBR},
        {DFL, DFR, UFR, UFL}
};

U64 keys[7][7][3];
U64 alg_keys[64][9];

U64 random_U64(){
    return ((uint64_t)rand() << 33) ^
           ((uint64_t)rand() << 2)  ^
           (uint64_t)rand();
}

void print_u64(U64 x){
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            int bit = 63 - (row * 8 + col);
            putchar((x >> bit) & 1 ? '1' : '0');
            putchar(' ');
            putchar(' ');
        }
        putchar('\n');
    }
    printf("\n");
}

int htr_keys_found = 0;
U64 HTR_KEYS[24];

Cube POSSIBLE_HTR_STATES[24];

void htr_search(int depth, Cube *cube){
    U64 h = cube->hash();
    int found_key = 0;
    for (int i = 0; i < htr_keys_found; i++){
        if (h == HTR_KEYS[i])
            found_key = 1;
    }

    if (!found_key){
        HTR_KEYS[htr_keys_found] = h;
        memcpy(&POSSIBLE_HTR_STATES[htr_keys_found], cube, sizeof(Cube));
        htr_keys_found++;
    }

    if (depth == 0)
        return;

    for (int move = R2; move <= F2; move++){
        cube->make_move(move);
        htr_search(depth-1, cube);
        cube->pop();
    }
}

void init_htr(){
    memset(HTR_KEYS, 0, sizeof(HTR_KEYS));
    Cube cube = Cube();
    htr_search(4, &cube);
}

void init_hash(){

    //Regular hash keys
    for (int i = 0; i < 7; i++){
        for (int j = 0; j < 7; j++){
            for (int k = 0; k < 3; k++){
                keys[i][j][k] = random_U64();
            }
        }
    }

    //Hash keys for Alg()
    for (int i = 0; i < 64; i++){
        for (int j = 0; j < 9; j++){
            alg_keys[i][j] = random_U64();
        }
    }
}

//be careful of the order
//some depend on others
void init_cube(){
    init_hash();
    init_htr();
}

Cube::Cube() {
    reset();
}

void Cube::reset() {
    for (int i = 0; i < 7; i++){
        corners[i] = encode_corner(i, NT);
    }
    reset_history();
}

void Cube::print_corner_binary(uint8_t corner) {
    for (int i = 7; i >= 0; --i) {
        printf("%d", (corner >> i) & 1);
    }
    printf("\n");
}

uint8_t Cube::encode_corner(uint8_t position, uint8_t orientation) {
    assert(position < 7);
    assert(orientation < 3);
    return position | (orientation << 3);
}

uint8_t Cube::corner_index(uint8_t corner) {
    return corner & (uint8_t)7;
}

uint8_t Cube::corner_orientation(uint8_t corner) {
    return (corner >> 3) & (uint8_t)3;
}

int Cube::find_corner(int corner){
    for (int i = 0; i < 7; i++){
        if (corner_index(corners[i]) == corner)
            return i;
    }
    assert(0);
}

void Cube::make_move(int move, int change_history) {
    assert(move <= F2);
    if (is_qt(move)){
        four_cycle(qt_cycles[move][0], qt_cycles[move][1], qt_cycles[move][2], qt_cycles[move][3]);
        if (move == F || move == FP){
            twist_F();
        } else if (move == R || move == RP){
            twist_R();
        }
    } else {
        //TODO make this not bad
        if (move == R2){
            make_move(R, 0);
            make_move(R, 0);
        }
        if (move == F2){
            make_move(F, 0);
            make_move(F, 0);
        }
        if (move == U2){
            make_move(U, 0);
            make_move(U, 0);
        }
    }
    if (change_history)
        update_history(move);
}

void Cube::four_cycle(uint8_t i0, uint8_t i1, uint8_t i2, uint8_t i3) {
    uint8_t tmp = corners[i3];
    corners[i3] = corners[i2];
    corners[i2] = corners[i1];
    corners[i1] = corners[i0];
    corners[i0] = tmp;
}

void Cube::print_corners() {
    for (unsigned char corner : corners){
        printf("%d : %d\n", corner_index(corner), corner_orientation(corner));
    }
    printf("\n");
}

void Cube::twist_R() {
    left_twist(UFR);
    right_twist(UBR);
    left_twist(DBR);
    right_twist(DFR);

}

void Cube::twist_F() {
    right_twist(UFR);
    left_twist(DFR);
    right_twist(DFL);
    left_twist(UFL);
}

int right_twist_table[3] = {1, 2, 0};
int left_twist_table[3] = {2, 0, 1};

void Cube::right_twist(int index) {
    corners[index] = encode_corner(corner_index(corners[index]), right_twist_table[corner_orientation(corners[index])]);
}

void Cube::left_twist(int index) {
    corners[index] = encode_corner(corner_index(corners[index]), left_twist_table[corner_orientation(corners[index])]);
}

void Cube::parse_alg(char *alg) {
    while (*alg) {
        int move = -1;

        switch (*alg) {
            case 'R':
                if (alg[1] == '\'') { move = RP; alg += 2; }
                else if (alg[1] == '2') { move = R2; alg += 2; }
                else { move = R; alg += 1; }
                break;

            case 'U':
                if (alg[1] == '\'') { move = UP; alg += 2; }
                else if (alg[1] == '2') { move = U2; alg += 2; }
                else { move = U; alg += 1; }
                break;

            case 'F':
                if (alg[1] == '\'') { move = FP; alg += 2; }
                else if (alg[1] == '2') { move = F2; alg += 2; }
                else { move = F; alg += 1; }
                break;

            case ' ':
                alg++;
                continue;
        }

        if (move != -1)
            make_move(move);
    }
}

int Cube::is_solved() {
    for (int i = 0; i < 7; i++){
        if (corners[i] != i)
            return 0;
    }
    return 1;
}

void Cube::set_random_state() {
    for(int i = 0; i < 7; i++){
        two_cycle(i, rand() % 7);
    }
    int parity = 0;
    for(int i = 0; i < 6; i++){
        int orientation = rand() % 3;
        corners[i] = encode_corner(corner_index(corners[i]), orientation);
        parity += orientation;
    }
    corners[6] = encode_corner(corner_index(corners[6]), parity%3);
}

void Cube::two_cycle(uint8_t i0, uint8_t i1) {
    uint8_t tmp = corners[i1];
    corners[i1] = corners[i0];
    corners[i0] = tmp;
}

U64 Cube::hash() {
    U64 h = 0;
    for (int i = 0; i < 7; i++){
        h ^= keys[i][corner_index(corners[i])][corner_orientation(corners[i])];
    }
    return h;
}

void Cube::reset_history() {
    memset(history, 0, sizeof(history));
    memset(stack, 0, sizeof(stack));
    ply = 0;
}

void Cube::update_history(int move) {
    history[ply] = hash();
    stack[ply] = move;
    ply++;
}

//    R, U, F, RP, UP, FP, R2, U2, F2

int CANCELLING_MOVES[9][3] = {
        {R, RP, R2},
        {U, UP, U2},
        {F, FP, F2},
        {R, RP, R2},
        {U, UP, U2},
        {F, FP, F2},
        {R, RP, R2},
        {U, UP, U2},
        {F, FP, F2}
};

//TODO improve this function to filter more
int Cube::is_repetition(int move) {
    if (ply == 0)
        return 0;

    //Check for direct cancellation (e.g R R)
    for (int i = 0; i < 3; i++){
        if (move == CANCELLING_MOVES[stack[ply-1]][i])
            return 1;
    }

    return 0;
}

char* MOVE_TO_STR[9] = {
        (char*)"R",
        (char*)"U",
        (char*)"F",
        (char*)"R'",
        (char*)"U'",
        (char*)"F'",
        (char*)"R2",
        (char*)"U2",
        (char*)"F2",
};

void Cube::print_stack() {
    for (int i = 0; i < ply; i++){
        printf("%s ", MOVE_TO_STR[stack[i]]);
    }
    printf("\n");
}
//R, U, F, RP, UP, FP, R2, U2, F2
int INVERSE_MOVES[9] = {RP, UP, FP, R, U, F, R2, U2, F2};

void Cube::pop() {
    assert(ply > 0);
    make_move(INVERSE_MOVES[stack[ply-1]]);

    stack[ply-1] = 0;
    history[ply-1] = 0;
    ply--;

    stack[ply-1] = 0;
    history[ply-1] = 0;
    ply--;

}

int Cube::is_htr() {
    U64 h = hash();
    for (int i = 0; i < htr_keys_found; i++){
        if (h == HTR_KEYS[i])
            return 1;
    }
    return 0;
}

int Cube::prevmove() {
    assert(ply > 0);
    return stack[ply-1];
}

int U64_scan(U64 x, U64* arr, int length){
    for (int i = 0; i < length; i++){
        if (arr[i] == x)
            return 1;
    }
    return 0;
}

Alg::Alg() {
    memset(moves, 0, sizeof(moves));
    length = 0;
}

void Alg::append(int move) {
    moves[length] = move;
    length++;
    assert(length < 64);
}

void Alg::from_cube(Cube *cube) {
    memcpy(moves, cube->stack, sizeof(moves));
    length = cube->ply;
}

void Alg::print() {
    for (int i = 0; i < length; i++){
        printf("%s ", MOVE_TO_STR[moves[i]]);
    }
    printf("\n");
}

#ifdef WASM
EM_JS(void, add_solution, (const char* sol_ptr), {
    window.addSolution(UTF8ToString(sol_ptr));
});
#endif

std::string ROTATION_NAMES[8][3] = {
    {"(x2 y) ", "(z') ", "(x) "},
    {"(x2 y2) ", "(x y') ", "(z y') "},
    {"(x2 y') ", "(z y2) ", "(x' y2) "},
    {"(x2) ", "(x' y) ", "(z' y) "},
    {"(y) ", "(z' y2) ", "(x') "},
    {"(y2) ", "(x' y') ", "(z y) "},
    {"(y') ", "(z) ", "(x y2) "},
    {"() ", "(x y) ", "(z' y') "}
};

void Alg::send() {
    for (int corner = 0; corner < 8; corner++){
        for (int orientation = 0; orientation < 3; orientation++){
            rotate(corner, orientation);
        }
    }

    for (int corner = 0; corner < 8; corner++){
        for (int orientation = 0; orientation < 3; orientation++){
            memcpy(moves, all_angles[corner][orientation], sizeof(moves));
            std::string output = std::to_string(score()) + ",";
            output += ROTATION_NAMES[corner][orientation];
            for (int i = 0; i < length; i++){
                output += MOVE_TO_STR[moves[i]];
                output += (std::string)" ";
            }

            #ifdef WASM
            add_solution(output.c_str());
            #endif
        }
    }

    
}

U64 Alg::hash() {
    U64 h = 0;
    for (int i = 0; i < length; i++){
        h ^= alg_keys[i][moves[i]];
    }
    return h;
}

//R, U, F, RP, UP, FP, R2, U2, F2

enum{R_WRIST, U_FLICK, U_PINCH, U_PUSH, F_FLICK, F_PINCH, RP_WRIST, UP_FLICK, FP_PINCH, FP_THUMB, R2_UP, R2_DOWN, U2_LEFT, U2_RIGHT, F2_DOUBLE, FP_FLICK};
enum{REGRIP_NONE, REGRIP_UP, REGRIP_DOWN, REGRIP_DOUBLE_UP, REGRIP_DOUBLE_DOWN};

std::string FINGERTRICK_NAMES[] = {"R_WRIST", "U_FLICK", "U_PINCH", "U_PUSH", "F_FLICK", "F_PINCH", "RP_WRIST", "UP_FLICK", "FP_PINCH", "FP_THUMB", "R2_UP", "R2_DOWN", "U2_LEFT", "U2_RIGHT", "F2_DOUBLE", "FP_FLICK"};
std::string REGRIP_NAMES[] = {"REGRIP_NONE", "REGRIP_UP", "REGRIP_DOWN", "REGRIP_DOUBLE_UP", "REGRIP_DOUBLE_DOWN"};

void Alg::gen(int depth, int handpos) {

    if (depth == length){
        // memcpy(all_fingertrick_combinations[fingertrick_combinations_found], fingertrick_stack, sizeof(fingertrick_stack));
        int s = score_fingertricks();
        fingertricks_found++;
        if (s < minScore){
            minScore = s;
        }
        // assert(fingertrick_combinations_found < 8192*2);
        return;
    }

    int move = moves[depth];
    assert(handpos == -1 || handpos == 0 || handpos == 1);

    fingertrick_stack[depth][2] = handpos;

    if (move == R){
        if (handpos == 0 || handpos == -1){
            fingertrick_stack[depth][0] = R_WRIST;
            fingertrick_stack[depth][1] = REGRIP_NONE;
            gen(depth+1, handpos + 1);
        }
        else if (handpos == 1){
            return;
        }
    } else if (move == RP){
        if (handpos == 0 || handpos == 1){
            fingertrick_stack[depth][0] = RP_WRIST;
            fingertrick_stack[depth][1] = REGRIP_NONE;
            gen(depth+1, handpos - 1);
        }
        else if (handpos == -1){
            return;
        }

    } else if (move == R2){
        if (handpos == -1){
            fingertrick_stack[depth][0] = R2_UP;
            fingertrick_stack[depth][1] = REGRIP_NONE;
            gen(depth+1, handpos * -1);
        } else if (handpos == 1) {
            fingertrick_stack[depth][0] = R2_DOWN;
            fingertrick_stack[depth][1] = REGRIP_NONE;
            gen(depth + 1, handpos * -1);
        } else if (handpos == 0){
            return;
        }
    } else if (move == U){
        if (handpos == 0) {
            fingertrick_stack[depth][0] = U_FLICK;
            fingertrick_stack[depth][1] = REGRIP_NONE;
            gen(depth+1, handpos);
        } else if (handpos == 1) {
            fingertrick_stack[depth][0] = U_PINCH;
            fingertrick_stack[depth][1] = REGRIP_NONE;
            gen(depth+1, handpos);
        }
        else if (handpos == -1) {
            fingertrick_stack[depth][0] = U_PUSH;
            fingertrick_stack[depth][1] = REGRIP_NONE;
            gen(depth+1, handpos);
        }
    } else if (move == UP){
        fingertrick_stack[depth][0] = UP_FLICK;
        fingertrick_stack[depth][1] = REGRIP_NONE;
        gen(depth+1, handpos);

        if (handpos < 1) {
            fingertrick_stack[depth][0] = UP_FLICK;
            fingertrick_stack[depth][1] = REGRIP_UP;
            gen(depth + 1, handpos+1);
        }

        if (handpos > -1) {
            fingertrick_stack[depth][0] = UP_FLICK;
            fingertrick_stack[depth][1] = REGRIP_DOWN;
            gen(depth + 1, handpos-1);
        }

        if (handpos == 1) {
            fingertrick_stack[depth][0] = UP_FLICK;
            fingertrick_stack[depth][1] = REGRIP_DOUBLE_DOWN;
            gen(depth + 1, handpos-2);
        }

        if (handpos == -1) {
            fingertrick_stack[depth][0] = UP_FLICK;
            fingertrick_stack[depth][1] = REGRIP_DOUBLE_UP;
            gen(depth + 1, handpos+2);
        }
    } else if (move == U2){
        fingertrick_stack[depth][0] = U2_LEFT;
        fingertrick_stack[depth][1] = REGRIP_NONE;
        gen(depth+1, handpos);

        if (handpos == 0){
            fingertrick_stack[depth][0] = U2_RIGHT;
            fingertrick_stack[depth][1] = REGRIP_NONE;
            gen(depth+1, handpos);
        }

        if (handpos < 1) {
            fingertrick_stack[depth][0] = U2_LEFT;
            fingertrick_stack[depth][1] = REGRIP_UP;
            gen(depth + 1, handpos+1);
        }

        if (handpos > -1) {
            fingertrick_stack[depth][0] = U2_LEFT;
            fingertrick_stack[depth][1] = REGRIP_DOWN;
            gen(depth + 1, handpos-1);
        }

        if (handpos == 1) {
            fingertrick_stack[depth][0] = U2_LEFT;
            fingertrick_stack[depth][1] = REGRIP_DOUBLE_DOWN;
            gen(depth + 1, handpos-2);
        }

        if (handpos == -1) {
            fingertrick_stack[depth][0] = U2_LEFT;
            fingertrick_stack[depth][1] = REGRIP_DOUBLE_UP;
            gen(depth + 1, handpos+2);
        }
    } else if (move == F) {
        if (handpos == -1) {
            fingertrick_stack[depth][0] = F_FLICK;
            fingertrick_stack[depth][1] = REGRIP_NONE;
            gen(depth + 1, handpos);
        } else if (handpos == 0){
            fingertrick_stack[depth][0] = F_PINCH;
            fingertrick_stack[depth][1] = REGRIP_NONE;
            gen(depth + 1, handpos);
        } else {
            return;
        }
    } else if (move == FP) {
        if (handpos == -1){
            fingertrick_stack[depth][0] = FP_FLICK;
            fingertrick_stack[depth][1] = REGRIP_NONE;
            gen(depth + 1, handpos);
        }
        
        if (handpos == 0){
            // if (moves[depth+1] != U){
                // fingertrick_stack[depth][0] = FP_THUMB;
                // fingertrick_stack[depth][1] = REGRIP_NONE;
                // gen(depth + 1, handpos);
            // }

            fingertrick_stack[depth][0] = FP_PINCH;
            fingertrick_stack[depth][1] = REGRIP_NONE;
            gen(depth + 1, handpos);
        }
    } else if (move == F2) {
        if (handpos == -1){
            fingertrick_stack[depth][0] = F2_DOUBLE;
            fingertrick_stack[depth][1] = REGRIP_NONE;
            gen(depth + 1, handpos);
        } else {
            return;
        }
    }
}

int FINGERTRICK_SCORES[16] = {
46, //        R_WRIST,
40, //        U_FLICK,
56, //        U_PINCH,
60, //        U_PUSH, Manual 50->
50, //        F_FLICK,
60, //        F_PINCH,
46, //        RP_WRIST,
45, //        UP_FLICK,
60, //        FP_PINCH, This one is manual
75, //        FP_THUMB,
70, //        R2_UP,
70, //        R2_DOWN,
75, //        U2_LEFT,
70, //        U2_RIGHT,
80,//        F2_DOUBLE
45 // FP_FLICK
};

enum {LEFT_POINTER, LEFT_MIDDLE, RIGHT_POINTER, RIGHT_MIDDLE, DOUBLE_LEFT_FINGER, DOUBLE_RIGHT_FINGER, NO_FINGER};

int FINGER_NEEDED[16] = {
        NO_FINGER, //        R_WRIST,
        RIGHT_POINTER, //        U_FLICK,
        RIGHT_POINTER, //        U_PINCH,
        LEFT_POINTER, //        U_PUSH,
        RIGHT_POINTER, //        F_FLICK,
        RIGHT_POINTER, //        F_PINCH,
        NO_FINGER, //        RP_WRIST,
        LEFT_POINTER, //        UP_FLICK,
        LEFT_POINTER, //        FP_PINCH,
        NO_FINGER, //        FP_THUMB,
        NO_FINGER, //        R2_UP,
        NO_FINGER, //        R2_DOWN,
        DOUBLE_LEFT_FINGER, //        U2_LEFT,
        DOUBLE_RIGHT_FINGER, //        U2_RIGHT,
        DOUBLE_RIGHT_FINGER,//        F2_DOUBLE
        LEFT_POINTER // FP_FLICK
};

//How long does a regrip take? This can sometimes
//be mitigated by doing the regrip in parallel with moves
//like U' or U2'
int REGRIP_SCORES[] = {
0,//        REGRIP_NONE,
110,//        REGRIP_UP,
110,//        REGRIP_DOWN,
220,//        REGRIP_DOUBLE_UP,
220//        REGRIP_DOUBLE_DOWN
};

//For some fingertricks like R U R' U
//you can avoid a finger reset on the second U
//by doing the move with your middle finger.
int can_use_middle_finger(int ft, int handpos){
//            70, //        U_FLICK,
//            100, //        U_PINCH,
//            70, //        F_FLICK,
//            130, //        F_PINCH,
//            70, //        UP_FLICK,
//            140, //        FP_PINCH,
//            160, //        U2_LEFT,
//            160, //        U2_RIGHT,
//            160//        F2_DOUBLE
    if (ft == U_FLICK){
        if (handpos == 0)
            return 1;
        return 0;
    }
    if (ft == U_PINCH){
        return 0;
    }
    if (ft == F_FLICK){
        assert(handpos == -1);
        return 1;
    }
    if (ft == F_PINCH){
        return 0;
    }
    if (ft == UP_FLICK){
        return 1;
    }
    if (ft == FP_PINCH){
        return 0;
    }
    if (ft == U2_LEFT){
        return 0;
    }
    if (ft == U2_RIGHT){
        return 0;
    }
    if (ft == F2_DOUBLE){
        return 0;
    } if (ft == FP_FLICK){
        return 0;
    }
    if (ft == U_PUSH){
        return 0;
    }

    assert(0);
}

int Alg::score_fingertricks() {
    int score = 0;
    int finger_usage[4] = {0, 0, 0, 0};

    int algStart = 0;
    int algEnd = length;
    if (algGeneratingMode && (moves[0] == U || moves[0] == UP || moves[0] == U2)){
        algStart = 1;
    }
    if (algGeneratingMode && (moves[length-1] == U || moves[length-1] == UP || moves[length-1] == U2)){
        algEnd--;
    }

    for (int i = algStart; i < algEnd; i++){
        int fingers_used_on_fingertrick[4] = {0, 0, 0, 0};
        
        int ft = fingertrick_stack[i][0];
        int movescore = 0;

        movescore += FINGERTRICK_SCORES[ft];
        movescore += REGRIP_SCORES[fingertrick_stack[i][1]];

        if (FINGER_NEEDED[ft] == LEFT_POINTER){
            if (finger_usage[LEFT_POINTER] > 0 && can_use_middle_finger(ft, fingertrick_stack[i][2])){
                movescore += finger_usage[LEFT_MIDDLE];
                finger_usage[LEFT_MIDDLE] = 90;
                finger_usage[LEFT_POINTER] = 90;
                fingers_used_on_fingertrick[LEFT_MIDDLE] = 1;
                fingers_used_on_fingertrick[LEFT_POINTER] = 1;
            } else {
                movescore += finger_usage[LEFT_POINTER];
                finger_usage[LEFT_POINTER] = 90;
                fingers_used_on_fingertrick[LEFT_POINTER] = 1;
            }
        } else if (FINGER_NEEDED[ft] == RIGHT_POINTER){
            if (finger_usage[RIGHT_POINTER] > 0 && can_use_middle_finger(ft, fingertrick_stack[i][2])) {
                movescore += finger_usage[RIGHT_MIDDLE];
                finger_usage[RIGHT_MIDDLE] = 90;
                finger_usage[RIGHT_POINTER] = 90;
                fingers_used_on_fingertrick[RIGHT_MIDDLE] = 1;
                fingers_used_on_fingertrick[RIGHT_POINTER] = 1;
            } else {
                movescore += finger_usage[RIGHT_POINTER];
                finger_usage[RIGHT_POINTER] = 90;
                fingers_used_on_fingertrick[RIGHT_POINTER] = 1;
            }
        } else if (FINGER_NEEDED[ft] == DOUBLE_LEFT_FINGER){
            movescore += finger_usage[LEFT_POINTER];
            finger_usage[LEFT_POINTER] = 90;
            finger_usage[LEFT_MIDDLE] = 90;
            fingers_used_on_fingertrick[LEFT_POINTER] = 1;
            fingers_used_on_fingertrick[LEFT_MIDDLE] = 1;
        } else if (FINGER_NEEDED[ft] == DOUBLE_RIGHT_FINGER){
            movescore += finger_usage[RIGHT_POINTER];
            finger_usage[RIGHT_POINTER] = 90;
            finger_usage[RIGHT_MIDDLE] = 90;
            fingers_used_on_fingertrick[RIGHT_POINTER] = 1;
            fingers_used_on_fingertrick[RIGHT_MIDDLE] = 1;
        }

        //decay
        for (int f = 0; f < 4; f++){
            if (finger_usage[f] > 0 && !fingers_used_on_fingertrick[f]){
                finger_usage[f] = std::max(finger_usage[f]-movescore, 0);
            }
        }

        if (i == algEnd - 1 ||
            (algGeneratingMode && !gegMode &&
                (moves[algEnd-1] == U || moves[algEnd-1] == UP || moves[algEnd-1] == U2) &&
                i == algEnd-2)){
            //you can do funky fingertricks for the last move 
            //so lets not put very much weight on it
            score += (movescore/4);
            return score;
        } else{
            score += movescore;
        }
    }
    return score;
}


//Take move and make it be on a different axis
//Example:
//original: U'
//newAxis: AXIS_FB
//output F'
int axis_transpose(int original, int newAxis){
    if (original == R2 || original == F2 || original == U2){
        if (newAxis == FB)
            return F2;
        if (newAxis == UD)
            return U2;
        if (newAxis == RL)
            return R2;
    }
    if (original == R || original == F || original == U){
        if (newAxis == FB)
            return F;
        if (newAxis == UD)
            return U;
        if (newAxis == RL)
            return R;
    }
    if (original == RP || original == FP || original == UP){
        if (newAxis == FB)
            return FP;
        if (newAxis == UD)
            return UP;
        if (newAxis == RL)
            return RP;
    }

    assert(0);
}

int MOVE_AXIS[9] = {RL, UD, FB, RL, UD, FB, RL, UD, FB};

//UD, RL, FB

// newMoves[moveId] = axis_transpose(moves[moveId], NEW_AXIS[anchor_index][anchor_orientation][MOVE_AXIS[moves[moveId]]]);

//Anchor Index
//Anchor Orientation
//Original Move Axis
//-> new move axis
int NEW_AXIS[7][3][3] = {
    //UBL, 
    {{UD, FB, RL},
    {RL, UD, FB},
    {FB, RL, UD}},
    //UBR, 
    {{UD, RL, FB},
    {RL, FB, UD},
    {FB, UD, RL}},
    //UFR, 
    {{UD, FB, RL},
    {RL, UD, FB},
    {FB, RL, UD}},
    //UFL, 
    {{UD, RL, FB},
    {RL, FB, UD},
    {FB, UD, RL}},
    //DFL, 
    {{UD, FB, RL},
    {RL, UD, FB},
    {FB, RL, UD}},
    //DFR, 
    {{UD, RL, FB},
    {RL, FB, UD},
    {FB, UD, RL}},
    //DBR
    {{UD, FB, RL},
    {RL, UD, FB},
    {FB, RL, UD}}
};

// char* angle_to_rotations[]

void Alg::rotate(int corner, int orientation){
    int newMoves[64];
    memset(newMoves, 0, sizeof(newMoves));

    //if the anchor is the DBL corner then it's super easy to rotate the solution. 
    if (corner == DBL){
        if (orientation == 0){
            memcpy(all_angles[corner][orientation], moves, sizeof(moves));
            return;
        }
        for (int i = 0; i < length; i++){
            if (MOVE_AXIS[moves[i]] == UD){
                if (orientation == 1)
                    newMoves[i] = axis_transpose(moves[i], RL);
                if (orientation == 2)
                    newMoves[i] = axis_transpose(moves[i], FB);
            }
            if (MOVE_AXIS[moves[i]] == RL){
                if (orientation == 1)
                    newMoves[i] = axis_transpose(moves[i], FB);
                if (orientation == 2)
                    newMoves[i] = axis_transpose(moves[i], UD);
            }
            if (MOVE_AXIS[moves[i]] == FB){
                if (orientation == 1)
                    newMoves[i] = axis_transpose(moves[i], UD);
                if (orientation == 2)
                    newMoves[i] = axis_transpose(moves[i], RL);
            }
        }
        
        memcpy(all_angles[corner][orientation], newMoves, sizeof(newMoves));
        return;
    }

    Cube cube = Cube();
    

    for (int moveId = 0; moveId < length; moveId++){

        //find location and orientation of anchor corner
        int anchor_index = cube.find_corner(corner);
        int anchor_orientation = (cube.corner_orientation(cube.corners[anchor_index]) + orientation) % 3;

        // printf("id:%d o:%d\n", anchor_index, anchor_orientation);
        // printf("Raw Move:%s\n", MOVE_TO_STR[moves[moveId]]);

        newMoves[moveId] = axis_transpose(moves[moveId], NEW_AXIS[anchor_index][anchor_orientation][MOVE_AXIS[moves[moveId]]]);
        // printf("New Move:%s\n\n", MOVE_TO_STR[newMoves[moveId]]);

        cube.make_move(moves[moveId]);
    }

    memcpy(all_angles[corner][orientation], newMoves, sizeof(newMoves));
}

int Alg::score() {
    minScore = 10000000;

    gen(0, 0);
    gen(0, -1);
    gen(0, 1);

    return minScore;

    // for (int i = 0; i < length; i++){
    //     printf("%s\n", FINGERTRICK_NAMES[all_fingertrick_combinations[minId][i][0]].c_str());
    //     if (all_fingertrick_combinations[minId][i][1] != REGRIP_NONE){
    //         printf("%s\n", REGRIP_NAMES[all_fingertrick_combinations[minId][i][1]].c_str());
    //     }
    // }
}
