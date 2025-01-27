#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/adc.h"
#include "pico/bootrom.h"
#include "pio_matrix.pio.h"


#define NUM_PIXELS 25
#define OUT_PIN 7
#define NUM_PADROES 14
#define FPS 5
#define FRAME_TIME (3000/FPS)
#define FRAMES_PER_ANIMATION 5
#define TRANSITION_TIME 1000


typedef struct {
    double frames[FRAMES_PER_ANIMATION][25];
    double r, g, b;
} Animacao;


static Animacao animacoes[NUM_PADROES];
static uint8_t padrao_atual = 0;
static uint8_t frame_atual = 0;
static absolute_time_t proximo_frame;


void imprimir_binario(int num) {
    for (int i = 31; i >= 0; i--) {
        (num & (1 << i)) ? printf("1") : printf("0");
    }
}

uint32_t matrix_rgb(double b, double r, double g) {
    unsigned char R = r * 255;
    unsigned char G = g * 255;
    unsigned char B = b * 255;
    return (G << 24) | (R << 16) | (B << 8);
}


void init_quadrado(int index) {
    // Frame 1 - Apenas os cantos
    double frame1[25] = {
        1.0, 0.0, 0.0, 0.0, 1.0,
        0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0,
        1.0, 0.0, 0.0, 0.0, 1.0
    };

    // Frame 2 - Bordas superiores e inferiores
    double frame2[25] = {
        1.0, 1.0, 1.0, 1.0, 1.0,
        0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0,
        1.0, 1.0, 1.0, 1.0, 1.0
    };

    // Frame 3 - Bordas laterais adicionadas
    double frame3[25] = {
        1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 0.0, 0.0, 0.0, 1.0,
        1.0, 0.0, 0.0, 0.0, 1.0,
        1.0, 0.0, 0.0, 0.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0
    };

    // Frame 4 - Início do preenchimento interno
    double frame4[25] = {
        1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 0.0, 1.0, 1.0,
        1.0, 0.0, 0.0, 0.0, 1.0,
        1.0, 1.0, 0.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0
    };

    // Frame 5 - Quadrado completo com centro
    double frame5[25] = {
        1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0
    };

    memcpy(animacoes[index].frames[0], frame1, sizeof(frame1));
    memcpy(animacoes[index].frames[1], frame2, sizeof(frame2));
    memcpy(animacoes[index].frames[2], frame3, sizeof(frame3));
    memcpy(animacoes[index].frames[3], frame4, sizeof(frame4));
    memcpy(animacoes[index].frames[4], frame5, sizeof(frame5));
}

