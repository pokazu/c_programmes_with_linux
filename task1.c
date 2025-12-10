/* Ермилова Елизавета КВБО-02-23
   Многопоточное приложение с тремя потоками
   1 поток - генерирует случайно целое число каждую секунду
   2 поток - если значение четное, вычисляет квадрат числа и выводит
   3 поток - если значение нечетное, выводит куб числа
*/

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

// Глобальные переменные для синхронизации
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_even = PTHREAD_COND_INITIALIZER;    // Для четных чисел
pthread_cond_t cond_odd = PTHREAD_COND_INITIALIZER;     // Для нечетных чисел

// Разделяемые данные
int generated_number = 0;
int number_is_ready = 0;  // Флаг: 0 - нет числа, 1 - есть число для обработки
int program_running = 1;  // Флаг работы программы

// Поток 1: Генератор случайных чисел
void* number_generator(void* arg) {
    printf("Поток-генератор: запущен\n");
    
    while (program_running) {
        sleep(1); 
        
        pthread_mutex_lock(&mutex);
        
        if (!program_running) {
            pthread_mutex_unlock(&mutex);
            break;
        }
        
        // Генерируем число от 1 до 100
        generated_number = rand() % 100 + 1;
        number_is_ready = 1;
        
        printf("Генератор: сгенерировано число %d\n", generated_number);
        
        // Определяем, четное или нечетное число
        if (generated_number % 2 == 0) {
            // Четное - будим поток для квадрата
            pthread_cond_signal(&cond_even);
        } else {
            // Нечетное - будим поток для куба
            pthread_cond_signal(&cond_odd);
        }
        
        // Ждем, пока число не будет обработано
        while (number_is_ready == 1 && program_running) {
            pthread_cond_wait(&cond_even, &mutex);
        }
        
        pthread_mutex_unlock(&mutex);
    }
    
    printf("Поток-генератор: завершен\n");
    return NULL;
}

// Поток 2: Обработчик четных чисел (квадрат)
void* even_number_handler(void* arg) {
    printf("Поток для четных чисел: запущен\n");
    
    while (1) {
        pthread_mutex_lock(&mutex);
        
        // Ждем, пока не появится четное число для обработки
        while ((number_is_ready == 0 || generated_number % 2 != 0) && program_running) {
            pthread_cond_wait(&cond_even, &mutex);
        }
        
        // Проверяем, не пора ли завершать работу
        if (!program_running) {
            pthread_mutex_unlock(&mutex);
            break;
        }
        
        if (number_is_ready == 1 && generated_number % 2 == 0) {
            // Вычисляем квадрат числа
            int square = generated_number * generated_number;
            printf("Квадрат: число %d -> квадрат %d\n", generated_number, square);
            
            // Помечаем число как обработанное
            number_is_ready = 0;
            
            // Будим генератор для создания нового числа
            pthread_cond_signal(&cond_even);
        }
        
        pthread_mutex_unlock(&mutex);
    }
    
    printf("Поток для четных чисел: завершен\n");
    return NULL;
}

// Поток 3: Обработчик нечетных чисел (куб)
void* odd_number_handler(void* arg) {
    printf("Поток для нечетных чисел: запущен\n");
    
    while (1) {
        pthread_mutex_lock(&mutex);
        
        // Ждем, пока не появится нечетное число для обработки
        while ((number_is_ready == 0 || generated_number % 2 == 0) && program_running) {
            pthread_cond_wait(&cond_odd, &mutex);
        }
        
        // Проверяем, не пора ли завершать работу
        if (!program_running) {
            pthread_mutex_unlock(&mutex);
            break;
        }
        
        if (number_is_ready == 1 && generated_number % 2 != 0) {
            // Вычисляем куб числа
            int cube = generated_number * generated_number * generated_number;
            printf("Куб: число %d -> куб %d\n", generated_number, cube);
            
            // Помечаем число как обработанное
            number_is_ready = 0;
            
            // Будим генератор для создания нового числа
            pthread_cond_signal(&cond_even);
        }
        
        pthread_mutex_unlock(&mutex);
    }
    
    printf("Поток для нечетных чисел: завершен\n");
    return NULL;
}

int main() {
    pthread_t generator_thread, even_thread, odd_thread;
    
    // Инициализация генератора случайных чисел
    srand(time(NULL));
    
    // Создание потоков
    pthread_create(&generator_thread, NULL, number_generator, NULL);
    pthread_create(&even_thread, NULL, even_number_handler, NULL);
    pthread_create(&odd_thread, NULL, odd_number_handler, NULL);
    
    sleep(10);
    
    // Завершение работы
    printf("\nЗавершаем работу программы...\n");
    
    // Устанавливаем флаг завершения и будим ВСЕ потоки
    pthread_mutex_lock(&mutex);
    program_running = 0;
    
    // будим оба потока обработчиков
    pthread_cond_signal(&cond_even);
    pthread_cond_signal(&cond_odd);
    
    // будим генератор, если он ждет
    pthread_cond_signal(&cond_even);
    
    pthread_mutex_unlock(&mutex);
    
    // Ожидаем завершения всех потоков
    pthread_join(generator_thread, NULL);
    pthread_join(even_thread, NULL);
    pthread_join(odd_thread, NULL);
    
    printf("\nВсе потоки завершены. Программа окончена.\n");
    
    // Уничтожение мьютекса и условных переменных
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond_even);
    pthread_cond_destroy(&cond_odd);
    
    return 0;
}
