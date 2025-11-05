/*
 * ========================================
 * ARQUIVO DE CABEÇALHO: src/game.h (VERSÃO FINAL DE JOGABILIDADE)
 * ========================================
 */

#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include <stdbool.h> 

// --- Constantes de Jogo ---
#define INTERVALO_SPAWN_INICIAL 3.0f 
#define PONTOS_PARA_SUBIR_NIVEL 200  
#define INTERVALO_SPAWN_MINIMO 0.7f 

// Tamanho JOGÁVEL do Pinguim (50x55)
#define PINGUIM_LARGURA_BASE 50 
#define PINGUIM_ALTURA_BASE 55

// Tamanho de visualização/hitbox dos Obstáculos (Base 40x40)
#define OBSTACULO_LARGURA_BASE 40 
#define OBSTACULO_ALTURA_BASE 40 

// Largura do buraco (4 e 5 blocos)
#define LARGURA_BURACO_BLOCO 4 

// --- NOVAS CONSTANTES DO POWER-UP ---
#define PONTUACAO_MINIMA_POWERUP 2000
#define CICLE_SPAWN_POWERUP 5
#define DISTANCIA_IMORTALIDADE 500
// ------------------------------------


// --- Structs ---
typedef struct {
    char nome[4];
    int pontuacao;
} Score;

typedef struct {
    Vector2 position;
    float velocidade_y;
    bool estaNoChao;
    bool puloDuploDisponivel;
    Rectangle hitbox;
    
    bool imortal;
    int imortal_distancia_restante;

    Texture2D texAndando; 
    Texture2D texPulando;
    Texture2D texGoldAndando;
    Texture2D texGoldPulando;
} Pinguim;

typedef struct {
    Rectangle hitbox;
    int tipo; 
    Texture2D textura;
} Obstaculo;

// --- Listas Encadeadas ---
typedef struct NoObstaculo {
    Obstaculo* obstaculo;
    struct NoObstaculo* proximo;
} NoObstaculo;

// --- Estrutura Principal do Jogo ---
typedef struct {
    int pontuacao;
    bool rodando;
    float velocidadeJogo;
    NoObstaculo* listaDeObstaculos; 
    Score topScores[3];
    
    float contadorSpawn;
    float intervaloSpawnAtual;
    int proximoNivelPontuacao;
    
    RenderTexture2D target; 
    
    // Texturas do Jogo
    Texture2D texPinguimAndando;
    Texture2D texPinguimPulando;
    Texture2D texObstaculoTerrestre;
    Texture2D texObstaculoAereo;
    Texture2D texObstaculoVertical;
    Texture2D texPowerUp;
    Texture2D texPinguimGoldAndando;
    Texture2D texPinguimGoldPulando;
    
    int power_up_aereo_counter;
    
} EstadoJogo;


// --- Funções (Protótipos) ---
void InitGame(EstadoJogo* estado, Pinguim* pinguim);
void UpdateGame(EstadoJogo* estado, Pinguim* pinguim);
void DrawGame(EstadoJogo* estado, Pinguim* pinguim);
void UnloadGame(EstadoJogo* estado, Pinguim* pinguim); 

void carregarHighScores(Score topScores[3]);
void salvarHighScores(Score topScores[3]);
int obterRanking(Score topScores[3], int pontuacaoAtual); 
void adicionarNovoScore(Score topScores[3], int pontuacaoAtual, char* nome, int ranking); 
void adicionarObstaculo(NoObstaculo** lista, int pontuacao, EstadoJogo* estado);

#endif // GAME_H