// Função para inicializar os frames do X
void init_x(int index) {
    // Frame 1 - X normal
    double frame1[25] = {
        1.0, 0.0, 0.0, 0.0, 1.0,
        0.0, 1.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 1.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 1.0, 0.0,
        1.0, 0.0, 0.0, 0.0, 1.0
    };
    
    // Frame 2 - X expandindo
    double frame2[25] = {
        1.0, 0.0, 0.0, 0.0, 1.0,
        0.0, 1.0, 0.5, 1.0, 0.0,
        0.0, 0.5, 1.0, 0.5, 0.0,
        0.0, 1.0, 0.5, 1.0, 0.0,
        1.0, 0.0, 0.0, 0.0, 1.0
    };
    
    // Frame 3 - X reduzindo e piscando
    double frame3[25] = {
        1.0, 0.0, 0.0, 0.0, 1.0,
        0.0, 1.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 1.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 1.0, 0.0,
        1.0, 0.0, 0.0, 0.0, 1.0
    };
    
    // Frame 4 - X com bordas brilhantes
    double frame4[25] = {
        1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 0.5, 0.5, 0.5, 1.0,
        1.0, 0.5, 0.0, 0.5, 1.0,
        1.0, 0.5, 0.5, 0.5, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0
    };
    
    // Frame 5 - X com centro
    double frame5[25] = {
        1.0, 0.0, 0.0, 0.0, 1.0,
        0.0, 1.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 1.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 1.0, 0.0,
        1.0, 0.0, 0.0, 0.0, 1.0
    };
    // Frames similares para as outras animações...
    memcpy(animacoes[index].frames[0], frame1, sizeof(frame1));
    memcpy(animacoes[index].frames[1], frame2, sizeof(frame2));
    memcpy(animacoes[index].frames[2], frame3, sizeof(frame3));
    memcpy(animacoes[index].frames[3], frame4, sizeof(frame4));
    memcpy(animacoes[index].frames[4], frame5, sizeof(frame5));
}
// Função para inicializar os frames do diamante
void init_diamante(int index) {
    // Frame 1 - Ponto central
    double frame1[25] = {
        0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0
    };
    
    // Frame 2 - Cruz pequena
    double frame2[25] = {
        0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0, 0.0,
        0.0, 1.0, 1.0, 1.0, 0.0,
        0.0, 0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0
    };
    
    // Frame 3 - Diamante básico
    double frame3[25] = {
        0.0, 0.0, 1.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 1.0, 0.0,
        1.0, 0.0, 0.0, 0.0, 1.0,
        0.0, 1.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 1.0, 0.0, 0.0
    };
    
    // Frame 4 - Diamante expandido
    double frame4[25] = {
        0.0, 1.0, 1.0, 1.0, 0.0,
        1.0, 0.0, 0.0, 0.0, 1.0,
        1.0, 0.0, 0.0, 0.0, 1.0,
        1.0, 0.0, 0.0, 0.0, 1.0,
        0.0, 1.0, 1.0, 1.0, 0.0
    };
    
    // Frame 5 - Diamante completo com brilho
    double frame5[25] = {
        1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 0.0, 0.0, 0.0, 1.0,
        1.0, 0.0, 0.0, 0.0, 1.0,
        1.0, 0.0, 0.0, 0.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0
    };

    memcpy(animacoes[index].frames[0], frame1, sizeof(frame1));
    memcpy(animacoes[index].frames[1], frame2, sizeof(frame2));
    memcpy(animacoes[index].frames[2], frame3, sizeof(frame3));
    memcpy(animacoes[index].frames[3], frame4, sizeof(frame4));
    memcpy(animacoes[index].frames[4], frame5, sizeof(frame5));
}


// Função para inicializar os frames do círculo

void init_circulo(int index) {
    // Frame 1 - Círculo inicial
    double frame1[25] = {
        0.0, 1.0, 1.0, 1.0, 0.0,
        1.0, 0.0, 0.0, 0.0, 1.0,
        1.0, 0.0, 0.0, 0.0, 1.0,
        1.0, 0.0, 0.0, 0.0, 1.0,
        0.0, 1.0, 1.0, 1.0, 0.0
    };
    
    // Frame 2 - Rotação 45°
    double frame2[25] = {
        0.0, 0.0, 1.0, 1.0, 0.0,
        0.0, 1.0, 0.0, 0.0, 1.0,
        1.0, 0.0, 0.0, 0.0, 1.0,
        1.0, 0.0, 0.0, 1.0, 0.0,
        0.0, 1.0, 1.0, 0.0, 0.0
    };
    
    // Frame 3 - Rotação 90°
    double frame3[25] = {
        0.0, 0.0, 1.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 1.0, 0.0,
        1.0, 0.0, 0.0, 0.0, 1.0,
        0.0, 1.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 1.0, 0.0, 0.0
    };
    
    // Frame 4 - Rotação 135°
    double frame4[25] = {
        0.0, 0.0, 1.0, 1.0, 0.0,
        1.0, 0.0, 0.0, 0.0, 1.0,
        1.0, 0.0, 0.0, 0.0, 1.0,
        0.0, 1.0, 0.0, 1.0, 0.0,
        0.0, 1.0, 1.0, 0.0, 0.0
    };
    
    // Frame 5 - Rotação 180°
    double frame5[25] = {
        0.0, 1.0, 1.0, 1.0, 0.0,
        1.0, 0.0, 0.0, 0.0, 1.0,
        1.0, 0.0, 0.0, 0.0, 1.0,
        1.0, 0.0, 0.0, 0.0, 1.0,
        0.0, 1.0, 1.0, 1.0, 0.0
    };

    memcpy(animacoes[index].frames[0], frame1, sizeof(frame1));
    memcpy(animacoes[index].frames[1], frame2, sizeof(frame2));
    memcpy(animacoes[index].frames[2], frame3, sizeof(frame3));
    memcpy(animacoes[index].frames[3], frame4, sizeof(frame4));
    memcpy(animacoes[index].frames[4], frame5, sizeof(frame5));
}

