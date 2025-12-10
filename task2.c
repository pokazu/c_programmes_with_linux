/* Ермилова Елизавета КВБО-02-23
   Программа "Накорми кота"
   Владелец кормит 5 котов
   Максимальное кол-во корма от 100 до 1000 граммов
   Каждый кот съедает 100 граммов корма в день, коты могут есть одновременно
*/

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define MAX_FOOD 1000    // Максимальное количество корма в кормушке
#define MIN_FOOD 100     // Минимальное количество корма в кормушке
#define CATS_COUNT 5     // Количество котов
#define CAT_EAT_AMOUNT 100 // Сколько грамм съедает кот за один прием

pthread_mutex_t food_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t food_cond = PTHREAD_COND_INITIALIZER;  // Условная переменная для корма

int food_in_bowl = 500;  // Начальное количество корма в кормушке
int program_running = 1; // Флаг работы программы

// Поток владельца (добавляет корм)
void* owner_thread(void* arg) {
    printf("Владелец: запущен (контролирую корм в кормушке)\n");
    
    while (program_running) {
        sleep(2);  // Владелец проверяет кормушку каждые 2 секунды
        
        pthread_mutex_lock(&food_mutex);
        
        printf("Владелец: проверяю кормушку. Сейчас корма: %d грамм\n", food_in_bowl);
        
        // Если корма меньше минимального, добавляем
        if (food_in_bowl < MIN_FOOD) {
            int food_to_add = MAX_FOOD - food_in_bowl;
            food_in_bowl = MAX_FOOD;
            printf("Владелец: добавляю %d грамм корма. Теперь в кормушке: %d грамм\n", 
                   food_to_add, food_in_bowl);
            
            // Будим всех котов, если они ждали корма
            pthread_cond_broadcast(&food_cond);
        }
        // Если корма больше максимального
        else if (food_in_bowl > MAX_FOOD) {
            food_in_bowl = MAX_FOOD;
            printf("Владелец: слишком много корма! Оставляю максимум: %d грамм\n", food_in_bowl);
        }
        else {
            printf("Владелец: корма достаточно (%d грамм), не добавляю\n", food_in_bowl);
        }
        
        pthread_mutex_unlock(&food_mutex);
    }
    
    printf("Владелец: завершаю работу\n");
    return NULL;
}

// Поток кота (ест корм)
void* cat_thread(void* arg) {
    int cat_id = *(int*)arg;
    
    printf("Кот %d: запущен\n", cat_id);
    
    while (program_running) {
        // Кот ест 3 раза в день (условно)
        for (int meal = 0; meal < 3 && program_running; meal++) {
            pthread_mutex_lock(&food_mutex);
            
            // Ждем, пока будет достаточно корма
            while (food_in_bowl < CAT_EAT_AMOUNT && program_running) {
                printf("Кот %d: жду, пока появится корм... (сейчас %d грамм)\n", 
                       cat_id, food_in_bowl);
                pthread_cond_wait(&food_cond, &food_mutex);
            }
            
            if (program_running && food_in_bowl >= CAT_EAT_AMOUNT) {
                // Кот ест
                food_in_bowl -= CAT_EAT_AMOUNT;
                printf("Кот %d: съел %d грамм корма. Осталось в кормушке: %d грамм\n", 
                       cat_id, CAT_EAT_AMOUNT, food_in_bowl);
            }
            
            pthread_mutex_unlock(&food_mutex);
            
            // Имитация времени приема пищи
            if (program_running) {
                usleep(500000 + rand() % 1000000);  // 0.5-1.5 секунды
            }
        }
        
        // Имитация перерыва между "днями"
        if (program_running) {
            sleep(1);
        }
    }
    
    printf("Кот %d: завершаю работу\n", cat_id);
    return NULL;
}

int main() {
    pthread_t owner, cats[CATS_COUNT];
    int cat_ids[CATS_COUNT];
    
    // Инициализация генератора случайных чисел
    srand(time(NULL));
    
    // Создание потока владельца
    pthread_create(&owner, NULL, owner_thread, NULL);
    
    // Создание потоков котов
    for (int i = 0; i < CATS_COUNT; i++) {
        cat_ids[i] = i + 1;  // Нумерация котов
        pthread_create(&cats[i], NULL, cat_thread, &cat_ids[i]);
    }
    
    sleep(15);
    
    // Завершение работы
    printf("\nЗавершаем программу...\n");
    program_running = 0;
    
    // Будим все потоки для корректного завершения
    pthread_mutex_lock(&food_mutex);
    pthread_cond_broadcast(&food_cond);
    pthread_mutex_unlock(&food_mutex);
    
    // Ожидаем завершения всех потоков
    pthread_join(owner, NULL);
    for (int i = 0; i < CATS_COUNT; i++) {
        pthread_join(cats[i], NULL);
    }
    
    printf("\nИтоговое количество корма в кормушке: %d грамм\n", food_in_bowl);
    printf("Все потоки завершены. Программа окончена.\n");
    
    // Уничтожение мьютекса и условной переменной
    pthread_mutex_destroy(&food_mutex);
    pthread_cond_destroy(&food_cond);
    
    return 0;
}
