/*
 * ========================================
 * ARQUIVO DE CABEÇALHO: src/game.h (VERSÃO FINAL SINCRONIZADA)
 * ========================================
 */

#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include <stdbool.h> 

// --- Constantes ---
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

// --- NOVAS CONSTANTES DO POWER-UP (SÓ IMORTALIDADE) ---
#define PONTUACAO_MINIMA_IMORTALIDADE 2000
#define CICLE_SPAWN_IMORTALIDADE 5
#define DISTANCIA_HABILIDADE 500
// ------------------------------------


// --- Structs ---
typedef struct {
    char nome[4];
    int pontuacao;
} Score;

typedef struct {
    Rectangle hitbox;
    int tipo; // 0=Sólido, 1=Power-Up, 2=Buraco
    Texture2D textura;
} Obstaculo;

// --- Listas Encadeadas (Ordem Corrigida) ---
typedef struct NoObstaculo {
    Obstaculo* obstaculo;
    struct NoObstaculo* proximo;
} NoObstaculo;

typedef struct {
    Vector2 position;
    float velocidade_y;
    bool estaNoChao;
    // CORREÇÃO: ADICIONAMOS O MEMBRO QUE FALTAVA
    int puloMaximo; // 2 (normal)
    // ----------------------------------------
    
    Rectangle hitbox;

    bool imortal_ativo;
    int habilidade_distancia_restante;

    Texture2D texAndando;
    Texture2D texPulando;
    Texture2D texGoldAndando;
    Texture2D texGoldPulando;
} Pinguim;

// --- Estrutura Principal do Jogo ---
typedef struct {
    int pontuacao;
    bool rodando;
    float velocidadeJogo;
    float velocidadeBase; 
    NoObstaculo* listaDeObstaculos; 
    Score topScores[3];

    float contadorSpawn;
    float intervaloSpawnAtual;
    int proximoNivelPontuacao;

    RenderTexture2D target;

    // Texturas
    Texture2D texPinguimAndando;
    Texture2D texPinguimPulando;
    Texture2D texObstaculoTerrestre;
    Texture2D texObstaculoAereo;
    Texture2D texObstaculoVertical;
    Texture2D texPowerUpImortal; 
    Texture2D texPinguimGoldAndando;
    Texture2D texPinguimGoldPulando;

    int power_up_aereo_counter;
    int power_up_terrestre_counter; 
    
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