// Função para inicializar os frames do triângulo

void init_triangulo(int index) {
    // Frame 1 - 5% intensidade
    double frame1[25] = {
        0.0, 0.0, 0.05, 0.0, 0.0,
        0.0, 0.05, 0.0, 0.05, 0.0,
        0.05, 0.0, 0.0, 0.0, 0.05,
        0.05, 0.05, 0.05, 0.05, 0.05,
        0.0, 0.0, 0.0, 0.0, 0.0
    };
    
    // Frame 2 - 25% intensidade
    double frame2[25] = {
        0.0, 0.0, 0.25, 0.0, 0.0,
        0.0, 0.25, 0.0, 0.25, 0.0,
        0.25, 0.0, 0.0, 0.0, 0.25,
        0.25, 0.25, 0.25, 0.25, 0.25,
        0.0, 0.0, 0.0, 0.0, 0.0
    };
    
    // Frame 3 - 50% intensidade
    double frame3[25] = {
        0.0, 0.0, 0.5, 0.0, 0.0,
        0.0, 0.5, 0.0, 0.5, 0.0,
        0.5, 0.0, 0.0, 0.0, 0.5,
        0.5, 0.5, 0.5, 0.5, 0.5,
        0.0, 0.0, 0.0, 0.0, 0.0
    };
    
    // Frame 4 - 75% intensidade
    double frame4[25] = {
        0.0, 0.0, 0.75, 0.0, 0.0,
        0.0, 0.75, 0.0, 0.75, 0.0,
        0.75, 0.0, 0.0, 0.0, 0.75,
        0.75, 0.75, 0.75, 0.75, 0.75,
        0.0, 0.0, 0.0, 0.0, 0.0
    };
    
    // Frame 5 - 100% intensidade
    double frame5[25] = {
        0.0, 0.0, 1.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 1.0, 0.0,
        1.0, 0.0, 0.0, 0.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0,
        0.0, 0.0, 0.0, 0.0, 0.0
    };

    memcpy(animacoes[index].frames[0], frame1, sizeof(frame1));
    memcpy(animacoes[index].frames[1], frame2, sizeof(frame2));
    memcpy(animacoes[index].frames[2], frame3, sizeof(frame3));
    memcpy(animacoes[index].frames[3], frame4, sizeof(frame4));
    memcpy(animacoes[index].frames[4], frame5, sizeof(frame5));
}

void init_coracao(int index) {
    // Frame 1 - Coração pequeno
    double frame1[25] = {
        0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 1.0, 0.0,
        0.0, 1.0, 1.0, 1.0, 0.0,
        0.0, 0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0
    };
    
    // Frame 2 - Coração médio
    double frame2[25] = {
        0.0, 1.0, 0.0, 1.0, 0.0,
        1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0,
        0.0, 1.0, 1.0, 1.0, 0.0,
        0.0, 0.0, 1.0, 0.0, 0.0
    };
    
    // Frame 3 - Coração grande
    double frame3[25] = {
        0.0, 1.0, 0.0, 1.0, 0.0,
        1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0,
        0.0, 1.0, 1.0, 1.0, 0.0,
        0.0, 0.0, 1.0, 0.0, 0.0
    };
    
    // Frame 4 - Coração médio
    double frame4[25] = {
        0.0, 1.0, 0.0, 1.0, 0.0,
        1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0,
        0.0, 1.0, 1.0, 1.0, 0.0,
        0.0, 0.0, 1.0, 0.0, 0.0
    };
    
    // Frame 5 - Coração pequeno
    double frame5[25] = {
        0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 1.0, 0.0,
        0.0, 1.0, 1.0, 1.0, 0.0,
        0.0, 0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0
    };

    memcpy(animacoes[index].frames[0], frame1, sizeof(frame1));
    memcpy(animacoes[index].frames[1], frame2, sizeof(frame2));
    memcpy(animacoes[index].frames[2], frame3, sizeof(frame3));
    memcpy(animacoes[index].frames[3], frame4, sizeof(frame4));
    memcpy(animacoes[index].frames[4], frame5, sizeof(frame5));
}

