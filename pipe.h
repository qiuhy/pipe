#ifndef _PIPE_H
#define _PIPE_H

typedef enum { NONE = 0, STAR, PIPE_I, PIPE_L, PIPE_T, DEST } cell_type;
typedef enum { D_N = 0, D_E, D_S, D_W } cell_direction;

typedef union {
    struct {
        unsigned int N : 1;
        unsigned int E : 1;
        unsigned int S : 1;
        unsigned int W : 1;
        unsigned int : 4;
        unsigned int T : 3; // cell_type
        unsigned int D : 2; // cell_direction
        unsigned int L : 1; // linked
    };
    unsigned int value;
} pipe_cell;

void cell_rotate(int x, int y, int r);
pipe_cell get_cell(int x, int y);
char *get_cell_item(pipe_cell c);
int generate_cells(int width, int height);
int check_cells();

#endif