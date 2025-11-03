/*
 * ========================================
 * ARQUIVO: src/game.h (Atualizado)
 * ========================================
 */

#ifndef GAME_H
#define GAME_H

// --- Structs (permanecem iguais) ---
typedef struct {
    int x;
    int y;
    float velocidade_y;
    int estaNoChao;
    int puloDuploDisponivel;
    char* arteASCII;
} Pinguim;

typedef struct {
    int x;
    int y;
    int largura;
    int altura;
    int tipo; 
    char* arteASCII;
} Obstaculo;

typedef struct NoObstaculo {
    Obstaculo* obstaculo;
    struct NoObstaculo* proximo;
} NoObstaculo;

typedef struct {
    int pontuacao;
    int rodando;
    float velocidadeJogo;
    NoObstaculo* listaDeObstaculos; 
} EstadoJogo;


/*
 * ========================================
 * FUNÇÕES (Protótipos)
 * ========================================
 */

Pinguim criarPinguim(int x, int y);
void atualizarPinguim(Pinguim* pinguim);
void pinguimPular(Pinguim* pinguim);

// <-- MUDANÇA AQUI: Função agora recebe a pontuação -->
void adicionarObstaculo(NoObstaculo** lista, int pontuacao); 

void atualizarObstaculos(NoObstaculo** lista, float velocidade);
void desenharObstaculos(NoObstaculo* lista);
void liberarListaObstaculos(NoObstaculo** lista);

void inicializarJogo(EstadoJogo* estado, Pinguim* pinguim);
void desenharJogo(EstadoJogo* estado, Pinguim* pinguim);

#endif // GAME_H
