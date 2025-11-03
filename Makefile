#
# Makefile para o Projeto Pengoo com RAYLIB
#

# 1. Compilador
CC = gcc

# 2. Nome do executável final
TARGET = pengoo

# 3. Nossos arquivos .c (Note que só temos o main.c e o game.c agora)
#    (Vamos criar o game.c no próximo passo)
SRCS = src/main.c src/game.c

# 4. Converte a lista de .c para .o (arquivos objeto)
OBJS = $(SRCS:.c=.o)

# 5. Flags de Compilação (Onde achar o raylib.h)
#    -Wall = Todos os avisos
#    -I/usr/local/include = Onde o 'sudo make install' colocou o raylib.h
#    -Isrc = Onde está o nosso game.h
CFLAGS = -Wall -I/usr/local/include -Isrc -std=c99 -Wno-unused-variable

# 6. Flags de Linkagem (Como conectar com a Raylib)
#    -L/usr/local/lib = Onde o 'sudo make install' colocou o libraylib.a
#    -lraylib = O nome da biblioteca Raylib
#    -lGL -lm -lpthread -ldl -lrt -lX11 = Outras bibliotecas que a Raylib precisa
LDFLAGS = -L/usr/local/lib -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

# 7. Regra principal: o que fazer quando você digita 'make'
all: $(TARGET)

# 8. Regra de "linkagem": junta tudo para criar o 'pengoo'
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

# 9. Regra de compilação: como transformar um .c em .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# 10. Regra 'clean': apaga tudo que foi compilado
clean:
	rm -f $(OBJS) $(TARGET)
