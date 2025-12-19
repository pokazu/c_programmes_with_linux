/* Ермилова Елизавета КВБО-02-23
   "Производитель-потребитель" - очередь мообщений с использованием семафоров
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define MAX_QUEUE_SIZE 10
#define MAX_MSG_LENGTH 80

typedef struct {
    char messages[MAX_QUEUE_SIZE][MAX_MSG_LENGTH + 1];  // +1 для нуль-терминатора
    int front;
    int rear;
    int count;
    int dropped;
    sem_t empty_sem;
    sem_t full_sem;
    pthread_mutex_t mutex;
} mymsg_queue;

// Инициализация очереди
void mymsginit(mymsg_queue *q) {
    q->front = 0;
    q->rear = 0;
    q->count = 0;
    q->dropped = 0;
    
    // Инициализация семафоров
    sem_init(&q->empty_sem, 0, MAX_QUEUE_SIZE);  // изначально все места свободны
    sem_init(&q->full_sem, 0, 0);                // изначально очередь пуста
    
    // Инициализация мьютекса
    pthread_mutex_init(&q->mutex, NULL);
    
    printf("Очередь инициализирована\n");
}

// Разблокировка всех операций
void mymsgdrop(mymsg_queue *q) {
    pthread_mutex_lock(&q->mutex);
    q->dropped = 1;
    
    // Разблокировка всех ожидающих операций
    for (int i = 0; i < MAX_QUEUE_SIZE; i++) {
        sem_post(&q->empty_sem);
        sem_post(&q->full_sem);
    }
    
    pthread_mutex_unlock(&q->mutex);
    printf("Очередь отброшена - все операции разблокированы\n");
}

// Уничтожение очереди
void mymsgdestroy(mymsg_queue *q) {
    sem_destroy(&q->empty_sem);
    sem_destroy(&q->full_sem);
    pthread_mutex_destroy(&q->mutex);
    printf("Очередь уничтожена\n");
}

// Добавление сообщения в очередь
int mymsgput(mymsg_queue *q, char *msg) {
    // Ожидание свободного места
    sem_wait(&q->empty_sem);
    
    pthread_mutex_lock(&q->mutex);
    
    // Проверка, была ли очередь отброшена
    if (q->dropped) {
        pthread_mutex_unlock(&q->mutex);
        return 0;
    }
    
    // Копирование сообщения (максимум 80 символов)
    int len = strlen(msg);
    if (len > MAX_MSG_LENGTH) {
        len = MAX_MSG_LENGTH;
    }
    
    strncpy(q->messages[q->rear], msg, len);
    q->messages[q->rear][len] = '\0';
    
    printf("Производитель добавил: '%.80s' (длина: %d)\n", q->messages[q->rear], len);
    
    q->rear = (q->rear + 1) % MAX_QUEUE_SIZE;
    q->count++;
    
    pthread_mutex_unlock(&q->mutex);
    
    // Уведомление о новом сообщении
    sem_post(&q->full_sem);
    
    return len;
}

// Получение сообщения из очереди
int mymsgget(mymsg_queue *q, char *buf, size_t bufsize) {
    // Ожидание сообщения в очереди
    sem_wait(&q->full_sem);
    
    pthread_mutex_lock(&q->mutex);
    
    // Проверка, была ли очередь отброшена
    if (q->dropped) {
        pthread_mutex_unlock(&q->mutex);
        return 0;
    }
    
    // Извлечение сообщения
    int len = strlen(q->messages[q->front]);
    if (bufsize - 1 < len) {
        len = bufsize - 1;
    }
    
    strncpy(buf, q->messages[q->front], len);
    buf[len] = '\0';
    
    printf("Потребитель получил: '%.80s' (длина: %d)\n", buf, len);
    
    q->front = (q->front + 1) % MAX_QUEUE_SIZE;
    q->count--;
    
    pthread_mutex_unlock(&q->mutex);
    
    // Уведомление о свободном месте
    sem_post(&q->empty_sem);
    
    return len;
}

// Функция производителя
void* producer(void* arg) {
    mymsg_queue *q = (mymsg_queue*)arg;
    int message_num = 0;
    
    while (!q->dropped) {
        char msg[100];
        snprintf(msg, sizeof(msg), "Сообщение %d от производителя %lu", 
                 ++message_num, pthread_self() % 100);
        
        mymsgput(q, msg);
        
        usleep(rand() % 500000);  //  задержка до 0.5 секунды
    }
    
    printf("Производитель %lu завершил работу\n", pthread_self() % 100);
    return NULL;
}

// Функция потребителя
void* consumer(void* arg) {
    mymsg_queue *q = (mymsg_queue*)arg;
    
    while (!q->dropped) {
        char buf[MAX_MSG_LENGTH + 1];
        mymsgget(q, buf, sizeof(buf));
        
        // Имитация обработки сообщения
        usleep(rand() % 500000);  // Случайная задержка до 0.5 секунды
    }
    
    printf("Потребитель %lu завершил работу\n", pthread_self() % 100);
    return NULL;
}

int main() {
    mymsg_queue q;
    pthread_t producers[2], consumers[2];
    
    // Инициализация очереди
    mymsginit(&q);
    
    // Создание потоков-производителей
    for (int i = 0; i < 2; i++) {
        pthread_create(&producers[i], NULL, producer, &q);
    }
    
    // Создание потоков-потребителей
    for (int i = 0; i < 2; i++) {
        pthread_create(&consumers[i], NULL, consumer, &q);
    }
    
    // Даем потокам поработать 5 секунд
    sleep(5);
    
    // Отбрасываем очередь (разблокируем все операции)
    mymsgdrop(&q);
    
    // Ожидание завершения потоков
    for (int i = 0; i < 2; i++) {
        pthread_join(producers[i], NULL);
        pthread_join(consumers[i], NULL);
    }
    
    // Уничтожение очереди
    mymsgdestroy(&q);
    
    return 0;
}
