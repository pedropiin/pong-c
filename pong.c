#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <sys/select.h>

#define COLS 40
#define ROWS 20
#define COL_BARRA_ESQUERDA 2
#define COL_BARRA_DIREITA 37
#define TAMANHO_BARRA 5
#define BASE_BARRA(i) (i + 5)

void printa_tabuleiro() {
    printf("┌");
    for (int i = 0; i < COLS; i++) {
        printf("─");
    }
    printf("┐\n");

    for (int j = 0; j < ROWS; j++) {
        printf("|");
        for (int i = 0; i < COLS; i++) {
            if (i == (COLS / 2)) {
                printf("|");
            } else {
                printf(" ");
            }
        }
        printf("|\n");
    }

    printf("└");
    for (int i = 0; i < COLS; i++) {
        printf("─");
    }
    printf("┘\n");

}

struct termios transicao_canonico() {
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    return oldt;
}

void atualiza_barras(int dir_esquerda, int dir_direita, int *ybarra_esquerda, int *ybarra_direita) {
    if (*ybarra_esquerda <= 13 && *ybarra_esquerda >= 1) {
        *ybarra_esquerda = (*ybarra_esquerda + dir_esquerda + ROWS) % ROWS;
    } else if (*ybarra_esquerda >= 13 && dir_esquerda != 1) {
        *ybarra_esquerda = (*ybarra_esquerda + dir_esquerda + ROWS) % ROWS;
    } else if (*ybarra_esquerda <= 1 && dir_esquerda != -1) {
        *ybarra_esquerda = (*ybarra_esquerda + dir_esquerda + ROWS) % ROWS;
    }

    if (*ybarra_direita <= 13 && *ybarra_direita >= 1) {
        *ybarra_direita = (*ybarra_direita + dir_direita + ROWS) % ROWS;
    } else if (*ybarra_direita >= 13 && dir_direita != 1) {
        *ybarra_direita = (*ybarra_direita + dir_direita + ROWS) % ROWS;
    } else if (*ybarra_direita <= 1 && dir_direita != -1) {
        *ybarra_direita = (*ybarra_direita + dir_direita + ROWS) % ROWS;
    }
}

int checa_fim_jogo(int pontos_esquerda, int pontos_direita) {
    if (pontos_esquerda == 5 || pontos_direita == 5) {
        return 1;
    }
    return 0;
}

void desenha_barras(int ybarra_esquerda, int ybarra_direita) {
    for (int i = 1; i <= TAMANHO_BARRA + 1; i++) {
        printf("\e[%iB\e[%iC▓", ybarra_esquerda + i, COL_BARRA_ESQUERDA + 1);
        printf("\e[%iF", ybarra_esquerda + i);
    }
    for (int i = 0; i <= TAMANHO_BARRA; i++) {
        printf("\e[%iB\e[%iC▓", ybarra_direita + i, COL_BARRA_DIREITA + 1);
        printf("\e[%iF", ybarra_direita + i);
    }
    fflush(stdout);
}

void le_comandos(int *dir_esquerda, int *dir_direita, int *quit, int ybarra_esquerda, int ybarra_direita) {
    struct timeval tv;
    fd_set fds;
    tv.tv_sec = 0;
    tv.tv_usec = 0;

    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);
    if (FD_ISSET(STDIN_FILENO, &fds)) {
        int tecla = getchar();
        if (tecla == 'q') {
            *quit = 1;
        } else if (tecla == 'w') {
            *dir_esquerda = -1;
        } else if (tecla == 's') {
            *dir_esquerda = 1;
        } else if (tecla == 'o') {
            *dir_direita = -1;
        } else if (tecla == 'l') {
            *dir_direita = 1;
        }
    }
}


int main() {
    printf("\e[?25l"); //esconder cursor

    struct termios oldt = transicao_canonico();

    int ybarra_esquerda = 8, ybarra_direita = 8;
    int quit = 0, fim_jogo = 0, pontos_esquerda = 0, pontos_direita = 0;
    int dir_esquerda = 0, dir_direita = 0;

    while (!quit && !fim_jogo) {
        printa_tabuleiro();
        printf("\e[%iA", ROWS + 2);

        atualiza_barras(dir_esquerda, dir_direita, &ybarra_esquerda, &ybarra_direita);
        fim_jogo = checa_fim_jogo(pontos_esquerda, pontos_direita);
        desenha_barras(ybarra_esquerda, ybarra_direita);

        usleep(5 * 1000000 / 60);

        le_comandos(&dir_esquerda, &dir_direita, &quit, ybarra_esquerda, ybarra_direita);

    }


    printf("\e[?25h"); //reaparecer cursor
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    system("clear");
    return 0;
}