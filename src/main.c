/*
 * ========================================
 * ARQUIVO PRINCIPAL: src/main.c (COM GRAVIDADE E QUIT 'q')
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
#include <fcntl.h>  // <-- MUDANÇA: Para o Quit Fácil (fcntl)

// --- Constantes do Jogo ---
#define LARGURA_TELA 80
#define ALTURA_TELA 24
#define PISO (ALTURA_TELA - 2)
#define GRAVIDADE 0.5f         // <-- MUDANÇA: Adicionada a gravidade

int main() {
    
    // --- 1. Inicialização ---
    EstadoJogo estado;
    Pinguim pinguim;

    screenInit(1);
    keyboardInit();
    
    // --- MUDANÇA: Comando para o Quit Fácil ---
    // Coloca a leitura do teclado (stdin) em modo "não-bloqueante"
    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);

    estado.rodando = 1;
    estado.pontuacao = 0;
    estado.velocidadeJogo = 1.0f;
    estado.listaDeObstaculos = NULL;

    pinguim.x = 5;
    pinguim.y = 10; // <-- MUDANÇA: Pinguim começa no ar
    pinguim.velocidade_y = 0.0f;
    pinguim.estaNoChao = 0; // <-- MUDANÇA: Ele não está no chão
    pinguim.puloDuploDisponivel = 1;
    pinguim.arteASCII = "P"; 

    // --- 2. Game Loop ---
    while (estado.rodando) {
        
        // --- 2.1 Processar Inputs --- // <-- MUDANÇA: Quit Fácil
        char tecla = '\0';
        read(STDIN_FILENO, &tecla, 1); // Tenta ler 1 tecla

        if (tecla == 'q') {
            estado.rodando = 0; // Para o jogo
        }
        
        // --- 2.2 Atualizar Lógica ---  // <-- MUDANÇA: LÓGICA DA FÍSICA
        
        // 1. Aplica a gravidade à velocidade vertical
        pinguim.velocidade_y = pinguim.velocidade_y + GRAVIDADE;
        
        // 2. Atualiza a posição Y do pinguim baseado na velocidade
        pinguim.y = (int)(pinguim.y + pinguim.velocidade_y);

        // 3. Checa se o pinguim bateu no chão
        if (pinguim.y >= PISO) {
            pinguim.y = PISO;
            pinguim.velocidade_y = 0;
            pinguim.estaNoChao = 1;
            pinguim.puloDuploDisponivel = 1;
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
        printf("Pontuação: %d (Aperte 'q' para sair)", estado.pontuacao); // <-- MUDANÇA

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





