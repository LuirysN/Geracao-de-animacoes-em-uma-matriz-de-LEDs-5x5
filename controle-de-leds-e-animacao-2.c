#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"

#define LED_PIN 14        // Pino conectado à matriz de LEDs WS2812
#define NUM_LEDS 25       // Número total de LEDs na matriz 5x5
#define BRIGHTNESS 255    // Intensidade máxima dos LEDs

void set_led_color(PIO pio, uint sm, uint led_index, uint32_t color) {
    // Define a cor para um LED específico
    for (int i = 0; i < NUM_LEDS; i++) {
        if (i == led_index) {
            pio_sm_put_blocking(pio, sm, color << 8);
        } else {
            pio_sm_put_blocking(pio, sm, 0); // LEDs apagados
        }
    }
}

void animate_leds(PIO pio, uint sm) {
    // Animação com 5 frames
    uint32_t colors[5] = {
        (BRIGHTNESS << 8),               // Azul
        (BRIGHTNESS << 16),              // Vermelho
        (BRIGHTNESS << 8) | BRIGHTNESS,  // Ciano
        (BRIGHTNESS << 16) | BRIGHTNESS, // Magenta
        BRIGHTNESS                       // Verde
    };

    for (int frame = 0; frame < 5; frame++) {
        for (int i = 0; i < NUM_LEDS; i++) {
            set_led_color(pio, sm, i, colors[frame]);
            sleep_ms(100); // Tempo entre cada LED para fluidez
        }
    }
}

// Função para configurar os LEDs na cor azul
void set_leds_blue(PIO pio, uint sm) {
    uint32_t blue_color = (BRIGHTNESS << 8); // RGB: Azul = 0x0000FF, codificado como GGRRBB
    for (int i = 0; i < NUM_LEDS; i++) {
        pio_sm_put_blocking(pio, sm, blue_color << 8); // Envia cor azul para todos os LEDs
    }
}

// Função para configurar os LEDs na cor vermelha com 80% de intensidade
void set_leds_red(PIO pio, uint sm) {
    uint32_t red_color = (BRIGHTNESS << 16); // RGB: Vermelho = 0xFF0000
    for (int i = 0; i < NUM_LEDS; i++) {
        pio_sm_put_blocking(pio, sm, (red_color * 0.8) << 8); // Envia cor vermelha a 80% da intensidade
    }
}

int main() {
    stdio_init_all();
    
    // Inicializa PIO e configura o estado da máquina para os LEDs WS2812
    PIO pio = pio0;
    uint sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, LED_PIN, 800000, false);

     printf("Sistema iniciado. Pressione 'B' para azul ou 'C' para vermelho com 80%% de intensidade.\n");

    // Loop principal
    while (true) {
        char key = getchar(); // Recebe a tecla pressionada
        if (key == 'B' || key == 'b') {
            printf("Tecla B pressionada. Acionando LEDs em azul.\n");
            set_leds_blue(pio, sm); // Liga os LEDs na cor azul
        } else if (key == 'C' || key == 'c') {
            printf("Tecla C pressionada. Acionando LEDs em vermelho com 80%% de intensidade.\n");
            set_leds_red_80(pio, sm); // Liga os LEDs na cor vermelha com 80% de intensidade
        }
    }

    return 0;
}