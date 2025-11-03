/*
 * ========================================
 * ARQUIVO DE LÓGICA: src/game.c (VERSÃO ESTÁVEL - COLISÃO SIMPLES)
 * ========================================
 */

#include "game.h"
#include <stdlib.h> // Para malloc, free, rand
#include <string.h> // Para strcpy, memset
#include <stdio.h>  // Para FILE, fopen, etc.
#include <time.h>   // Para srand

// --- Constantes do Jogo ---
#define LARGURA_TELA 800
#define ALTURA_TELA 450
#define PISO (ALTURA_TELA - 40)
#define GRAVIDADE 0.25f
#define FORCA_PULO -8.0f

// Constantes dos Obstáculos
#define LARGURA_BURACO 80
#define OBSTACULO_LARGURA 30
#define OBSTACULO_ALTURA 30

// Constantes de Dificuldade
#define INTERVALO_SPAWN_INICIAL 3.0f // (O seu ajuste de spawn lento)
#define PONTOS_PARA_SUBIR_NIVEL 200
#define INTERVALO_SPAWN_MINIMO 0.7f
#define ARQUIVO_SCORES "highscores.txt"

// --- Funções de High Score (Lógica Pura - Sem Mudanças) ---
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


// --- Funções da Lista Encadeada (Obstáculos) ---
void adicionarObstaculo(NoObstaculo** lista, int pontuacao, EstadoJogo* estado) {
    int tipo;
    if (pontuacao < 650) {
        tipo = rand() % 4;
    } else {
        tipo = rand() % 4;
        if (tipo == 0) { tipo = 1; }
        else if (tipo == 1) { tipo = 2; }
        else if (tipo == 2) { tipo = 3; }
        else if (tipo == 3) { tipo = 4; }
    }

    if (tipo == 3) { // Terrestre 2x1
        Obstaculo* obsBaixo = (Obstaculo*)malloc(sizeof(Obstaculo));
        NoObstaculo* noBaixo = (NoObstaculo*)malloc(sizeof(NoObstaculo));
        if (obsBaixo == NULL || noBaixo == NULL) { return; }
        obsBaixo->hitbox.x = LARGURA_TELA + 5;
        obsBaixo->hitbox.y = PISO - OBSTACULO_ALTURA;
        obsBaixo->hitbox.width = OBSTACULO_LARGURA;
        obsBaixo->hitbox.height = OBSTACULO_ALTURA;
        obsBaixo->textura = estado->texObstaculoTerrestre;
        obsBaixo->tipo = 0; // Sólido
        noBaixo->obstaculo = obsBaixo;
        noBaixo->proximo = *lista;
        *lista = noBaixo;

        Obstaculo* obsCima = (Obstaculo*)malloc(sizeof(Obstaculo));
        NoObstaculo* noCima = (NoObstaculo*)malloc(sizeof(NoObstaculo));
        if (obsCima == NULL || noCima == NULL) { return; }
        obsCima->hitbox.x = LARGURA_TELA + 5;
        obsCima->hitbox.y = PISO - (OBSTACULO_ALTURA * 2);
        obsCima->hitbox.width = OBSTACULO_LARGURA;
        obsCima->hitbox.height = OBSTACULO_ALTURA;
        obsCima->textura = estado->texObstaculoTerrestre;
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

    novoObs->hitbox.x = LARGURA_TELA + 5;
    novoObs->tipo = tipo;

    if (tipo == 0) { // Terrestre (Simples 1#)
        novoObs->hitbox.y = PISO - OBSTACULO_ALTURA;
        novoObs->hitbox.width = OBSTACULO_LARGURA;
        novoObs->hitbox.height = OBSTACULO_ALTURA;
        novoObs->textura = estado->texObstaculoTerrestre;
        novoObs->tipo = 0;
    }
    else if (tipo == 1) { // Aéreo (Várias Alturas)
        novoObs->hitbox.width = OBSTACULO_LARGURA;
        novoObs->hitbox.height = OBSTACULO_ALTURA;
        novoObs->textura = estado->texObstaculoAereo;
        novoObs->tipo = 0;
        
        // (Ajuste da altura dos aéreos que você pediu)
        int alturaSorteada = rand() % 3;
        if (alturaSorteada == 0) { novoObs->hitbox.y = PISO - 120; }
        else if (alturaSorteada == 1) { novoObs->hitbox.y = PISO - 100; }
        else { novoObs->hitbox.y = PISO - 80; }
    }
    else if (tipo == 2) { // Buraco (Normal ou Grande)
        novoObs->hitbox.y = PISO; // Posição Y do buraco
        novoObs->hitbox.height = 40; // Altura do buraco (tamanho do chão)
        novoObs->tipo = 2; // Buraco (tratado como colisão)
        novoObs->hitbox.width = (pontuacao > 650) ? (int)(LARGURA_BURACO * 1.5) : LARGURA_BURACO;
    }
    else if (tipo == 4) { // Terrestre 3# Vertical
        novoObs->hitbox.y = PISO - (OBSTACULO_ALTURA * 3);
        novoObs->hitbox.width = OBSTACULO_LARGURA;
        novoObs->hitbox.height = OBSTACULO_ALTURA * 3;
        novoObs->textura = estado->texObstaculoVertical;
        novoObs->tipo = 0;
    }

    novoNo->obstaculo = novoObs;
    novoNo->proximo = *lista;
    *lista = novoNo;
}


// --- Funções Principais do Jogo (Tradução para Raylib) ---

void InitGame(EstadoJogo* estado, Pinguim* pinguim) {
    srand(time(NULL));

    estado->texPinguim = LoadTexture("pinguim.png");
    estado->texObstaculoTerrestre = LoadTexture("obstaculo_terrestre.png");
    estado->texObstaculoAereo = LoadTexture("obstaculo_aereo.png");
    estado->texObstaculoVertical = LoadTexture("obstaculo_vertical.png");

    estado->rodando = true;
    estado->pontuacao = 0;
    estado->velocidadeJogo = 2.5f;
    estado->listaDeObstaculos = NULL;
    estado->contadorSpawn = 0.0f;
    estado->intervaloSpawnAtual = INTERVALO_SPAWN_INICIAL;
    estado->proximoNivelPontuacao = PONTOS_PARA_SUBIR_NIVEL;

    pinguim->position = (Vector2){ 50, PISO - 32 };
    pinguim->velocidade_y = 0;
    pinguim->estaNoChao = true;
    pinguim->puloDuploDisponivel = true;
    pinguim->textura = estado->texPinguim;
    pinguim->hitbox = (Rectangle){ 50, pinguim->position.y, 32, 32 };

    carregarHighScores(estado->topScores);
}


// <-- MUDANÇA: LÓGICA DE FÍSICA E COLISÃO REVERTIDA PARA A VERSÃO ESTÁVEL -->
void UpdateGame(EstadoJogo* estado, Pinguim* pinguim) {
    if (!estado->rodando) return;

    // --- 1. Inputs ---
    if (IsKeyPressed(KEY_Q)) {
        estado->rodando = false;
    }
    if (IsKeyPressed(KEY_SPACE)) {
        if (pinguim->estaNoChao) {
            pinguim->velocidade_y = FORCA_PULO;
            pinguim->estaNoChao = false;
            pinguim->puloDuploDisponivel = true;
        } else if (pinguim->puloDuploDisponivel) {
            pinguim->velocidade_y = FORCA_PULO;
            pinguim->puloDuploDisponivel = false;
        }
    }

    // --- 2. Física da Gravidade (Lógica Antiga) ---
    pinguim->velocidade_y += GRAVIDADE;
    pinguim->position.y += pinguim->velocidade_y;
    pinguim->hitbox.y = pinguim->position.y;

    // (Trava o pinguim no chão)
    if (pinguim->position.y >= PISO - pinguim->hitbox.height) {
        pinguim->position.y = PISO - pinguim->hitbox.height;
        pinguim->velocidade_y = 0;
        pinguim->estaNoChao = true;
        pinguim->puloDuploDisponivel = true;
    }

    // --- 3. Atualiza Pontuação e Dificuldade ---
    estado->pontuacao++;
    if (estado->pontuacao > estado->proximoNivelPontuacao) {
        if (estado->velocidadeJogo < 10.0f) { estado->velocidadeJogo += 0.5f; }
        if (estado->intervaloSpawnAtual > INTERVALO_SPAWN_MINIMO) { estado->intervaloSpawnAtual -= 0.1f; }
        estado->proximoNivelPontuacao += PONTOS_PARA_SUBIR_NIVEL;
    }

    // --- 4. Lógica de Spawn ---
    estado->contadorSpawn += GetFrameTime();
    if (estado->contadorSpawn >= estado->intervaloSpawnAtual) {
        adicionarObstaculo(&estado->listaDeObstaculos, estado->pontuacao, estado);
        estado->contadorSpawn = 0.0f;
    }

    // --- 5. Mover Obstáculos e Limpar Lista ---
    NoObstaculo* atual = estado->listaDeObstaculos;
    NoObstaculo* anterior = NULL;
    while (atual != NULL) {
        atual->obstaculo->hitbox.x -= estado->velocidadeJogo;

        if (atual->obstaculo->hitbox.x + atual->obstaculo->hitbox.width < 0) {
            NoObstaculo* noParaRemover = atual;
            if (anterior == NULL) { estado->listaDeObstaculos = atual->proximo; }
            else { anterior->proximo = atual->proximo; }
            atual = atual->proximo;
            free(noParaRemover->obstaculo);
            free(noParaRemover);
        } else {
            anterior = atual;
            atual = atual->proximo;
        }
    }

    // --- 6. Colisão (Lógica Antiga, que funciona) ---
    atual = estado->listaDeObstaculos;
    while (atual != NULL) {
        Obstaculo* obs = atual->obstaculo;
        
        // Colisão com Sólidos (Tipo 0)
        if (obs->tipo == 0) {
            if (CheckCollisionRecs(pinguim->hitbox, obs->hitbox)) {
                estado->rodando = false; 
            }
        }
        // Colisão com Buraco (Tipo 2)
        else if (obs->tipo == 2) {
            // Se o pinguim ESTÁ NO CHÃO e toca o buraco, ele morre.
            if (pinguim->estaNoChao && CheckCollisionRecs(pinguim->hitbox, obs->hitbox)) {
                estado->rodando = false; 
            }
        }
        atual = atual->proximo;
    }
}


// --- Função de Desenho (sem mudanças) ---
void DrawGame(EstadoJogo* estado, Pinguim* pinguim) {
    BeginDrawing();

    ClearBackground(RAYWHITE);
    
    // Desenha o chão preto
    DrawRectangle(0, PISO, LARGURA_TELA, 40, BLACK);
    
    // Desenha o pinguim preto (placeholder)
    DrawRectangleRec(pinguim->hitbox, BLACK);

    // Desenha os obstáculos
    NoObstaculo* atual = estado->listaDeObstaculos;
    while (atual != NULL) {
        Obstaculo* obs = atual->obstaculo;
        if (obs->tipo == 0) { // Sólido
            DrawRectangleRec(obs->hitbox, BLACK);
        }
        else if (obs->tipo == 2) { // Buraco
            // Desenha um retângulo BRANCO para "apagar" o chão
            DrawRectangleRec(obs->hitbox, RAYWHITE); 
        }
        atual = atual->proximo;
    }

    // Desenha o HUD
    char hud[200];
    sprintf(hud, "Pontos: %d | Velocidade: %.1f | RECORDE: %s %d",
            estado->pontuacao, estado->velocidadeJogo, estado->topScores[0].nome, estado->topScores[0].pontuacao);
    DrawText(hud, 10, 10, 20, BLACK);

    // Mensagem de Game Over
    if (!estado->rodando) {
        DrawText("G A M E   O V E R", LARGURA_TELA/2 - MeasureText("G A M E   O V E R", 40)/2, ALTURA_TELA/2 - 20, 40, RED);
    }

    EndDrawing();
}


// --- Função de Limpeza (sem mudanças) ---
void UnloadGame(EstadoJogo* estado) {
    UnloadTexture(estado->texPinguim);
    UnloadTexture(estado->texObstaculoTerrestre);
    UnloadTexture(estado->texObstaculoAereo);
    UnloadTexture(estado->texObstaculoVertical);

    NoObstaculo* atual = estado->listaDeObstaculos;
    while (atual != NULL) {
        NoObstaculo* proximo = atual->proximo;
        free(atual->obstaculo);
        free(atual);
        atual = proximo;
    }
}
