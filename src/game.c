/*
 * ========================================
 * ARQUIVO DE LÓGICA: src/game.c (VERSÃO FINAL COMPLETA E CORRIGIDA)
 * ========================================
 */

#include "game.h"
#include <stdlib.h> 
#include <string.h> 
#include <stdio.h>  
#include <time.h>   

// --- Constantes do Jogo ---
#define LARGURA_TELA 800
#define ALTURA_TELA 450
#define PISO (ALTURA_TELA - 40)
#define GRAVIDADE 0.25f
#define FORCA_PULO -8.0f 
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
    
    int tipo = 0; // Inicializa com um valor seguro

    // --- Lógica de Spawn do Power-Up (Prioridade Máxima) ---
    if (pontuacao >= PONTUACAO_MINIMA_POWERUP && estado->power_up_aereo_counter >= CICLE_SPAWN_POWERUP - 1) {
        tipo = 5; // Tipo 5 é Power-up
        estado->power_up_aereo_counter = 0; // Reseta o contador
    } 
    // --- Lógica de Spawn Normal (Se não for Power-Up) ---
    else {
        // Decide se é um obstáculo pré-650 ou pós-650
        if (pontuacao < 650) {
            tipo = rand() % 4; // 0, 1, 2, 3
        } else {
            // Pós-650: Aumenta a variedade (e inclui o tipo 4 - Terrestre 3#)
            tipo = rand() % 5; // 0, 1, 2, 3, 4
        }
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
        
        // --- NOVO: Conta apenas se for o AÉREO (tipo 1) ---
        estado->power_up_aereo_counter++; 
        // ---------------------------------------------------

        int alturaSorteada = rand() % 3;
        if (alturaSorteada == 0) { novoObs->hitbox.y = PISO - 120; }
        else if (alturaSorteada == 1) { novoObs->hitbox.y = PISO - 100; }
        else { novoObs->hitbox.y = PISO - 80; }
    } 
    else if (tipo == 2) { // Buraco (Normal ou Grande)
        novoObs->hitbox.y = PISO; 
        novoObs->hitbox.height = 40;
        novoObs->tipo = 2; 
        
        // Buracos de 4x ou 5x o tamanho do bloco (40px)
        int buracoLargura = LARGURA_BURACO_BLOCO * OBSTACULO_LARGURA_BASE; 
        if (pontuacao > 650) {
            buracoLargura = (LARGURA_BURACO_BLOCO + 1) * OBSTACULO_LARGURA_BASE; 
        }
        novoObs->hitbox.width = buracoLargura;
    }
    else if (tipo == 4) { // Terrestre 3# Vertical
        obsAltura = OBSTACULO_ALTURA_BASE * 3;
        novoObs->hitbox.y = PISO - obsAltura;
        novoObs->hitbox.width = obsLargura;
        novoObs->hitbox.height = obsAltura;
        novoObs->textura = estado->texObstaculoVertical;
        novoObs->tipo = 0;
    }
    else if (tipo == 5) { // Power-Up de Imortalidade (Prioridade)
        novoObs->hitbox.width = obsLargura;
        novoObs->hitbox.height = obsAltura;
        novoObs->textura = estado->texPowerUp; 
        novoObs->tipo = 1; // Tipo 1 = Power-up (Coletável)
        
        int alturaSorteada = rand() % 3;
        if (alturaSorteada == 0) { novoObs->hitbox.y = PISO - 140; }
        else if (alturaSorteada == 1) { novoObs->hitbox.y = PISO - 120; }
        else { novoObs->hitbox.y = PISO - 100; }
    }


    novoNo->obstaculo = novoObs;
    novoNo->proximo = *lista;
    *lista = novoNo;
}


// --- Funções Principais do Jogo (Ajuste de Altura e Init) ---

