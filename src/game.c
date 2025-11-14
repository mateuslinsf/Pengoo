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
    // Define um espaçamento horizontal médio para distribuir as 10 nuvens.
    // O valor 150.0f é um espaçamento seguro para nuvens pequenas (LARGURA_TELA = 800).
    const float ESPACAMENTO_MEDIO = 150.0f;

    for (int i = 0; i < NUM_NUVENS; i++) {

        estado->nuvens[i].ativa = true;

        // Velocidade: entre 0.5f e 1.0f
        estado->nuvens[i].velocidade = VELOCIDADE_NUVEM + ((float)(rand() % 50) / 100.0f);

        // MODIFICADO: 'raio' agora representa a LARGURA da textura
        // Tamanho: entre 50 e 79
        estado->nuvens[i].raio = (float)(rand() % 30 + 50);

        // Cor: WHITE (255, 255, 255, 255).
        // Isso renderiza a textura .png com sua cor original.
        estado->nuvens[i].cor = (Color){ 255, 255, 255, 255 };

        // --- NOVO CÁLCULO DE POSIÇÃO X PARA GARANTIR ESPAÇAMENTO ---
        // Posição inicial: Começa em -200 (fora da tela esquerda) e espaça sequencialmente.
        // Adiciona um pequeno offset aleatório (-20 a +19) para evitar um alinhamento perfeito.
        float offset_x = (float)(rand() % 40 - 20);
        estado->nuvens[i].position.x = -200.0f + (float)i * ESPACAMENTO_MEDIO + offset_x;

        // Posição Y aleatória na metade superior da tela (céu)
        // (ALTURA_TELA / 2 - 50) garante que não toque o pinguim ou o chão.
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

// --- ADICIONADO: Função para centralizar o Game Over e calcular bônus ---
void FinalizarJogo(EstadoJogo* estado) {
    if (!estado->rodando) return; // Jogo já finalizado, não faz nada

    estado->rodando = false; // Para o jogo

    // Calcula o bônus APENAS UMA VEZ
    if (!estado->bonusCalculado) {
        estado->pontuacao += (estado->contGod * 500);
        estado->bonusCalculado = true;
    }
}

// --- REMOVIDA A FUNÇÃO 'RestartGame' ---


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
        // Posição ajustada: PISO - obsAltura + AJUSTE_VERTICAL
        novoObs->hitbox.y = PISO - obsAltura + AJUSTE_VERTICAL;
        novoObs->hitbox.width = obsLargura;
        novoObs->hitbox.height = obsAltura;
        novoObs->textura = estado->texObstaculoTerrestre2x1; // Urso Polar
        novoObs->tipo = 0; // Inimigo de dano
        novoObs->id_power_up = 0;

        novoNo->obstaculo = novoObs;
        novoNo->proximo = *lista;
        *lista = novoNo;

        // Contagem para EVO se for obstáculo terrestre
        if (pontuacao >= PONTUACAO_MINIMA_EVO) estado->power_up_terrestre_counter++;
        return;
    }

    Obstaculo* novoObs = (Obstaculo*)malloc(sizeof(Obstaculo));
    if (novoObs == NULL) return;
    NoObstaculo* novoNo = (NoObstaculo*)malloc(sizeof(NoObstaculo));
    if (novoNo == NULL) { free(novoObs); return; }

    novoObs->hitbox.x = LARGURA_TELA + 5;
    novoObs->tipo = 0;
    novoObs->id_power_up = 0; // Inicializa como não Power-up

    if (tipo == 0) { // Terrestre (Simples 1x1: Pedra 1)
        // Posição ajustada: PISO - obsAltura + AJUSTE_VERTICAL
        novoObs->hitbox.y = PISO - obsAltura + AJUSTE_VERTICAL;
        novoObs->hitbox.width = obsLargura;
        novoObs->hitbox.height = obsAltura;
        novoObs->textura = estado->texObstaculoTerrestre; // pedra1
        // CONTAGEM EVO: Apenas conta se for o inimigo 1x1 real E a pontuação for suficiente
        if (pontuacao >= PONTUACAO_MINIMA_EVO) estado->power_up_terrestre_counter++;
    }
    else if (tipo == 1) { // Aéreo (Pelicano Normal)
        novoObs->hitbox.width = obsLargura;
        novoObs->hitbox.height = obsAltura;
        novoObs->textura = estado->texObstaculoAereoNormal; // Pelicano

        estado->power_up_aereo_counter++; // Conta o spawn aéreo

        int alturaSorteada = rand() % 3;
        // Posições aéreas NÃO são ajustadas
        if (alturaSorteada == 0) { novoObs->hitbox.y = PISO - 120; }
        else if (alturaSorteada == 1) { novoObs->hitbox.y = PISO - 100; }
        else { novoObs->hitbox.y = PISO - 80; }
    }
    else if (tipo == 2) { // Buraco/Chão Quebrado
        // O hitbox deve estar onde o chão estaria.
        novoObs->hitbox.y = PISO;
        novoObs->hitbox.height = 40;
        novoObs->tipo = 2; // Buraco/Chão Quebrado (Dano)
        novoObs->textura = estado->texBuraco; // Textura do chão quebrado

        int buracoLargura = LARGURA_BURACO_BLOCO * OBSTACULO_LARGURA_BASE;
        if (pontuacao > 650) {
            buracoLargura = (LARGURA_BURACO_BLOCO + 1) * OBSTACULO_LARGURA_BASE;
        }
        novoObs->hitbox.width = buracoLargura;
    }
    else if (tipo == 4) { // Terrestre 3# Vertical (Pedra 3)
        obsAltura = OBSTACULO_ALTURA_BASE * 3;
        // Posição ajustada: PISO - obsAltura + 10 (do pedido anterior) + AJUSTE_VERTICAL
        novoObs->hitbox.y = PISO - obsAltura + 10 + AJUSTE_VERTICAL;
        novoObs->hitbox.width = obsLargura;
        novoObs->hitbox.height = obsAltura;
        novoObs->textura = estado->texObstaculoVertical; // pedra3
        novoObs->tipo = 0;
    }
    else if (tipo == TIPO_IMORTAL) { // Power-Up Imortalidade (Pelicano Gold)
        novoObs->hitbox.width = obsLargura;
        novoObs->hitbox.height = obsAltura;
        novoObs->textura = estado->texPowerUpImortal; // Pelicano Gold
        novoObs->tipo = 1; // Power-up (Coletável)
        novoObs->id_power_up = 1; // Imortal

        int alturaSorteada = rand() % 3;
        if (alturaSorteada == 0) { novoObs->hitbox.y = PISO - 140; }
        else if (alturaSorteada == 1) { novoObs->hitbox.y = PISO - 120; }
        else { novoObs->hitbox.y = PISO - 100; }
    }
    else if (tipo == TIPO_EVO) { // Power-Up Evo (Terrestre: pedra1_evo)
        obsAltura = OBSTACULO_ALTURA_BASE;
        // Posição ajustada: PISO - obsAltura + AJUSTE_VERTICAL
        novoObs->hitbox.y = PISO - obsAltura + AJUSTE_VERTICAL;
        novoObs->hitbox.width = obsLargura;
        novoObs->hitbox.height = obsAltura;
        novoObs->textura = estado->texPowerUpEvo; // pedra1_evo
        novoObs->tipo = 1; // Power-up (Coletável)
        novoObs->id_power_up = 2; // Evo
    }


    novoNo->obstaculo = novoObs;
    novoNo->proximo = *lista;
    *lista = novoNo;
}


