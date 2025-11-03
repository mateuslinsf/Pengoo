/*
 * ========================================
 * ARQUIVO PRINCIPAL: src/main.c (COM CORREÇÃO DO STACK SMASHING)
 * ========================================
 */

// Bibliotecas da cli-lib
#include "screen.h"
#include "keyboard.h"
#include "timer.h"

// Nosso arquivo com as structs do jogo
#include "game.h"

// Bibliotecas padrão
#include <stdio.h>
#include <unistd.h> // Para usleep() e read()
#include <fcntl.h>  // Para o Quit Fácil (fcntl)
#include <stdlib.h> // Para malloc(), free() e rand()
#include <time.h>   // Para srand()
#include <string.h> // Para memset()

// --- Constantes (sem mudanças) ---
#define LARGURA_TELA 80
#define ALTURA_TELA 24
#define PISO (ALTURA_TELA - 2)
#define GRAVIDADE 0.25f
#define FORCA_PULO -1.0f
#define ART_TERRESTRE "#" 
#define ART_AEREO "@"
#define LARGURA_BURACO 8
#define INTERVALO_SPAWN_INICIAL 100 
#define PONTOS_PARA_SUBIR_NIVEL 200  
#define INTERVALO_SPAWN_MINIMO 40   
#define ARQUIVO_SCORES "highscores.txt"

// --- Funções de Buffer e Obstáculos (sem mudanças) ---
void limparBuffer(char buffer[ALTURA_TELA][LARGURA_TELA]) {
    memset(buffer, ' ', sizeof(char) * ALTURA_TELA * LARGURA_TELA);
}

void imprimirBuffer(char buffer[ALTURA_TELA][LARGURA_TELA]) {
    screenClear();
    screenGotoxy(0, 0);
    for (int y = 0; y < ALTURA_TELA; y++) {
        for (int x = 0; x < LARGURA_TELA; x++) {
            putchar(buffer[y][x]);
        }
        putchar('\n');
    }
    screenUpdate();
}

void adicionarObstaculo(NoObstaculo** lista, int pontuacao) {
    int tipo = rand() % 4; 
    if (tipo == 3) {
        Obstaculo* obsBaixo = (Obstaculo*)malloc(sizeof(Obstaculo));
        NoObstaculo* noBaixo = (NoObstaculo*)malloc(sizeof(NoObstaculo));
        if (obsBaixo == NULL || noBaixo == NULL) { return; }
        obsBaixo->x = LARGURA_TELA - 2;
        obsBaixo->y = PISO;
        obsBaixo->largura = 1;
        obsBaixo->arteASCII = ART_TERRESTRE;
        obsBaixo->tipo = 0;
        noBaixo->obstaculo = obsBaixo;
        noBaixo->proximo = *lista;
        *lista = noBaixo;

        Obstaculo* obsCima = (Obstaculo*)malloc(sizeof(Obstaculo));
        NoObstaculo* noCima = (NoObstaculo*)malloc(sizeof(NoObstaculo));
        if (obsCima == NULL || noCima == NULL) { return; }
        obsCima->x = LARGURA_TELA - 2;
        obsCima->y = PISO - 1;
        obsCima->largura = 1;
        obsCima->arteASCII = ART_TERRESTRE;
        obsCima->tipo = 0;
        noCima->obstaculo = obsCima;
        noCima->proximo = *lista;
        *lista = noCima;
        return;
    }
    Obstaculo* novoObs = (Obstaculo*)malloc(sizeof(Obstaculo));
    if (novoObs == NULL) return; 
    NoObstaculo* novoNo = (NoObstaculo*)malloc(sizeof(NoObstaculo));
    if (novoNo == NULL) { free(novoObs); return; }
    novoObs->x = LARGURA_TELA - 2; 
    novoObs->tipo = tipo;
    if (tipo == 0) {
        novoObs->y = PISO;
        novoObs->largura = 1;
        novoObs->arteASCII = ART_TERRESTRE;
    } 
    else if (tipo == 1) {
        novoObs->largura = 1;
        novoObs->arteASCII = ART_AEREO;
        int alturaSorteada = rand() % 3;
        if (alturaSorteada == 0) { novoObs->y = PISO - 3; }
        else if (alturaSorteada == 1) { novoObs->y = PISO - 2; }
        else { novoObs->y = PISO - 1; }
    } 
    else if (tipo == 2) {
        novoObs->y = PISO + 1;
        novoObs->arteASCII = " ";
        novoObs->largura = (pontuacao > 650) ? (int)(LARGURA_BURACO * 1.5) : LARGURA_BURACO;
    }
    novoNo->obstaculo = novoObs;
    novoNo->proximo = *lista;
    *lista = novoNo;
}

