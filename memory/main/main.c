#include <stdio.h>
#include <string.h>
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define TAG "Memory"

// Sample task to test the memory
void sampleTask(void *param)
{
  int stackmem = uxTaskGetStackHighWaterMark(NULL);
  ESP_LOGI(TAG, "Available stack space in simpleTask = %d bytes", stackmem);

  // occupy 7000 bytes on stack
  //char buffer [4000];
  //memset(&buffer,1,sizeof(buffer)); // initialize it to 1 so that compiler doesn't optimize it 

  // infinite loop (since the FreeRTOS task should not return anything)
  while(true) vTaskDelay(1000);
}

// Print the amount of available DRAM and IRAM
void get_dram_iram()
{
    // We can also get available DRAM (in Bytes) using xPortGetFreeHeapSize()
    ESP_LOGI(TAG, "xPortGetFreeHeapSize %dk = DRAM", xPortGetFreeHeapSize());

    // Get entire RAM size (in Bytes)
    int entireRAM = heap_caps_get_free_size(MALLOC_CAP_32BIT);

    // Get availabe DRAM (in Bytes)
    int DRam = heap_caps_get_free_size(MALLOC_CAP_8BIT);

    // Get IRAM
    // IRAM = EntireRAM - DRAM (in Bytes)
    int IRam = heap_caps_get_free_size(MALLOC_CAP_32BIT) - heap_caps_get_free_size(MALLOC_CAP_8BIT);

    ESP_LOGI(TAG, "DRAM \t\t %d bytes", DRam);
    ESP_LOGI(TAG, "IRam \t\t %d bytes", IRam);

    // Find largest contiguous block of available memory (for malloc)
    int freeDRAM = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);
    ESP_LOGI(TAG, "Largest available DRAM block = %d bytes", freeDRAM);


    // Get the available stack memory for the 'current task'
    // Pass NULL for the "current task" else pass the task handler
    int stackmem = uxTaskGetStackHighWaterMark(NULL);
    ESP_LOGI(TAG, "Available stack space in main task = %d bytes", stackmem);
}

// FreeRTOS task to report free stack memory of all the running tasks
void get_free_memory(void *param)
{
  /*
  TaskStatus_t *task_array;
  volatile UBaseType_t num_tasks, x;
  uint32_t total_runtime, ulStatsAsPercentage;

  // Take a snapshot of the number of tasks in case it changes while this function is executing.
 uxArraySize = uxTaskGetNumberOfTasks();

 // Allocate a TaskStatus_t structure for each task.  An array could be
 // allocated statically at compile time.
 pxTaskStatusArray = pvPortMalloc( uxArraySize * sizeof( TaskStatus_t ) );

 if( pxTaskStatusArray != NULL )
 {
      // Generate raw status information about each task.
     //uxArraySize = uxTaskGetSystemState( pxTaskStatusArray, uxArraySize, &ulTotalRunTime );
     UBaseType_t numTaskPopulated = uxTaskGetSystemState(task_array, num_tasks, &total_runtime);

     // For percentage calculations.
     total_runtime /= 100UL;

     // Avoid divide by zero errors.
     if( total_runtime > 0 )
     {
       char *memory_stats = (char *)pvPortMalloc(30 * num_tasks + 53);
       if(memory_stats)
       {
         memset(memory_stats, (int)' ', (30 * num_tasks) + 53);
         char *current_spot = memory_stats;
         memcpy(memory_stats, "\nName\t\t\t\tStack\t\tCPU\t\tCore\n=======================================\n", 53);

         current_spot +=53;
         for(int i=0; i<numTaskPopulated; i++)
         {
           int task_runtime_percentage = task_array[i].ulRunTimeCounter / total_runtime;
           memcpy(current_spot, task_array[i].pcTaskName, strlen(task_array[i].pcTaskName));
           current_spot += 12;
           
         }

       }
     }

*/
 }


  // Get the free memory of all the running task
  uxTaskGetSystemState();

  // report memory every 5 sec
  vTaskDelay(5000 / portTICK_PERIOD_MS);
}


void app_main()
{
    /*
        DRAM ~ 293 KB
        Static DRAM ~ 10 KB (calculated using idf.py size)
        IRAM ~ 88 KB
        Static IRAM ~ 40 KB (calculated using idf.py size)
        CPU Cache = 64 KB (used by ESP to make chip faster)
        Reserved = 8 KB
        Total RAM - 293 + 10 + 88 + 40 + 64 + 8 = 503 (~9KB used for some other purpose e.g padding)
        
    */
  get_dram_iram();

  // Create a new task
  // task function, task name, stack depth, parameter to stack, task priority, task handle
  xTaskCreate(&sampleTask, "Sample Task", 8000, NULL, 1, NULL); // out of 8000 bytes, 400 bytes will be used by FreeRTOS
  
  xTaskCreate(&get_free_memory, "Memory", 4096, NULL, 1, NULL);
}