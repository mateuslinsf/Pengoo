/*
 * ========================================
 * ARQUIVO PRINCIPAL: src/main.c (VERSÃO FINAL COMPLETA - INIMIGO VERTICAL)
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
// <-- MUDANÇA: ART_TERRESTRE_3 não é mais usado diretamente como string -->
#define LARGURA_BURACO 8

// Constantes de Dificuldade
#define INTERVALO_SPAWN_INICIAL 100 
#define PONTOS_PARA_SUBIR_NIVEL 200  
#define INTERVALO_SPAWN_MINIMO 40   
#define ARQUIVO_SCORES "highscores.txt"

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
void adicionarObstaculo(NoObstaculo** lista, int pontuacao) {
    int tipo;
    if (pontuacao < 650) {
        tipo = rand() % 4; // 0=Terrestre 1#, 1=Aéreo, 2=Buraco 8, 3=Terrestre 2x1
    } else {
        tipo = rand() % 4; // 0=Aéreo, 1=Buraco 12, 2=Terrestre 2x1, 3=Terrestre 3# (vertical)
        if (tipo == 0) { tipo = 1; }
        else if (tipo == 1) { tipo = 2; }
        else if (tipo == 2) { tipo = 3; }
        else if (tipo == 3) { tipo = 4; } // Novo tipo 4 (Terrestre 3# Vertical)
    }

    if (tipo == 3) { // Terrestre 2x1
        Obstaculo* obsBaixo = (Obstaculo*)malloc(sizeof(Obstaculo));
        NoObstaculo* noBaixo = (NoObstaculo*)malloc(sizeof(NoObstaculo));
        if (obsBaixo == NULL || noBaixo == NULL) { return; }
        obsBaixo->x = LARGURA_TELA - 2;
        obsBaixo->y = PISO;
        obsBaixo->largura = 1;
        obsBaixo->arteASCII = ART_TERRESTRE;
        obsBaixo->tipo = 0; // Sólido
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
        obsCima->tipo = 0; // Sólido
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
    novoObs->tipo = tipo; // Tipo de obstáculo (0=sólido, 2=buraco, 4=3# vertical)

    if (tipo == 0) { // Terrestre (Simples 1#)
        novoObs->y = PISO;
        novoObs->largura = 1;
        novoObs->arteASCII = ART_TERRESTRE;
    } 
    else if (tipo == 1) { // Aéreo (Várias Alturas)
        novoObs->largura = 1;
        novoObs->arteASCII = ART_AEREO;
        int alturaSorteada = rand() % 3;
        if (alturaSorteada == 0) { novoObs->y = PISO - 3; }
        else if (alturaSorteada == 1) { novoObs->y = PISO - 2; }
        else { novoObs->y = PISO - 1; }
    } 
    else if (tipo == 2) { // Buraco (Normal ou Grande)
        novoObs->y = PISO + 1; // Posição Y fora da tela (abaixo do chão)
        novoObs->arteASCII = " "; // Arte invisível
        novoObs->largura = (pontuacao > 650) ? (int)(LARGURA_BURACO * 1.5) : LARGURA_BURACO;
    }
    // <-- MUDANÇA: Lógica para o Terrestre 3# Vertical -->
    else if (tipo == 4) { // Terrestre 3# Vertical
        novoObs->y = PISO;       // Posição Y da base
        novoObs->largura = 1;    // Largura de 1 (pois é vertical)
        novoObs->altura = 3;     // Altura de 3 caracteres
        novoObs->arteASCII = ART_TERRESTRE; // Usamos o mesmo '#'
        // A colisão e o desenho vão iterar pela altura
    }

    novoNo->obstaculo = novoObs;
    novoNo->proximo = *lista;
    *lista = novoNo;
}


// --- Funções de Leitura/Escrita de Arquivo (High Score) ---
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
            // int obsY = obs->y; // Não usamos mais obsY diretamente para colisao vertical

            // Colisão Horizontal (todos os sólidos)
            if ( (obsX_inicio <= pinguimX) && ((obsX_inicio + velocidadeInt) > pinguimX) ) {
                if (obs->tipo == 0) { // Obstáculo sólido (como #, @, e o novo 3# vertical)
                    // <-- MUDANÇA: Colisão com 3# vertical -->
                    // Verifica se o pinguim está na altura do obstáculo.
                    // Se o obstáculo tem altura (tipo 4), verifica todas as células.
                    if (obs->tipo == 4) { // É o obstáculo 3# vertical
                        for (int h = 0; h < obs->altura; h++) {
                            if (pinguimY == (obs->y - h)) { // Verifica de baixo para cima
                                estado.rodando = 0; // Game Over
                            }
                        }
                    } else { // Obstáculos sólidos normais (1x1 ou 2x1)
                        if (pinguimY == obs->y) {
                            estado.rodando = 0; // Game Over
                        }
                    }
                }
            }
            
            // Colisão com Buraco (Tipo 2)
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
            // <-- MUDANÇA: Desenho do 3# vertical -->
            if (obs->tipo == 4) { // Se for o obstáculo 3# vertical
                for (int h = 0; h < obs->altura; h++) {
                    int posY = obs->y - h; // Desenha de baixo para cima
                    if (obs->x >= 0 && obs->x < LARGURA_TELA && posY >= 0 && posY < ALTURA_TELA) {
                        telaBuffer[posY][obs->x] = *obs->arteASCII; // Desenha '#'
                    }
                }
            } else { // Outros obstáculos (horizontal, buraco)
                char* arte = obs->arteASCII; 
                for (int i = 0; i < obs->largura; i++) {
                    int posX = obs->x + i;
                    if (posX >= 0 && posX < LARGURA_TELA) {
                        telaBuffer[obs->y][posX] = *arte;
                    }
                }
            }
            atual = atual->proximo;
        }
        
        // 4. Desenha a Pontuação (HUD)
        char hud[LARGURA_TELA];
        sprintf(hud, "Pontos: %d | Velocidade: %.1f | RECORDE: %s %d (Aperte 'q')", 
                estado.pontuacao, estado.velocidadeJogo, estado.topScores[0].nome, estado.topScores[0].pontuacao);
                
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

    // (Restaura o terminal para o scanf)
    fcntl(STDIN_FILENO, F_SETFL, old_flags);

    // (Lógica de Fim de Jogo com Ranking)
    int ranking = obterRanking(estado.topScores, estado.pontuacao);
    if (ranking > 0) {
        printf("\n--- NOVO RECORDE! ---\n");
        printf("Sua pontuação: %d\n", estado.pontuacao);
        
        if (ranking == 1) { printf("Você ficou em 1º LUGAR!\n"); }
        else if (ranking == 2) { printf("Você ficou em 2º LUGAR!\n"); }
        else { printf("Você ficou em 3º LUGAR!\n"); }

        printf("Digite suas 3 iniciais (ex: LFG): ");
        char nome[10];
        scanf("%3s", nome);
        
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
