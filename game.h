#ifndef GAME_H
#define GAME_H

/*
 * ========================================
 * ESTRUTURAS DE DADOS (structs)
 * ========================================
 * Este arquivo define todas as "peças" do nosso jogo.
 */

// --- Requisito: Structs e Ponteiros ---

/**
 * @brief Define o jogador (nosso Pinguim).
 * Guarda a posição (x, y), velocidade e estado do pulo.
 */
typedef struct {
    int x;
    int y;
    float velocidade_y;      // Usado para gravidade e força do pulo
    int estaNoChao;          // 1 (true) se está no chão, 0 (false) se está no ar
    int puloDuploDisponivel; // 1 (true) se pode dar pulo duplo, 0 (false) se não
    char* arteASCII;         // Ponteiro para a arte do pinguim
} Pinguim;


/**
 * @brief Define um obstáculo genérico.
 * Pode ser um cacto, uma plataforma ou um buraco.
 */
typedef struct {
    int x;
    int y;
    int largura;
    int altura;
    int tipo; // Ex: 0 = cacto (morte), 1 = plataforma (pode subir)
    char* arteASCII;
} Obstaculo;


// --- Requisito: Listas Encadeadas e Alocação Dinâmica ---

/**
 * @brief Este é o "nó" da nossa lista encadeada de obstáculos.
 * Cada nó guarda um PONTEIRO para um obstáculo e aponta para o próximo nó.
 */
typedef struct NoObstaculo {
    Obstaculo* obstaculo;          // Ponteiro para o obstáculo (criado com malloc)
    struct NoObstaculo* proximo; // Ponteiro para o próximo nó na lista
} NoObstaculo;


// --- Estrutura Principal do Jogo ---

/**
 * @brief Guarda o estado geral do jogo.
 * Inclui a pontuação, se o jogo está rodando, e a lista de obstáculos.
 */
typedef struct {
    int pontuacao;
    int rodando;         // 1 (true) = jogo rodando, 0 (false) = game over
    float velocidadeJogo; // Quão rápido o cenário (e obstáculos) se movem
    
    // A cabeça da nossa lista encadeada de obstáculos
    NoObstaculo* listaDeObstaculos; 

} EstadoJogo;


/*
 * ========================================
 * FUNÇÕES (Protótipos)
 * ========================================
 * Aqui vamos listar as funções que vamos criar depois.
 */

// Funções do Pinguim
Pinguim criarPinguim(int x, int y);
void atualizarPinguim(Pinguim* pinguim); // Usa ponteiro para modificar o pinguim
void pinguimPular(Pinguim* pinguim);

// Funções dos Obstáculos (Lista Encadeada)
void adicionarObstaculo(NoObstaculo** lista, Obstaculo* obs); // Usa ponteiro de ponteiro
void atualizarObstaculos(NoObstaculo** lista, float velocidade);
void desenharObstaculos(NoObstaculo* lista);
void liberarListaObstaculos(NoObstaculo** lista);

// Funções do Jogo
void inicializarJogo(EstadoJogo* estado, Pinguim* pinguim);
void desenharJogo(EstadoJogo* estado, Pinguim* pinguim);


#endif // GAME_H