/*
 * ========================================
 * ARQUIVO DE LÓGICA: src/game.c (CORREÇÃO FINAL DE PLACEHOLDERS)
 * ========================================
 */

#include "game.h"
#include <stdlib.h> 
#include <string.h> 
#include <stdio.h>  
#include <time.h>   

// --- Constantes do Jogo (Permanecem aqui) ---
#define LARGURA_TELA 800
#define ALTURA_TELA 450
#define PISO (ALTURA_TELA - 40)
#define GRAVIDADE 0.25f
#define FORCA_PULO -8.0f 
#define LARGURA_BURACO 80
#define ARQUIVO_SCORES "highscores.txt"

// --- Funções de High Score (Não Mudam) ---
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

    int obsLargura = OBSTACULO_LARGURA_BASE;
    int obsAltura = OBSTACULO_ALTURA_BASE;

    if (tipo == 3) { // Terrestre 2x1
        obsAltura = OBSTACULO_ALTURA_BASE * 2;
        
        Obstaculo* obsBaixo = (Obstaculo*)malloc(sizeof(Obstaculo));
        NoObstaculo* noBaixo = (NoObstaculo*)malloc(sizeof(NoObstaculo));
        if (obsBaixo == NULL || noBaixo == NULL) { return; }
        obsBaixo->hitbox.x = LARGURA_TELA + 5;
        obsBaixo->hitbox.y = PISO - OBSTACULO_ALTURA_BASE;
        obsBaixo->hitbox.width = obsLargura;
        obsBaixo->hitbox.height = OBSTACULO_ALTURA_BASE;
        obsBaixo->textura = estado->texObstaculoTerrestre; 
        obsBaixo->tipo = 0;
        noBaixo->obstaculo = obsBaixo;
        noBaixo->proximo = *lista;
        *lista = noBaixo;

        Obstaculo* obsCima = (Obstaculo*)malloc(sizeof(Obstaculo));
        NoObstaculo* noCima = (NoObstaculo*)malloc(sizeof(NoObstaculo));
        if (obsCima == NULL || noCima == NULL) { return; }
        obsCima->hitbox.x = LARGURA_TELA + 5;
        obsCima->hitbox.y = PISO - (OBSTACULO_ALTURA_BASE * 2);
        obsCima->hitbox.width = obsLargura;
        obsCima->hitbox.height = OBSTACULO_ALTURA_BASE;
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
        novoObs->hitbox.y = PISO - obsAltura;
        novoObs->hitbox.width = obsLargura;
        novoObs->hitbox.height = obsAltura;
        novoObs->textura = estado->texObstaculoTerrestre;
        novoObs->tipo = 0;
    } 
    else if (tipo == 1) { // Aéreo (Várias Alturas)
        novoObs->hitbox.width = obsLargura;
        novoObs->hitbox.height = obsAltura;
        novoObs->textura = estado->texObstaculoAereo;
        novoObs->tipo = 0;
        int alturaSorteada = rand() % 3;
        if (alturaSorteada == 0) { novoObs->hitbox.y = PISO - 120; }
        else if (alturaSorteada == 1) { novoObs->hitbox.y = PISO - 100; }
        else { novoObs->hitbox.y = PISO - 80; }
    } 
    else if (tipo == 2) { // Buraco (Normal ou Grande)
        novoObs->hitbox.y = PISO; 
        novoObs->hitbox.height = 40;
        novoObs->tipo = 2; 
        novoObs->hitbox.width = (pontuacao > 650) ? (int)(LARGURA_BURACO * 1.5) : LARGURA_BURACO;
    }
    else if (tipo == 4) { // Terrestre 3# Vertical
        obsAltura = OBSTACULO_ALTURA_BASE * 3;
        novoObs->hitbox.y = PISO - obsAltura;
        novoObs->hitbox.width = obsLargura;
        novoObs->hitbox.height = obsAltura;
        novoObs->textura = estado->texObstaculoVertical;
        novoObs->tipo = 0;
    }

    novoNo->obstaculo = novoObs;
    novoNo->proximo = *lista;
    *lista = novoNo;
}


// --- Funções Principais do Jogo (O restante não muda) ---

void InitGame(EstadoJogo* estado, Pinguim* pinguim) {
    srand(time(NULL));

    // Carrega texturas (caminhos finais)
    estado->texPinguimAndando = LoadTexture("imagens_jogo/pengoo/pengoo_surfando.png");
    estado->texPinguimPulando = LoadTexture("imagens_jogo/pengoo/pengoo_pulando.png");
    estado->texObstaculoTerrestre = LoadTexture("imagens_jogo/inimigos/obstaculo_terrestre_1x1.png");
    estado->texObstaculoAereo = LoadTexture("imagens_jogo/inimigos/obstaculo_aereo_1x1.png");
    estado->texObstaculoVertical = LoadTexture("imagens_jogo/inimigos/obstaculo_terrestre_3x1.png");
    
    // Inicialização de estado
    estado->rodando = true;
    estado->pontuacao = 0;
    estado->velocidadeJogo = 2.5f;
    estado->listaDeObstaculos = NULL;
    estado->contadorSpawn = 0.0f;
    estado->intervaloSpawnAtual = INTERVALO_SPAWN_INICIAL;
    estado->proximoNivelPontuacao = PONTOS_PARA_SUBIR_NIVEL;

    pinguim->position = (Vector2){ 50, PISO - PINGUIM_ALTURA_BASE };
    pinguim->velocidade_y = 0;
    pinguim->estaNoChao = true;
    pinguim->puloDuploDisponivel = true;
    
    pinguim->texAndando = estado->texPinguimAndando;
    pinguim->texPulando = estado->texPinguimPulando;
    
    pinguim->hitbox = (Rectangle){ 50, pinguim->position.y, PINGUIM_LARGURA_BASE, PINGUIM_ALTURA_BASE };

    carregarHighScores(estado->topScores);
}


