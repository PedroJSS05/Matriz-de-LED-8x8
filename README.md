# 8x8 LED Matrix Arcade (Arduino Uno)

Este projeto implementa um console de jogos retrô utilizando uma matriz de LED 8x8, controlada por um microcontrolador **Arduino UNO** e um contador **CD4017BE**. O sistema utiliza o conceito de **Framebuffer** e multiplexação de hardware para renderizar gráficos em tempo real.

## Funcionalidades
* **Menu de Seleção:** Interface visual para escolher entre os jogos.
* **Snake:** O jogo da cobrinha com detecção de colisão e crescimento.
* **Space Invaders:** Inimigos em bloco, movimentação lateral/descendente e sistema de tiro.
* **Tetris:** Lógica de encaixe de peças e limpeza de linhas (adaptado para 8x8).

## Hardware e Componentes

A arquitetura foca na economia de pinos, utilizando o **CD4017** para varredura de linhas e o Arduino como *sink* de corrente para as colunas.

* **1x** Arduino UNO
* **1x** Matriz de LED 1088BS
* **1x** Contador CD4017BE
* **16x** Resistores de 1kΩ (Proteção)
* **4x** Pushbuttons (Controles)

### Pinagem (Pinout)

| Componente | Pino Arduino | Função |
| :--- | :--- | :--- |
| **CD4017** | 10 | Clock (Avança Linha) |
| **CD4017** | 11 | Reset (Volta para Linha 0) |
| **Matriz (Colunas)** | 2 a 9 | Catodos (0 a 7) - Lógica *Active-Low* |
| **Botão Direita** | A0 | Entrada (Pull-up) |
| **Botão Esquerda** | A1 | Entrada (Pull-up) |
| **Botão Cima/Ação** | A2 | Entrada (Pull-up) |
| **Botão Baixo** | A3 | Entrada (Pull-up) |

## Funcionamento Técnico

### Multiplexação e Persistência de Visão (POV)
O **CD4017** atua como o driver das linhas (Anodos). O Arduino envia um pulso de clock rápido, fazendo com que apenas uma linha seja ativada por vez. Enquanto uma linha está ativa (VCC), o Arduino coloca os pinos das colunas desejadas em `LOW` (GND), completando o circuito. Devido à alta frequência de atualização, o olho humano percebe a imagem como se todos os LEDs estivessem acesos simultaneamente.

### Lógica de Software (Framebuffer)
O estado da tela é armazenado em um array de 8 bytes (64 bits). Cada bit representa um pixel (1 para aceso, 0 para apagado). As funções dos jogos manipulam esse buffer e a função de renderização descarrega os dados no hardware bit a bit.