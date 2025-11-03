/*
 * ========================================
 * ARQUIVO PRINCIPAL: src/main.c (COM DIFICULDADE PROGRESSIVA)
 * ========================================
 */

// ... (includes: screen.h, keyboard.h, timer.h, game.h, stdio.h, unistd.h, fcntl.h) ...
#include "screen.h"
#include "keyboard.h"
#include "timer.h"
#include "game.h"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h> // Para malloc(), free() e rand()
#include <time.h>   // Para srand()
#include <string.h> // Para memset()

// --- Constantes do Jogo ---
#define LARGURA_TELA 80
#define ALTURA_TELA 24
#define PISO (ALTURA_TELA - 2)
#define GRAVIDADE 0.25f
#define FORCA_PULO -1.0f

// Constantes dos Obstáculos
#define ART_TERRESTRE "#" 
#define ART_AEREO "@"
#define LARGURA_BURACO 8

// <-- NOVO: Constantes de Dificuldade -->
#define INTERVALO_SPAWN_INICIAL 100 // Começa com 1 obstáculo a cada 100 frames
#define PONTOS_PARA_SUBIR_NIVEL 200  // A cada 200 pontos, aumenta a dificuldade
#define INTERVALO_SPAWN_MINIMO 40   // Limite máximo de frequência

// ... (Funções limparBuffer e imprimirBuffer permanecem iguais) ...
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

// ... (Função adicionarObstaculo permanece igual) ...
void adicionarObstaculo(NoObstaculo** lista) {
    Obstaculo* novoObs = (Obstaculo*)malloc(sizeof(Obstaculo));
    if (novoObs == NULL) return; 

    NoObstaculo* novoNo = (NoObstaculo*)malloc(sizeof(NoObstaculo));
    if (novoNo == NULL) {
        free(novoObs); 
        return;
    }

    novoObs->x = LARGURA_TELA - 2; 
    int tipo = rand() % 3;
    novoObs->tipo = tipo;

    if (tipo == 0) { // Terrestre
        novoObs->y = PISO;
        novoObs->largura = 1;
        novoObs->arteASCII = ART_TERRESTRE;
    } else if (tipo == 1) { // Aéreo
        novoObs->y = PISO - 3; 
        novoObs->largura = 1;
        novoObs->arteASCII = ART_AEREO;
    } else { // Buraco
        novoObs->y = PISO + 1;
        novoObs->largura = LARGURA_BURACO;
        novoObs->arteASCII = " ";
    }

    novoNo->obstaculo = novoObs;
    novoNo->proximo = *lista;
    *lista = novoNo;
}


int main() {
    
    // --- 1. Inicialização ---
    EstadoJogo estado;
    Pinguim pinguim;
    char telaBuffer[ALTURA_TELA][LARGURA_TELA]; 
    srand(time(NULL));

    // <-- NOVO: Variáveis de Dificuldade -->
    int contadorSpawn = 0;
    int intervaloSpawnAtual = INTERVALO_SPAWN_INICIAL;
    int proximoNivelPontuacao = PONTOS_PARA_SUBIR_NIVEL;

    screenInit(1);
    keyboardInit();
    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);

    estado.rodando = 1;
    estado.pontuacao = 0;
    estado.velocidadeJogo = 1.0f; // <-- NOVO: Velocidade inicial
    estado.listaDeObstaculos = NULL;

    pinguim.x = 5;
    pinguim.y = 10; 
    pinguim.velocidade_y = 0.0f;
    pinguim.estaNoChao = 0;
    pinguim.puloDuploDisponivel = 1;
    pinguim.arteASCII = "P"; 

    // --- 2. Game Loop ---
    while (estado.rodando) {
        
        // --- 2.1 Processar Inputs ---
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
        
        // --- 2.2 Atualizar Lógica ---
        
        // (Gravidade do Pinguim)
        pinguim.velocidade_y = pinguim.velocidade_y + GRAVIDADE;
        pinguim.y = (int)(pinguim.y + pinguim.velocidade_y);
        if (pinguim.y >= PISO) {
            pinguim.y = PISO;
            pinguim.velocidade_y = 0;
            pinguim.estaNoChao = 1;
            pinguim.puloDuploDisponivel = 1;
        }

        // <-- NOVO: Atualiza Pontuação e Dificuldade -->
        estado.pontuacao++; // Ganha 1 ponto por frame

        if (estado.pontuacao > proximoNivelPontuacao) {
            // 1. Aumenta a velocidade (quantidade)
            if (estado.velocidadeJogo < 3.0f) { // Limite de velocidade
                estado.velocidadeJogo += 0.2f; 
            }
            
            // 2. Aumenta a frequência (diminui o tempo)
            if (intervaloSpawnAtual > INTERVALO_SPAWN_MINIMO) {
                intervaloSpawnAtual -= 5; // Fica 5 frames mais rápido
            }
            
            // Define o próximo "marco"
            proximoNivelPontuacao += PONTOS_PARA_SUBIR_NIVEL;
        }


        // <-- NOVO: Lógica de Spawn (usa a variável) -->
        contadorSpawn++;
        if (contadorSpawn > intervaloSpawnAtual) { // Usa a variável
            adicionarObstaculo(&estado.listaDeObstaculos);
            contadorSpawn = 0;
        }

        // Mover Obstáculos (usa a velocidade variável)
        NoObstaculo* atual = estado.listaDeObstaculos;
        NoObstaculo* anterior = NULL;

        while (atual != NULL) {
            // Move o obstáculo (agora usa a velocidade do estado)
            atual->obstaculo->x -= (int)estado.velocidadeJogo; 

            if (atual->obstaculo->x + atual->obstaculo->largura < 0) {
                NoObstaculo* noParaRemover = atual;
                if (anterior == NULL) { 
                    estado.listaDeObstaculos = atual->proximo;
                } else { 
                    anterior->proximo = atual->proximo;
                }
                atual = atual->proximo; 
                
                free(noParaRemover->obstaculo);
                free(noParaRemover);
            } else {
                anterior = atual;
                atual = atual->proximo;
            }
        }

        
        // --- 2.3 Renderizar (Usando a Matriz Buffer) ---
        limparBuffer(telaBuffer);

        // 1. Desenha Pinguim
        telaBuffer[pinguim.y][pinguim.x] = *pinguim.arteASCII; 

        // 2. Desenha Obstáculos
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

        // 3. Desenha o Chão
        for (int x = 0; x < LARGURA_TELA; x++) {
            if (telaBuffer[PISO + 1][x] == ' ') { 
                telaBuffer[PISO + 1][x] = '-';
            }
        }
        
        // 4. Desenha a Pontuação (HUD)
        char hud[LARGURA_TELA];
        // <-- NOVO: Mostra a velocidade e o tempo de spawn para debug -->
        sprintf(hud, "Pontos: %d | Velocidade: %.1f | Spawn a cada: %d (Aperte 'q')", 
                estado.pontuacao, estado.velocidadeJogo, intervaloSpawnAtual);
                
        for(int i = 0; hud[i] != '\0' && i < LARGURA_TELA; i++) {
            telaBuffer[0][i] = hud[i];
        }

        // 5. Imprime o Buffer
        imprimirBuffer(telaBuffer);

        // --- 2.4 Controle de "Frame Rate" ---
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

    printf("Game Over! Pontuação Final: %d\n", estado.pontuacao);

    return 0;
}





