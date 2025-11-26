# Pengoo! ❄️

## Descrição do Projeto 

O **Pengoo** é um game baseado no cliff jump (pou), em que o personagem principal, Pengo, surfa em uma prancha e deve saltar e desviar de declives, obstáculos e  encostas para evitar cair ou colidir.

## Mecânicas do Game 🎮

  <li> Jogador pressiona o botão espaço para fazer o Pengo pular, se apertar duas vezes faz um pulo duplo.
  <li> Ao coletar o Cristal Roxo, Jogador ganha o poder de pulo triplo temporariamente com o Pengo EVO.
  <li> Ao coletar a Gaviota especial, Jogador ganha o poder de imortalidade temporária com o Pengo GOLD.
  <li> Ao coletar os dois poderes, Jogador fica com os dois poderes ao mesmo tempo temporariamente e o Pengo se transfoma em Pengo GOD.
  <li> Jogador deve desviar de ursos polares, obstáculos de gelo, gaivotas e gelo quebrando no chão.


## Poderes do Pengo:

<img src="imagens_jogo/pengoo/pengooevo_surfando.png" alt="Pengoevo_surfando" width="250"/> 
<img src="imagens_jogo/pengoo/pengoogold_surfando.png" alt="Pengogold_surfando" width="250"/> 
<img src="imagens_jogo/pengoo/pengoogod_surfando.png" alt="Pengogod_surfando" width="225"/> 

---

## Imagens do Game :
<img src="imagens_jogo/cenario/capa_inicial.jpeg" alt="Capa inicial" width="250"/>  
<img src="imagens_jogo/cenario/gameplay.png" alt="game play" width="250"/> 
<img src="imagens_jogo/cenario/gameover.png" alt=" game over " width="250"/> 


---

## 💻 Como Baixar e Compilar
Para jogar **Pengoo!**, você precisará clonar o repositório e compilar o código-fonte.

### 1. Pré-requisitos
Certifique-se de ter instalado em sua máquina:

- Git  
- Compilador C (GCC)  
- Raylib (Instalada via Homebrew no macOS ou gerenciador de pacotes no Linux)  

No Windows, utilize o **WSL + Ubuntu** para compilar o jogo.

---

### 2. Clonando o Repositório
Abra seu terminal e execute:

git clone https://github.com/mateuslinsf/Pengoo.git

cd Pengoo


---

### 3. Compilando e Rodando

---

### 🍎 macOS (Via Homebrew)
Se você usa macOS e instalou a Raylib via Homebrew, utilize:
gcc -std=c99 *.c -o pengoo -lraylib -lm -lpthread -ldl -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo


Para rodar o jogo:

./pengoo


---

### 🐧 Linux (Ubuntu/Debian)
Certifique-se de ter as dependências instaladas e rode:

gcc -std=c99 *.c -o pengoo -lraylib -lm -lpthread -ldl -lGL

Para rodar:

./pengoo


---

### 🪟 Windows (via WSL + Ubuntu)
⚠️ O jogo foi desenvolvido e testado apenas em Linux.  
Para jogar no Windows, utilize o **WSL** e instale o Ubuntu:

Instale o WSL:

wsl --install


Depois, no Ubuntu:

sudo apt update
sudo apt install gcc make libraylib-dev
gcc -std=c99 *.c -o pengoo -lraylib -lm -lpthread -ldl -lGL


Para rodar:

./pengoo


## Equipe de Desenvolvimento 👥: 

* **Luiz Felipe Siqueira** - [@LuizNoCode](https://github.com/LuizNoCode)
* **Mateus Lins Farias** - [@mateuslinsf](https://github.com/mateuslinsf)
* **Pedro David Oliveira Baia** - [@Pedrodavidob](https://github.com/Pedrodavidob)
