#include "pipe.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>

// { "═║╚╔╗╝╠╦╩╣", "━┃┗┏┓┛┣┻┳┫", " ⍗⍐⍇⍈  ⍓⍌⍃⍄  ⍅⍆⍏⍖ "█", "⛋",⌼ "⛒", "⛝""};
// {"⇨⇩⇦⇧ ⮕⬇⬅⬆ ⭲⭳⭰⭱ ⮆⮇⮄⮅ ▶▲▼◀  ▷△▽◁"};

char *none_item = " ";
char *pipe_L_item[] = {"╚", "╔", "╗", "╝", "┗", "┏", "┓", "┛"};
char *pipe_I_item[] = {"║", "═", "║", "═", "┃", "━", "┃", "━"};
char *pipe_T_item[] = {"╠", "╦", "╣", "╩", "┣", "┳", "┫", "┻"};
char *recv_item[] = {"⍌", "⍃", "⍓", "⍄", "⛒", "⛒", "⛒", "⛒"};
char *star_item[] = {"△", "▷", "▽", "◁"};

pipe_cell *buff = NULL;
int width = 0;
int height = 0;

typedef union {
    struct {
        unsigned char x;
        unsigned char y;
    };
    unsigned int p;
} position;

int star_x;
int star_y;

void cell_rotate_r(pipe_cell *c, int r) {
    pipe_cell b;
    b.value = c->value;

    while (r & 3) {
        b.value = c->value;
        if (r > 0) { // 顺时针
            c->E = b.N;
            c->S = b.E;
            c->W = b.S;
            c->N = b.W;
            c->D++;
            r--;
        } else { // 逆时针
            c->E = b.S;
            c->S = b.W;
            c->W = b.N;
            c->N = b.E;
            c->D--;
            r++;
        }
    }
}

void cell_rotate(int x, int y, int r) { // 旋转r次 r>0 顺时针
    pipe_cell *c;
    c = &buff[y * width + x];
    cell_rotate_r(c, r);
}

char *get_cell_item(pipe_cell c) { // 获取显示字符
    switch (c.T) {
    case STAR:
        return star_item[c.D];
        break;
    case PIPE_I:
        if (c.L)
            return pipe_I_item[c.D + 4];
        else
            return pipe_I_item[c.D];
        break;
    case PIPE_L:
        if (c.L)
            return pipe_L_item[c.D + 4];
        else
            return pipe_L_item[c.D];
        break;
    case PIPE_T:
        if (c.L)
            return pipe_T_item[c.D + 4];
        else
            return pipe_T_item[c.D];
        break;
    case DEST:
        if (c.L)
            return recv_item[c.D + 4];
        else
            return recv_item[c.D];
        break;
    default:
        return none_item;
    }
}

pipe_cell get_cell(int x, int y) { return buff[y * width + x]; }

pipe_cell new_cell(int type) {
    pipe_cell c;
    c.value = 0;
    c.T = type;
    c.D = D_N;
    switch (c.T) {
    case STAR:
        c.N = 1;
        break;
    case PIPE_I:
        c.N = 1;
        c.S = 1;
        break;
    case PIPE_L:
        c.N = 1;
        c.E = 1;
        break;
    case PIPE_T:
        c.N = 1;
        c.E = 1;
        c.S = 1;
        break;
    case DEST:
        c.N = 1;
        break;
    default:
        break;
    }
    return c;
}

void put_cell(int x, int y, int t, int r, int l) {
    pipe_cell c;
    c = new_cell(t);
    if (r)
        cell_rotate_r(&c, r);
    c.L = l;
    buff[y * width + x].value = c.value;
}

int get_dir(int fx, int fy, int tx, int ty) {
    if (fx == tx) {
        if (fy == 0 && ty == height - 1)
            return D_N;
        if (ty == 0 && fy == height - 1)
            return D_S;
        return (fy > ty) ? D_N : D_S;
    } else {
        if (fx == 0 && tx == width - 1)
            return D_W;
        if (tx == 0 && fx == width - 1)
            return D_E;
        return (fx > tx) ? D_W : D_E;
    }
}

int put_path(position *path, int path_size) {
    int fx, fy;
    int cx, cy;
    int fd, td;
    int t, r;
    cx = path->x;
    cy = path->y;
    path++;
    r = get_dir(cx, cy, path->x, path->y);
    put_cell(cx, cy, DEST, r, 1);

    for (int i = 1; i < path_size - 1; i++) {
        fx = cx;
        fy = cy;
        cx = path->x;
        cy = path->y;
        path++;
        fd = get_dir(cx, cy, fx, fy);
        td = get_dir(cx, cy, path->x, path->y);

        t = (abs(fd - td) == 2) ? PIPE_I : PIPE_L;
        if (t == PIPE_I)
            r = fd;
        else if ((fd == D_N && td == D_W) || (td == D_N && fd == D_W))
            r = D_W;
        else
            r = __min(fd, td);

        put_cell(cx, cy, t, r, 1);
    }
}

int add_path(int x, int y) { // 添加第一个通路
    int path_size = abs(x - star_x) + abs(y - star_y) + 1;
    position path[path_size];

    path[0].x = x;
    path[0].y = y;
    path[path_size - 1].x = star_x;
    path[path_size - 1].y = star_y;

    for (int i = 1; i < path_size - 1; i++) {
        if (rand() % 2) {
            if (x != star_x)
                (x > star_x) ? x-- : x++;
            else
                (y > star_y) ? y-- : y++;
        } else {
            if (y == star_y)
                (x > star_x) ? x-- : x++;
            else
                (y > star_y) ? y-- : y++;
        }
        path[i].x = x;
        path[i].y = y;
    }

    put_path(path, path_size);

    cell_rotate(star_x, star_y, get_dir(star_x, star_y, x, y));
    return 1;
}

