/*
 * ========================================
 * ARQUIVO: src/game.h (Atualizado com Ranking)
 * ========================================
 */

#ifndef GAME_H
#define GAME_H

// --- Structs (sem mudanças) ---
typedef struct {
    char nome[4]; // 3 letras (ex: 'LFG') + 1 caractere nulo '\0'
    int pontuacao;
} Score;

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
    Score topScores[3];
} EstadoJogo;


/*
 * ========================================
 * FUNÇÕES (Protótipos)
 * ========================================
 */

// --- Funções de High Score (Atualizadas) ---
void carregarHighScores(Score topScores[3]);
void salvarHighScores(Score topScores[3]);
// <-- MUDANÇA AQUI: Renomeada e com retorno de ranking -->
int obterRanking(Score topScores[3], int pontuacaoAtual); 
// <-- MUDANÇA AQUI: Agora recebe o ranking para inserir certo -->
void adicionarNovoScore(Score topScores[3], int pontuacaoAtual, char* nome, int ranking); 


// --- Funções do Jogo (Sem mudanças) ---
Pinguim criarPinguim(int x, int y);
void atualizarPinguim(Pinguim* pinguim);
void pinguimPular(Pinguim* pinguim);

void adicionarObstaculo(NoObstaculo** lista, int pontuacao); 
void atualizarObstaculos(NoObstaculo** lista, float velocidade);
void desenharObstaculos(NoObstaculo* lista);
void liberarListaObstaculos(NoObstaculo** lista);

void inicializarJogo(EstadoJogo* estado, Pinguim* pinguim);
void desenharJogo(EstadoJogo* estado, Pinguim* pinguim);

#endif // GAME_H
