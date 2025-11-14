/*
 * ========================================
 * ARQUIVO DE LÓGICA: src/game.c (VERSÃO FINAL COM BÔNUS GOD)
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

// Ajuste de Posição Vertical para Obstáculos Terrestres
#define AJUSTE_VERTICAL 5

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


// --- Funções de Nuvens ---
void InitNuvens(EstadoJogo* estado) {
    const float ESPACAMENTO_MEDIO = 150.0f;
    for (int i = 0; i < NUM_NUVENS; i++) {
        estado->nuvens[i].ativa = true;
        estado->nuvens[i].velocidade = VELOCIDADE_NUVEM + ((float)(rand() % 50) / 100.0f);
        estado->nuvens[i].raio = (float)(rand() % 30 + 50);
        estado->nuvens[i].cor = (Color){ 255, 255, 255, 255 };
        float offset_x = (float)(rand() % 40 - 20);
        estado->nuvens[i].position.x = -200.0f + (float)i * ESPACAMENTO_MEDIO + offset_x;
        estado->nuvens[i].position.y = (float)(rand() % (ALTURA_TELA / 2 - 50));
    }
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

// FinalizarJogo agora também calcula o ranking
void FinalizarJogo(EstadoJogo* estado) {
    if (estado->bonusCalculado) return; // Só roda uma vez

    estado->rodando = false; // Para o jogo

    // Calcula o bônus
    estado->pontuacao += (estado->contGod * 500);
    estado->bonusCalculado = true;
    
    // Calcula e armazena o ranking
    estado->rankingJogador = obterRanking(estado->topScores, estado->pontuacao);
}


// --- Funções da Lista Encadeada (Obstáculos) ---
void adicionarObstaculo(NoObstaculo** lista, int pontuacao, EstadoJogo* estado) {
    int tipo = 0;

    // 1. Prioridade: Power-Up Imortalidade (Aéreo)
    if (pontuacao >= PONTUACAO_MINIMA_IMORTALIDADE && estado->power_up_aereo_counter >= CICLE_SPAWN_IMORTALIDADE) {
        tipo = TIPO_IMORTAL; // 5 (Imortalidade - Pelicano Gold)
        estado->power_up_aereo_counter = 0;
    }
    // 2. Prioridade: Power-Up Evo (Terrestre - pós 2000 pontos)
    else if (pontuacao >= PONTUACAO_MINIMA_EVO && estado->power_up_terrestre_counter >= CICLE_SPAWN_EVO) {
        tipo = TIPO_EVO; // 6 (Evo - Pedra EVO)
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

    // Se tipo == 3, criamos o Urso Polar (1x2)
    if (tipo == 3) {

        Obstaculo* novoObs = (Obstaculo*)malloc(sizeof(Obstaculo));
        if (novoObs == NULL) return;
        NoObstaculo* novoNo = (NoObstaculo*)malloc(sizeof(NoObstaculo));
        if (novoNo == NULL) { free(novoObs); return; }

        obsAltura = OBSTACULO_ALTURA_BASE * 2; // Urso é 1x2

        novoObs->hitbox.x = LARGURA_TELA + 5;
        novoObs->hitbox.y = PISO - obsAltura + AJUSTE_VERTICAL;
        novoObs->hitbox.width = obsLargura;
        novoObs->hitbox.height = obsAltura;
        novoObs->textura = estado->texObstaculoTerrestre2x1; // Urso Polar
        novoObs->tipo = 0; // Inimigo de dano
        novoObs->id_power_up = 0;

        novoNo->obstaculo = novoObs;
        novoNo->proximo = *lista;
        *lista = novoNo;

        if (pontuacao >= PONTUACAO_MINIMA_EVO) estado->power_up_terrestre_counter++;
        return;
    }

    Obstaculo* novoObs = (Obstaculo*)malloc(sizeof(Obstaculo));
    if (novoObs == NULL) return;
    NoObstaculo* novoNo = (NoObstaculo*)malloc(sizeof(NoObstaculo));
    if (novoNo == NULL) { free(novoObs); return; }

    novoObs->hitbox.x = LARGURA_TELA + 5;
    novoObs->tipo = 0;
    novoObs->id_power_up = 0; 

    if (tipo == 0) { // Terrestre (Simples 1x1: Pedra 1)
        novoObs->hitbox.y = PISO - obsAltura + AJUSTE_VERTICAL;
        novoObs->hitbox.width = obsLargura;
        novoObs->hitbox.height = obsAltura;
        novoObs->textura = estado->texObstaculoTerrestre;
        if (pontuacao >= PONTUACAO_MINIMA_EVO) estado->power_up_terrestre_counter++;
    }
    else if (tipo == 1) { // Aéreo (Pelicano Normal)
        novoObs->hitbox.width = obsLargura;
        novoObs->hitbox.height = obsAltura;
        novoObs->textura = estado->texObstaculoAereoNormal;
        estado->power_up_aereo_counter++; 
        int alturaSorteada = rand() % 3;
        if (alturaSorteada == 0) { novoObs->hitbox.y = PISO - 120; }
        else if (alturaSorteada == 1) { novoObs->hitbox.y = PISO - 100; }
        else { novoObs->hitbox.y = PISO - 80; }
    }
    else if (tipo == 2) { // Buraco/Chão Quebrado
        novoObs->hitbox.y = PISO;
        novoObs->hitbox.height = 40;
        novoObs->tipo = 2;
        novoObs->textura = estado->texBuraco;
        int buracoLargura = LARGURA_BURACO_BLOCO * OBSTACULO_LARGURA_BASE;
        if (pontuacao > 650) {
            buracoLargura = (LARGURA_BURACO_BLOCO + 1) * OBSTACULO_LARGURA_BASE;
        }
        novoObs->hitbox.width = buracoLargura;
    }
    else if (tipo == 4) { // Terrestre 3# Vertical (Pedra 3)
        obsAltura = OBSTACULO_ALTURA_BASE * 3;
        novoObs->hitbox.y = PISO - obsAltura + 10 + AJUSTE_VERTICAL;
        novoObs->hitbox.width = obsLargura;
        novoObs->hitbox.height = obsAltura;
        novoObs->textura = estado->texObstaculoVertical;
        novoObs->tipo = 0;
    }
    else if (tipo == TIPO_IMORTAL) { // Power-Up Imortalidade (Pelicano Gold)
        novoObs->hitbox.width = obsLargura;
        novoObs->hitbox.height = obsAltura;
        novoObs->textura = estado->texPowerUpImortal;
        novoObs->tipo = 1;
        novoObs->id_power_up = 1; 
        int alturaSorteada = rand() % 3;
        if (alturaSorteada == 0) { novoObs->hitbox.y = PISO - 140; }
        else if (alturaSorteada == 1) { novoObs->hitbox.y = PISO - 120; }
        else { novoObs->hitbox.y = PISO - 100; }
    }
    else if (tipo == TIPO_EVO) { // Power-Up Evo (Terrestre: pedra1_evo)
        obsAltura = OBSTACULO_ALTURA_BASE;
        novoObs->hitbox.y = PISO - obsAltura + AJUSTE_VERTICAL;
        novoObs->hitbox.width = obsLargura;
        novoObs->hitbox.height = obsAltura;
        novoObs->textura = estado->texPowerUpEvo;
        novoObs->tipo = 1; 
        novoObs->id_power_up = 2;
    }

    novoNo->obstaculo = novoObs;
    novoNo->proximo = *lista;
    *lista = novoNo;
}


// --- Funções Principais do Jogo ---

void InitGame(EstadoJogo* estado, Pinguim* pinguim) {
    srand((unsigned int)time(NULL));

    estado->texCapa = LoadTexture("imagens_jogo/cenario/capa_inicial.jpeg");
    if (estado->texCapa.id > 0) SetTextureFilter(estado->texCapa, TEXTURE_FILTER_BILINEAR);

    estado->texPinguimAndando = LoadTexture("imagens_jogo/pengoo/pengoo_surfando.png");
    estado->texPinguimPulando = LoadTexture("imagens_jogo/pengoo/pengoo_pulando.png");
    estado->texPinguimGoldAndando = LoadTexture("imagens_jogo/pengoo/pengoogold_surfando.png");
    estado->texPinguimGoldPulando = LoadTexture("imagens_jogo/pengoo/pengoogold_pulando.png");
    estado->texPinguimEvoAndando = LoadTexture("imagens_jogo/pengoo/pengooevo_surfando.png");
    estado->texPinguimEvoPulando = LoadTexture("imagens_jogo/pengoo/pengooevo_pulando.png");
    estado->texPinguimGodAndando = LoadTexture("imagens_jogo/pengoo/pengoogod_surfando.png");
    estado->texPinguimGodPulando = LoadTexture("imagens_jogo/pengoo/t.png");
    estado->texChao = LoadTexture("imagens_jogo/cenario/chao.png");
    estado->texBuraco = LoadTexture("imagens_jogo/cenario/buraco.png");
    estado->texNuvem = LoadTexture("imagens_jogo/cenario/nuvens.png");
    estado->texObstaculoTerrestre = LoadTexture("imagens_jogo/inimigos/pedra1.png");
    estado->texObstaculoAereoNormal = LoadTexture("imagens_jogo/inimigos/pelicano.png");
    estado->texObstaculoTerrestre2x1 = LoadTexture("imagens_jogo/inimigos/urso_polar.png");
    estado->texObstaculoVertical = LoadTexture("imagens_jogo/inimigos/pedra3.png");
    estado->texPowerUpImortal = LoadTexture("imagens_jogo/inimigos/pelicanogold.png");
    estado->texPowerUpEvo = LoadTexture("imagens_jogo/inimigos/pedra1_evo.png");

    if (estado->texPinguimAndando.id > 0) SetTextureFilter(estado->texPinguimAndando, TEXTURE_FILTER_BILINEAR);
    if (estado->texPinguimPulando.id > 0) SetTextureFilter(estado->texPinguimPulando, TEXTURE_FILTER_BILINEAR);
    if (estado->texObstaculoTerrestre.id > 0) SetTextureFilter(estado->texObstaculoTerrestre, TEXTURE_FILTER_BILINEAR);
    if (estado->texObstaculoAereoNormal.id > 0) SetTextureFilter(estado->texObstaculoAereoNormal, TEXTURE_FILTER_BILINEAR);
    if (estado->texObstaculoTerrestre2x1.id > 0) SetTextureFilter(estado->texObstaculoTerrestre2x1, TEXTURE_FILTER_BILINEAR);
    if (estado->texObstaculoVertical.id > 0) SetTextureFilter(estado->texObstaculoVertical, TEXTURE_FILTER_BILINEAR);
    if (estado->texPowerUpImortal.id > 0) SetTextureFilter(estado->texPowerUpImortal, TEXTURE_FILTER_BILINEAR);
    if (estado->texPowerUpEvo.id > 0) SetTextureFilter(estado->texPowerUpEvo, TEXTURE_FILTER_BILINEAR);
    if (estado->texPinguimGoldAndando.id > 0) SetTextureFilter(estado->texPinguimGoldAndando, TEXTURE_FILTER_BILINEAR);
    if (estado->texPinguimGoldPulando.id > 0) SetTextureFilter(estado->texPinguimGoldPulando, TEXTURE_FILTER_BILINEAR);
    if (estado->texPinguimEvoAndando.id > 0) SetTextureFilter(estado->texPinguimEvoAndando, TEXTURE_FILTER_BILINEAR);
    if (estado->texPinguimEvoPulando.id > 0) SetTextureFilter(estado->texPinguimEvoPulando, TEXTURE_FILTER_BILINEAR);
    if (estado->texPinguimGodAndando.id > 0) SetTextureFilter(estado->texPinguimGodAndando, TEXTURE_FILTER_BILINEAR);
    if (estado->texPinguimGodPulando.id > 0) SetTextureFilter(estado->texPinguimGodPulando, TEXTURE_FILTER_BILINEAR);
    if (estado->texChao.id > 0) SetTextureFilter(estado->texChao, TEXTURE_FILTER_BILINEAR);
    if (estado->texBuraco.id > 0) SetTextureFilter(estado->texBuraco, TEXTURE_FILTER_BILINEAR);
    if (estado->texNuvem.id > 0) SetTextureFilter(estado->texNuvem, TEXTURE_FILTER_BILINEAR);

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
    estado->contGod = 0;
    estado->godAtivoAnterior = false;
    estado->bonusCalculado = false;

    // Inicializa novas variáveis
    estado->nomeIndex = 0;
    estado->rankingJogador = 0;
    estado->nomeJogador[0] = '\0';

    pinguim->position = (Vector2){ 50, PISO - PINGUIM_ALTURA_BASE };
    pinguim->velocidade_y = 0;
    pinguim->estaNoChao = true;
    pinguim->puloMaximo = 0;
    pinguim->imortal_ativo = false;
    pinguim->imortal_distancia_restante = 0;
    pinguim->evo_distancia_restante = 0;
    pinguim->hitbox = (Rectangle){ 50, pinguim->position.y, PINGUIM_LARGURA_BASE, PINGUIM_ALTURA_BASE };

    carregarHighScores(estado->topScores);
    InitNuvens(estado);
}


//
// UpdateGame agora gerencia TODOS os estados do jogo
//
void UpdateGame(EstadoJogo* estado, Pinguim* pinguim, GameScreen* currentScreen) {
    
    switch (*currentScreen) {
        
        case TELA_TITULO: {
            if (GetKeyPressed() != 0) {
                *currentScreen = TELA_TUTORIAL;
            }
        } break;
        
        case TELA_TUTORIAL: {
            if (GetKeyPressed() != 0) {
                *currentScreen = JOGANDO;
            }
        } break;
        
        case JOGANDO: {
            // Se o jogo não está rodando, é porque o jogador perdeu no frame anterior
            if (!estado->rodando) {
                if (estado->rankingJogador > 0) {
                    *currentScreen = FIM_DE_JOGO_INPUT;
                } else {
                    *currentScreen = FIM_DE_JOGO_FINAL;
                }
                return; // Sai do Update
            }
            
            // --- Lógica do Jogo Rodando (Código antigo do UpdateGame) ---
            if (IsKeyPressed(KEY_SPACE)) {
                if (pinguim->estaNoChao) {
                    pinguim->velocidade_y = FORCA_PULO;
                    pinguim->estaNoChao = false;
                    pinguim->puloMaximo = (pinguim->evo_distancia_restante > 0) ? 2 : 1;
                } else {
                    if (pinguim->puloMaximo > 0) {
                        pinguim->velocidade_y = FORCA_PULO;
                        pinguim->puloMaximo--;
                    }
                }
            }

            for (int i = 0; i < NUM_NUVENS; i++) {
                if (estado->nuvens[i].ativa) {
                    estado->nuvens[i].position.x -= estado->nuvens[i].velocidade;
                    if (estado->nuvens[i].position.x < -estado->nuvens[i].raio) {
                        estado->nuvens[i].position.x = LARGURA_TELA + (float)(rand() % 100 + 50);
                        estado->nuvens[i].position.y = (float)(rand() % (ALTURA_TELA / 2 - 50));
                        estado->nuvens[i].velocidade = VELOCIDADE_NUVEM + ((float)(rand() % 50) / 100.0f);
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
                    pinguim->puloMaximo = 0;
                } else {
                    pinguim->estaNoChao = false;
                }
            }

            if (pinguim->position.y > ALTURA_TELA) {
                FinalizarJogo(estado); // <--- Morte por queda
                return; 
            }

            estado->pontuacao++;

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
                    if (!pinguim->estaNoChao && pinguim->puloMaximo > 1) {
                        pinguim->puloMaximo = 1;
                    }
                }
            }

            pinguim->imortal_ativo = pinguim->imortal_distancia_restante > 0;
            bool isEvo = pinguim->evo_distancia_restante > 0;
            bool isGod = pinguim->imortal_ativo && isEvo;

            if (isGod && !estado->godAtivoAnterior) {
                estado->contGod++;
                estado->godAtivoAnterior = true;
            } else if (!isGod) {
                estado->godAtivoAnterior = false;
            }

            if (estado->pontuacao > estado->proximoNivelPontuacao) {
                if (estado->velocidadeJogo < 10.0f) { estado->velocidadeJogo += 0.5f; }
                if (estado->intervaloSpawnAtual > INTERVALO_SPAWN_MINIMO) {
                    estado->intervaloSpawnAtual -= 0.1f;
                }
                if (estado->proximoNivelPontuacao < 3000) {
                    estado->proximoNivelPontuacao += PONTOS_PARA_SUBIR_NIVEL;
                } else {
                    estado->proximoNivelPontuacao += 2000;
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
                    if (atual->obstaculo->tipo == 1) { 
                        if (atual->obstaculo->id_power_up == 1) {
                            pinguim->imortal_distancia_restante += DISTANCIA_IMORTALIDADE;
                            atual->obstaculo->tipo = -1;
                        }
                        else if (atual->obstaculo->id_power_up == 2) {
                            pinguim->evo_distancia_restante += DISTANCIA_EVO;
                            atual->obstaculo->tipo = -1;
                        }
                    }
                    else if (atual->obstaculo->tipo == 0) { 
                        if (!pinguim->imortal_ativo) {
                            FinalizarJogo(estado); // <--- Morte por colisão
                            break; 
                        } else {
                            atual->obstaculo->tipo = -1;
                        }
                    }
                    else if (atual->obstaculo->tipo == 2) { 
                        FinalizarJogo(estado); // <--- Morte por colisão (buraco)
                        break;
                    }
                }
                atual = atual->proximo;
            }
            // --- Fim da Lógica do Jogo Rodando ---
            
        } break;
        
        case FIM_DE_JOGO_INPUT: {
            // Lógica de Captura de Tecla
            int key = GetCharPressed(); // Usar GetCharPressed para pegar A-Z
            
            // Pegar A-Z (maiúsculas)
            if (key >= 'a' && key <= 'z') key = key - 32; // Converter para maiúscula
            
            if (key >= 'A' && key <= 'Z') {
                if (estado->nomeIndex < 3) {
                    estado->nomeJogador[estado->nomeIndex] = (char)key;
                    estado->nomeIndex++;
                    estado->nomeJogador[estado->nomeIndex] = '\0';
                }
            }
            
            // Checar teclas especiais
            if (IsKeyPressed(KEY_BACKSPACE)) {
                if (estado->nomeIndex > 0) {
                    estado->nomeIndex--;
                    estado->nomeJogador[estado->nomeIndex] = '\0';
                }
            }
            
            if (IsKeyPressed(KEY_ENTER)) {
                if (estado->nomeIndex == 3) {
                    // Salvar o score
                    adicionarNovoScore(estado->topScores, estado->pontuacao, estado->nomeJogador, estado->rankingJogador);
                    salvarHighScores(estado->topScores);
                    *currentScreen = FIM_DE_JOGO_FINAL;
                }
            }
            
        } break;
        
        case FIM_DE_JOGO_FINAL: {
            if (IsKeyPressed(KEY_X)) {
                CloseWindow();
            }
        } break;
    }
}

// --- NOVA FUNÇÃO HELPER ---
// Desenha apenas o cenário do jogo (céu, nuvens, chão, pinguim, obstáculos)
void DrawGameScene(EstadoJogo* estado, Pinguim* pinguim) {
    ClearBackground(SKYBLUE);

    // --- Desenha as Nuvens ---
    for (int i = 0; i < NUM_NUVENS; i++) {
        Nuvem* nuvem = &estado->nuvens[i];
        if (nuvem->ativa && estado->texNuvem.id > 0) {
            float ratio = (float)estado->texNuvem.height / (float)estado->texNuvem.width;
            float altura = nuvem->raio * ratio;
            Rectangle sourceRect = { 0.0f, 0.0f, (float)estado->texNuvem.width, (float)estado->texNuvem.height };
            Rectangle destRect = { nuvem->position.x, nuvem->position.y, nuvem->raio, altura };
            DrawTexturePro(estado->texNuvem, sourceRect, destRect, (Vector2){0,0}, 0.0f, nuvem->cor);
        }
    }

    // --- 1. Desenha o Chão ---
    int numTiles = LARGURA_TELA / OBSTACULO_LARGURA_BASE;
    if (estado->texChao.id > 0) {
        for (int i = 0; i < numTiles; i++) {
            DrawTexturePro(estado->texChao,
                           (Rectangle){0, 0, (float)estado->texChao.width, (float)estado->texChao.height},
                           (Rectangle){ (float)(i * OBSTACULO_LARGURA_BASE), PISO, (float)OBSTACULO_LARGURA_BASE, (float)OBSTACULO_ALTURA_BASE },
                           (Vector2){0, 0}, 0.0f, WHITE);
        }
    } else {
        DrawRectangle(0, PISO, LARGURA_TELA, 40, BLACK);
    }

    // --- 2. Desenha o Pinguim ---
    bool isImortal = pinguim->imortal_distancia_restante > 0;
    bool isEvo = pinguim->evo_distancia_restante > 0;
    bool isGod = isImortal && isEvo;

    Texture2D texAtual;
    if (isGod) texAtual = pinguim->estaNoChao ? estado->texPinguimGodAndando : estado->texPinguimGodPulando;
    else if (isImortal) texAtual = pinguim->estaNoChao ? estado->texPinguimGoldAndando : estado->texPinguimGoldPulando;
    else if (isEvo) texAtual = pinguim->estaNoChao ? estado->texPinguimEvoAndando : estado->texPinguimEvoPulando;
    else texAtual = pinguim->estaNoChao ? estado->texPinguimAndando : estado->texPinguimPulando;

    Rectangle sourceRect = { 0.0f, 0.0f, (float)texAtual.width, (float)texAtual.height };
    Rectangle destRect = { pinguim->position.x, pinguim->position.y, (float)PINGUIM_LARGURA_BASE, (float)PINGUIM_ALTURA_BASE };
    DrawTexturePro(texAtual, sourceRect, destRect, (Vector2){0,0}, 0.0f, WHITE);


    // --- 3. Desenha os Obstáculos ---
    NoObstaculo* atual = estado->listaDeObstaculos;
    while (atual != NULL) {
        Obstaculo* obs = atual->obstaculo;
        if (obs->tipo == 0 || obs->tipo == 1) { 
            Texture2D obsTex = obs->textura;
            if (obsTex.id > 0) {
                DrawTexturePro(obsTex, (Rectangle){0, 0, (float)obsTex.width, (float)obsTex.height}, obs->hitbox, (Vector2){0,0}, 0.0f, WHITE);
            } else {
                Color fallbackColor = BLACK;
                if (obs->id_power_up == 1) fallbackColor = YELLOW;
                else if (obs->id_power_up == 2) fallbackColor = VIOLET;
                DrawRectangleRec(obs->hitbox, fallbackColor);
            }
        }
        // --- CORREÇÃO AQUI ---
        // Adicionado o argumento '0.0f' para a rotação
        else if (obs->tipo == 2) { 
            DrawTexturePro(obs->textura, (Rectangle){0, 0, (float)obs->textura.width, (float)obs->textura.height}, obs->hitbox, (Vector2){0,0}, 0.0f, WHITE);
        }
        // --- FIM DA CORREÇÃO ---
        atual = atual->proximo;
    }
}


//
// DrawGame agora usa a função helper e desenha as telas de Game Over
//
void DrawGame(EstadoJogo* estado, Pinguim* pinguim, GameScreen currentScreen) {

    switch (currentScreen) {
        
        case TELA_TITULO:
        case TELA_TUTORIAL: {
            // --- TELA DE TÍTULO / TUTORIAL ---
            ClearBackground(SKYBLUE); // Fundo para caso a textura falhe
            if (estado->texCapa.id > 0) {
                Rectangle sourceRect = { 0.0f, 0.0f, (float)estado->texCapa.width, (float)estado->texCapa.height };
                Rectangle destRect = { 0, 0, LARGURA_TELA, ALTURA_TELA };
                DrawTexturePro(estado->texCapa, sourceRect, destRect, (Vector2){ 0, 0 }, 0.0f, WHITE);
            } else {
                const char *text = (currentScreen == TELA_TITULO) ? "Erro ao carregar capa_inicial.jpeg" : "Erro ao carregar tutorial";
                DrawText(text, LARGURA_TELA/2 - MeasureText(text, 20)/2, ALTURA_TELA/2 - 10, 20, RED);
            }
        } break;
        
        case JOGANDO: {
            // --- TELA DE JOGO (RODANDO) ---
            DrawGameScene(estado, pinguim); // Desenha o jogo
            
            // --- Desenha o HUD ---
            bool isImortal = pinguim->imortal_distancia_restante > 0;
            bool isEvo = pinguim->evo_distancia_restante > 0;
            bool isGod = isImortal && isEvo;
            char statusName[10] = "";
            int remainingDistance = 0;

            if (isGod) {
                strcpy(statusName, "GOD");
                remainingDistance = (pinguim->imortal_distancia_restante < pinguim->evo_distancia_restante) ? pinguim->imortal_distancia_restante : pinguim->evo_distancia_restante;
            } else if (isImortal) {
                strcpy(statusName, "Gold");
                remainingDistance = pinguim->imortal_distancia_restante;
            } else if (isEvo) {
                strcpy(statusName, "EVO");
                remainingDistance = pinguim->evo_distancia_restante;
            }
            
            char hud[200];
            char power_up_status[60] = "";
            if (isImortal || isEvo) {
                sprintf(power_up_status, " | %s! (%d)", statusName, remainingDistance);
            }
            sprintf(hud, "Pontos: %d | Velocidade: %.1f%s | RECORDE: %s %d",
                    estado->pontuacao, estado->velocidadeJogo, power_up_status, estado->topScores[0].nome, estado->topScores[0].pontuacao);
            DrawText(hud, 10, 10, 20, BLACK);
            
        } break;
        
        case FIM_DE_JOGO_INPUT: {
            // --- TELA DE INPUT DE NOME ---
            DrawGameScene(estado, pinguim); // Desenha o jogo pausado no fundo
            
            // Overlay escuro
            DrawRectangle(0, 0, LARGURA_TELA, ALTURA_TELA, (Color){ 0, 0, 0, 150 });
            
            // Textos
            DrawText("G A M E   O V E R", LARGURA_TELA/2 - MeasureText("G A M E   O V E R", 40)/2, ALTURA_TELA/2 - 120, 40, RED);

            char textoRank[100];
            sprintf(textoRank, "NOVO RECORDE! %dº LUGAR!", estado->rankingJogador);
            DrawText(textoRank, LARGURA_TELA/2 - MeasureText(textoRank, 30)/2, ALTURA_TELA/2 - 60, 30, YELLOW);

            char textoPrompt[100];
            sprintf(textoPrompt, "DIGITE SUAS 3 INICIAIS: %s", estado->nomeJogador);
            DrawText(textoPrompt, LARGURA_TELA/2 - MeasureText(textoPrompt, 20)/2, ALTURA_TELA/2 + 10, 20, WHITE);

            // Cursor piscante
            if (((int)(GetTime() * 2)) % 2 == 0 && estado->nomeIndex < 3) {
                int textWidth = MeasureText(textoPrompt, 20);
                int promptBaseWidth = MeasureText("DIGITE SUAS 3 INICIAIS: ", 20);
                DrawText("_", (LARGURA_TELA/2 - textWidth/2) + promptBaseWidth + MeasureText(estado->nomeJogador, 20), ALTURA_TELA/2 + 10, 20, WHITE);
            }

            if (estado->nomeIndex == 3) {
                 DrawText("Pressione ENTER para confirmar", LARGURA_TELA/2 - MeasureText("Pressione ENTER para confirmar", 20)/2, ALTURA_TELA - 40, 20, LIME);
            }

        } break;
        
        case FIM_DE_JOGO_FINAL: {
            // --- TELA FINAL (SCORE + TOP 3) ---
            DrawGameScene(estado, pinguim); // Desenha o jogo pausado no fundo

            // Overlay escuro
            DrawRectangle(0, 0, LARGURA_TELA, ALTURA_TELA, (Color){ 0, 0, 0, 150 });

            // "GAME OVER"
            DrawText("G A M E   O V E R", LARGURA_TELA/2 - MeasureText("G A M E   O V E R", 40)/2, ALTURA_TELA/2 - 140, 40, RED);

            // --- Textos do Score ---
            int bonusGod = (estado->contGod * 500);
            int pontuacaoBase = estado->pontuacao - bonusGod;

            char textoScoreBase[100];
            char textoBonus[100];
            char textoTotal[100];

            sprintf(textoScoreBase, "Pontuacao Base: %d", pontuacaoBase);
            sprintf(textoBonus, "Bonus GOD (x%d): %d", estado->contGod, bonusGod);
            sprintf(textoTotal, "TOTAL: %d", estado->pontuacao);

            int y_start = ALTURA_TELA/2 - 80; // Posição Y inicial dos textos
            DrawText(textoScoreBase, LARGURA_TELA/2 - MeasureText(textoScoreBase, 20)/2, y_start, 20, WHITE);
            DrawText(textoBonus, LARGURA_TELA/2 - MeasureText(textoBonus, 20)/2, y_start + 30, 20, YELLOW);
            DrawText(textoTotal, LARGURA_TELA/2 - MeasureText(textoTotal, 30)/2, y_start + 70, 30, GREEN);

            // --- Desenha o TOP 3 ---
            int y_top3 = y_start + 120;
            DrawText("--- TOP 3 SCORES ---", LARGURA_TELA/2 - MeasureText("--- TOP 3 SCORES ---", 20)/2, y_top3, 20, WHITE);

            char scoreLinha[100];
            for (int i = 0; i < 3; i++) {
                sprintf(scoreLinha, "%d. %s .... %d", i + 1, estado->topScores[i].nome, estado->topScores[i].pontuacao);
                DrawText(scoreLinha, LARGURA_TELA/2 - MeasureText(scoreLinha, 20)/2, y_top3 + 30 + (i * 25), 20, WHITE);
            }

            // --- Texto de Sair ---
            if (((int)(GetTime() * 2)) % 2 == 0) { 
                char textoOpcoes[100];
                sprintf(textoOpcoes, "Pressione [X] para sair");
                DrawText(textoOpcoes, LARGURA_TELA/2 - MeasureText(textoOpcoes, 20)/2, ALTURA_TELA - 40, 20, WHITE);
            }
        } break;
    }
}


void UnloadGame(EstadoJogo* estado, Pinguim* pinguim) {
    UnloadTexture(estado->texCapa);
    UnloadTexture(estado->texPinguimAndando);
    UnloadTexture(estado->texPinguimPulando);
    UnloadTexture(estado->texObstaculoTerrestre);
    UnloadTexture(estado->texObstaculoAereoNormal);
    UnloadTexture(estado->texObstaculoTerrestre2x1);
    UnloadTexture(estado->texObstaculoVertical);
    UnloadTexture(estado->texPowerUpImortal);
    UnloadTexture(estado->texPowerUpEvo);
    UnloadTexture(estado->texPinguimGoldAndando);
    UnloadTexture(estado->texPinguimGoldPulando);
    UnloadTexture(estado->texPinguimEvoAndando);
    UnloadTexture(estado->texPinguimEvoPulando);
    UnloadTexture(estado->texPinguimGodAndando);
    UnloadTexture(estado->texPinguimGodPulando);
    UnloadTexture(estado->texChao);
    UnloadTexture(estado->texBuraco);
    UnloadTexture(estado->texNuvem);

    NoObstaculo* atual = estado->listaDeObstaculos;
    while (atual != NULL) {
        NoObstaculo* proximo = atual->proximo;
        free(atual->obstaculo);
        free(atual);
        atual = proximo;
    }
}