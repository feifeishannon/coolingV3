/**
 * @file loghelper.h
 * @author your name (you@domain.com)
 * @brief ��Ϣ�����������ʹ�ö�������-��������ģʽ��MPSC��
 *        Ԥ���ӿڹ����������ߴ���log��Ϣ������Ϣ���У�
 *          
 * @version 0.1
 * @date 2023-07-31
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX_LOG_MESSAGES 100


// ����ڵ�ṹ
struct Node {
    char* log;
    struct Node* next;
};

// ������нṹ
struct Queue {
    struct Node* head;
    struct Node* tail;
};

/**
 * @brief ʵ��������ж��������
 * 
 */
typedef struct
{
    // char *info[1000];                       /* Һ���������Ϣ����,1k����*/
    Queue queue;
    UART_HandleTypeDef *huart;
    void (* initQueue)(Queue* queue);       /*!< �����û�ͨѶ�ӿ�   */
    void (* enqueue)(Queue* queue, char* log);        /*!< ����Һ�������  ���鹤��Ƶ��20hz   */      
    void (* Run)();        /*!< ����Һ�������  ���鹤��Ƶ��20hz   */      

} Log_HandleTypeDef;


extern MPSCQueue logQueue;

void* producer(void* arg);
