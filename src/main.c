/*
 * ARQUIVO DE TESTE: src/main.c
 * Objetivo: Testar se a cli-lib e o Makefile funcionam.
 */

// Incluímos os cabeçalhos da biblioteca que estão em 'libs/cli-lib/include/'
#include "screen.h"
#include "keyboard.h"
#include "timer.h"

#include <stdio.h> // Para a função printf
#include <unistd.h>

int main() {

    // 1. Inicia a biblioteca de tela
    // (O '1' provavelmente significa "esconder o cursor")
    screenInit(1);

    // 2. Limpa a tela do terminal
    screenClear();

    // 3. Move o cursor para a posição (x=10, y=5)
    screenGotoxy(10, 5);

    // 4. Imprime a mensagem
    printf("Olá Pinguim! Projeto configurado.");

    // 5. Move o cursor para outra linha
    screenGotoxy(10, 7);
    printf("A cli-lib está funcionando!");

    // 6. Atualiza a tela (algumas bibliotecas precisam disso para mostrar)
    screenUpdate();

    // 7. Espera 3 segundos (3000 milissegundos) para podermos ler
    sleep(3);

    // 8. Restaura o terminal ao estado original antes de sair
    screenDestroy();

    return 0;
}




