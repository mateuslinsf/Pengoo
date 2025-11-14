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
    GameScreen currentScreen = TELA_TITULO; // Começa na TELA_TITULO

    // Cria o Target 800x450
    estado.target = LoadRenderTexture(GAME_VIRTUAL_WIDTH, GAME_VIRTUAL_HEIGHT);
    SetTextureFilter(estado.target.texture, TEXTURE_FILTER_BILINEAR);

    // InitGame() carrega TODAS as texturas (Capa, Pinguim, etc)
    InitGame(&estado, &pinguim);

    // Salva o estado normal do terminal (para o printf final, se houver)
    // int old_flags = fcntl(STDIN_FILENO, F_GETFL); // Removido pois não é mais necessário

    // Loop principal
    while (!WindowShouldClose()) {

        // Toggle Fullscreen com F11
        if (IsKeyPressed(KEY_F11)) {
            ToggleFullscreen();
        }
        
        // --- ALTERAÇÃO AQUI ---
        // UpdateGame agora gerencia TODAS as atualizações e transições de tela
        UpdateGame(&estado, &pinguim, &currentScreen);
        // --- FIM DA ALTERAÇÃO ---


        // --- 1. Renderização no Target Virtual ---
        BeginTextureMode(estado.target);
            // DrawGame vai desenhar a tela correta (Titulo, Tutorial, Jogo, etc)
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

    // --- ALTERAÇÃO AQUI ---
    // A lógica de High Score do terminal foi REMOVIDA
    // (ela agora acontece dentro do jogo)
    // --- FIM DA ALTERAÇÃO ---
    
    // fcntl(STDIN_FILENO, F_SETFL, old_flags); // Removido

    return 0;
}