#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"

#define LED_PIN 14        // Pino conectado à matriz de LEDs WS2812
#define NUM_LEDS 25       // Número total de LEDs na matriz 5x5
#define BRIGHTNESS 255    // Intensidade máxima dos LEDs

void set_leds_blue(PIO pio, uint sm) {
    uint32_t blue_color = (BRIGHTNESS << 8); // RGB: Azul = 0x0000FF, codificado como GGRRBB
    for (int i = 0; i < NUM_LEDS; i++) {
        pio_sm_put_blocking(pio, sm, blue_color << 8); // Envia cor azul para todos os LEDs
    }
}
void set_leds_green(PIO pio, uint sm) {
    uint32_t green_color = (BRIGHTNESS / 2) << 16; //RGB verde(0x00FF00), para 50%de intensidade/2=0x007F00
    for (int i = 0; i < NUM_LEDS; i++) {
        pio_sm_put_blocking(pio, sm, green_color << 8); // Envia a cor verde para todos os LEDs
    }
}
void set_leds_white(PIO pio, uint sm) {
    uint32_t white_color = 51 | (51 << 8) | (51 << 16); // Branco com intensidade 51 em RGB(20%de cada cor)
    for (int i = 0; i < NUM_LEDS; i++) {
        pio_sm_put_blocking(pio, sm, white_color << 8); // Envia a cor branca para todos os LEDs
    }
}

 void animation_3(){                            //vetores para as cinco imagens
double desenho_jk1[25] =   {1.0, 0.0, 0.0, 0.0, 1.0,
                            1.0, 1.0, 0.0, 1.0, 1.0, 
                            1.0, 1.0, 1.0, 1.0,1.0,
                            0.0, 1.0, 1.0, 1.0, 0.0,
                            0.0, 0.0, 1.0, 0.0, 0.0};

double desenho_jk2[25] =   {1.0, 0.0, 0.0, 0.0, 1.0,
                            1.0, 1.0, 0.0, 1.0, 1.0, 
                            1.0, 1.0, 1.0, 1.0, 1.0,
                            1.0, 1.0, 0.0, 1.0, 1.0,
                            1.0, 0.0, 0.0, 0.0, 1.0};

double desenho_jk3[25] =   {0.0, 0.0, 0.0, 0.0, 0.0,
                            0.0, 0.0, 0.0, 0.0, 0.0, 
                            0.0, 0.0, 1.0, 0.0, 0.0,
                            0.0, 1.0, 1.0, 1.0, 0.0,
                            1.0, 1.0, 1.0, 1.0, 1.0};

double desenho_jk4[25] =   {1.0, 1.0, 1.0, 1.0, 1.0,
                            0.0, 1.0, 1.0, 1.0, 0.0, 
                            0.0, 0.0, 1.0, 0.0, 0.0,
                            0.0, 0.0, 0.0, 0.0, 0.0,
                            0.0, 0.0, 0.0, 0.0, 0.0};

double desenho_jk5[25] =   {1.0, 0.0, 0.0, 0.0, 0.0,
                            1.0, 1.0, 0.0, 0.0, 0.0, 
                            1.0, 1.0, 1.0, 0.0, 0.0,
                            0.0, 0.0, 0.0, 1.0, 1.0,
                            0.0, 0.0, 0.0, 0.0, 1.0};
 }

 
int main() {
    stdio_init_all();
    
    // Inicializa PIO e configura o estado da máquina para os LEDs WS2812
    PIO pio = pio0;
    uint sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, LED_PIN, 800000, false);

    printf("Sistema iniciado. Pressione 'B' no teclado.\n");

    // Loop principal
    while (true) {
        char key = getchar(); // Recebe a tecla pressionada
        if (key == 'B' || key == 'b') {
            printf("Tecla B pressionada. Acionando LEDs em azul.\n");
            set_leds_blue(pio, sm); // Liga os LEDs na cor azul
        }
         if (key == 'D' || key == 'd') {
            printf("Tecla D pressionada. Acionando LEDs em verde.\n");
            set_leds_green(pio, sm); // Liga os LEDs na cor verde
        }
         if (key == '#' || key == '#') {
            printf("Tecla # pressionada. Acionando LEDs em branco.\n");
            set_leds_white(pio, sm); // Liga os LEDs na cor branca
        }

    }

    return 0;
}