int get_freecell(position *p) {
    position *freecell, *pf;
    pipe_cell *pb = buff;
    int free_count = 0;

    freecell = (position *)calloc(width * height, sizeof(position));
    pf = freecell;
    for (int y = 0; y < height; y++)
        for (int x = 0; x < width; x++) {
            if (pb->value == 0) {
                pf->x = x;
                pf->y = y;
                free_count++;
                pf++;
            }
            pb++;
        }
    if (free_count)
        p->p = freecell[rand() % free_count].p;
    free(freecell);
    return free_count;
}

position get_near_pos(position pf, int d) {
    position pt;
    pt.p = pf.p;
    switch (d) {
    case D_N:
        (pf.y == 0) ? pt.y = height - 1 : pt.y--;
        break;
    case D_S:
        (pf.y == height - 1) ? pt.y = 0 : pt.y++;
        break;
    case D_E:
        (pf.x == width - 1) ? pt.x = 0 : pt.x++;
        break;
    case D_W:
        (pf.x == 0) ? pt.x = width - 1 : pt.x--;
        break;
    default:
        break;
    }
    return pt;
}

int set_nextpath(position *path, int *path_size) {
    int dir[] = {D_N, D_E, D_S, D_W};
    int i, t, d = 0;
    for (i = 0; i < 4; i++) { // 随机方向
        while (d == i)
            d = rand() % 4;
        t = dir[d];
        dir[d] = dir[i];
        dir[i] = t;
    }
    position pos;
    pipe_cell c;
    int inpath = 0;
    for (i = 0; i < 4; i++) {
        pos = get_near_pos(path[(*path_size) - 1], dir[i]);
        c = get_cell(pos.x, pos.y);
        if (c.T == NONE) {
            for (t = 0; t < (*path_size); t++) {
                if (path[t].p == pos.p) {
                    inpath = 1;
                    break;
                }
            }
            if (inpath == 0) {
                path[*path_size].p = pos.p;
                (*path_size)++;
                if (set_nextpath(path, path_size))
                    return 1;
                (*path_size)--;
            }
        } else if (c.T == PIPE_I || c.T == PIPE_L) {
            path[*path_size].p = pos.p;
            (*path_size)++;
            return 1;
        }
    }
    return 0;
}

void set_pipe_T(position p, position f) {
    pipe_cell c;
    int r = 0, d;
    c = get_cell(p.x, p.y);
    d = get_dir(p.x, p.y, f.x, f.y);
    d = (c.value & 0xf) | (1 << d);
    for (int i = 0; i < 4; i++) {
        if ((d & (1 << i)) == 0) {
            r = (i + 1) % 4;
            break;
        }
    }
    put_cell(p.x, p.y, PIPE_T, r, 1);
}

int find_nextpath() { // 获取下一个通路
    position *path;
    position pos;
    pipe_cell c;

    int path_size = 1;
    int i, x, y;
    if (get_freecell(&pos) == 0)
        return 0;

    path = (position *)calloc(width * height, sizeof(position));
    path[0] = pos; // 起点随机

    if (set_nextpath(path, &path_size)) {
        put_path(path, path_size);
        set_pipe_T(path[path_size - 1], path[path_size - 2]); // 修改连接点
    } else {
        path_size = 1;
    }
    free(path);
    return path_size - 1;
}

int check_cell(position fp, int fd) {
    position np;
    pipe_cell *c;

    np = get_near_pos(fp, fd);
    c = &buff[np.y * width + np.x];
    if (c->T == NONE || c->T == STAR)
        return 0;
    else if (c->L)
        return 1;
    else if (fd == D_N && c->S == 1)
        c->L = 1;
    else if (fd == D_E && c->W == 1)
        c->L = 1;
    else if (fd == D_S && c->N == 1)
        c->L = 1;
    else if (fd == D_W && c->E == 1)
        c->L = 1;

    if (c->L) {
        int ret = 1;
        if (c->T == DEST) {
            ret = 1;
        } else {
            int td = (fd + 2) % 4;
            for (int i = 0; i < 4; i++) {
                if ((i != td) && (c->value & (1 << i)))
                    if (check_cell(np, i) == 0)
                        ret = 0;
            }
        }
        return ret;
    } else
        return 0;
}

int check_cells() { // 检查联通状态 返回 1 全部联通 否则 0
    pipe_cell *c = buff;

    for (int i = 0; i < width * height; i++, c++) {
        if (c->T)
            c->L = 0;
    }
    position ps;
    ps.x = star_x;
    ps.y = star_y;
    return check_cell(ps, get_cell(star_x, star_y).D);
}

int mix_up() { // 打乱
    pipe_cell *c = buff;
    for (int i = 0; i < width * height; i++, c++) {
        if (c->T)
            cell_rotate_r(c, rand() % 4);
    }
    check_cells();
}

int generate_cells(int buff_width, int buff_height) { // 重新生成
    int x, y;

    width = buff_width;
    height = buff_height;
    if (buff)
        free(buff);
    buff = (pipe_cell *)calloc(width * height, sizeof(pipe_cell));
    if (buff == NULL)
        exit(0);
    srand((unsigned int)time(NULL));

    x = rand() % width;
    y = rand() % height;
    put_cell(x, y, STAR, 0, 1);
    star_x = x;
    star_y = y;

    while (abs(x - star_x) < 3) {
        x = rand() % width;
    }
    while (abs(y - star_y) < 3) {
        y = rand() % height;
    }

    int max_count = (width * height / 6);
    int count = add_path(x, y);
    while (--max_count)
        if (find_nextpath())
            count++;
    mix_up();
    return count;
}
