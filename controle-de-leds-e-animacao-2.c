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
    }

    return 0;
}