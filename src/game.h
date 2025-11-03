#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include <stdbool.h> 

// --- Constantes ---
#define INTERVALO_SPAWN_INICIAL 3.0f 
#define PONTOS_PARA_SUBIR_NIVEL 200  
#define INTERVALO_SPAWN_MINIMO 0.7f 
#define PINGUIM_LARGURA 40 
#define PINGUIM_ALTURA 40  


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
    
    // Texturas do Jogo
    Texture2D texPinguimAndando;
    Texture2D texPinguimPulando;
    Texture2D texObstaculoTerrestre;
    Texture2D texObstaculoAereo;
    Texture2D texObstaculoVertical;
    
} EstadoJogo;


// --- Funções (Protótipos) ---
void InitGame(EstadoJogo* estado, Pinguim* pinguim);
void UpdateGame(EstadoJogo* estado, Pinguim* pinguim);
void DrawGame(EstadoJogo* estado, Pinguim* pinguim);
void UnloadGame(EstadoJogo* estado, Pinguim* pinguim); // Agora recebe Pinguim*

void carregarHighScores(Score topScores[3]);
void salvarHighScores(Score topScores[3]);
int obterRanking(Score topScores[3], int pontuacaoAtual); 
void adicionarNovoScore(Score topScores[3], int pontuacaoAtual, char* nome, int ranking); 
void adicionarObstaculo(NoObstaculo** lista, int pontuacao, EstadoJogo* estado);

#endif // GAME_H