// Função para inicializar os frames do mais
 void init_mais(int index) {
    // Frame 1 - Centro do mais
    double frame1[25] = {
        0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0
    };
    
    // Frame 2 - Braços pequenos
    double frame2[25] = {
        0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0, 0.0,
        0.0, 1.0, 1.0, 1.0, 0.0,
        0.0, 0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0
    };
    
    // Frame 3 - Braços médios
    double frame3[25] = {
        0.0, 0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0, 0.0,
        1.0, 1.0, 1.0, 1.0, 1.0,
        0.0, 0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0, 0.0
    };
    
    // Frame 4 - Braços quase completos
    double frame4[25] = {
        0.0, 0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0, 0.0,
        1.0, 1.0, 1.0, 1.0, 1.0,
        0.0, 0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0, 0.0
    };
    
    // Frame 5 - Mais completo
    double frame5[25] = {
        0.0, 0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0, 0.0,
        1.0, 1.0, 1.0, 1.0, 1.0,
        0.0, 0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0, 0.0
    };

    memcpy(animacoes[index].frames[0], frame1, sizeof(frame1));
    memcpy(animacoes[index].frames[1], frame2, sizeof(frame2));
    memcpy(animacoes[index].frames[2], frame3, sizeof(frame3));
    memcpy(animacoes[index].frames[3], frame4, sizeof(frame4));
    memcpy(animacoes[index].frames[4], frame5, sizeof(frame5));
}
 void init_espiral(int index) {
    // Frame 1 - Centro
    double frame1[25] = {
        0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0
    };

    // Frame 2 - Primeira volta
    double frame2[25] = {
        0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 1.0, 1.0, 0.0,
        0.0, 1.0, 1.0, 1.0, 0.0,
        0.0, 1.0, 1.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0
    };

    // Frame 3 - Segunda volta
    double frame3[25] = {
        1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0
    };

    // Frame 4 - Reduzindo
    double frame4[25] = {
        0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 1.0, 1.0, 0.0,
        0.0, 1.0, 1.0, 1.0, 0.0,
        0.0, 1.0, 1.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0
    };

    // Frame 5 - Voltando ao centro
    double frame5[25] = {
        0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0
    };

    memcpy(animacoes[index].frames[0], frame1, sizeof(frame1));
    memcpy(animacoes[index].frames[1], frame2, sizeof(frame2));
    memcpy(animacoes[index].frames[2], frame3, sizeof(frame3));
    memcpy(animacoes[index].frames[3], frame4, sizeof(frame4));
    memcpy(animacoes[index].frames[4], frame5, sizeof(frame5));
}

void init_onda(int index) {
    // Frame 1 - Onda início
    double frame1[25] = {
        1.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 1.0
    };

    // Frame 2
    double frame2[25] = {
        0.0, 1.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 1.0,
        1.0, 0.0, 0.0, 0.0, 0.0
    };

    // Frame 3
    double frame3[25] = {
        0.0, 0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 1.0,
        1.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0, 0.0
    };

    // Frame 4
    double frame4[25] = {
        0.0, 0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 1.0,
        1.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0, 0.0
    };

    // Frame 5
    double frame5[25] = {
        0.0, 0.0, 0.0, 0.0, 1.0,
        1.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 1.0, 0.0
    };

    memcpy(animacoes[index].frames[0], frame1, sizeof(frame1));
    memcpy(animacoes[index].frames[1], frame2, sizeof(frame2));
    memcpy(animacoes[index].frames[2], frame3, sizeof(frame3));
    memcpy(animacoes[index].frames[3], frame4, sizeof(frame4));
    memcpy(animacoes[index].frames[4], frame5, sizeof(frame5));
}

