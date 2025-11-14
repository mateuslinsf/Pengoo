/*
 * ========================================
 * ARQUIVO PRINCIPAL: src/main.c (VERSÃO FINAL COM RESIZE E RENDERIZACAO CORRETA)
 * ========================================
 */

#include "raylib.h"
#include "game.h"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <string.h>

#define GAME_VIRTUAL_WIDTH 800
#define GAME_VIRTUAL_HEIGHT 450

int main(void) {

    // Configurações iniciais
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_FULLSCREEN_MODE);

    // Inicia a janela na resolução do monitor primário
    int monitor = GetCurrentMonitor();
    int screenWidth = GetMonitorWidth(monitor);
    int screenHeight = GetMonitorHeight(monitor);

    InitWindow(screenWidth, screenHeight, "Pengoo - O Jogo do Pinguim");

    InitAudioDevice();
    SetTargetFPS(60);

    // Inicialização da Câmera Virtual e Jogo
    EstadoJogo estado = {0};
    Pinguim pinguim = {0};

    // Variável para controlar a tela atual
    GameScreen currentScreen = TELA_TITULO;

    // Cria o Target 800x450
    estado.target = LoadRenderTexture(GAME_VIRTUAL_WIDTH, GAME_VIRTUAL_HEIGHT);
    SetTextureFilter(estado.target.texture, TEXTURE_FILTER_BILINEAR);

    InitGame(&estado, &pinguim);

    // Salva o estado normal do terminal
    int old_flags = fcntl(STDIN_FILENO, F_GETFL);

    // Loop principal
    while (!WindowShouldClose()) {

        // Entrada da Tela de Título
        if (currentScreen == TELA_TITULO) {
            if (GetKeyPressed() != 0) {
                currentScreen = JOGANDO;
            }
        }

        // Toggle Fullscreen com F11
        if (IsKeyPressed(KEY_F11)) {
            ToggleFullscreen();
        }

        // Atualização do jogo
        if (currentScreen == JOGANDO ||
            (currentScreen == FIM_DE_JOGO && !estado.rodando)) {

            UpdateGame(&estado, &pinguim);

            if (!estado.rodando) {
                currentScreen = FIM_DE_JOGO;
            }
        }

        // --- 1. Renderização no Target Virtual ---
        BeginTextureMode(estado.target);
            DrawGame(&estado, &pinguim, currentScreen);
        EndTextureMode();

        // --- 2. Desenha na tela real com Letterbox ---
        BeginDrawing();
            ClearBackground(BLACK);

            float scale = fminf(
                (float)GetScreenWidth() / GAME_VIRTUAL_WIDTH,
                (float)GetScreenHeight() / GAME_VIRTUAL_HEIGHT
            );

            int offsetX = (GetScreenWidth() - (GAME_VIRTUAL_WIDTH * scale)) / 2;
            int offsetY = (GetScreenHeight() - (GAME_VIRTUAL_HEIGHT * scale)) / 2;

            DrawTexturePro(
                estado.target.texture,
                (Rectangle){0, 0, estado.target.texture.width, -estado.target.texture.height},
                (Rectangle){offsetX, offsetY, GAME_VIRTUAL_WIDTH * scale, GAME_VIRTUAL_HEIGHT * scale},
                (Vector2){0, 0},
                0.0f,
                WHITE
            );
        EndDrawing();
    }

    // --- 3. Finalização ---
    UnloadRenderTexture(estado.target);
    UnloadGame(&estado, &pinguim);
    CloseWindow();

    // Restaura o terminal
    fcntl(STDIN_FILENO, F_SETFL, old_flags);

    // High Score ao final
    if (currentScreen == FIM_DE_JOGO) {

        int ranking = obterRanking(estado.topScores, estado.pontuacao);

        if (ranking > 0) {

            printf("\n--- NOVO RECORDE! ---\n");
            printf("Sua pontuação: %d\n", estado.pontuacao);

            if (ranking == 1)      printf("Você ficou em 1º LUGAR!\n");
            else if (ranking == 2) printf("Você ficou em 2º LUGAR!\n");
            else                   printf("Você ficou em 3º LUGAR!\n");

            printf("Digite suas 3 iniciais (ex: LFG): ");
            char nome[10];
            scanf("%3s", nome);

            adicionarNovoScore(estado.topScores, estado.pontuacao, nome, ranking);
            salvarHighScores(estado.topScores);

            printf("Score salvo!\n");

        } else {
            printf("Game Over! Pontuação Final: %d\n", estado.pontuacao);
        }
    }

    printf("\n--- TOP 3 SCORES ---\n");
    for (int i = 0; i < 3; i++) {
        printf("%d. %s .... %d\n", i + 1,
               estado.topScores[i].nome,
               estado.topScores[i].pontuacao);
    }

    return 0;
}