void UpdateGame(EstadoJogo* estado, Pinguim* pinguim) {
    if (!estado->rodando) return;

    if (IsKeyPressed(KEY_Q)) { estado->rodando = false; }
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

    pinguim->velocidade_y += GRAVIDADE;
    pinguim->position.y += pinguim->velocidade_y;
    pinguim->hitbox.y = pinguim->position.y;

    if (pinguim->position.y >= PISO - pinguim->hitbox.height) {
        bool pinguimSobreBuraco = false;
        NoObstaculo* atual = estado->listaDeObstaculos;
        while (atual != NULL) {
            if (atual->obstaculo->tipo == 2) { 
                if (CheckCollisionRecs(pinguim->hitbox, atual->obstaculo->hitbox)) {
                    pinguimSobreBuraco = true;
                    break;
                }
            }
            atual = atual->proximo;
        }

        if (!pinguimSobreBuraco) {
            pinguim->position.y = PISO - pinguim->hitbox.height;
            pinguim->velocidade_y = 0;
            pinguim->estaNoChao = true;
            pinguim->puloDuploDisponivel = true;
        } 
    }

    if (pinguim->position.y > ALTURA_TELA) { estado->rodando = false; }

    estado->pontuacao++;
    if (estado->pontuacao > estado->proximoNivelPontuacao) {
        if (estado->velocidadeJogo < 10.0f) { estado->velocidadeJogo += 0.5f; }
        if (estado->intervaloSpawnAtual > INTERVALO_SPAWN_MINIMO) { estado->intervaloSpawnAtual -= 0.1f; }
        estado->proximoNivelPontuacao += PONTOS_PARA_SUBIR_NIVEL;
    }

    estado->contadorSpawn += GetFrameTime();
    if (estado->contadorSpawn >= estado->intervaloSpawnAtual) {
        adicionarObstaculo(&estado->listaDeObstaculos, estado->pontuacao, estado);
        estado->contadorSpawn = 0.0f;
    }

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

    atual = estado->listaDeObstaculos;
    while (atual != NULL) {
        Obstaculo* obs = atual->obstaculo;
        
        if (obs->tipo == 0) {
            if (CheckCollisionRecs(pinguim->hitbox, obs->hitbox)) {
                estado->rodando = false; 
            }
        }
        else if (obs->tipo == 2) {
            if (pinguim->estaNoChao && CheckCollisionRecs(pinguim->hitbox, obs->hitbox)) {
                estado->rodando = false; 
            }
        }
        atual = atual->proximo;
    }
}


void DrawGame(EstadoJogo* estado, Pinguim* pinguim) {
    ClearBackground(SKYBLUE); 
    
    DrawRectangle(0, PISO, LARGURA_TELA, 40, BLACK);
    
    // Desenha a TEXTURA do Pinguim
    Texture2D texAtual = pinguim->estaNoChao ? estado->texPinguimAndando : estado->texPinguimPulando;
    Rectangle sourceRect = { 0.0f, 0.0f, (float)texAtual.width, (float)texAtual.height }; 
    Rectangle destRect = { pinguim->position.x, pinguim->position.y, (float)PINGUIM_LARGURA_BASE, (float)PINGUIM_ALTURA_BASE };
    Vector2 origin = { 0, 0 };
    float rotation = 0.0f; 
    DrawTexturePro(texAtual, sourceRect, destRect, origin, rotation, WHITE);

    // --- MUDANÇA: Desenha os inimigos (com fallback) ---
    NoObstaculo* atual = estado->listaDeObstaculos;
    while (atual != NULL) {
        Obstaculo* obs = atual->obstaculo;
        
        // Se a textura carregou (ID != 0) E não for um buraco
        if (obs->tipo == 0) { 
            Texture2D obsTex = obs->textura;
            
            // Desenha a TEXTURA se ela foi carregada
            if (obsTex.id > 0) {
                 DrawTexturePro(obsTex, 
                               (Rectangle){0, 0, (float)obsTex.width, (float)obsTex.height},
                               obs->hitbox, 
                               origin, rotation, WHITE);
            } else {
                // FALLBACK: Desenha o placeholder PRETO se a textura FALHOU
                DrawRectangleRec(obs->hitbox, BLACK);
            }
        }
        else if (obs->tipo == 2) { 
            // Buraco
            DrawRectangleRec(obs->hitbox, RAYWHITE); 
        }
        
        atual = atual->proximo;
    }
    // --- FIM DA MUDANÇA ---


    char hud[200];
    sprintf(hud, "Pontos: %d | Velocidade: %.1f | RECORDE: %s %d",
            estado->pontuacao, estado->velocidadeJogo, estado->topScores[0].nome, estado->topScores[0].pontuacao);
    DrawText(hud, 10, 10, 20, BLACK);

    if (!estado->rodando) {
        DrawText("G A M E   O V E R", LARGURA_TELA/2 - MeasureText("G A M E   O V E R", 40)/2, ALTURA_TELA/2 - 20, 40, RED);
    }
}


void UnloadGame(EstadoJogo* estado, Pinguim* pinguim) {
    UnloadTexture(estado->texPinguimAndando);
    UnloadTexture(estado->texPinguimPulando);
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
