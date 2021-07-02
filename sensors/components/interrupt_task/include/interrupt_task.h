#ifndef _INTERRUPT_TASK_H_
#define _INTERRUPT_TASK_H_


void interrupt_task(void *params);
xSemaphoreHandle pushButtonSemaphore;
xQueueHandle interputQueue;

#endif
