/*
 * ========================================
 * ARQUIVO DE CABEÇALHO: src/game.h (Versão Raylib)
 * ========================================
 * Este ficheiro define as estruturas e funções do nosso jogo.
 */

#ifndef GAME_H
#define GAME_H

// Inclui a biblioteca Raylib (para tipos como Texture2D, Rectangle, etc.)
#include "raylib.h"

// --- Requisito: Structs ---

typedef struct {
    char nome[4]; // 3 letras + 1 caractere nulo '\0'
    int pontuacao;
} Score;

typedef struct {
    Vector2 position;     // Posição (x, y) - tipo da Raylib
    float velocidade_y;
    bool estaNoChao;
    bool puloDuploDisponivel;
    Texture2D textura;    // <-- MUDANÇA: A imagem .png do pinguim
    Rectangle hitbox;     // <-- MUDANÇA: O retângulo de colisão
} Pinguim;

typedef struct {
    Rectangle hitbox;     // <-- MUDANÇA: Posição (x,y) e tamanho (largura, altura)
    int tipo;             // 0=Sólido (pedra, aéreo), 2=Buraco
    Texture2D textura;    // <-- MUDANÇA: A imagem .png do obstáculo
} Obstaculo;

// --- Requisito: Listas Encadeadas ---
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
    
    // --- Novas variáveis de estado do Raylib ---
    float contadorSpawn; // Contador de tempo (em segundos)
    float intervaloSpawnAtual;
    int proximoNivelPontuacao;
    
    // --- Texturas (Imagens) ---
    // Precisamos de carregar as imagens uma vez e usá-las várias vezes
    Texture2D texPinguim;
    Texture2D texObstaculoTerrestre;
    Texture2D texObstaculoAereo;
    Texture2D texObstaculoVertical; // Para o 3#
    
} EstadoJogo;


/*
 * ========================================
 * FUNÇÕES (Protótipos)
 * ========================================
 * Estas são as funções que o nosso main.c vai chamar.
 * A lógica delas está no game.c
 */

// --- Funções Principais do Jogo ---
void InitGame(EstadoJogo* estado, Pinguim* pinguim); // Inicializa o jogo, carrega texturas
void UpdateGame(EstadoJogo* estado, Pinguim* pinguim); // Atualiza a física, inputs, lógica
void DrawGame(EstadoJogo* estado, Pinguim* pinguim);   // Desenha tudo na tela
void UnloadGame(EstadoJogo* estado); // Liberta a memória (texturas, lista encadeada)

// --- Funções de Lógica (internas, chamadas por UpdateGame) ---
void carregarHighScores(Score topScores[3]);
void salvarHighScores(Score topScores[3]);
int obterRanking(Score topScores[3], int pontuacaoAtual); 
void adicionarNovoScore(Score topScores[3], int pontuacaoAtual, char* nome, int ranking); 
void adicionarObstaculo(NoObstaculo** lista, int pontuacao, EstadoJogo* estado); // Passa o estado para aceder às texturas

#endif // GAME_H
