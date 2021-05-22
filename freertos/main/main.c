#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h" // For mutex and semaphore
#include "freertos/event_groups.h" // For event group
#include "esp_system.h" // get the time since how long the chip is on
#include "esp_timer.h" // for high resolution timer
#include "driver/gpio.h"

// Receiver handler for Task Notification demo
static TaskHandle_t receiverHandler = NULL;

// Create handler for semaphore (for mutex)
xSemaphoreHandle mutexBus;

// Create handler for semaphore (binary)
xSemaphoreHandle binSemaphore;

// Create handler for queue
xQueueHandle queue;

// Create handler for event group
EventGroupHandle_t evtGrp;
const int gotHttp = BIT0; // (1 << 0) flag to check if http is triggered
const int gotBLE = BIT1; // (1 << 1) flag to check if ble is triggered


// Task 1 for TaskNotification Demo
void sender(void *params)
{
    while(true)
    {
        // Notify the receiver
        // xTaskNotifyGive(receiverHandler);
        // vTaskDelay(1000 / portTICK_PERIOD_MS);
        

        // Sending simple data along with notification
        // Possible values for 3rd param: eSetBits, eSetValueWithOverwrite
        xTaskNotify(receiverHandler, (1<<0), eSetBits); // sending binary data
        vTaskDelay(1000 / portTICK_PERIOD_MS);

        xTaskNotify(receiverHandler, (1<<1), eSetBits);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

}

// Task 2 for TaskNotification Demo
void receiver(void *params)
{
    uint state;

    while(true)
    {
        // Take the notification from the sender
        // Usecase: count the number of times xTaskNotifyGive() is invoked
        // pdTRUE: Reset the counter.
        // portMAX_DELAY will halt the below line of code
        // used with xTaskNotifyGive
        //int count = ulTaskNotifyTake(pdTRUE, portMAX_DELAY); // Print 2 since xTaskNotifyGive is called twice

        //int count = ulTaskNotifyTake(pdFALSE, portMAX_DELAY); // Print 1, 2
        //printf("Received the Task Notification %d times\n", count);

        // used with xTaskNotify() while sending data
        xTaskNotifyWait(0xffffffff, 0, &state, portMAX_DELAY);
        printf("[Task Notification Demo] Received data from Task Notification %d times\n", state);
        
    }

}

// method which simulates write operation for mutex demo
// This can be any bus like serial, I2C, SPI
// This is a shared resource and only 1 task can use it at a time
void writeToBus(char *message)
{
    printf(message);
}

// Task 1 for Mutex Demo
// Both temperature and humidity task uses the shared resource - writeToBus()
void read_temperature(void *param)
{
    while(true)
    {
        printf("[Mutex Demo] Reading temperature... \n");
        // Block for 1 sec
        if(xSemaphoreTake(mutexBus, 1000 / portTICK_PERIOD_MS))
        {
            // If we can take the semaphore, write to the bus
            writeToBus("[Mutex Demo] Temperature is 30 degree C \n");

            // Release the mutex after using the shared resource
            xSemaphoreGive(mutexBus);
        }
        else
        {
            // if we cannot take the semaphore
            printf("[Mutex Demo] Writing temperature timed out \n");
        }
        
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }


}

// Task 2 for Mutex Demo
// Both temperature and humidity task uses the shared resource - writeToBus()
void read_humidity(void *param)
{
    while(true)
    {
        printf("[Mutex Demo] Reading humidity... \n");

        // Block for 1 sec
        if(xSemaphoreTake(mutexBus, 1000 / portTICK_PERIOD_MS))
        {
            writeToBus("[Mutex Demo] Humidity is 50\% \n");

            // Release the mutex after using the shared resource
            xSemaphoreGive(mutexBus);
        }
        else
        {
            // if we cannot take the semaphore
            printf("[Mutex Demo] Writing humidity timed out \n");
        }

        // Release the mutex

        
        vTaskDelay(2000 / portTICK_PERIOD_MS); // simulate that humidity write takes more time
    }
    
}


// Task 1 for Semaphore Demo
// This task simultate that it is listening for HTTP connection 
void listenForHTTP(void *param)
{
    while(true)
    {
        // After receiving a message, run some task like processHTTPRequest()
        printf("[Semaphore Demo] Received HTTP Message\n"); 

        // Notify
        xSemaphoreGive(binSemaphore);

        vTaskDelay(2000 / portTICK_PERIOD_MS);

    }
}

// Task 2 for Semaphore Demo
void processHTTPRequest(void *param)
{
    while(true)
    {
        // Wait here till some other task give this semaphore
        xSemaphoreTake(binSemaphore, portMAX_DELAY); // After executing this line, this task goes to sleep and it doesn't take any CPUs

        printf("[Semaphore Demo] Processing HTTP response ...\n");

        //vTaskDelay(2000 / portTICK_PERIOD_MS);

    }
}

// Task 1 for Queue Demo
// This task simultate that it is listening for HTTP connection 
void listenForHTTPQueue(void *param)
{
    // count the number of connections
    int count = 0;

    while(true)
    {
        count++;
        printf("[Queue Demo] Received HTTP Message\n"); 

        // Send the count to the other task via queue
        // Wait for 1 sec while sending to queue
        long ok = xQueueSend(queue, &count, 1000 / portTICK_PERIOD_MS);
        if (ok)
        {
            // Since the queue size is 3, we can add max 3 messages to queue without reading it
            printf("[Queue Demo] Added message to queue\n");
        }
        else
        {
            // If we try to add more message to queue without reading, we reach this part of the code
            printf("[Queue Demo] Failed to add message to queue\n");
        }

        vTaskDelay(2000 / portTICK_PERIOD_MS);

    }
}

// Task 2 for Queue Demo
void processHTTPRequestQueue(void *param)
{
    
    while(true)
    {
        // Data to receive from queue
        int rxInt;

        // Read data from queue
        // If there is no data in the queue, this task will go to sleep after 5 sec (it will not consume any CPU cycle)
        if(xQueueReceive(queue, &rxInt , 5000 / portTICK_PERIOD_MS))
        {
            printf("[Queue Demo] Processing HTTP response with the queue message %d ...\n", rxInt);
        }

        //vTaskDelay(2000 / portTICK_PERIOD_MS);

    }
}


// Task 1 for Event Group Demo
void listenForHTTPEventGroup(void *param)
{
    while(true)
    {
        // Set the HTTP bit
        xEventGroupSetBits(evtGrp, gotHttp);
        printf("[Event Group Demo] Received HTTP\n"); 
        
        vTaskDelay(2000 / portTICK_PERIOD_MS);

    }
}

// Task 2 for Event Group Demo
void listenForBluetoothEventGroup(void *param)
{
    while(true)
    {
        // Set the BLE bit
        xEventGroupSetBits(evtGrp, gotBLE);
        printf("[Event Group Demo] Received Bluetooth\n"); 

        vTaskDelay(5000 / portTICK_PERIOD_MS);

    }
}

// Task 3 for Event Group Demo
void init_task(void *param)
{
    while(true)
    {
        // This task is triggered only after completing HTTP and BLE task

        // Monitor the http and ble bits
        // 3rd argument: clear the argument. If false, it will fire once 
        // 4th arg: wait for all the bits. 
        // Wait here till we have both the bits set
        xEventGroupWaitBits(evtGrp, gotHttp | gotBLE, true, true, portMAX_DELAY);
        printf("[Event Group Demo] Received HTTP and BLE\n");
    }
}


// Callback function for timer demo (low resolution timer)
void on_timer(TimerHandle_t xTimer)
{
    // Time elapsed in ms since the chip has started
    printf("[Timer Demo] Time hit %lld\n", esp_timer_get_time() /1000);
}

// High resolution timer for timer demo
void timer_callback(void *args)
{
    // value of static variable will be stored in memory
    // Next time when this method is called, its value will be retained
    static bool on;

    // toggle the gpio
    on = !on;
    gpio_set_level(GPIO_NUM_4, on);
}

void app_main(void)
{
    // Task Notification 
    // Requires a handler, for e.g on receiver which sender can utilize
    // xTaskCreate(&receiver, "receiver", 2048, NULL, 2, &receiverHandler);
    // xTaskCreate(&sender, "sender", 2048, NULL, 2, NULL);

    // Mutex Demo
    // Create mutex
    // mutexBus = xSemaphoreCreateMutex();
    // xTaskCreate(&read_temperature, "temperature", 2048, NULL, 2, NULL);
    // xTaskCreate(&read_humidity, "humidity", 2048, NULL, 2, NULL);

    // Semaphore Demo
    // Create binary semaphore
    // binSemaphore = xSemaphoreCreateBinary();
    // xTaskCreate(&listenForHTTP, "listen http", 2048, NULL, 2, NULL);
    // xTaskCreate(&processHTTPRequest, "process http", 2048, NULL, 2, NULL);

    // Queue Demo
    // Create a queue
    // param1: total lenght of queue 
    // param2: size of each item in the queue
    // queue = xQueueCreate(3, sizeof(int));
    // xTaskCreate(&listenForHTTPQueue, "listen http queue", 2048, NULL, 2, NULL);
    // xTaskCreate(&processHTTPRequestQueue, "process http queue", 2048, NULL, 2, NULL);

    // Event Group Demo
    // Create event group
    // evtGrp = xEventGroupCreate();
    // xTaskCreate(&listenForHTTPEventGroup, "listen http event group", 2048, NULL, 1, NULL);
    // xTaskCreate(&listenForBluetoothEventGroup, "listen bluetooth event group", 2048, NULL, 1, NULL);
    // xTaskCreate(&init_task, "init task", 2048, NULL, 1, NULL);

    // Timer demo
    // Get the time in ms since the chip is up
    printf("App started %lld\n", esp_timer_get_time() / 1000);

    // Create the timer
    // param 2: amount of time/ticks before the timer kicks in
    // param 3: repeat or reload. Fire once or fire again and again. here after every 2 sec, the timer handler will executed
    // param 4: set ID
    // param 5: callback function once the timer has reached
    TimerHandle_t xTimer = xTimerCreate("my timer", pdMS_TO_TICKS(2000),true,NULL, on_timer);

    // Start the timer
    // 2nd param: time to wait before the timer is started
    xTimerStart(xTimer,0);

    // High resoultion timer demo
    // Use GPIO for this demo
    gpio_pad_select_gpio(GPIO_NUM_4);
    gpio_set_direction(GPIO_NUM_4, GPIO_MODE_OUTPUT);

    const esp_timer_create_args_t esp_timer_create_args = {
        .callback = timer_callback,
        .name = "My timer"};

    // Create the timer handler. It is used to start and stop the timer
    esp_timer_handle_t esp_timer_handle;

    // Create the timer
    esp_timer_create(&esp_timer_create_args, &esp_timer_handle);

    // Fire the timer once after 20 micro seconds
    //esp_timer_start_once(esp_timer_handle, 20);

    // Fire the timer periodically after every 50 microsecond
    esp_timer_start_periodic(esp_timer_handle, 50);

    int x = 0;
    while(true)
    {
        // Dump the timer output 
        esp_timer_dump(stdout);

        vTaskDelay(pdMS_TO_TICKS(1000));

        // Quit after invoking the timer 5 times
        // Clean up code
        if(x++ == 5)
        {
            // Stop the timer
            esp_timer_stop(esp_timer_handle);

            // Delete the timer
            esp_timer_delete(esp_timer_handle);
        }
    }


    
}