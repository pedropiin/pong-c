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
#define BASE_BARRA(i) (i + 4)

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

void atualiza_barras(int *dir_esquerda, int *dir_direita, int *ybarra_esquerda, int *ybarra_direita) {
    if (*ybarra_esquerda <= 13 && *ybarra_esquerda >= 1) {
        *ybarra_esquerda = (*ybarra_esquerda + *dir_esquerda + ROWS) % ROWS;
    } else if (*ybarra_esquerda >= 13 && *dir_esquerda != 1) {
        *ybarra_esquerda = (*ybarra_esquerda + *dir_esquerda + ROWS) % ROWS;
    } else if (*ybarra_esquerda <= 1 && *dir_esquerda != -1) {
        *ybarra_esquerda = (*ybarra_esquerda + *dir_esquerda + ROWS) % ROWS;
    } else if (*ybarra_esquerda >= 13 && *dir_esquerda == 1) {
        *dir_esquerda = 0;
    } else if (*ybarra_esquerda <=1 && *dir_esquerda == -1) {
        *dir_esquerda = 0;
    }

    if (*ybarra_direita <= 13 && *ybarra_direita >= 1) {
        *ybarra_direita = (*ybarra_direita + *dir_direita + ROWS) % ROWS;
    } else if (*ybarra_direita >= 13 && *dir_direita != 1) {
        *ybarra_direita = (*ybarra_direita + *dir_direita + ROWS) % ROWS;
    } else if (*ybarra_direita <= 1 && *dir_direita != -1) {
        *ybarra_direita = (*ybarra_direita + *dir_direita + ROWS) % ROWS;
    } else if (*ybarra_direita >= 13 && *dir_direita == 1) {
        *dir_direita = 0;
    } else if (*ybarra_direita <= 1 && *dir_direita == -1) {
        *dir_direita = 0;
    }
}

void game_over(int lado_vencedor) {
    if (lado_vencedor == 0) {
        printf("\e[%iB\e[%iC Esquerda Venceu! ", ROWS / 2, COLS / 2 - 5);
    } else {
        printf("\e[%iB\e[%iC Direita Venceu! ", ROWS / 2, COLS / 2 - 5);
    }
    printf("\e[%iF", ROWS / 2);
    fflush(stdout);
    getchar();
}

int checa_fim_jogo(int pontos_esquerda, int pontos_direita) {
    if (pontos_esquerda == 5) {
        game_over(0);
        return 1;
    } else if (pontos_direita == 5) {
        game_over(1);
        return 1;
    }
    return 0;
}

