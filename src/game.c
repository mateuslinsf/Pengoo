/*
 * ========================================
 * ARQUIVO DE LÓGICA: src/game.c (VERSÃO COM EVO E GOD MODE - CORREÇÃO FINAL DE PULOS)
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

// Redefinindo para o novo nome da constante para manter a compatibilidade
#define DISTANCIA_HABILIDADE DISTANCIA_IMORTALIDADE 


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
        tipo = TIPO_IMORTAL; // 5 (Imortalidade - Amarelo)
        estado->power_up_aereo_counter = 0;
    }
    // 2. Prioridade: Power-Up Evo (Terrestre - pós 2000 pontos)
    else if (pontuacao >= PONTUACAO_MINIMA_EVO && estado->power_up_terrestre_counter >= CICLE_SPAWN_EVO) {
        tipo = TIPO_EVO; // 6 (Evo - Roxo)
        estado->power_up_terrestre_counter = 0;
    }
    // 3. Spawn Normal
    else {
        if (pontuacao < 650) {
            tipo = rand() % 4; // 0, 1, 2, 3
        } else {
            tipo = rand() % 5; // 0, 1, 2, 3, 4 (Pós-650)
        }
    }


    int obsLargura = OBSTACULO_LARGURA_BASE;
    int obsAltura = OBSTACULO_ALTURA_BASE;

    if (tipo == 3) { // Terrestre 2x1 (Spawna 2 blocos tipo 0)
        obsAltura = OBSTACULO_ALTURA_BASE * 2;
        
        // Bloco de baixo
        Obstaculo* obsBaixo = (Obstaculo*)malloc(sizeof(Obstaculo));
        NoObstaculo* noBaixo = (NoObstaculo*)malloc(sizeof(NoObstaculo));
        if (obsBaixo == NULL || noBaixo == NULL) { return; }
        obsBaixo->hitbox.x = LARGURA_TELA + 5;
        obsBaixo->hitbox.y = PISO - OBSTACULO_ALTURA_BASE;
        obsBaixo->hitbox.width = obsLargura;
        obsBaixo->hitbox.height = OBSTACULO_ALTURA_BASE;
        obsBaixo->textura = estado->texObstaculoTerrestre; 
        obsBaixo->tipo = 0;
        obsBaixo->id_power_up = 0; // Não é power-up
        noBaixo->obstaculo = obsBaixo;
        noBaixo->proximo = *lista;
        *lista = noBaixo;

        // Bloco de cima
        Obstaculo* obsCima = (Obstaculo*)malloc(sizeof(Obstaculo));
        NoObstaculo* noCima = (NoObstaculo*)malloc(sizeof(NoObstaculo));
        if (obsCima == NULL || noCima == NULL) { return; }
        obsCima->hitbox.x = LARGURA_TELA + 5;
        obsCima->hitbox.y = PISO - (OBSTACULO_ALTURA_BASE * 2);
        obsCima->hitbox.width = obsLargura;
        obsCima->hitbox.height = OBSTACULO_ALTURA_BASE;
        obsCima->textura = estado->texObstaculoTerrestre;
        obsCima->tipo = 0;
        obsCima->id_power_up = 0; // Não é power-up
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
    novoObs->tipo = 0; 
    novoObs->id_power_up = 0; // Inicializa como não Power-up

    if (tipo == 0) { // Terrestre (Simples 1x1)
        novoObs->hitbox.y = PISO - obsAltura;
        novoObs->hitbox.width = obsLargura;
        novoObs->hitbox.height = obsAltura;
        novoObs->textura = estado->texObstaculoTerrestre;
        // CONTAGEM EVO: Apenas conta se for o inimigo 1x1 real E a pontuação for suficiente
        if (pontuacao >= PONTUACAO_MINIMA_EVO) estado->power_up_terrestre_counter++;
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
    }
    else if (tipo == TIPO_IMORTAL) { // Power-Up Imortalidade (Aéreo)
        novoObs->hitbox.width = obsLargura;
        novoObs->hitbox.height = obsAltura;
        novoObs->textura = estado->texPowerUpImortal; 
        novoObs->tipo = 1; // Power-up (Coletável)
        novoObs->id_power_up = 1; // Imortal
        
        int alturaSorteada = rand() % 3;
        if (alturaSorteada == 0) { novoObs->hitbox.y = PISO - 140; }
        else if (alturaSorteada == 1) { novoObs->hitbox.y = PISO - 120; }
        else { novoObs->hitbox.y = PISO - 100; }
    }
    else if (tipo == TIPO_EVO) { // Power-Up Evo (Terrestre)
        obsAltura = OBSTACULO_ALTURA_BASE;
        novoObs->hitbox.y = PISO - obsAltura;
        novoObs->hitbox.width = obsLargura;
        novoObs->hitbox.height = obsAltura;
        // Não carregamos uma textura específica, será desenhado como um bloco VIOLET/ROXO
        novoObs->textura = estado->texPowerUpEvo; 
        novoObs->tipo = 1; // Power-up (Coletável)
        novoObs->id_power_up = 2; // Evo
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
    estado->texPowerUpEvo = LoadTexture("imagens_jogo/inimigos/powerup_evo_placeholder.png"); // NOVA TEXTURA PLACEHOLDER
    estado->texPinguimGoldAndando = LoadTexture("imagens_jogo/pengoo/pengoogold_surfando.png");
    estado->texPinguimGoldPulando = LoadTexture("imagens_jogo/pengoo/pengoogold_pulando.png");
    
    // Aplica filtro de qualidade
    SetTextureFilter(estado->texPinguimAndando, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(estado->texPinguimPulando, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(estado->texObstaculoTerrestre, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(estado->texObstaculoAereo, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(estado->texObstaculoVertical, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(estado->texPowerUpImortal, TEXTURE_FILTER_BILINEAR); 
    SetTextureFilter(estado->texPowerUpEvo, TEXTURE_FILTER_BILINEAR); // NOVO
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
    pinguim->puloMaximo = 0; // 0 pulos aéreos restantes
    
    pinguim->texAndando = estado->texPinguimAndando;
    pinguim->texPulando = estado->texPinguimPulando;
    pinguim->texGoldAndando = estado->texPinguimGoldAndando;
    pinguim->texGoldPulando = estado->texPinguimGoldPulando;
    
    pinguim->imortal_ativo = false; 
    pinguim->imortal_distancia_restante = 0; // NOVO: Timer Imortal
    pinguim->evo_distancia_restante = 0;     // NOVO: Timer Evo
    
    pinguim->hitbox = (Rectangle){ 50, pinguim->position.y, PINGUIM_LARGURA_BASE, PINGUIM_ALTURA_BASE };

    carregarHighScores(estado->topScores);
}


void UpdateGame(EstadoJogo* estado, Pinguim* pinguim) {
    if (!estado->rodando) return;

    // Lógica de Pulo
    if (IsKeyPressed(KEY_SPACE)) {
        if (pinguim->estaNoChao) {
            // 1. PULO NO CHÃO (Primeiro Pulo da sequência)
            pinguim->velocidade_y = FORCA_PULO;
            pinguim->estaNoChao = false;
            
            // Seta o número de pulos aéreos restantes.
            // Pulo Normal (Duplo) = 1 pulo aéreo restante
            // Pulo EVO (Triplo) = 2 pulos aéreos restantes
            pinguim->puloMaximo = (pinguim->evo_distancia_restante > 0) ? 2 : 1;
            
        } else {
            // 2. PULOS AÉREOS (Consumo)
            if (pinguim->puloMaximo > 0) {
                pinguim->velocidade_y = FORCA_PULO;
                pinguim->puloMaximo--; // Consome um pulo aéreo
            }
        }
    }

    // Física
    pinguim->velocidade_y += GRAVIDADE;
    pinguim->position.y += pinguim->velocidade_y;
    pinguim->hitbox.y = pinguim->position.y;

    // Colisão com o chão/reset de pulo
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
            pinguim->puloMaximo = 0; // Reset para 0 pulos aéreos restantes
        } else {
             pinguim->estaNoChao = false;
        }
    }

    if (pinguim->position.y > ALTURA_TELA) { estado->rodando = false; }

    estado->pontuacao++;
    
    // Decrementa os timers independentes dos power-ups
    if (pinguim->imortal_distancia_restante > 0) {
        pinguim->imortal_distancia_restante--;
        if (pinguim->imortal_distancia_restante <= 0) {
            pinguim->imortal_distancia_restante = 0;
        }
    }
    
    if (pinguim->evo_distancia_restante > 0) {
        pinguim->evo_distancia_restante--;
        if (pinguim->evo_distancia_restante <= 0) {
            pinguim->evo_distancia_restante = 0;
            
            // Se o EVO acabar no meio do ar, o pinguim é limitado ao pulo duplo (1 pulo aéreo restante)
            if (!pinguim->estaNoChao && pinguim->puloMaximo > 1) {
                 pinguim->puloMaximo = 1;
            }
        }
    }

    // Deriva a imortalidade para a lógica de colisão
    pinguim->imortal_ativo = pinguim->imortal_distancia_restante > 0;
    
    // ... Restante da lógica de jogo (Spawn, Velocidade, Remoção de Obstáculos) permanece inalterada ...
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
            
            if (atual->obstaculo->tipo == 1) { // Power-Up (tipo=1 para ambos EVO e IMORTAL)
                
                // --- Power-up IMORTAL (id_power_up == 1) ---
                if (atual->obstaculo->id_power_up == 1) { 
                    pinguim->imortal_distancia_restante += DISTANCIA_IMORTALIDADE; 
                    atual->obstaculo->tipo = -1; 
                    
                }
                
                // --- Power-up EVO (id_power_up == 2) ---
                else if (atual->obstaculo->id_power_up == 2) { 
                    pinguim->evo_distancia_restante += DISTANCIA_EVO; 
                    atual->obstaculo->tipo = -1;
                }
                
            }
            else if (atual->obstaculo->tipo == 0) { // Inimigo/Dano
                if (!pinguim->imortal_ativo) { 
                    estado->rodando = false; 
                } else {
                    atual->obstaculo->tipo = -1; 
                }
            }
            else if (atual->obstaculo->tipo == 2) { // Buraco
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
    
    // --- Lógica de Status de Power-Up e Fusão GOD ---
    bool isImortal = pinguim->imortal_distancia_restante > 0;
    bool isEvo = pinguim->evo_distancia_restante > 0;
    bool isGod = isImortal && isEvo;

    char statusName[10] = "";
    int remainingDistance = 0;

    if (isGod) {
        strcpy(statusName, "GOD");
        // Distância restante é o mínimo entre os dois (regra de fusão natural)
        remainingDistance = (pinguim->imortal_distancia_restante < pinguim->evo_distancia_restante)
                            ? pinguim->imortal_distancia_restante
                            : pinguim->evo_distancia_restante;
    } else if (isImortal) {
        strcpy(statusName, "IMORTAL");
        remainingDistance = pinguim->imortal_distancia_restante;
    } else if (isEvo) {
        strcpy(statusName, "EVO");
        remainingDistance = pinguim->evo_distancia_restante;
    }
    // ------------------------------------

    // Desenha o Pinguim
    Texture2D texAtual;
    
    if (isImortal || isGod) { // Pinguim Dourado (Imortal ou God)
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
            Color fallbackColor = BLACK; // Default para inimigo
            
            if (obs->id_power_up == 1) { // Imortal
                fallbackColor = YELLOW;
            } else if (obs->id_power_up == 2) { // Evo
                fallbackColor = VIOLET; // Roxo
            }
            
            if (obsTex.id > 0) { 
                // Se for EVO, desenha o retângulo VIOLET (Roxo) conforme pedido
                if (obs->id_power_up == 2) {
                     DrawRectangleRec(obs->hitbox, VIOLET);
                } 
                // Se for Imortal, desenha a textura normalmente (que é amarela/dourada)
                else {
                    DrawTexturePro(obsTex, 
                                   (Rectangle){0, 0, (float)obsTex.width, (float)obsTex.height},
                                   obs->hitbox, 
                                   origin, rotation, WHITE);
                }
            } else {
                // Fallback (inimigo preto, Imortal amarelo, EVO roxo)
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
    char power_up_status[60] = ""; 

    if (isImortal || isEvo) {
        sprintf(power_up_status, " | %s! (%d)", statusName, remainingDistance);
    } 
    
    sprintf(hud, "Pontos: %d | Velocidade: %.1f%s | RECORDE: %s %d",
            estado->pontuacao, estado->velocidadeJogo, power_up_status, estado->topScores[0].nome, estado->topScores[0].pontuacao);
            
    DrawText(hud, 10, 10, 20, BLACK);

    if (!estado->rodando) {
        DrawText("G A M E    O V E R", LARGURA_TELA/2 - MeasureText("G A M E    O V E R", 40)/2, ALTURA_TELA/2 - 20, 40, RED);
    }
}


void UnloadGame(EstadoJogo* estado, Pinguim* pinguim) {
    UnloadTexture(estado->texPinguimAndando);
    UnloadTexture(estado->texPinguimPulando);
    UnloadTexture(estado->texObstaculoTerrestre);
    UnloadTexture(estado->texObstaculoAereo);
    UnloadTexture(estado->texObstaculoVertical);
    UnloadTexture(estado->texPowerUpImortal);
    UnloadTexture(estado->texPowerUpEvo); // NOVO
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