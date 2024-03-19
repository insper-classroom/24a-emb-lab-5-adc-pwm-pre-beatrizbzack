#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>

#include "pico/stdlib.h"
#include <stdio.h>

#include "data.h"
QueueHandle_t xQueueData;

// Implementação do filtro de média móvel
#define WINDOW_SIZE 5
int window[WINDOW_SIZE];
int window_index = 0;

int moving_average(int new_value) {
    // Adiciona o novo valor ao vetor de janela
    window[window_index] = new_value;
    
    // Atualiza o índice da janela circularmente
    window_index = (window_index + 1) % WINDOW_SIZE;

    // Calcula a média dos valores na janela
    int sum = 0;
    for (int i = 0; i < WINDOW_SIZE; i++) {
        sum += window[i];
    }
    return sum / WINDOW_SIZE;
}

void data_task(void *p) {
    vTaskDelay(pdMS_TO_TICKS(400));

    int data_len = sizeof(sine_wave_four_cycles) / sizeof(sine_wave_four_cycles[0]);
    for (int i = 0; i < data_len; i++) {
        xQueueSend(xQueueData, &sine_wave_four_cycles[i], 1000000);
    }

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void process_task(void *p) {
    int data = 0;

    while (true) {
        if (xQueueReceive(xQueueData, &data, 100)) {
            // Aplica o filtro de média móvel
            int filtered_data = moving_average(data);
            
            // Imprime o dado filtrado na UART
            printf("%d\n", filtered_data);

            // Deixa este atraso
            vTaskDelay(pdMS_TO_TICKS(50));
        }
    }
}

int main() {
    stdio_init_all();

    xQueueData = xQueueCreate(64, sizeof(int));

    xTaskCreate(data_task, "Data task ", 4096, NULL, 1, NULL);
    xTaskCreate(process_task, "Process task", 4096, NULL, 1, NULL);

    vTaskStartScheduler();

    while (true)
        ;
}
