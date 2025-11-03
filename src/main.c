/*
 * ========================================
 * ARQUIVO PRINCIPAL: src/main.c (Versão Raylib - GAME OVER CORRIGIDO)
 * ========================================
 */

#include "raylib.h"
#include "game.h"
#include <stdio.h> // Para printf e scanf no final
#include <unistd.h> // Para STDIN_FILENO
#include <fcntl.h>  // Para fcntl

// --- Constantes da Janela ---
#define LARGURA_TELA 800
#define ALTURA_TELA 450

int main(void) {
    
    // --- 1. Inicialização ---
    EstadoJogo estado;
    Pinguim pinguim;

    InitWindow(LARGURA_TELA, ALTURA_TELA, "Pengoo (Versão Raylib)");
    InitAudioDevice();
    SetTargetFPS(60);

    InitGame(&estado, &pinguim);

    int old_flags = fcntl(STDIN_FILENO, F_GETFL);


    // --- 2. Game Loop ---
    while (estado.rodando && !WindowShouldClose()) {
        
        UpdateGame(&estado, &pinguim);
        DrawGame(&estado, &pinguim);
    }

    // --- 3. Finalização ---
    
    // <-- MUDANÇA: Lógica de Game Over -->
    // O loop quebrou (ou 'q' ou morreu)
    // Se morreu (e não foi pelo 'X' da janela), mostre a tela de Game Over por 2 segundos
    if (!WindowShouldClose()) {
        
        // Desenha a tela final (com a mensagem "GAME OVER")
        // (A função DrawGame vai checar 'estado.rodando == false' e mostrar o texto)
        DrawGame(&estado, &pinguim);
        
        // Espera 2 segundos
        WaitTime(2.0); // (2.0 segundos)
    }

    // Agora sim, limpa a memória e fecha a janela
    UnloadGame(&estado);
    CloseAudioDevice();
    CloseWindow();

    // --- Lógica de Fim de Jogo (High Score) ---
    // (O resto do código (scanf, printf) permanece igual)
    
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
