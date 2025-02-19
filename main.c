#include "pipe.h"
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#define ESC "\e"
#define CSI "\e["

int screen_width = 0;
int screen_height = 0;

int buff_width = 0;
int buff_height = 0;
HANDLE h_in;
HANDLE h_out;

void init_buff();
void draw_buff();
void print_cell(int x, int y, pipe_cell);

void onClick(int x, int y, int b);
void onMouseMove(int x, int y);
void onStart();
void onQuit();
void onError(char *s) { fprintf(stderr, "%s\n", s); }

void onKey(KEY_EVENT_RECORD *e) {
    if (e->bKeyDown) {
        switch (e->uChar.AsciiChar) {
        case '\x1b':
            onQuit();
            break;
        case 'r':
            if (e->dwControlKeyState & (LEFT_ALT_PRESSED | RIGHT_ALT_PRESSED))
                onStart();
            break;
        default:
            break;
        }
    }
}

void onMouse(MOUSE_EVENT_RECORD *e) {
    char s[10];
    sprintf(s, "(%d,%d)", e->dwMousePosition.X, e->dwMousePosition.Y);
    printf(CSI "%d;%dH", screen_height, screen_width - 20);
    printf("E:%02x B:%02x %10s", e->dwEventFlags, e->dwButtonState & 0xff, s);

    if (e->dwMousePosition.X > 0 && e->dwMousePosition.X < buff_width + 1 && e->dwMousePosition.Y > 0 &&
        e->dwMousePosition.Y < buff_height + 1) {
        if (e->dwEventFlags & MOUSE_MOVED) {
            onMouseMove(e->dwMousePosition.X - 1, e->dwMousePosition.Y - 1);
        } else if (e->dwButtonState & 0x1f) {
            onClick(e->dwMousePosition.X - 1, e->dwMousePosition.Y - 1, e->dwButtonState);
        }
    }

    switch (e->dwEventFlags) {
    case DOUBLE_CLICK:
        if (e->dwMousePosition.Y == screen_height - 1) {
            if (e->dwMousePosition.X > 0 && e->dwMousePosition.X < 10)
                onQuit();
            else if (e->dwMousePosition.X > 11 && e->dwMousePosition.X < 21)
                onStart();
        }
        break;
    default:
        break;
    }
}

void onResize(WINDOW_BUFFER_SIZE_RECORD *e) { ; }

void onMouseMove(int x, int y) { print_cell(40, screen_height, get_cell(x, y)); }

void onClick(int x, int y, int b) {
    cell_rotate(x, y, (b == FROM_LEFT_1ST_BUTTON_PRESSED) ? -1 : 1);
    check_cells();
    draw_buff();
    print_cell(x + 2, y + 2, get_cell(x, y));
    print_cell(40, screen_height, get_cell(x, y));
}

int processInput() {
    INPUT_RECORD ir;
    DWORD n;

    if (ReadConsoleInput(h_in, &ir, 1, &n)) {
        switch (ir.EventType) {
        case KEY_EVENT: // keyboard input
            onKey(&ir.Event.KeyEvent);
            break;
        case MOUSE_EVENT: // mouse input
            onMouse(&ir.Event.MouseEvent);
            break;
        case WINDOW_BUFFER_SIZE_EVENT: // scrn buf. resizing
            onResize(&ir.Event.WindowBufferSizeEvent);
            break;
        case FOCUS_EVENT: // disregard focus events
        case MENU_EVENT:  // disregard menu events
            break;
        default:
            onError("Unknown event type");
            break;
        }
    }
    return 1;
}

void processOutput() { fflush(stdout); }

void draw_interface() {
    int i;

    printf(CSI "0m");   // reset text color
    printf(CSI "1;1H"); // move cursor
    printf(CSI "2J");   // clear
    printf("┌");
    for (i = 1; i < screen_width - 1; i++) {
        printf("─");
    }
    printf("┐");

    for (i = 2; i < screen_height - 1; i++) {
        printf(CSI "%d;1H", i);
        printf("│");
        printf(CSI "%dG", screen_width);
        printf("│");
    }

    printf(CSI "%d;1H", screen_height - 1);
    printf("└");
    for (i = 1; i < screen_width - 1; i++) {
        printf("─");
    }
    printf("┘");

    printf(CSI "7m");
    printf(CSI "%d;2H", screen_height);
    printf("退出(Esc)");

    printf(CSI "%d;12H", screen_height);
    printf(" 重置(R) ");
    printf(CSI "0m");
}

void init_buff() {
    buff_width = screen_width - 2;
    buff_height = screen_height - 3;
    generate_cells(buff_width, buff_height);
}

void draw_buff() {
    int count = 0;
    int linked = 0;
    pipe_cell c;
    for (int y = 0; y < buff_height; y++) {
        for (int x = 0; x < buff_width; x++) {
            c = get_cell(x, y);
            if (c.T == DEST) {
                count++;
                if (c.L)
                    linked++;
            }
            print_cell(x + 2, y + 2, c);
        }
    }
    printf(CSI "%d;25H", screen_height);
    if (linked == count) {
        printf(CSI "92m"); // 粗体绿色
        printf("你赢了！");
        printf(CSI "0m");
    } else {
        printf("%d / %d ", linked, count);
    }
}

void print_cell(int x, int y, pipe_cell c) {
    printf(CSI "%d;%dH", y, x); // move cursor
    if (c.T == STAR)
        printf(CSI "41;36m");
    else if (c.L)
        printf(CSI "36m");
    printf("%s", get_cell_item(c));
    printf(CSI "0m");
}

void onStart() {
    CONSOLE_SCREEN_BUFFER_INFO bi;
    GetConsoleScreenBufferInfo(h_out, &bi);
    screen_width = bi.srWindow.Right - bi.srWindow.Left + 1;
    screen_height = bi.srWindow.Bottom - bi.srWindow.Top + 1;
    init_buff();
    draw_interface();
    draw_buff();
}

void init_win() {
    DWORD mode;
    SetConsoleCP(CP_UTF8);
    h_in = GetStdHandle(STD_INPUT_HANDLE);
    GetConsoleMode(h_in, &mode);

    mode |= ENABLE_WINDOW_INPUT;
    mode |= ENABLE_MOUSE_INPUT;
    mode |= ENABLE_EXTENDED_FLAGS;
    mode &= ~ENABLE_QUICK_EDIT_MODE;
    SetConsoleMode(h_in, mode);

    h_out = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleOutputCP(CP_UTF8);
}

void init() {
    setvbuf(stdout, NULL, _IOFBF, 4096); // 设置全局缓冲
    printf(CSI "?1049h");                // ʹ使用备用缓冲区
    printf(CSI "?25l");                  // hide cursor
    init_win();
    onStart();
}

void onQuit() {
    printf(CSI "?1049l");
    exit(0);
}

int main() {
    init();
    while (processInput())
        processOutput();
    onQuit();
}
