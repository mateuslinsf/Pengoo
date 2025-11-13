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

// --- CONSTANTES DO POWER-UP IMORTALIDADE ---
#define PONTUACAO_MINIMA_IMORTALIDADE 2000
#define CICLE_SPAWN_IMORTALIDADE 5
#define DISTANCIA_IMORTALIDADE 500
#define TIPO_IMORTAL 5 // Tipo usado na lógica de spawn
// ------------------------------------

// --- NOVAS CONSTANTES DO POWER-UP EVO (Pulo Triplo) ---
#define PONTUACAO_MINIMA_EVO 2000
#define CICLE_SPAWN_EVO 4      // O 5º (depois de 4) será o EVO
#define DISTANCIA_EVO 500
#define TIPO_EVO 6             // Novo tipo usado na lógica de spawn
// ------------------------------------

//--- Nuvens ---
#define NUM_NUVENS 10//Máximo de nuvens na tela
#define VELOCIDADE_NUVEM 0.5f//Velocidade horizontal lenta

// --- Estruturas de Nuvens ---
typedef struct {
    Vector2 position;
    Color cor;
    float raio; // MODIFICADO: Será usado como LARGURA da textura
    float velocidade; 
    bool ativa; 
} Nuvem;


// --- Structs ---
typedef struct {
    char nome[4];
    int pontuacao;
} Score;

typedef struct {
    Rectangle hitbox;
    int tipo; // 0=Sólido/Dano, 1=Power-Up Coletável, 2=Buraco/Chão Quebrado
    Texture2D textura;
    int id_power_up; // 1=Imortal, 2=Evo (Para diferenciar Power-ups com tipo=1)
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
    int puloMaximo; 
    
    Rectangle hitbox;

    // Power-ups (VARIAVEIS INDEPENDENTES)
    int imortal_distancia_restante; 
    int evo_distancia_restante;     
    bool imortal_ativo; 
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

    // Texturas Pinguim (Todas carregadas aqui)
    Texture2D texPinguimAndando;
    Texture2D texPinguimPulando;
    Texture2D texPinguimGoldAndando;
    Texture2D texPinguimGoldPulando;
    Texture2D texPinguimEvoAndando;
    Texture2D texPinguimEvoPulando;
    Texture2D texPinguimGodAndando; 
    Texture2D texPinguimGodPulando; 

    // Textura do CENÁRIO
    Texture2D texChao; // chao.png
    Texture2D texBuraco; // buraco.png
    Texture2D texNuvem; // nuvens.png (ADICIONADO)
    
    // Texturas de Inimigos 
    Texture2D texObstaculoTerrestre; // pedra1.png (1x1)
    Texture2D texObstaculoAereoNormal; // pelicano.png (Aéreo Normal)
    Texture2D texObstaculoTerrestre2x1; // urso_polar.png (1x2)
    Texture2D texObstaculoVertical; // pedra3.png (3x1)

    // Texturas de Power-Up
    Texture2D texPowerUpImortal; // pelicanogold.png (Imortal Power-up)
    Texture2D texPowerUpEvo; // pedra1_evo.png (EVO Power-up)

    int power_up_aereo_counter;
    int power_up_terrestre_counter; 
    Nuvem nuvens[NUM_NUVENS];
    
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
void InitNuvens(EstadoJogo* estado);

#endif // GAME_H