Este código utiliza as libs do FFMPEG e o OpenCV  para abrir um arquivo de vídeo (video.mp4) e exibir o mesmo em uma janela.
Portanto, é necessário ter o FFMPEG e o OpenCV instalados na máquina.

# Instalação da Dependências
1. Para instalar o FFMPEG, basta executar o comando:
   sudo apt-get install libavcodec-dev libavformat-dev libavutil-dev libswscale-dev

2. Para instalar o OpenCV, basta executar o comando:
   sudo apt-get install libopencv-dev

# Execução
1. Para executar o código, basta executar o comando: 
   ./player

# Compilação
1. Após a instalação das dependências, basta compilar o código com o comando:
   g++ -g player.cpp -o player `pkg-config --cflags --libs libavcodec libavformat libavutil libswscale opencv4`

2. Para sair do player, basta manter pressionado a tecla ESC.
   O codec utilizado pelo vídeo é impresso no terminal.
