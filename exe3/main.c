#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>

#include "pico/stdlib.h"
#include <stdio.h>

#include "data.h"

// Implementação do filtro de média móvel
#define WINDOW_SIZE 5

// Estrutura para a média móvel
typedef struct {
    int window[WINDOW_SIZE];
    int window_index;
} MovingAverage;

// Função para inicializar a estrutura da média móvel
void init_moving_average(MovingAverage *ma) {
    for (int i = 0; i < WINDOW_SIZE; i++) {
        ma->window[i] = 0;
    }
    ma->window_index = 0;
}

// Função para calcular a média móvel
int moving_average(MovingAverage *ma, int new_value) {
    // Adiciona o novo valor ao vetor de janela
    ma->window[ma->window_index] = new_value;

    // Atualiza o índice da janela circularmente
    ma->window_index = (ma->window_index + 1) % WINDOW_SIZE;

    // Calcula a média dos valores na janela
    int sum = 0;
    for (int i = 0; i < WINDOW_SIZE; i++) {
        sum += ma->window[i];
    }
    return sum / WINDOW_SIZE;
}

// Tarefa responsável por gerar dados
void data_task(void *pvParameters) {
    QueueHandle_t *xQueueData = (QueueHandle_t *)pvParameters;

    vTaskDelay(pdMS_TO_TICKS(400));

    int data_len = sizeof(sine_wave_four_cycles) / sizeof(sine_wave_four_cycles[0]);
    for (int i = 0; i < data_len; i++) {
        xQueueSend(*xQueueData, &sine_wave_four_cycles[i], 1000000);
    }

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// Tarefa responsável por processar dados
void process_task(void *pvParameters) {
    QueueHandle_t *xQueueData = (QueueHandle_t *)pvParameters;
    MovingAverage ma;
    init_moving_average(&ma);

    int data = 0;

    while (true) {
        if (xQueueReceive(*xQueueData, &data, 100)) {
            // Aplica o filtro de média móvel
            int filtered_data = moving_average(&ma, data);

            // Imprime o dado filtrado na UART
            printf("%d\n", filtered_data);

            // Deixa este atraso
            vTaskDelay(pdMS_TO_TICKS(50));
        }
    }
}

int main() {
    stdio_init_all();

    QueueHandle_t xQueueData = xQueueCreate(64, sizeof(int));

    xTaskCreate(data_task, "Data task ", 4096, &xQueueData, 1, NULL);
    xTaskCreate(process_task, "Process task", 4096, &xQueueData, 1, NULL);

    vTaskStartScheduler();

    while (true)
        ;
}
