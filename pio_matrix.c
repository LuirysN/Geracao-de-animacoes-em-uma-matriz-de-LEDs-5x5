#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/adc.h"
#include "pico/bootrom.h"
#include "hardware/gpio.h"

// Include the header file where pio_matrix_pio is defined
#include "pio_matrix.pio.h"


// Definir FPS
#define FPS 10  // 10 frames por segundo, ou seja, 100ms por frame

// Definir o número de LEDs
#define NUM_PIXELS 25

// Definir o pino de saída
#define OUT_PIN 7

// Botões para interrupção
const uint button_0 = 5;
const uint button_1 = 6;

#define ROW_START 2  // GP2
#define ROW_END 5    // GP5
#define COL_START 6  // GP6
#define COL_END 9    // GP9

// Função para definir a cor do LED (conversão RGB)
// Correção: Define corretamente o retorno como uint32_t
uint32_t matrix_rgb(double r, double g, double b);

// Função para controlar a cor dos LEDs
void set_led_color(PIO pio, uint sm, double r, double g, double b) {
    uint32_t valor_led;
    for (int i = 0; i < NUM_PIXELS; i++) {
        valor_led = matrix_rgb(r, g, b);  // Chama a função corrigida
        pio_sm_put_blocking(pio, sm, valor_led);
    }
}

// Função para desligar todos os LEDs
void desligar_leds(PIO pio, uint sm) {
    set_led_color(pio, sm, 0.0, 0.0, 0.0);  // Todos os LEDs apagados
}

// Função para definir a cor do LED (conversão RGB)
// Correção: Define corretamente o retorno como uint32_t
uint32_t matrix_rgb(double r, double g, double b) {
    // Convertendo os valores de cor (0.0 - 1.0) para o intervalo (0 - 255)
    unsigned char R = (unsigned char)(r * 255);
    unsigned char G = (unsigned char)(g * 255);
    unsigned char B = (unsigned char)(b * 255);
    return (G << 24) | (R << 16) | (B << 8);  // Retorna um valor de 32 bits no formato ARGB
}

// Função de configuração do PIO para controle da matriz de LEDs
void configurar_pio(PIO pio, uint sm) {
    // Configuração do PIO e frequência de clock
    bool ok = set_sys_clock_khz(128000, false);
    if (ok) printf("Clock set to %ld\n", clock_get_hz(clk_sys));

    // Usar o programa PIO correto
    uint offset = pio_add_program(pio, &pio_matrix_program);  // Corrigido o nome do programa PIO
    pio_matrix_program_init(pio, sm, offset, OUT_PIN);  // Corrigido a chamada para o programa PIO correto
}

// Função para capturar a tecla pressionada (simulada, pode ser substituída por leitura de teclado matricial)
uint tecla_pressionada() {
    // Aqui você pode integrar a função do teclado matricial
    // Retorna a tecla pressionada: 0, 1, 2... até 9, A, B, C, D, #
    return 0; // Exemplo: tecla 0 pressionada
}

// Função para animação da tecla pressionada (simulação de animações)
void animacao_tecla(uint tecla, PIO pio, uint sm) {
    double r = 0.0, g = 0.0, b = 0.0;

    switch (tecla) {
        case '0': 
            // Animação para tecla 0
            printf("Iniciando animação para tecla 0\n");
            for (int i = 0; i < 5; i++) {
                set_led_color(pio, sm, 1.0, 0.0, 0.0);  // Exemplo de animação em vermelho
                sleep_ms(100);
                set_led_color(pio, sm, 0.0, 1.0, 0.0);  // Verde
                sleep_ms(100);
            }
            break;
        case '1': 
            // Animação para tecla 1
            printf("Iniciando animação para tecla 1\n");
            for (int i = 0; i < 5; i++) {
                set_led_color(pio, sm, 0.0, 0.0, 1.0);  // Azul
                sleep_ms(100);
                set_led_color(pio, sm, 1.0, 1.0, 0.0);  // Amarelo
                sleep_ms(100);
            }
            break;
        // Animações para outras teclas, siga o padrão similar
        case 'A':  
            // Desligar todos os LEDs
            desligar_leds(pio, sm);
            break;
        case 'B':  
            // Todos os LEDs azuis
            set_led_color(pio, sm, 0.0, 0.0, 1.0);
            break;
        case 'C':  
            // Todos os LEDs vermelhos
            set_led_color(pio, sm, 0.8, 0.0, 0.0);
            break;
        case 'D':  
            // Todos os LEDs verdes
            set_led_color(pio, sm, 0.0, 0.5, 0.0);
            break;
        case '#':  
            // Todos os LEDs brancos
            set_led_color(pio, sm, 0.2, 0.2, 0.2);
            break;
        default:
            break;
    }
}

