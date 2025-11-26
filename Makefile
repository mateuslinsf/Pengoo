# 1. Compilador
CC = gcc

# 2. Nome do executável
TARGET = pengoo

# 3. arquivos .c 
SRCS = src/main.c src/game.c

# 4. Converte a lista de .c para .o 
OBJS = $(SRCS:.c=.o)

# 5. Onde achar o raylib.h
CFLAGS = -Wall -I/usr/local/include -Isrc -std=c99 -Wno-unused-variable

# 6. conecta com a Raylib
LDFLAGS = -L/usr/local/lib -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
