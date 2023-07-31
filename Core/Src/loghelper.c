/**
 * @file loghelper.c
 * @author your name (you@domain.com)
 * @brief ��Ϣ�����������ʹ�ö�������-��������ģʽ��MPSC��
 *        Ԥ���ӿڹ����������ߴ���log��Ϣ������Ϣ���У�
 *          
 * @version 0.1
 * @date 2023-07-31
 * 
 * @todo ��Ҫ�󶨶�ʱ����������������Լ�����log���ܡ�
 *       ʹ�÷��ж������ʽʱ��Ӧ��֤���ʱ��ɿأ�����ʱ������������ݽ�ǿ�Ʒ��أ��¸������������
 * @copyright Copyright (c) 2023
 * 
 */

#include "loghelper.h"

//��ƥ��printϵ�к����Ķ����ļ�
#include "usb_device.h"

// ��ʼ������
staic void initQueue(struct Queue* queue) {
    queue->head = NULL;
    queue->tail = NULL;
}

// ���
staic void enqueue(struct Queue* queue, char* log) {
    struct Node* newNode = (struct Node*)malloc(sizeof(struct Node));
    if (newNode == NULL) {
        // �ڴ����ʧ��
        perror("Memory allocation error");
        exit(EXIT_FAILURE);//todo ����֤�����Ƿ�ᵼ�³����ܷ�
    }

    newNode->log = log;
    newNode->next = NULL;

    if (queue->tail != NULL) {
        queue->tail->next = newNode;
    }

    queue->tail = newNode;

    if (queue->head == NULL) {
        queue->head = newNode;
    }
}

// ����
staic char* dequeue(struct Queue* queue) {
    if (queue->head == NULL) {
        return NULL; // ����Ϊ��
    }

    char* log = queue->head->log;
    struct Node* temp = queue->head;

    queue->head = queue->head->next;

    if (queue->head == NULL) {
        queue->tail = NULL;
    }

    free(temp);
    return log;
}

// // ʾ������1����Ϊ������1
// void producerFunction1(struct Queue* queue) {
//     char* log1 = "Log message from producerFunction1";
//     enqueue(queue, log1);
// }


// ʾ�������ߺ���
staic void consumer(struct Queue* queue) {
    while (true) {
        char* log = dequeue(queue);
        if (log != NULL) {
            printfln("Consumer: %s\n", log);
            free(log); // ע�⣺ʹ������־����Ҫ�ͷ��ڴ�
        }
        // �ڴ˿�������ʵ����ӳ٣�����ʹ�����������Ȼ������������æ�ȴ�
    }
}

void LogerCreate(UART_HandleTypeDef *huartcooling) {
    struct Queue queue;
    initQueue(&queue);

    // ���������ж����ͬ�������ߺ���
    producerFunction1(&queue);
    producerFunction2(&queue);
    producerFunction1(&queue);

    // �����ߺ���
    consumerFunction(&queue);

    return 0;
}
