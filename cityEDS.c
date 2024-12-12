#include <stdio.h>
#include <unistd.h>

#include "data.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"


void cityEDS_entry_point(void) {

    // Initialize departments
    init_departments(&police, &ambulance, &fire);

    // Event queue
    QueueHandle_t event_queue = xQueueCreate(10, sizeof(Event));

    // Task handles
    TaskHandle_t dispatcher_handle = NULL;
    TaskHandle_t log_task_handle;

    // Create dispatcher task
    xTaskCreate(dispatcher_task, "Dispatcher", configMINIMAL_STACK_SIZE, event_queue, tskIDLE_PRIORITY + 1, &dispatcher_handle);

    // Create department tasks
    xTaskCreate(department_task, "Police", configMINIMAL_STACK_SIZE, &police, tskIDLE_PRIORITY + 1, NULL);
    xTaskCreate(department_task, "Ambulance", configMINIMAL_STACK_SIZE, &ambulance, tskIDLE_PRIORITY + 1, NULL);
    xTaskCreate(department_task, "Fire", configMINIMAL_STACK_SIZE, &fire, tskIDLE_PRIORITY + 1, NULL);

    // Create log task
    xTaskCreate(log_task, "LogTask", configMINIMAL_STACK_SIZE,  &log_queue, tskIDLE_PRIORITY + 1, &log_task_handle);    

    // Create event generator task
    xTaskCreate(generate_event, "EventGen", configMINIMAL_STACK_SIZE, &event_queue, tskIDLE_PRIORITY + 1, NULL);
    
    // Start scheduler
    vTaskStartScheduler();

    // If scheduler returns, write logs to file
    write_log_to_file();
    }
    
