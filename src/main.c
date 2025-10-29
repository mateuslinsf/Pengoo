/*
 * ========================================
 * ARQUIVO PRINCIPAL: src/main.c
 * ========================================
 * Este arquivo contém o "Game Loop" principal.
 */

// Bibliotecas da cli-lib
#include "screen.h"
#include "keyboard.h"
#include "timer.h"

// Nosso arquivo com as structs do jogo
#include "game.h"

// Bibliotecas padrão
#include <stdio.h>
#include <unistd.h> // Para a função usleep()

// --- Constantes do Jogo ---
#define LARGURA_TELA 80
#define ALTURA_TELA 24
#define PISO (ALTURA_TELA - 2) // Posição Y do chão

int main() {
    
    // --- 1. Inicialização ---
    EstadoJogo estado;
    Pinguim pinguim;

    screenInit(1); // O '1' esconde o cursor
    keyboardInit();

    estado.rodando = 1; // 1 = true
    estado.pontuacao = 0;
    estado.velocidadeJogo = 1.0f;
    estado.listaDeObstaculos = NULL;

    pinguim.x = 5;
    pinguim.y = PISO; 
    pinguim.velocidade_y = 0.0f;
    pinguim.estaNoChao = 1;
    pinguim.puloDuploDisponivel = 1;
    pinguim.arteASCII = "P"; 

    // --- 2. Game Loop ---
    while (estado.rodando) {
        
        // --- 2.1 Processar Inputs ---
        // (A cli-lib não tem uma função de input que não trava o jogo)
        // (Vamos ter que implementar 'kbhit()' nós mesmos depois)
        // (Por enquanto, para sair do jogo, aperte Ctrl+C no terminal)


        // --- 2.2 Atualizar Lógica ---
        // (nada ainda)

        // --- 2.3 Renderizar (Desenhar) a Tela ---
        screenClear(); 
        
        screenGotoxy(pinguim.x, pinguim.y);
        printf("%s", pinguim.arteASCII);
        
        screenGotoxy(0, PISO + 1);
        for(int i = 0; i < LARGURA_TELA; i++) {
            printf("-");
        }

        screenGotoxy(0, 0); 
        // Vamos remover o "(Aperte 'q' para sair)" por enquanto
        printf("Pontuação: %d", estado.pontuacao); 

        screenUpdate(); 

        // --- 2.4 Controle de "Frame Rate" ---
        // Usamos usleep (microssegundos) da <unistd.h>
        // 33000 microssegundos = 33 milissegundos = ~30 FPS
        usleep(33000); 
    }

    // --- 3. Finalização ---
    screenDestroy();
    keyboardDestroy();

    printf("Game Over! Pontuação Final: %d\n", estado.pontuacao);

    return 0;
}





