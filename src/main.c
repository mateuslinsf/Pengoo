/*
 * ========================================
 * ARQUIVO PRINCIPAL: src/main.c (Versão Raylib)
 * ========================================
 * Este ficheiro é o "coração" do jogo.
 * Ele apenas cria a janela e corre o loop principal.
 */

#include "raylib.h"
#include "game.h"
#include <stdio.h> // Para printf e scanf no final
#include <unistd.h> // Para STDIN_FILENO
#include <fcntl.h>  // Para fcntl

// --- Constantes da Janela ---
// (Estes valores TÊM de ser iguais aos de game.c)
#define LARGURA_TELA 800
#define ALTURA_TELA 450

int main(void) {
    
    // --- 1. Inicialização ---
    EstadoJogo estado;
    Pinguim pinguim;

    // Cria a janela gráfica
    InitWindow(LARGURA_TELA, ALTURA_TELA, "Pengoo (Versão Raylib)");
    
    // Inicializa o áudio (necessário para sons, se quisermos adicionar)
    InitAudioDevice();

    // Trava o FPS (não precisamos mais de usleep)
    SetTargetFPS(60);

    // Chama a nossa função em game.c para carregar tudo
    // (texturas, high scores, física inicial)
    InitGame(&estado, &pinguim);

    // Salva o estado normal do terminal (para o scanf funcionar no fim)
    int old_flags = fcntl(STDIN_FILENO, F_GETFL);


    // --- 2. Game Loop ---
    // O loop corre enquanto o 'estado' estiver 'rodando' E
    // o utilizador não carregar no 'X' da janela.
    while (estado.rodando && !WindowShouldClose()) {
        
        // --- 2.1 Atualizar Lógica ---
        // (Chama a função em game.c que faz tudo: 
        //  input, física, colisão, spawn, etc.)
        UpdateGame(&estado, &pinguim);

        // --- 2.2 Desenhar ---
        // (Chama a função em game.c que desenha tudo)
        DrawGame(&estado, &pinguim);
    }

    // --- 3. Finalização ---
    
    // Liberta a memória (texturas, lista encadeada)
    UnloadGame(&estado);
    
    // Fecha a janela gráfica
    CloseAudioDevice();
    CloseWindow();

    // --- Lógica de Fim de Jogo (High Score) ---
    // (Esta lógica é quase idêntica à do jogo em terminal)
    
    // Restaura o terminal para o modo normal (para o scanf funcionar)
    fcntl(STDIN_FILENO, F_SETFL, old_flags);

    int ranking = obterRanking(estado.topScores, estado.pontuacao);

    if (ranking > 0) {
        printf("\n--- NOVO RECORDE! ---\n");
        printf("Sua pontuação: %d\n", estado.pontuacao);
        
        if (ranking == 1) { printf("Você ficou em 1º LUGAR!\n"); }
        else if (ranking == 2) { printf("Você ficou em 2º LUGAR!\n"); }
        else { printf("Você ficou em 3º LUGAR!\n"); }

        printf("Digite suas 3 iniciais (ex: LFG): ");
        char nome[10];
        scanf("%3s", nome);
        
        adicionarNovoScore(estado.topScores, estado.pontuacao, nome, ranking);
        salvarHighScores(estado.topScores);
        printf("Score salvo!\n");
    } else {
        printf("Game Over! Pontuação Final: %d\n", estado.pontuacao);
    }
    
    printf("\n--- TOP 3 SCORES ---\n");
    for (int i = 0; i < 3; i++) {
        printf("%d. %s .... %d\n", i+1, estado.topScores[i].nome, estado.topScores[i].pontuacao);
    }

    return 0;
}