// --- Funções de High Score (sem mudanças) ---
void carregarHighScores(Score topScores[3]) {
    for (int i = 0; i < 3; i++) {
        strcpy(topScores[i].nome, "---");
        topScores[i].pontuacao = 0;
    }
    FILE* f = fopen(ARQUIVO_SCORES, "r");
    if (f == NULL) { return; }
    for (int i = 0; i < 3; i++) {
        fscanf(f, "%s %d\n", topScores[i].nome, &topScores[i].pontuacao);
    }
    fclose(f);
}

void salvarHighScores(Score topScores[3]) {
    FILE* f = fopen(ARQUIVO_SCORES, "w");
    if (f == NULL) { return; }
    for (int i = 0; i < 3; i++) {
        fprintf(f, "%s %d\n", topScores[i].nome, topScores[i].pontuacao);
    }
    fclose(f);
}

int obterRanking(Score topScores[3], int pontuacaoAtual) {
    if (pontuacaoAtual > topScores[0].pontuacao) { return 1; }
    if (pontuacaoAtual > topScores[1].pontuacao) { return 2; }
    if (pontuacaoAtual > topScores[2].pontuacao) { return 3; }
    return 0;
}

void adicionarNovoScore(Score topScores[3], int pontuacaoAtual, char* nome, int ranking) {
    Score novoScore;
    novoScore.pontuacao = pontuacaoAtual;
    strcpy(novoScore.nome, nome);
    if (ranking == 1) {
        topScores[2] = topScores[1];
        topScores[1] = topScores[0];
        topScores[0] = novoScore;
    } 
    else if (ranking == 2) {
        topScores[2] = topScores[1];
        topScores[1] = novoScore;
    }
    else if (ranking == 3) {
        topScores[2] = novoScore;
    }
}