void desenha_barras(int ybarra_esquerda, int ybarra_direita) {
    for (int i = 1; i <= TAMANHO_BARRA + 1; i++) {
        printf("\e[%iB\e[%iC▓", ybarra_esquerda + i, COL_BARRA_ESQUERDA + 1);
        printf("\e[%iF", ybarra_esquerda + i);
    }
    for (int i = 1; i <= TAMANHO_BARRA + 1; i++) {
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

void calcula_direcao_bolinha_esquerda(int *xdir_bola, int *ydir_bola, int dir_esquerda) {
    *xdir_bola = 1;
    //bolinha reta
    if (*ydir_bola == 0) {
        if (dir_esquerda == -1) { //barra esta subindo
            *ydir_bola = -1;
        } else if (dir_esquerda == 1) { //barra esta descendo
            *ydir_bola = 1;
        } else if (dir_esquerda == 0) { //barra esta parada
            *ydir_bola = 0;
        }

    //bolinha subindo
    } else if (*ydir_bola == -1) { 
        if (dir_esquerda == -1) { //barra esta subindo
            *ydir_bola = -1;
        } else if (dir_esquerda == 1) { //barra esta descendo
            *ydir_bola = 0;
        } else if (dir_esquerda == 0) { //barra esta parada
            *ydir_bola = -1;
        }

    //bolinha descendo
    } else if (*ydir_bola == 1) {
        if (dir_esquerda == -1) { //barra esta subindo
            *ydir_bola = 0;
        } else if (dir_esquerda == 1) { //barra esta descendo
            *ydir_bola = 1;
        } else if (dir_esquerda == 0) { //barra esta parada
            *ydir_bola = 1;
        }
    }

}

void calcula_direcao_bolinha_direita(int *xdir_bola, int *ydir_bola, int dir_direita) {
    *xdir_bola = -1;
    //bolinha reta
    if (*ydir_bola == 0) {
        if (dir_direita == -1) { //barra esta subindo
            *ydir_bola = -1;
        } else if (dir_direita == 1) { //barra esta descendo
            *ydir_bola = 1;
        } else if (dir_direita == 0) { //barra esta parada
            *ydir_bola = 0;
        }

    //bolinha subindo
    } else if (*ydir_bola == -1) {
        if (dir_direita == -1) { //barra esta subindo
            *ydir_bola = -1;
        } else if (dir_direita == 1) { //barra esta descendo
            *ydir_bola = 0;
        } else if (dir_direita == 0) { //barra esta parada
            *ydir_bola = -1;
        }

    //bolinha descendo
    } else if (*ydir_bola == 1) {
        if (dir_direita == -1) { //barra esta subindo
            *ydir_bola = 0;
        } else if (dir_direita == 1) { //barra esta descendo
            *ydir_bola = 1;
        } else if (dir_direita == 0) { //barra esta parada
            *ydir_bola = 1;
        }
    }

}

void checa_posicao_bolinha(int *xdir_bola, int *ydir_bola, int *xbola, int *ybola, int ybarra_esquerda, int ybarra_direita, int *pontos_esquerda, int *pontos_direita, int *num_rebotes, int dir_esquerda, int dir_direita) {
    // checa se bola esta na barra da esquerda
    if (*xbola == COL_BARRA_ESQUERDA + 1 && ybarra_esquerda <= *ybola && *ybola <= BASE_BARRA(ybarra_esquerda)) {
        calcula_direcao_bolinha_esquerda(xdir_bola, ydir_bola, dir_esquerda);
        *num_rebotes += 1;

    // checa se esta na barra da direita
    } else if (*xbola == COL_BARRA_DIREITA - 1 && ybarra_direita <= *ybola && *ybola <= BASE_BARRA(ybarra_direita)) {
        calcula_direcao_bolinha_direita(xdir_bola, ydir_bola, dir_direita);
        *num_rebotes += 1;

    // checa se a bola passou da barra da esquerda
    } else if (*xbola < COL_BARRA_ESQUERDA) {
        *xdir_bola = -1;
        *ydir_bola = 0;
        *xbola = COLS / 2;
        *ybola = ROWS / 2;
        *pontos_direita += 1;
        *num_rebotes = 0;
        usleep(1000000);

    // checa se a bola passou da barra da direita
    } else if (*xbola > COL_BARRA_DIREITA) {
        *xdir_bola = 1;
        *ydir_bola = 0;
        *xbola = COLS / 2;
        *ybola = ROWS / 2;
        *pontos_esquerda += 1;
        *num_rebotes = 0;
        usleep(1000000);
    
    // checa se a bola bateu no teto
    } else if (*ybola == 0 || *ybola == ROWS) {
        *ydir_bola = *ydir_bola * (-1);
    }
}

void atualiza_coordenadas_bolinha(int xdir_bola, int ydir_bola, int *xbola, int *ybola) {
    *xbola = (*xbola + xdir_bola + COLS) % COLS;
    *ybola = (*ybola + ydir_bola + ROWS) % ROWS;
}

void printa_bolinha(int xdir_bola, int ydir_bola, int xbola, int ybola) {
    printf("\e[%iB\e[%iC●", ybola + 1, xbola + 1);
    printf("\e[%iF", ybola + 1);
    fflush(stdout);
}

void printa_pontuacao(int pontos_esquerda, int pontos_direita) {
    printf("\e[%iB\e[%iC%d\e[%iC%d", 2, (COLS / 2) - 3, pontos_direita, 6, pontos_esquerda);
    printf("\e[%iF", 2);
    fflush(stdout);
}


int main() {
    printf("\e[?25l"); //esconder cursor

    struct termios oldt = transicao_canonico();

    int ybarra_esquerda = 8, ybarra_direita = 8;
    int quit = 0, fim_jogo = 0, pontos_esquerda = 0, pontos_direita = 0, num_rebotes = 0;
    int dir_esquerda = 0, dir_direita = 0;
    int xbola = COLS / 2, ybola = ROWS / 2, xdir_bola = -1, ydir_bola = 0;

    while (!quit && !fim_jogo) {
        printa_tabuleiro();
        printf("\e[%iA", ROWS + 2);

        atualiza_barras(&dir_esquerda, &dir_direita, &ybarra_esquerda, &ybarra_direita);
        fim_jogo = checa_fim_jogo(pontos_esquerda, pontos_direita);
        desenha_barras(ybarra_esquerda, ybarra_direita);
        
        checa_posicao_bolinha(&xdir_bola, &ydir_bola, &xbola, &ybola, ybarra_esquerda, ybarra_direita, &pontos_esquerda, &pontos_direita, &num_rebotes, dir_esquerda, dir_direita);
        atualiza_coordenadas_bolinha(xdir_bola, ydir_bola, &xbola, &ybola);
        printa_bolinha(xdir_bola, ydir_bola, xbola, ybola);
        printa_pontuacao(pontos_esquerda, pontos_direita);

        usleep(5 * 1000000 / (80 * (1 + (0.15 * num_rebotes))));

        le_comandos(&dir_esquerda, &dir_direita, &quit, ybarra_esquerda, ybarra_direita);
    }


    printf("\e[?25h"); //reaparecer cursor
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    system("clear");
    return 0;
}