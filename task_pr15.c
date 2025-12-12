/* Ермилова Елизавета КВБО-02-23
   Производственная линия
   Программа имитирует работу производственной линии
   Изделие собирается из детали С и модуля,
   модуль состоит из деталей А и В.
   Изготовление А - 1 сек, В - 2 сек, С - 3 сек
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>

#define NUM_PRODUCTS 5  // Количество изделий для производства

// Семафоры для деталей
sem_t sem_a;    // Деталь A
sem_t sem_b;    // Деталь B
sem_t sem_c;    // Деталь C
sem_t sem_module; // Модуль (A+B)
sem_t sem_product; // Готовое изделие

// Счетчики произведенных деталей
int count_a = 0, count_b = 0, count_c = 0;
int count_modules = 0, count_products = 0;

// Мьютекс для вывода
pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;

// Функция для форматированного вывода с временной меткой
void print_with_time(const char* message) {
    pthread_mutex_lock(&print_mutex);
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    printf("[%02d:%02d:%02d] %s\n", t->tm_hour, t->tm_min, t->tm_sec, message);
    pthread_mutex_unlock(&print_mutex);
}

// Поток для производства детали A
void* produce_a(void* arg) {
    for (int i = 0; i < NUM_PRODUCTS; i++) {
        print_with_time("Начало производства детали A");
        sleep(1);  // Имитация времени производства
        
        count_a++;
        print_with_time("Деталь A готова");
        
        // Освобождаем семафор для детали A
        sem_post(&sem_a);
    }
    return NULL;
}

// Поток для производства детали B
void* produce_b(void* arg) {
    for (int i = 0; i < NUM_PRODUCTS; i++) {
        print_with_time("Начало производства детали B");
        sleep(2);  // Имитация времени производства
        
        count_b++;
        print_with_time("Деталь B готова");
        
        // Освобождаем семафор для детали B
        sem_post(&sem_b);
    }
    return NULL;
}

// Поток для производства детали C
void* produce_c(void* arg) {
    for (int i = 0; i < NUM_PRODUCTS; i++) {
        print_with_time("Начало производства детали C");
        sleep(3);  // Имитация времени производства
        
        count_c++;
        print_with_time("Деталь C готова");
        
        // Освобождаем семафор для детали C
        sem_post(&sem_c);
    }
    return NULL;
}

// Поток для сборки модуля (A+B)
void* assemble_module(void* arg) {
    for (int i = 0; i < NUM_PRODUCTS; i++) {
        // Ждем, пока будут готовы обе детали A и B
        sem_wait(&sem_a);
        sem_wait(&sem_b);
        
        print_with_time("Начало сборки модуля (A+B)");
        sleep(1);  // Имитация времени сборки
        
        count_modules++;
        print_with_time("Модуль (A+B) собран");
        
        // Освобождаем семафор для модуля
        sem_post(&sem_module);
    }
    return NULL;
}

// Поток для сборки изделия (Модуль + C)
void* assemble_product(void* arg) {
    for (int i = 0; i < NUM_PRODUCTS; i++) {
        // Ждем, пока будут готовы модуль и деталь C
        sem_wait(&sem_module);
        sem_wait(&sem_c);
        
        print_with_time("Начало сборки изделия (Модуль + C)");
        sleep(2);  // Имитация времени сборки
        
        count_products++;
        print_with_time("ИЗДЕЛИЕ ГОТОВО!");
        
        // Освобождаем семафор для готового изделия
        sem_post(&sem_product);
    }
    return NULL;
}

int main() {
    pthread_t thread_a, thread_b, thread_c, thread_module, thread_product;
    
    printf("Планируемое количество изделий: %d\n", NUM_PRODUCTS);
    printf("Теоретическое время цикла: 2 секунды на изделие\n");
    printf("Ожидаемое общее время: ~%d секунд\n\n", NUM_PRODUCTS * 2);
    
    // Инициализация семафоров
    sem_init(&sem_a, 0, 0);
    sem_init(&sem_b, 0, 0);
    sem_init(&sem_c, 0, 0);
    sem_init(&sem_module, 0, 0);
    sem_init(&sem_product, 0, 0);
    
    // Запоминаем время начала
    time_t start_time = time(NULL);
    
    // Создание потоков
    pthread_create(&thread_a, NULL, produce_a, NULL);
    pthread_create(&thread_b, NULL, produce_b, NULL);
    pthread_create(&thread_c, NULL, produce_c, NULL);
    pthread_create(&thread_module, NULL, assemble_module, NULL);
    pthread_create(&thread_product, NULL, assemble_product, NULL);
    
    // Ожидание завершения производства всех изделий
    for (int i = 0; i < NUM_PRODUCTS; i++) {
        sem_wait(&sem_product);
        printf("\n=== Изделие №%d успешно произведено ===\n", i + 1);
    }
    
    // Ожидание завершения всех потоков
    pthread_join(thread_a, NULL);
    pthread_join(thread_b, NULL);
    pthread_join(thread_c, NULL);
    pthread_join(thread_module, NULL);
    pthread_join(thread_product, NULL);
    
    // Расчет времени выполнения
    time_t end_time = time(NULL);
    int total_time = (int)(end_time - start_time);
    
    // Вывод статистики
    printf("Произведено деталей A: %d\n", count_a);
    printf("Произведено деталей B: %d\n", count_b);
    printf("Произведено деталей C: %d\n", count_c);
    printf("Собрано модулей (A+B): %d\n", count_modules);
    printf("Собрано изделий: %d\n", count_products);
    printf("Общее время работы: %d секунд\n", total_time);
    printf("Среднее время на изделие: %.2f секунд\n", 
           (float)total_time / NUM_PRODUCTS);
    printf("Эффективность: %.1f%% (от теоретической)\n", 
           (2.0 * NUM_PRODUCTS * 100.0) / total_time);
    
    // Уничтожение семафоров и мьютекса
    sem_destroy(&sem_a);
    sem_destroy(&sem_b);
    sem_destroy(&sem_c);
    sem_destroy(&sem_module);
    sem_destroy(&sem_product);
    pthread_mutex_destroy(&print_mutex);
    
    return 0;
}