// --- Funções Principais do Jogo ---

void InitGame(EstadoJogo* estado, Pinguim* pinguim) {
    srand((unsigned int)time(NULL));

    // Carrega a textura da capa
    estado->texCapa = LoadTexture("imagens_jogo/cenario/capa_inicial.jpeg");
    if (estado->texCapa.id > 0) SetTextureFilter(estado->texCapa, TEXTURE_FILTER_BILINEAR);
    // ------------------------------------

    // Carrega texturas do Pinguim (Base, Gold, EVO)
    estado->texPinguimAndando = LoadTexture("imagens_jogo/pengoo/pengoo_surfando.png");
    estado->texPinguimPulando = LoadTexture("imagens_jogo/pengoo/pengoo_pulando.png");
    estado->texPinguimGoldAndando = LoadTexture("imagens_jogo/pengoo/pengoogold_surfando.png");
    estado->texPinguimGoldPulando = LoadTexture("imagens_jogo/pengoo/pengoogold_pulando.png");
    estado->texPinguimEvoAndando = LoadTexture("imagens_jogo/pengoo/pengooevo_surfando.png");
    estado->texPinguimEvoPulando = LoadTexture("imagens_jogo/pengoo/pengooevo_pulando.png");
    estado->texPinguimGodAndando = LoadTexture("imagens_jogo/pengoo/pengoogod_surfando.png");
    estado->texPinguimGodPulando = LoadTexture("imagens_jogo/pengoo/t.png");

    // Carrega Textura do CENÁRIO
    estado->texChao = LoadTexture("imagens_jogo/cenario/chao.png");
    estado->texBuraco = LoadTexture("imagens_jogo/cenario/buraco.png");
    estado->texNuvem = LoadTexture("imagens_jogo/cenario/nuvens.png");

    // Carrega Texturas de Obstáculos e Power-Ups
    estado->texObstaculoTerrestre = LoadTexture("imagens_jogo/inimigos/pedra1.png"); // 1x1
    estado->texObstaculoAereoNormal = LoadTexture("imagens_jogo/inimigos/pelicano.png"); // Pelicano Normal
    estado->texObstaculoTerrestre2x1 = LoadTexture("imagens_jogo/inimigos/urso_polar.png"); // 2x1 (Urso Polar)
    estado->texObstaculoVertical = LoadTexture("imagens_jogo/inimigos/pedra3.png"); // 3x1 (Pedra 3)

    estado->texPowerUpImortal = LoadTexture("imagens_jogo/inimigos/pelicanogold.png"); // Pelicano Gold (Imortal)
    estado->texPowerUpEvo = LoadTexture("imagens_jogo/inimigos/pedra1_evo.png"); // Imagem EVO

    // Aplica filtro de qualidade (verificar se as IDs são válidas antes)
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

    // --- ADICIONADO: Inicializa variáveis do Bônus GOD ---
    estado->contGod = 0;
    estado->godAtivoAnterior = false;
    estado->bonusCalculado = false;
    // --------------------------------------------------

    // Inicializa o Pinguim
    pinguim->position = (Vector2){ 50, PISO - PINGUIM_ALTURA_BASE };
    pinguim->velocidade_y = 0;
    pinguim->estaNoChao = true;
    pinguim->puloMaximo = 0; // 0 pulos aéreos restantes

    pinguim->imortal_ativo = false;
    pinguim->imortal_distancia_restante = 0;
    pinguim->evo_distancia_restante = 0;

    pinguim->hitbox = (Rectangle){ 50, pinguim->position.y, PINGUIM_LARGURA_BASE, PINGUIM_ALTURA_BASE };

    carregarHighScores(estado->topScores);
    InitNuvens(estado);
}