void init_ziguezague(int index) {
    // Frame 1
    double frame1[25] = {
        1.0, 0.0, 0.0, 0.0, 0.0,
        1.0, 0.0, 0.0, 0.0, 0.0,
        1.0, 0.0, 0.0, 0.0, 0.0,
        1.0, 0.0, 0.0, 0.0, 0.0,
        1.0, 0.0, 0.0, 0.0, 0.0
    };

    // Frame 2
    double frame2[25] = {
        0.0, 0.0, 0.0, 0.0, 1.0,
        0.0, 0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 1.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0, 0.0,
        1.0, 0.0, 0.0, 0.0, 0.0
    };

    // Frame 3
    double frame3[25] = {
        1.0, 1.0, 1.0, 1.0, 1.0,
        0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0
    };

    // Frame 4
    double frame4[25] = {
        0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0,
        1.0, 1.0, 1.0, 1.0, 1.0,
        0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0
    };

    // Frame 5
    double frame5[25] = {
        0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0,
        1.0, 1.0, 1.0, 1.0, 1.0
    };

    memcpy(animacoes[index].frames[0], frame1, sizeof(frame1));
    memcpy(animacoes[index].frames[1], frame2, sizeof(frame2));
    memcpy(animacoes[index].frames[2], frame3, sizeof(frame3));
    memcpy(animacoes[index].frames[3], frame4, sizeof(frame4));
    memcpy(animacoes[index].frames[4], frame5, sizeof(frame5));
}
void init_carinha(int index) {
    // Frame 1 - Olhos abertos
    double frame1[25] = {
        0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0,
        1.0, 0.0, 0.0, 0.0, 1.0,
        0.0, 1.0, 1.0, 1.0, 0.0
    };
    
    // Frame 2 - Olhos meio fechados
    double frame2[25] = {
        0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.5, 0.0, 0.5, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0,
        1.0, 0.0, 0.0, 0.0, 1.0,
        0.0, 1.0, 1.0, 1.0, 0.0
    };
    
    // Frame 3 - Olhos fechados
    double frame3[25] = {
        0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0,
        1.0, 0.0, 0.0, 0.0, 1.0,
        0.0, 1.0, 1.0, 1.0, 0.0
    };
    
    // Frame 4 - Olhos meio abertos
    double frame4[25] = {
        0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.5, 0.0, 0.5, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0,
        1.0, 0.0, 0.0, 0.0, 1.0,
        0.0, 1.0, 1.0, 1.0, 0.0
    };
    
    // Frame 5 - Olhos totalmente abertos
    double frame5[25] = {
        0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0,
        1.0, 0.0, 0.0, 0.0, 1.0,
        0.0, 1.0, 1.0, 1.0, 0.0
    };

    memcpy(animacoes[index].frames[0], frame1, sizeof(frame1));
    memcpy(animacoes[index].frames[1], frame2, sizeof(frame2));
    memcpy(animacoes[index].frames[2], frame3, sizeof(frame3));
    memcpy(animacoes[index].frames[3], frame4, sizeof(frame4));
    memcpy(animacoes[index].frames[4], frame5, sizeof(frame5));
}
    
   
void init_animacoes() {
    // Inicializa animações com frames
    for (int i = 0; i <11; i++) {
        switch(i) {
            case 0:
                init_quadrado(i);
                animacoes[i].r = 0.0; animacoes[i].g = 0.0; animacoes[i].b = 1.0;
                break;
            case 1:
                init_x(i);
                animacoes[i].r = 0.0; animacoes[i].g = 1.0; animacoes[i].b = 0.0;
                break;
            case 2: 
                init_diamante(i);
                animacoes[i].r = 1.0; animacoes[i].g = 0.0; animacoes[i].b = 0.0;
                break;
            case 3: 
                init_circulo(i);
                animacoes[i].r = 0.8; animacoes[i].g = 0.0; animacoes[i].b = 0.8;
                break;
            case 4: 
                init_mais(i);
                animacoes[i].r = 1.0; animacoes[i].g = 1.0; animacoes[i].b = 0.0;
                break;
            case 5: 
                init_triangulo(i);
                animacoes[i].r = 0.0; animacoes[i].g = 1.0; animacoes[i].b = 1.0;
                break;
            case 6: 
                init_espiral(i);
                animacoes[i].r = 0.0; animacoes[i].g = 1.0; animacoes[i].b = 1.0; // Ciano
                break;
            case 7: 
                init_onda(i);
                animacoes[i].r = 1.0; animacoes[i].g = 0.0; animacoes[i].b = 1.0; // Magenta
                break;
            case 8:
                init_ziguezague(i);
                animacoes[i].r = 1.0; animacoes[i].g = 1.0; animacoes[i].b = 0.0; // Amarelo
                break;
            case 9: 
                 init_carinha(i);
                 animacoes[i].r = 1.0; animacoes[i].g = 1.0; animacoes[i].b = 0.0;    // Amarelo
                  break;
            case 10: 
                init_coracao(i);
                animacoes[i].r = 1.0; animacoes[i].g = 0.0; animacoes[i].b = 0.0;
                break;

        }
    }

    // Inicializa telas cheias (sem animação)
    for (int i = 11; i < NUM_PADROES; i++) {
        for (int f = 0; f < FRAMES_PER_ANIMATION; f++) {
            for (int p = 0; p < NUM_PIXELS; p++) {
                animacoes[i].frames[f][p] = 1.0;
            }
        }
    }

    // Define cores das telas cheias
    animacoes[11].r = 0.0; animacoes[11].g = 0.0; animacoes[11].b = 1.0;    // Azul
    animacoes[12].r = 0.8; animacoes[12].g = 0.0; animacoes[12].b = 0.0;    // Vermelho 80%
    animacoes[13].r = 0.0; animacoes[13].g = 0.5; animacoes[13].b = 0.0;    // Verde 50%
    animacoes[14].r = 0.2; animacoes[14].g = 0.2; animacoes[14].b = 0.2;    // Branco 20%
}

