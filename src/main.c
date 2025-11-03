/*
 * ========================================
 * ARQUIVO PRINCIPAL: src/main.c (COM PULO E GRAVIDADE)
 * ========================================
 */

// Bibliotecas da cli-lib
#include "screen.h"
#include "keyboard.h"
#include "timer.h"

// Nosso arquivo com as structs do jogo
#include "game.h"

// Bibliotecas padrão
#include <stdio.h>
#include <unistd.h> // Para usleep() e read()
#include <fcntl.h>  // Para o Quit Fácil (fcntl)

// --- Constantes do Jogo ---
#define LARGURA_TELA 80
#define ALTURA_TELA 24
#define PISO (ALTURA_TELA - 2)
#define GRAVIDADE 0.5f
#define FORCA_PULO -1.5f // <-- MUDANÇA PULO: Força do pulo (negativo é para cima)

int main() {
    
    // --- 1. Inicialização ---
    EstadoJogo estado;
    Pinguim pinguim;

    screenInit(1);
    keyboardInit();
    
    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);

    estado.rodando = 1;
    estado.pontuacao = 0;
    estado.velocidadeJogo = 1.0f;
    estado.listaDeObstaculos = NULL;

    pinguim.x = 5;
    pinguim.y = 10; 
    pinguim.velocidade_y = 0.0f;
    pinguim.estaNoChao = 0;
    pinguim.puloDuploDisponivel = 1;
    pinguim.arteASCII = "P"; 

    // --- 2. Game Loop ---
    while (estado.rodando) {
        
        // --- 2.1 Processar Inputs ---
        char tecla = '\0';
        read(STDIN_FILENO, &tecla, 1); 

        if (tecla == 'q') {
            estado.rodando = 0;
        }

        // --- MUDANÇA PULO: Lógica do Pulo/Pulo Duplo ---
        if (tecla == ' ') { // ' ' (espaço) é a tecla de pulo
            if (pinguim.estaNoChao) {
                // Pulo Normal
                pinguim.velocidade_y = FORCA_PULO;
                pinguim.estaNoChao = 0;
                pinguim.puloDuploDisponivel = 1; // Habilita o pulo duplo
            } else if (pinguim.puloDuploDisponivel) {
                // Pulo Duplo
                pinguim.velocidade_y = FORCA_PULO; // Dá outro impulso
                pinguim.puloDuploDisponivel = 0; // Desabilita até tocar o chão
            }
        }
        
        // --- 2.2 Atualizar Lógica (Gravidade) ---
        pinguim.velocidade_y = pinguim.velocidade_y + GRAVIDADE;
        pinguim.y = (int)(pinguim.y + pinguim.velocidade_y);

        if (pinguim.y >= PISO) {
            pinguim.y = PISO;
            pinguim.velocidade_y = 0;
            pinguim.estaNoChao = 1;
            pinguim.puloDuploDisponivel = 1; // Reseta o pulo duplo no chão
        }
        
        // --- 2.3 Renderizar (Desenhar) a Tela ---
        screenClear(); 
        
        screenGotoxy(pinguim.x, pinguim.y);
        printf("%s", pinguim.arteASCII);
        
        screenGotoxy(0, PISO + 1);
        for(int i = 0; i < LARGURA_TELA; i++) {
            printf("-");
        }

        screenGotoxy(0, 0); 
        printf("Pontuação: %d (Aperte 'q' para sair)", estado.pontuacao);

        screenUpdate(); 

        // --- 2.4 Controle de "Frame Rate" ---
        usleep(33000); 
    }

    // --- 3. Finalização ---
    screenDestroy();
    keyboardDestroy();

    printf("Game Over! Pontuação Final: %d\n", estado.pontuacao);

    return 0;
}





