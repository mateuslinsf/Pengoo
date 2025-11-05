/*
 * ========================================
 * ARQUIVO DE LÓGICA: src/game.c (VERSÃO FINAL ESTÁVEL COM LIMITE DE DIFICULDADE)
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
    int tipo = 0;
    
    // 1. Prioridade: Power-Up Imortalidade (Aéreo)
    if (pontuacao >= PONTUACAO_MINIMA_IMORTALIDADE && estado->power_up_aereo_counter >= CICLE_SPAWN_IMORTALIDADE) {
        tipo = 5; // Imortalidade (Amarelo)
        estado->power_up_aereo_counter = 0;
    }
    // 2. Spawn Normal
    else {
        if (pontuacao < 650) {
            tipo = rand() % 4; // 0, 1, 2, 3
        } else {
            tipo = rand() % 5; // 0, 1, 2, 3, 4 (Pós-650)
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
        
        if (tipo < 5) estado->power_up_terrestre_counter++; // Conta spawn se for obstáculo
        return;
    }

    Obstaculo* novoObs = (Obstaculo*)malloc(sizeof(Obstaculo));
    if (novoObs == NULL) return; 
    NoObstaculo* novoNo = (NoObstaculo*)malloc(sizeof(NoObstaculo));
    if (novoNo == NULL) { free(novoObs); return; }

    novoObs->hitbox.x = LARGURA_TELA + 5;
    novoObs->tipo = 0; 

    if (tipo == 0) { // Terrestre (Simples 1#)
        novoObs->hitbox.y = PISO - obsAltura;
        novoObs->hitbox.width = obsLargura;
        novoObs->hitbox.height = obsAltura;
        novoObs->textura = estado->texObstaculoTerrestre;
        if (tipo < 5) estado->power_up_terrestre_counter++; // Conta spawn se for obstáculo
    } 
    else if (tipo == 1) { // Aéreo (Várias Alturas)
        novoObs->hitbox.width = obsLargura;
        novoObs->hitbox.height = obsAltura;
        novoObs->textura = estado->texObstaculoAereo;
        
        estado->power_up_aereo_counter++; // Conta o spawn aéreo

        int alturaSorteada = rand() % 3;
        if (alturaSorteada == 0) { novoObs->hitbox.y = PISO - 120; }
        else if (alturaSorteada == 1) { novoObs->hitbox.y = PISO - 100; }
        else { novoObs->hitbox.y = PISO - 80; }
    } 
    else if (tipo == 2) { // Buraco 
        novoObs->hitbox.y = PISO; 
        novoObs->hitbox.height = 40;
        novoObs->tipo = 2; // Buraco
        
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
        if (tipo < 5) estado->power_up_terrestre_counter++; // Conta spawn se for obstáculo
    }
    else if (tipo == 5) { // Power-Up Imortalidade (Aéreo)
        novoObs->hitbox.width = obsLargura;
        novoObs->hitbox.height = obsAltura;
        novoObs->textura = estado->texPowerUpImortal; 
        novoObs->tipo = 1; // Power-up
        
        int alturaSorteada = rand() % 3;
        if (alturaSorteada == 0) { novoObs->hitbox.y = PISO - 140; }
        else if (alturaSorteada == 1) { novoObs->hitbox.y = PISO - 120; }
        else { novoObs->hitbox.y = PISO - 100; }
    }
    else if (tipo == 6) { // Tipo 6 (Fallback para não quebrar)
        novoObs->hitbox.y = PISO - obsAltura;
        novoObs->hitbox.width = obsLargura;
        novoObs->hitbox.height = obsAltura;
        novoObs->textura = estado->texObstaculoTerrestre; // Fallback
        novoObs->tipo = 0; 
    }


    novoNo->obstaculo = novoObs;
    novoNo->proximo = *lista;
    *lista = novoNo;
}


// --- Funções Principais do Jogo ---

void InitGame(EstadoJogo* estado, Pinguim* pinguim) {
    srand(time(NULL));

    // Carrega texturas
    estado->texPinguimAndando = LoadTexture("imagens_jogo/pengoo/pengoo_surfando.png");
    estado->texPinguimPulando = LoadTexture("imagens_jogo/pengoo/pengoo_pulando.png");
    estado->texObstaculoTerrestre = LoadTexture("imagens_jogo/inimigos/obstaculo_terrestre_1x1.png");
    estado->texObstaculoAereo = LoadTexture("imagens_jogo/inimigos/obstaculo_aereo_1x1.png");
    estado->texObstaculoVertical = LoadTexture("imagens_jogo/inimigos/obstaculo_terrestre_3x1.png");
    
    // Carrega Texturas Douradas e Power-Up
    estado->texPowerUpImortal = LoadTexture("imagens_jogo/inimigos/powerup_invencivel.png"); 
    estado->texPinguimGoldAndando = LoadTexture("imagens_jogo/pengoo/pengoogold_surfando.png");
    estado->texPinguimGoldPulando = LoadTexture("imagens_jogo/pengoo/pengoogold_pulando.png");
    
    // Aplica filtro de qualidade
    SetTextureFilter(estado->texPinguimAndando, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(estado->texPinguimPulando, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(estado->texObstaculoTerrestre, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(estado->texObstaculoAereo, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(estado->texObstaculoVertical, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(estado->texPowerUpImortal, TEXTURE_FILTER_BILINEAR); 
    SetTextureFilter(estado->texPinguimGoldAndando, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(estado->texPinguimGoldPulando, TEXTURE_FILTER_BILINEAR);
    
    // Inicialização de estado
    estado->rodando = true;
    estado->pontuacao = 0;
    estado->velocidadeJogo = 2.5f;
    estado->velocidadeBase = 2.5f;
    estado->listaDeObstaculos = NULL;
    
    estado->contadorSpawn = 0.0f;
    estado->intervaloSpawnAtual = INTERVALO_SPAWN_INICIAL;
    estado->proximoNivelPontuacao = PONTOS_PARA_SUBIR_NIVEL;
    estado->power_up_aereo_counter = 0;
    estado->power_up_terrestre_counter = 0;

    // Inicializa o Pinguim
    pinguim->position = (Vector2){ 50, PISO - PINGUIM_ALTURA_BASE };
    pinguim->velocidade_y = 0;
    pinguim->estaNoChao = true;
    pinguim->puloMaximo = 2;
    
    pinguim->texAndando = estado->texPinguimAndando;
    pinguim->texPulando = estado->texPinguimPulando;
    pinguim->texGoldAndando = estado->texPinguimGoldAndando;
    pinguim->texGoldPulando = estado->texPinguimGoldPulando;
    
    pinguim->imortal_ativo = false; 
    pinguim->habilidade_distancia_restante = 0;
    
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
            pinguim->puloMaximo = 2; // Pulo Duplo
        } else {
            if (pinguim->puloMaximo > 1) {
                pinguim->velocidade_y = FORCA_PULO;
                pinguim->puloMaximo--;
            }
        }
    }

    pinguim->velocidade_y += GRAVIDADE;
    pinguim->position.y += pinguim->velocidade_y;
    pinguim->hitbox.y = pinguim->position.y;

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
            pinguim->position.y = PISO - PINGUIM_ALTURA_BASE;
            pinguim->velocidade_y = 0;
            pinguim->estaNoChao = true;
            pinguim->puloMaximo = 2;
        } else {
             pinguim->estaNoChao = false;
        }
    }

    if (pinguim->position.y > ALTURA_TELA) { estado->rodando = false; }

    estado->pontuacao++;
    
    if (pinguim->imortal_ativo) {
        pinguim->habilidade_distancia_restante--;
        if (pinguim->habilidade_distancia_restante <= 0) {
            pinguim->imortal_ativo = false;
        }
    }
    
    if (estado->pontuacao > estado->proximoNivelPontuacao) {
        if (estado->velocidadeJogo < 10.0f) { estado->velocidadeJogo += 0.5f; }
        // CORREÇÃO: Lógica de aceleração infinita (Hardcore Mode)
        if (estado->intervaloSpawnAtual > INTERVALO_SPAWN_MINIMO) { 
            estado->intervaloSpawnAtual -= 0.1f; 
        }
        
        if (estado->proximoNivelPontuacao < 3000) {
            estado->proximoNivelPontuacao += PONTOS_PARA_SUBIR_NIVEL; // +200
        } else {
            estado->proximoNivelPontuacao += 2000; // +2000 (Hardcore)
        }
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

        if (atual->obstaculo->hitbox.x + atual->obstaculo->hitbox.width < 0 || atual->obstaculo->tipo == -1) {
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
        if (CheckCollisionRecs(pinguim->hitbox, atual->obstaculo->hitbox)) {
            
            if (atual->obstaculo->tipo == 1) { // Power-Up
                
                // Ativa a imortalidade
                pinguim->imortal_ativo = true;
                pinguim->habilidade_distancia_restante = DISTANCIA_HABILIDADE;
                atual->obstaculo->tipo = -1; 
                
            }
            else if (atual->obstaculo->tipo == 0) { 
                if (!pinguim->imortal_ativo) {
                    estado->rodando = false; 
                } else {
                    atual->obstaculo->tipo = -1; 
                }
            }
            else if (atual->obstaculo->tipo == 2) {
                if (pinguim->estaNoChao) {
                    estado->rodando = false; 
                }
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
    
    if (pinguim->imortal_ativo) {
        texAtual = pinguim->estaNoChao ? estado->texPinguimGoldAndando : estado->texPinguimGoldPulando;
    } else {
        texAtual = pinguim->estaNoChao ? estado->texPinguimAndando : estado->texPinguimPulando;
    }
    
    Rectangle sourceRect = { 0.0f, 0.0f, (float)texAtual.width, (float)texAtual.height }; 
    Rectangle destRect = { pinguim->position.x, pinguim->position.y, (float)PINGUIM_LARGURA_BASE, (float)PINGUIM_ALTURA_BASE };
    
    Vector2 origin = { 0, 0 };
    float rotation = 0.0f; 
    
    DrawTexturePro(texAtual, sourceRect, destRect, origin, rotation, WHITE);

    // Desenha os obstáculos
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
    char imortal_status[40] = "";
    
    if (pinguim->imortal_ativo) {
        sprintf(imortal_status, " | IMORTAL! (%d)", pinguim->habilidade_distancia_restante);
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
    UnloadTexture(estado->texPowerUpImortal);
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