// Interrupt handler for GPIO
void gpio_irq_handler(uint gpio, uint32_t events) {
    // Handle the interrupt (e.g., set a flag, read the button state, etc.)
    printf("Interrupt on GPIO %d with events %d\n", gpio, events);
}

// Função para ler teclado matricial
char ler_teclado() {
    char teclas[4][4] = {
        {'1','2','3','A'},
        {'4','5','6','B'},
        {'7','8','9','C'},
        {'*','0','#','D'}
    };
    
    for (int col = COL_START; col <= COL_END; col++) {
        gpio_set_dir(col, GPIO_OUT);
        gpio_put(col, 0);
        
        for (int row = ROW_START; row <= ROW_END; row++) {
            if (!gpio_get(row)) {
                gpio_set_dir(col, GPIO_IN);
                return teclas[row-ROW_START][col-COL_START];
            }
        }
        gpio_set_dir(col, GPIO_IN);
    }
    return '\0';
}

// Função principal
int main() {
    PIO pio = pio0;
    bool ok;
    uint sm;
    
    // Configurações iniciais
    ok = set_sys_clock_khz(128000, false);
    stdio_init_all();
    printf("iniciando a transmissão PIO");
    if (ok) printf("clock set to %ld\n", clock_get_hz(clk_sys));

    uint offset = pio_add_program(pio, &pio_matrix_program);
    sm = pio_claim_unused_sm(pio, true);
    pio_matrix_program_init(pio, sm, offset, OUT_PIN);

    // Inicializar GPIOs para o teclado
    for (int i = ROW_START; i <= ROW_END; i++) {
        gpio_init(i);
        gpio_set_dir(i, GPIO_IN);
        gpio_pull_up(i);
    }
    
    for (int i = COL_START; i <= COL_END; i++) {
        gpio_init(i);
        gpio_set_dir(i, GPIO_IN);
        gpio_pull_up(i);
    }

    while (true) {
        char tecla = ler_teclado();

        if (tecla != '\0') {
            switch(tecla) {
                case 'A':
                    desligar_leds(pio, sm);
                    break;
                case 'B':
                    set_led_color(pio, sm, 0.0, 0.0, 1.0); // Azul 100%
                    break;
                case 'C':
                    set_led_color(pio, sm, 0.8, 0.0, 0.0); // Vermelho 80%
                    break;
                case 'D':
                    set_led_color(pio, sm, 0.0, 0.5, 0.0); // Verde 50%
                    break;
                case '#':
                    set_led_color(pio, sm, 0.2, 0.2, 0.2); // Branco 20%
                    break;
                // Adicionar casos para números 0-9 (animações)
            }
        }
        sleep_ms(100);  // Debounce
    }
}

/**
 * Define a cor dos LEDs na matriz.
 *
 * @param pio PIO utilizado para controlar os LEDs.
 * @param sm Máquina de estado (State Machine) utilizada.
 * @param vermelho Valor da cor vermelha (0.0 a 1.0).
 * @param verde Valor da cor verde (0.0 a 1.0).
 * @param azul Valor da cor azul (0.0 a 1.0).
 */
void definir_cor_led(PIO pio, uint sm, double vermelho, double verde, double azul) {
    uint32_t valor_led;
    
    // Itera sobre cada pixel e define sua cor
    for (int i = 0; i < NUM_PIXELS; i++) {
        // Converte os valores RGB para o formato apropriado para o LED
        valor_led = matrix_rgb(vermelho, verde, azul);
        
        // Envia o valor da cor para a máquina de estado do PIO
        pio_sm_put_blocking(pio, sm, valor_led);
    }
}