//
// LÓGICA DE 'UpdateGame' (REMOVIDA A OPÇÃO DE REINICIAR)
//
void UpdateGame(EstadoJogo* estado, Pinguim* pinguim) {
    
    if (!estado->rodando) {
        // O jogo está parado (GAME OVER)
        
        if (IsKeyPressed(KEY_X)) {
            CloseWindow(); // Sinaliza para fechar
        }
        // Se o jogo está parado, sempre retorna para não executar a lógica abaixo
        return; 
    }

    // --- O JOGO ESTÁ RODANDO ---


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

    // --- Lógica de Movimento e Respawn das Nuvens ---
    for (int i = 0; i < NUM_NUVENS; i++) {
        if (estado->nuvens[i].ativa) {
            // Move a nuvem para a esquerda
            estado->nuvens[i].position.x -= estado->nuvens[i].velocidade;

            // Se a nuvem saiu da tela pela esquerda, reposicione-a na direita
            // MODIFICADO: Lógica de respawn usa 'raio' (largura)
            if (estado->nuvens[i].position.x < -estado->nuvens[i].raio) {
                // Respawn aleatório na direita, fora da tela
                estado->nuvens[i].position.x = LARGURA_TELA + (float)(rand() % 100 + 50);
                // Nova altura e velocidade aleatória
                estado->nuvens[i].position.y = (float)(rand() % (ALTURA_TELA / 2 - 50));
                estado->nuvens[i].velocidade = VELOCIDADE_NUVEM + ((float)(rand() % 50) / 100.0f);
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

    // MODIFICADO: Chama a função de finalizar jogo
    if (pinguim->position.y > ALTURA_TELA) {
        FinalizarJogo(estado);
        return; // Retorna pois o jogo acabou
    }

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

    // --- ADICIONADO: Lógica do Contador GOD ---
    bool isEvo = pinguim->evo_distancia_restante > 0;
    bool isGod = pinguim->imortal_ativo && isEvo;

    if (isGod && !estado->godAtivoAnterior) {
        // O jogador acabou de ENTRAR no modo GOD
        estado->contGod++;
        estado->godAtivoAnterior = true;
    } else if (!isGod) {
        // Reseta o tracker se o modo GOD acabar
        estado->godAtivoAnterior = false;
    }
    // ----------------------------------------

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

            if (atual->obstaculo->tipo == 1) { // Power-Up

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
                    FinalizarJogo(estado); // MODIFICADO
                    break; // Sai do loop de colisão pois o jogo acabou
                } else {
                    atual->obstaculo->tipo = -1;
                }
            }
            else if (atual->obstaculo->tipo == 2) { // BURACO/CHÃO QUEBRADO (MORTE POR COLISÃO)
                FinalizarJogo(estado); // MODIFICADO
                break; // Sai do loop de colisão pois o jogo acabou
            }
        }

        atual = atual->proximo;
    }
}


