

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

    
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_FULLSCREEN_MODE);

    // Inicia a janela na resolução do monitor
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

    // Cria o Target 
    estado.target = LoadRenderTexture(GAME_VIRTUAL_WIDTH, GAME_VIRTUAL_HEIGHT);
    SetTextureFilter(estado.target.texture, TEXTURE_FILTER_BILINEAR);

    // carrega TODAS as texturas 
    InitGame(&estado, &pinguim);

    // Loop principal
    while (!WindowShouldClose()) {

        // Fullscreen com F11
        if (IsKeyPressed(KEY_F11)) {
            ToggleFullscreen();
        }
        
        // UpdateGame gerencia as atualizações e transições de tela
        UpdateGame(&estado, &pinguim, &currentScreen);
        

        // --- 1. Renderização no Target Virtual ---
        BeginTextureMode(estado.target);
            // DrawGame desenha a tela (Titulo, Tutorial, Jogo, etc)
            DrawGame(&estado, &pinguim, currentScreen);
        EndTextureMode();

        // --- 2. Desenha na tela real com letterbox ---
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

    return 0;
}