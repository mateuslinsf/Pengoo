/*
 * ========================================
 * ARQUIVO PRINCIPAL: src/main.c (COM INIMIGOS VARIADOS)
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

// Constantes de Dificuldade
#define INTERVALO_SPAWN_INICIAL 100 
#define PONTOS_PARA_SUBIR_NIVEL 200  
#define INTERVALO_SPAWN_MINIMO 40   

// --- Funções do Buffer de Tela (Matriz) ---
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

// --- Funções da Lista Encadeada (Obstáculos) ---
// <-- MUDANÇA AQUI: Função agora recebe a pontuação -->
void adicionarObstaculo(NoObstaculo** lista, int pontuacao) {
    
    // <-- MUDANÇA: Agora sorteia 4 tipos (0=Terrestre, 1=Aéreo, 2=Buraco, 3=Terrestre 2x1) -->
    int tipo = rand() % 4; 

    // --- Pedido 2: Obstáculo Terrestre 2x1 (1 de largura, 2 de altura) ---
    if (tipo == 3) {
        // Criar o obstáculo de baixo
        Obstaculo* obsBaixo = (Obstaculo*)malloc(sizeof(Obstaculo));
        NoObstaculo* noBaixo = (NoObstaculo*)malloc(sizeof(NoObstaculo));
        if (obsBaixo == NULL || noBaixo == NULL) { /* Falha na alocação */ return; }
        
        obsBaixo->x = LARGURA_TELA - 2;
        obsBaixo->y = PISO; // No chão
        obsBaixo->largura = 1;
        obsBaixo->arteASCII = ART_TERRESTRE;
        obsBaixo->tipo = 0; // Tipo 0 (sólido) para colisão
        
        noBaixo->obstaculo = obsBaixo;
        noBaixo->proximo = *lista; // Adiciona na lista
        *lista = noBaixo;

        // Criar o obstáculo de cima
        Obstaculo* obsCima = (Obstaculo*)malloc(sizeof(Obstaculo));
        NoObstaculo* noCima = (NoObstaculo*)malloc(sizeof(NoObstaculo));
        if (obsCima == NULL || noCima == NULL) { /* Falha na alocação */ return; }
        
        obsCima->x = LARGURA_TELA - 2;
        obsCima->y = PISO - 1; // Logo acima do chão
        obsCima->largura = 1;
        obsCima->arteASCII = ART_TERRESTRE;
        obsCima->tipo = 0; // Tipo 0 (sólido) para colisão
        
        noCima->obstaculo = obsCima;
        noCima->proximo = *lista; // Adiciona na lista
        *lista = noCima;
        
        return; // Já criamos os dois obstáculos, podemos sair
    }

    // --- Se não for 2x1, é um obstáculo normal ---
    
    // Alocação Dinâmica (Obstáculo)
    Obstaculo* novoObs = (Obstaculo*)malloc(sizeof(Obstaculo));
    if (novoObs == NULL) return; 

    // Alocação Dinâmica (Nó da Lista)
    NoObstaculo* novoNo = (NoObstaculo*)malloc(sizeof(NoObstaculo));
    if (novoNo == NULL) {
        free(novoObs); 
        return;
    }

    // Configuração do Obstáculo
    novoObs->x = LARGURA_TELA - 2; 
    novoObs->tipo = tipo;

    if (tipo == 0) { // Tipo 0: Terrestre (Simples)
        novoObs->y = PISO;
        novoObs->largura = 1;
        novoObs->arteASCII = ART_TERRESTRE;
    } 
    else if (tipo == 1) { // --- Pedido 1: Aéreo (Várias Alturas) ---
        novoObs->largura = 1;
        novoObs->arteASCII = ART_AEREO;
        
        int alturaSorteada = rand() % 3; // Sorteia 0, 1, ou 2
        if (alturaSorteada == 0) {
            novoObs->y = PISO - 3; // Original (alto)
        } else if (alturaSorteada == 1) {
            novoObs->y = PISO - 2; // "um pouco mais abaixo"
        } else {
            novoObs->y = PISO - 1; // "um pouco mais abaixo ainda"
        }
    } 
    else if (tipo == 2) { // --- Pedido 3 & 4: Buraco (Normal ou Grande) ---
        novoObs->y = PISO + 1; // Posição do chão
        novoObs->arteASCII = " "; // Buracos são ' '

        // Checa a pontuação para decidir o tamanho
        if (pontuacao > 650) {
            novoObs->largura = (int)(LARGURA_BURACO * 1.5); // 8 * 1.5 = 12
        } else {
            novoObs->largura = LARGURA_BURACO; // 8
        }
    }

    // Adiciona na Lista Encadeada
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

    int contadorSpawn = 0;
    int intervaloSpawnAtual = INTERVALO_SPAWN_INICIAL;
    int proximoNivelPontuacao = PONTOS_PARA_SUBIR_NIVEL;

    screenInit(1);
    keyboardInit();
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

        // (Atualiza Pontuação e Dificuldade)
        estado.pontuacao++; 
        if (estado.pontuacao > proximoNivelPontuacao) {
            if (estado.velocidadeJogo < 3.0f) { estado.velocidadeJogo += 0.2f; }
            if (intervaloSpawnAtual > INTERVALO_SPAWN_MINIMO) { intervaloSpawnAtual -= 5; }
            proximoNivelPontuacao += PONTOS_PARA_SUBIR_NIVEL;
        }

        // (Lógica de Spawn)
        contadorSpawn++;
        if (contadorSpawn > intervaloSpawnAtual) {
            // <-- MUDANÇA AQUI: Passa a pontuação para a função -->
            adicionarObstaculo(&estado.listaDeObstaculos, estado.pontuacao);
            contadorSpawn = 0;
        }

        // (Mover Obstáculos e Limpar)
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

        // (Detecção de Colisão Robusta)
        atual = estado.listaDeObstaculos;
        while (atual != NULL) {
            Obstaculo* obs = atual->obstaculo;
            int velocidadeInt = (int)estado.velocidadeJogo;
            int pinguimX = pinguim.x; 
            int pinguimY = pinguim.y;
            int obsX_inicio = obs->x;
            int obsX_fim = obs->x + obs->largura;
            int obsY = obs->y;

            if ( (obsX_inicio <= pinguimX) && 
                 ((obsX_inicio + velocidadeInt) > pinguimX) ) 
            {
                if ( (obs->tipo == 0) && (pinguimY == obsY) ) { // Tipo 0 = Sólido (qualquer altura)
                    estado.rodando = 0; // Game Over
                }
            }

            if (obs->tipo == 2) {
                if ( (pinguimY == PISO) && 
                     (pinguimX >= obsX_inicio) && 
                     (pinguimX < obsX_fim) ) 
                {
                    estado.rodando = 0; // Game Over
                }
            }
            atual = atual->proximo;
        }

        
        // --- 2.3 Renderizar (Usando a Matriz Buffer) ---
        limparBuffer(telaBuffer);

        // 1. Desenha Pinguim
        telaBuffer[pinguim.y][pinguim.x] = *pinguim.arteASCII; 

        // 2. Desenha o Chão (PRIMEIRO)
        for (int x = 0; x < LARGURA_TELA; x++) {
            telaBuffer[PISO + 1][x] = '-';
        }

        // 3. Desenha os Obstáculos (DEPOIS)
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
        
        // 4. Desenha a Pontuação (HUD)
        char hud[LARGURA_TELA];
        sprintf(hud, "Pontos: %d | Velocidade: %.1f | Spawn a cada: %d (Aperte 'q')", 
                estado.pontuacao, estado.velocidadeJogo, intervaloSpawnAtual);
                
        for(int i = 0; hud[i] != '\0' && i < LARGURA_TELA; i++) {
            telaBuffer[0][i] = hud[i];
        }

        // (Mensagem de Game Over)
        if (estado.rodando == 0) {
            char* gameOverMsg = "G A M E   O V E R";
            int startX = (LARGURA_TELA - strlen(gameOverMsg)) / 2;
            int startY = ALTURA_TELA / 2;
            for(int i = 0; gameOverMsg[i] != '\0'; i++) {
                telaBuffer[startY][startX + i] = gameOverMsg[i];
            }
        }

        // 5. Imprime o Buffer
        imprimirBuffer(telaBuffer);
        
        if (estado.rodando == 0) {
            usleep(2000000); // Pausa por 2s
        }

        // --- 2.4 Controle de "Frame Rate" ---
        usleep(33000); 
    }

    // --- 3. Finalização ---
    // (Libera memória da lista encadeada)
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
