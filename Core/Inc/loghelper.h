/**
 * @file loghelper.h
 * @author your name (you@domain.com)
 * @brief 信息输出控制器，使用多生产者-单消费者模式（MPSC）
 *        预留接口供其他生产者传递log信息进入消息队列，
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


// 定义节点结构
struct Node {
    char* log;
    struct Node* next;
};

// 定义队列结构
struct Queue {
    struct Node* head;
    struct Node* tail;
};

/**
 * @brief 实体调试需判定对齐规则
 * 
 */
typedef struct
{
    // char *info[1000];                       /* 液冷控制器信息描述,1k缓存*/
    Queue queue;
    UART_HandleTypeDef *huart;
    void (* initQueue)(Queue* queue);       /*!< 配置用户通讯接口   */
    void (* enqueue)(Queue* queue, char* log);        /*!< 启动液冷控制器  建议工作频率20hz   */      
    void (* Run)();        /*!< 启动液冷控制器  建议工作频率20hz   */      

} Log_HandleTypeDef;


extern MPSCQueue logQueue;

void* producer(void* arg);
