#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include <stdbool.h>

// --- Enum para gerenciar as telas do jogo ---
typedef enum GameScreen {
    TELA_TITULO = 0,
    JOGANDO,
    FIM_DE_JOGO
} GameScreen;

// --- Constantes ---
#define INTERVALO_SPAWN_INICIAL 3.0f
#define PONTOS_PARA_SUBIR_NIVEL 200
#define INTERVALO_SPAWN_MINIMO 0.7f

// Tamanho jogável do Pinguim (50x55)
#define PINGUIM_LARGURA_BASE 50
#define PINGUIM_ALTURA_BASE 55

// Tamanho de visualização/hitbox dos Obstáculos (Base 40x40)
#define OBSTACULO_LARGURA_BASE 40
#define OBSTACULO_ALTURA_BASE 40

// Largura do buraco (4 blocos)
#define LARGURA_BURACO_BLOCO 4

// --- Constantes do Power-Up Imortalidade ---
#define PONTUACAO_MINIMA_IMORTALIDADE 2000
#define CICLE_SPAWN_IMORTALIDADE 5
#define DISTANCIA_IMORTALIDADE 500
#define TIPO_IMORTAL 5

// --- Constantes do Power-Up EVO (Pulo Triplo) ---
#define PONTUACAO_MINIMA_EVO 2000
#define CICLE_SPAWN_EVO 4
#define DISTANCIA_EVO 500
#define TIPO_EVO 6

// --- Nuvens ---
#define NUM_NUVENS 10
#define VELOCIDADE_NUVEM 0.5f

// --- Estrutura de Nuvens ---
typedef struct {
    Vector2 position;
    Color cor;        // Cor de tint (tingimento)
    float raio;       // Usado como largura da textura
    float velocidade;
    bool ativa;
} Nuvem;

// --- Estruturas ---
typedef struct {
    char nome[4];
    int pontuacao;
} Score;

typedef struct {
    Rectangle hitbox;
    int tipo;               // 0 = Dano, 1 = Power-Up, 2 = Buraco
    Texture2D textura;
    int id_power_up;       // 1 = Imortal, 2 = Evo
} Obstaculo;

// --- Lista encadeada de obstáculos ---
typedef struct NoObstaculo {
    Obstaculo* obstaculo;
    struct NoObstaculo* proximo;
} NoObstaculo;

// --- Estrutura do Pinguim ---
typedef struct {
    Vector2 position;
    float velocidade_y;
    bool estaNoChao;
    int puloMaximo;

    Rectangle hitbox;

    // Power-ups
    int imortal_distancia_restante;
    int evo_distancia_restante;
    bool imortal_ativo;
} Pinguim;

// --- Estrutura principal do jogo ---
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

    // Textura da capa
    Texture2D texCapa;

    // Texturas do Pinguim
    Texture2D texPinguimAndando;
    Texture2D texPinguimPulando;
    Texture2D texPinguimGoldAndando;
    Texture2D texPinguimGoldPulando;
    Texture2D texPinguimEvoAndando;
    Texture2D texPinguimEvoPulando;
    Texture2D texPinguimGodAndando;
    Texture2D texPinguimGodPulando;

    // Texturas do cenário
    Texture2D texChao;
    Texture2D texBuraco;
    Texture2D texNuvem;

    // Texturas de inimigos
    Texture2D texObstaculoTerrestre;
    Texture2D texObstaculoAereoNormal;
    Texture2D texObstaculoTerrestre2x1;
    Texture2D texObstaculoVertical;

    // Texturas de Power-Up
    Texture2D texPowerUpImortal;
    Texture2D texPowerUpEvo;

    int power_up_aereo_counter;
    int power_up_terrestre_counter;
    Nuvem nuvens[NUM_NUVENS];

    // Variáveis de bônus GOD
    int contGod;
    bool godAtivoAnterior;
    bool bonusCalculado;
} EstadoJogo;

// --- Protótipos de Funções ---
void InitGame(EstadoJogo* estado, Pinguim* pinguim);
void UpdateGame(EstadoJogo* estado, Pinguim* pinguim);
void DrawGame(EstadoJogo* estado, Pinguim* pinguim, GameScreen currentScreen);
void UnloadGame(EstadoJogo* estado, Pinguim* pinguim);

void carregarHighScores(Score topScores[3]);
void salvarHighScores(Score topScores[3]);
int obterRanking(Score topScores[3], int pontuacaoAtual);
void adicionarNovoScore(Score topScores[3], int pontuacaoAtual, char* nome, int ranking);
void adicionarObstaculo(NoObstaculo** lista, int pontuacao, EstadoJogo* estado);
void InitNuvens(EstadoJogo* estado);
void FinalizarJogo(EstadoJogo* estado);

// --- ALTERAÇÃO AQUI ---
// (REMOVIDO PROTÓTIPO 'RestartGame')

#endif // GAME_H