void InitGame(EstadoJogo* estado, Pinguim* pinguim) {
    srand(time(NULL));

    // Carrega texturas (caminhos finais)
    estado->texPinguimAndando = LoadTexture("imagens_jogo/pengoo/pengoo_surfando.png");
    estado->texPinguimPulando = LoadTexture("imagens_jogo/pengoo/pengoo_pulando.png");
    estado->texObstaculoTerrestre = LoadTexture("imagens_jogo/inimigos/obstaculo_terrestre_1x1.png");
    estado->texObstaculoAereo = LoadTexture("imagens_jogo/inimigos/obstaculo_aereo_1x1.png");
    estado->texObstaculoVertical = LoadTexture("imagens_jogo/inimigos/obstaculo_terrestre_3x1.png");
    
    // Carrega Texturas Douradas e Power-Up
    estado->texPowerUp = LoadTexture("imagens_jogo/inimigos/powerup_invencivel.png"); 
    estado->texPinguimGoldAndando = LoadTexture("imagens_jogo/pengoo/pengoogold_surfando.png");
    estado->texPinguimGoldPulando = LoadTexture("imagens_jogo/pengoo/pengoogold_pulando.png");
    
    // Aplica filtro de qualidade
    SetTextureFilter(estado->texPinguimAndando, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(estado->texPinguimPulando, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(estado->texObstaculoTerrestre, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(estado->texObstaculoAereo, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(estado->texObstaculoVertical, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(estado->texPowerUp, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(estado->texPinguimGoldAndando, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(estado->texPinguimGoldPulando, TEXTURE_FILTER_BILINEAR);
    
    // Inicialização de estado
    estado->rodando = true;
    estado->pontuacao = 0;
    estado->velocidadeJogo = 2.5f;
    estado->listaDeObstaculos = NULL;
    
    estado->contadorSpawn = 0.0f;
    estado->intervaloSpawnAtual = INTERVALO_SPAWN_INICIAL;
    estado->proximoNivelPontuacao = PONTOS_PARA_SUBIR_NIVEL;
    estado->power_up_aereo_counter = 0;

    // Inicializa o Pinguim (posição e hitbox)
    // CORREÇÃO: Posição Y deve ser PISO - ALTURA_BASE
    pinguim->position = (Vector2){ 50, PISO - PINGUIM_ALTURA_BASE };
    pinguim->velocidade_y = 0;
    pinguim->estaNoChao = true;
    pinguim->puloDuploDisponivel = true;
    
    pinguim->texAndando = estado->texPinguimAndando;
    pinguim->texPulando = estado->texPinguimPulando;
    pinguim->texGoldAndando = estado->texPinguimGoldAndando;
    pinguim->texGoldPulando = estado->texPinguimGoldPulando;
    
    pinguim->imortal = false;
    pinguim->imortal_distancia_restante = 0;
    
    // Hitbox: 50x55 pixels
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

    // Lógica de pouso usa a constante correta
    if (pinguim->position.y >= PISO - PINGUIM_ALTURA_BASE) {
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
            pinguim->position.y = PISO - PINGUIM_ALTURA_BASE; // Trava na altura correta
            pinguim->velocidade_y = 0;
            pinguim->estaNoChao = true;
            pinguim->puloDuploDisponivel = true;
        } 
    }

    if (pinguim->position.y > ALTURA_TELA) { estado->rodando = false; }

    estado->pontuacao++;
    
    // Lógica de Duração da Imortalidade
    if (pinguim->imortal) {
        pinguim->imortal_distancia_restante--;
        if (pinguim->imortal_distancia_restante <= 0) {
            pinguim->imortal = false;
        }
    }
    
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
        
        if (obs->tipo == 1) { 
             if (CheckCollisionRecs(pinguim->hitbox, obs->hitbox)) {
                pinguim->imortal = true;
                pinguim->imortal_distancia_restante = DISTANCIA_IMORTALIDADE;
                obs->tipo = -1; // Marca para remoção
             }
        }
        
        if (obs->tipo == 0) { 
            if (CheckCollisionRecs(pinguim->hitbox, obs->hitbox)) {
                if (!pinguim->imortal) {
                    estado->rodando = false; // Game Over
                } else {
                    obs->tipo = -1; // Marca para remoção
                }
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
    
    // Desenha o Pinguim
    Texture2D texAtual;
    
    if (pinguim->imortal) {
        texAtual = pinguim->estaNoChao ? estado->texPinguimGoldAndando : estado->texPinguimGoldPulando;
    } else {
        texAtual = pinguim->estaNoChao ? estado->texPinguimAndando : estado->texPinguimPulando;
    }
    
    Rectangle sourceRect = { 0.0f, 0.0f, (float)texAtual.width, (float)texAtual.height }; 
    Rectangle destRect = { pinguim->position.x, pinguim->position.y, (float)PINGUIM_LARGURA_BASE, (float)PINGUIM_ALTURA_BASE };
    
    Vector2 origin = { 0, 0 };
    float rotation = 0.0f; 
    
    DrawTexturePro(texAtual, sourceRect, destRect, origin, rotation, WHITE);

    // Desenha os obstáculos (com fallback)
    NoObstaculo* atual = estado->listaDeObstaculos;
    while (atual != NULL) {
        Obstaculo* obs = atual->obstaculo;
        
        if (obs->tipo == 0 || obs->tipo == 1) { // Sólido e Power-Up
            Texture2D obsTex = obs->textura;
            
            Color fallbackColor = (obs->tipo == 1) ? YELLOW : BLACK;
            
            if (obsTex.id > 0) { 
                 DrawTexturePro(obsTex, 
                               (Rectangle){0, 0, (float)obsTex.width, (float)obsTex.height},
                               obs->hitbox, 
                               origin, rotation, WHITE);
            } else {
                DrawRectangleRec(obs->hitbox, fallbackColor);
            }
        }
        else if (obs->tipo == 2) { // Buraco
            DrawRectangleRec(obs->hitbox, RAYWHITE); 
        }
        
        atual = atual->proximo;
    }

    // Texto do HUD
    char hud[200];
    char imortal_status[30] = "";
    
    if (pinguim->imortal) {
        sprintf(imortal_status, " | IMORTAL! (%d)", pinguim->imortal_distancia_restante);
    }
    
    sprintf(hud, "Pontos: %d | Velocidade: %.1f%s | RECORDE: %s %d",
            estado->pontuacao, estado->velocidadeJogo, imortal_status, estado->topScores[0].nome, estado->topScores[0].pontuacao);
            
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
    UnloadTexture(estado->texPowerUp);
    UnloadTexture(estado->texPinguimGoldAndando);
    UnloadTexture(estado->texPinguimGoldPulando);
    
    NoObstaculo* atual = estado->listaDeObstaculos;
    while (atual != NULL) {
        NoObstaculo* proximo = atual->proximo;
        free(atual->obstaculo);
        free(atual);
        atual = proximo;
    }
}