int main() {
    
    // --- 1. Inicialização ---
    EstadoJogo estado;
    Pinguim pinguim;
    char telaBuffer[ALTURA_TELA][LARGURA_TELA]; 
    srand(time(NULL));

    int contadorSpawn = 0;
    int intervaloSpawnAtual = INTERVALO_SPAWN_INICIAL;
    int proximoNivelPontuacao = PONTOS_PARA_SUBIR_NIVEL;

    carregarHighScores(estado.topScores); 

    screenInit(1);
    keyboardInit();
    
    // (Correção do Stack Smashing 1/2)
    int old_flags = fcntl(STDIN_FILENO, F_GETFL);
    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);

    estado.rodando = 1;
    estado.pontuacao = 0;
    estado.velocidadeJogo = 1.0f; 
    estado.listaDeObstaculos = NULL;

    pinguim.x = 5;
    pinguim.y = 10; 
    pinguim.velocidade_y = 0.0f;
    pinguim.estaNoChao = 0;
    pinguim.puloDuploDisponivel = 1;
    pinguim.arteASCII = "P"; 

    // --- 2. Game Loop ---
    while (estado.rodando) {
        // ... (Loop principal do jogo - sem mudanças) ...
        char tecla = '\0';
        read(STDIN_FILENO, &tecla, 1); 

        if (tecla == 'q') { estado.rodando = 0; }
        if (tecla == ' ') { 
            if (pinguim.estaNoChao) {
                pinguim.velocidade_y = FORCA_PULO;
                pinguim.estaNoChao = 0;
                pinguim.puloDuploDisponivel = 1; 
            } else if (pinguim.puloDuploDisponivel) {
                pinguim.velocidade_y = FORCA_PULO; 
                pinguim.puloDuploDisponivel = 0; 
            }
        }
        
        pinguim.velocidade_y = pinguim.velocidade_y + GRAVIDADE;
        pinguim.y = (int)(pinguim.y + pinguim.velocidade_y);
        if (pinguim.y >= PISO) {
            pinguim.y = PISO;
            pinguim.velocidade_y = 0;
            pinguim.estaNoChao = 1;
            pinguim.puloDuploDisponivel = 1;
        }

        estado.pontuacao++; 
        if (estado.pontuacao > proximoNivelPontuacao) {
            if (estado.velocidadeJogo < 3.0f) { estado.velocidadeJogo += 0.2f; }
            if (intervaloSpawnAtual > INTERVALO_SPAWN_MINIMO) { intervaloSpawnAtual -= 5; }
            proximoNivelPontuacao += PONTOS_PARA_SUBIR_NIVEL;
        }

        contadorSpawn++;
        if (contadorSpawn > intervaloSpawnAtual) {
            adicionarObstaculo(&estado.listaDeObstaculos, estado.pontuacao);
            contadorSpawn = 0;
        }

        NoObstaculo* atual = estado.listaDeObstaculos;
        NoObstaculo* anterior = NULL;
        while (atual != NULL) {
            atual->obstaculo->x -= (int)estado.velocidadeJogo; 
            if (atual->obstaculo->x + atual->obstaculo->largura < 0) {
                NoObstaculo* noParaRemover = atual;
                if (anterior == NULL) { estado.listaDeObstaculos = atual->proximo; }
                else { anterior->proximo = atual->proximo; }
                atual = atual->proximo; 
                free(noParaRemover->obstaculo);
                free(noParaRemover);
            } else {
                anterior = atual;
                atual = atual->proximo;
            }
        }

        atual = estado.listaDeObstaculos;
        while (atual != NULL) {
            Obstaculo* obs = atual->obstaculo;
            int velocidadeInt = (int)estado.velocidadeJogo;
            int pinguimX = pinguim.x; 
            int pinguimY = pinguim.y;
            int obsX_inicio = obs->x;
            int obsX_fim = obs->x + obs->largura;
            int obsY = obs->y;
            if ( (obsX_inicio <= pinguimX) && ((obsX_inicio + velocidadeInt) > pinguimX) ) {
                if ( (obs->tipo == 0) && (pinguimY == obsY) ) {
                    estado.rodando = 0;
                }
            }
            if (obs->tipo == 2) {
                if ( (pinguimY == PISO) && (pinguimX >= obsX_inicio) && (pinguimX < obsX_fim) ) {
                    estado.rodando = 0;
                }
            }
            atual = atual->proximo;
        }
        
        limparBuffer(telaBuffer);

        telaBuffer[pinguim.y][pinguim.x] = *pinguim.arteASCII; 
        for (int x = 0; x < LARGURA_TELA; x++) {
            telaBuffer[PISO + 1][x] = '-';
        }
        atual = estado.listaDeObstaculos;
        while (atual != NULL) {
            Obstaculo* obs = atual->obstaculo;
            for (int i = 0; i < obs->largura; i++) {
                int posX = obs->x + i;
                if (posX >= 0 && posX < LARGURA_TELA) {
                    telaBuffer[obs->y][posX] = *obs->arteASCII; 
                }
            }
            atual = atual->proximo;
        }
        
        char hud[LARGURA_TELA];
        sprintf(hud, "Pontos: %d | RECORDE: %s %d (Aperte 'q')", 
                estado.pontuacao, estado.topScores[0].nome, estado.topScores[0].pontuacao);
                
        for(int i = 0; hud[i] != '\0' && i < LARGURA_TELA; i++) {
            telaBuffer[0][i] = hud[i];
        }

        if (estado.rodando == 0) {
            char* gameOverMsg = "G A M E   O V E R";
            int startX = (LARGURA_TELA - strlen(gameOverMsg)) / 2;
            int startY = ALTURA_TELA / 2;
            for(int i = 0; gameOverMsg[i] != '\0'; i++) {
                telaBuffer[startY][startX + i] = gameOverMsg[i];
            }
        }

        imprimirBuffer(telaBuffer);
        
        if (estado.rodando == 0) {
            usleep(2000000); // Pausa por 2s
        }
        usleep(33000); 
    }

    // --- 3. Finalização ---
    NoObstaculo* atual = estado.listaDeObstaculos; 
    while (atual != NULL) {
        NoObstaculo* proximo = atual->proximo;
        free(atual->obstaculo);
        free(atual);
        atual = proximo;
    }

    screenDestroy();
    keyboardDestroy();

    // (Correção do Stack Smashing 2/2)
    fcntl(STDIN_FILENO, F_SETFL, old_flags);

    int ranking = obterRanking(estado.topScores, estado.pontuacao);

    if (ranking > 0) {
        printf("\n--- NOVO RECORDE! ---\n");
        printf("Sua pontuação: %d\n", estado.pontuacao);
        
        if (ranking == 1) { printf("Você ficou em 1º LUGAR!\n"); }
        else if (ranking == 2) { printf("Você ficou em 2º LUGAR!\n"); }
        else { printf("Você ficou em 3º LUGAR!\n"); }

        printf("Digite suas 3 iniciais (ex: LFG): ");
        
        char nome[10];
        scanf("%3s", nome); // Agora o scanf vai funcionar
        
        adicionarNovoScore(estado.topScores, estado.pontuacao, nome, ranking);
        salvarHighScores(estado.topScores);
        
        printf("Score salvo!\n");

    } else {
        printf("Game Over! Pontuação Final: %d\n", estado.pontuacao);
    }
    
    printf("\n--- TOP 3 SCORES ---\n");
    for (int i = 0; i < 3; i++) {
        printf("%d. %s .... %d\n", i+1, estado.topScores[i].nome, estado.topScores[i].pontuacao);
    }

    return 0;
}