void DrawGame(EstadoJogo* estado, Pinguim* pinguim, GameScreen currentScreen) {
    // Lógica principal de desenho decide o que mostrar com base no estado.

    // --- ALTERAÇÃO AQUI ---
    // (SEÇÃO 'TELA_TITULO' FOI COMBINADA COM 'TELA_TUTORIAL')
    //
    if (currentScreen == TELA_TITULO || currentScreen == TELA_TUTORIAL) {
        // --- TELA DE TÍTULO / TUTORIAL ---
        ClearBackground(SKYBLUE); // Fundo para caso a textura falhe

        if (estado->texCapa.id > 0) {
            // Desenha a nova arte da capa, cobrindo toda a tela
            Rectangle sourceRect = { 0.0f, 0.0f, (float)estado->texCapa.width, (float)estado->texCapa.height };
            Rectangle destRect = { 0, 0, LARGURA_TELA, ALTURA_TELA };
            DrawTexturePro(estado->texCapa, sourceRect, destRect, (Vector2){ 0, 0 }, 0.0f, WHITE);
        } else {
            // Fallback: Se a textura falhar, mostra um aviso
            const char *text = (currentScreen == TELA_TITULO) ? 
                    "Erro ao carregar capa_inicial.jpeg" : "Erro ao carregar tutorial";
            DrawText(text, 
                     LARGURA_TELA/2 - MeasureText(text, 20)/2, 
                     ALTURA_TELA/2 - 10, 20, RED);
        }

    } else {
        // --- TELA DE JOGO (JOGANDO ou FIM_DE_JOGO) ---
        ClearBackground(SKYBLUE);

        // --- Desenha as Nuvens (No fundo, antes do chão) ---
        for (int i = 0; i < NUM_NUVENS; i++) {
            Nuvem* nuvem = &estado->nuvens[i];
            if (nuvem->ativa && estado->texNuvem.id > 0) {
                float ratio = (float)estado->texNuvem.height / (float)estado->texNuvem.width;
                float altura = nuvem->raio * ratio;
                Rectangle sourceRect = { 0.0f, 0.0f, (float)estado->texNuvem.width, (float)estado->texNuvem.height };
                Rectangle destRect = { nuvem->position.x, nuvem->position.y, nuvem->raio, altura };
                Vector2 origin = { 0, 0 };
                DrawTexturePro(estado->texNuvem, sourceRect, destRect, origin, 0.0f, nuvem->cor);
            }
        }

        // --- 1. Desenha o Chão (Chão agora usa a textura repetida) ---
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

        char statusName[10] = "";
        int remainingDistance = 0;

        if (isGod) {
            strcpy(statusName, "GOD");
            remainingDistance = (pinguim->imortal_distancia_restante < pinguim->evo_distancia_restante)
                                ? pinguim->imortal_distancia_restante
                                : pinguim->evo_distancia_restante;
        } else if (isImortal) {
            strcpy(statusName, "Gold");
            remainingDistance = pinguim->imortal_distancia_restante;
        } else if (isEvo) {
            strcpy(statusName, "EVO");
            remainingDistance = pinguim->evo_distancia_restante;
        }

        Texture2D texAtual;

        // Seleção de Textura
        if (isGod) {
            texAtual = pinguim->estaNoChao ? estado->texPinguimGodAndando : estado->texPinguimGodPulando;
        } else if (isImortal) {
            texAtual = pinguim->estaNoChao ? estado->texPinguimGoldAndando : estado->texPinguimGoldPulando;
        } else if (isEvo) {
            texAtual = pinguim->estaNoChao ? estado->texPinguimEvoAndando : estado->texPinguimEvoPulando;
        } else { // Normal
            texAtual = pinguim->estaNoChao ? estado->texPinguimAndando : estado->texPinguimPulando;
        }

        Color tint = WHITE;

        // Desenho Padrão (Força 50x55)
        Rectangle sourceRect = { 0.0f, 0.0f, (float)texAtual.width, (float)texAtual.height };
        Rectangle destRect = {
            pinguim->position.x,
            pinguim->position.y,
            (float)PINGUIM_LARGURA_BASE,
            (float)PINGUIM_ALTURA_BASE
        };

        Vector2 origin = { 0, 0 };
        float rotation = 0.0f;

        DrawTexturePro(texAtual, sourceRect, destRect, origin, rotation, tint);


        // --- 3. Desenha os Obstáculos ---
        NoObstaculo* atual = estado->listaDeObstaculos;
        while (atual != NULL) {
            Obstaculo* obs = atual->obstaculo;

            if (obs->tipo == 0 || obs->tipo == 1) { // Sólido e Power-Up
                Texture2D obsTex = obs->textura;

                if (obsTex.id > 0) {
                    DrawTexturePro(obsTex,
                                   (Rectangle){0, 0, (float)obsTex.width, (float)obsTex.height},
                                   obs->hitbox,
                                   origin, rotation, WHITE);
                } else {
                    // Fallback de cor, caso a textura não carregue
                    Color fallbackColor = BLACK;
                    if (obs->id_power_up == 1) { fallbackColor = YELLOW; }
                    else if (obs->id_power_up == 2) { fallbackColor = VIOLET; }
                    DrawRectangleRec(obs->hitbox, fallbackColor);
                }
            }
            else if (obs->tipo == 2) { // BURACO/CHÃO QUEBRADO
                    DrawTexturePro(obs->textura,
                                   (Rectangle){0, 0, (float)obs->textura.width, (float)obs->textura.height},
                                   obs->hitbox,
                                   origin, rotation, WHITE);
            }

            atual = atual->proximo;
        }

        // --- 4. HUD ---
        char hud[200];
        char power_up_status[60] = "";

        if (isImortal || isEvo) {
            sprintf(power_up_status, " | %s! (%d)", statusName, remainingDistance);
        }
            sprintf(hud, "Pontos: %d | Velocidade: %.1f%s | RECORDE: %s %d",
                    estado->pontuacao, estado->velocidadeJogo, power_up_status, estado->topScores[0].nome, estado->topScores[0].pontuacao);

        DrawText(hud, 10, 10, 20, BLACK);

        // --- 5. TELA DE GAME OVER (MODIFICADO) ---
        if (!estado->rodando) {
            // Desenha o background escurecido
            DrawRectangle(0, 0, LARGURA_TELA, ALTURA_TELA, (Color){ 0, 0, 0, 150 });

            DrawText("G A M E   O V E R", LARGURA_TELA/2 - MeasureText("G A M E   O V E R", 40)/2, ALTURA_TELA/2 - 60, 40, RED);

            // Prepara os textos do score
            int bonusGod = (estado->contGod * 500);
            int pontuacaoBase = estado->pontuacao - bonusGod;

            char textoScoreBase[100];
            char textoBonus[100];
            char textoTotal[100];

            sprintf(textoScoreBase, "Pontuacao Base: %d", pontuacaoBase);
            sprintf(textoBonus, "Bonus GOD (x%d): %d", estado->contGod, bonusGod);
            sprintf(textoTotal, "TOTAL: %d", estado->pontuacao);

            // Centraliza os textos
            int y_start = ALTURA_TELA/2 - 10;
            DrawText(textoScoreBase, LARGURA_TELA/2 - MeasureText(textoScoreBase, 20)/2, y_start, 20, WHITE);
            DrawText(textoBonus, LARGURA_TELA/2 - MeasureText(textoBonus, 20)/2, y_start + 30, 20, YELLOW);
            DrawText(textoTotal, LARGURA_TELA/2 - MeasureText(textoTotal, 30)/2, y_start + 70, 30, GREEN);
            
            // Texto de Sair
            if (((int)(GetTime() * 2)) % 2 == 0) { // Pisca
                char textoOpcoes[100];
                sprintf(textoOpcoes, "Pressione [X] para sair");
                DrawText(textoOpcoes, LARGURA_TELA/2 - MeasureText(textoOpcoes, 20)/2, y_start + 120, 20, WHITE);
            }
        }
    }
}


void UnloadGame(EstadoJogo* estado, Pinguim* pinguim) {
    // Descarrega a textura da Capa
    UnloadTexture(estado->texCapa);
    // -----------------------------------------

    UnloadTexture(estado->texPinguimAndando);
    UnloadTexture(estado->texPinguimPulando);

    // Descarrega texturas dos inimigos
    UnloadTexture(estado->texObstaculoTerrestre);
    UnloadTexture(estado->texObstaculoAereoNormal);
    UnloadTexture(estado->texObstaculoTerrestre2x1);
    UnloadTexture(estado->texObstaculoVertical);

    // Descarrega texturas de Power-Up
    UnloadTexture(estado->texPowerUpImortal);
    UnloadTexture(estado->texPowerUpEvo);

    // Descarrega texturas de Pinguim
    UnloadTexture(estado->texPinguimGoldAndando);
    UnloadTexture(estado->texPinguimGoldPulando);
    UnloadTexture(estado->texPinguimEvoAndando);
    UnloadTexture(estado->texPinguimEvoPulando);
    UnloadTexture(estado->texPinguimGodAndando);
    UnloadTexture(estado->texPinguimGodPulando);

    // Descarrega textura do CENÁRIO
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