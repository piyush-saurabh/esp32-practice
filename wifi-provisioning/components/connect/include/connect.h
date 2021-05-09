#ifndef _CONNECT_H_
#define _CONNECT_H_

void wifiInit(void *params);

extern xSemaphoreHandle initSemaphore; // for passing this semaphore to server.c

#endif