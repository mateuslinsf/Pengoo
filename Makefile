# 1. Compilador
CC = gcc

# 2. Flags de Compilação
# -Wall = Mostra todos os avisos (Warnings)
# -Ilibs/cli-lib/include = Avisa onde estão os arquivos .h da biblioteca
CFLAGS = -Wall -Ilibs/cli-lib/include

# 3. Nome do seu executável final
TARGET = pengoo

# 4. Arquivos .c que VÃO ser compilados
# (Note que não incluímos o 'libs/cli-lib/src/main.c', pois ele é o 'main' de exemplo da lib)
SRCS = src/main.c \
       libs/cli-lib/src/screen.c \
       libs/cli-lib/src/keyboard.c \
       libs/cli-lib/src/timer.c

# 5. Converte a lista de .c para .o (arquivos objeto)
OBJS = $(SRCS:.c=.o)

# 6. Regra principal: o que fazer quando você digita 'make'
all: $(TARGET)

# 7. Regra de "linkagem": junta todos os .o para criar o executável 'pengoo'
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET)

# 8. Regra de compilação: como transformar um .c em um .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# 9. Regra 'clean': apaga tudo que foi compilado para recomeçar
clean:
	rm -f $(OBJS) $(TARGET)

