/**
 * @file loghelper.c
 * @author your name (you@domain.com)
 * @brief 信息输出控制器，使用多生产者-单消费者模式（MPSC）
 *        预留接口供其他生产者传递log信息进入消息队列，
 *          
 * @version 0.1
 * @date 2023-07-31
 * 
 * @todo 需要绑定定时器，用以完成周期性检测输出log功能。
 *       使用非中断输出形式时，应保证输出时间可控，当超时候队列仍有数据将强制返回，下个周期再做输出
 * @copyright Copyright (c) 2023
 * 
 */

#include "loghelper.h"

//需匹配print系列函数的定向文件
#include "usb_device.h"

// 初始化队列
staic void initQueue(struct Queue* queue) {
    queue->head = NULL;
    queue->tail = NULL;
}

// 入队
staic void enqueue(struct Queue* queue, char* log) {
    struct Node* newNode = (struct Node*)malloc(sizeof(struct Node));
    if (newNode == NULL) {
        // 内存分配失败
        perror("Memory allocation error");
        exit(EXIT_FAILURE);//todo 需验证这里是否会导致程序跑飞
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

// 出队
staic char* dequeue(struct Queue* queue) {
    if (queue->head == NULL) {
        return NULL; // 队列为空
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

// // 示例函数1，作为生产者1
// void producerFunction1(struct Queue* queue) {
//     char* log1 = "Log message from producerFunction1";
//     enqueue(queue, log1);
// }


// 示例消费者函数
staic void consumer(struct Queue* queue) {
    while (true) {
        char* log = dequeue(queue);
        if (log != NULL) {
            printfln("Consumer: %s\n", log);
            free(log); // 注意：使用完日志后需要释放内存
        }
        // 在此可以添加适当的延迟，或者使用条件变量等机制来避免过度忙等待
    }
}

void LogerCreate(UART_HandleTypeDef *huartcooling) {
    struct Queue queue;
    initQueue(&queue);

    // 假设我们有多个不同的生产者函数
    producerFunction1(&queue);
    producerFunction2(&queue);
    producerFunction1(&queue);

    // 消费者函数
    consumerFunction(&queue);

    return 0;
}