void desenho_pio(double *desenho, uint32_t valor_led, PIO pio, uint sm, double r, double g, double b) {
    for (int16_t i = 0; i < NUM_PIXELS; i++) {
        valor_led = matrix_rgb(b * desenho[24-i], r * desenho[24-i], g * desenho[24-i]);
        pio_sm_put_blocking(pio, sm, valor_led);
    }
    imprimir_binario(valor_led);
}

int main() {
    PIO pio = pio0;
    bool ok;
    uint32_t valor_led;

    // Configuração do sistema
    ok = set_sys_clock_khz(128000, false);
    stdio_init_all();

    printf("Iniciando transmissão PIO\n");
    if (ok) printf("Clock configurado para %ld Hz\n", clock_get_hz(clk_sys));

    // Configuração do PIO
    uint offset = pio_add_program(pio, &pio_matrix_program);
    uint sm = pio_claim_unused_sm(pio, true);
    pio_matrix_program_init(pio, sm, offset, OUT_PIN);

    // Inicialização
    init_animacoes();
    proximo_frame = make_timeout_time_ms(FRAME_TIME);

    while (true) {
        if (absolute_time_diff_us(get_absolute_time(), proximo_frame) <= 0) {
            frame_atual = (frame_atual + 1) % FRAMES_PER_ANIMATION;
            if (frame_atual == 0) {
                padrao_atual = (padrao_atual + 1) % NUM_PADROES;
            }
            proximo_frame = make_timeout_time_ms(FRAME_TIME);
        }

        Animacao *anim_atual = &animacoes[padrao_atual];
        desenho_pio(anim_atual->frames[frame_atual], valor_led, pio, sm, 
                   anim_atual->r, anim_atual->g, anim_atual->b);

        printf("\nClock: %ld Hz, Padrão: %d, Frame: %d\r\n", 
               clock_get_hz(clk_sys), padrao_atual, frame_atual);
        sleep_ms(10);
    }
    
    return 0;
}