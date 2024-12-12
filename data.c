#include "data.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Global variables for departments
Department police, ambulance, fire;

// Log queue
QueueHandle_t log_queue;

// Initialize departments
int init_departments(Department* police, Department* ambulance, Department* fire) {
    LogMessage log_entry;

     // Create log queue        ??
    log_queue = xQueueCreate(LOG_QUEUE_SIZE, sizeof(LogMessage));
    if (log_queue == NULL) {
        printf("Error: Failed to create log queue\n");
        strncpy(log_entry.message, "Error creating log queue", sizeof(log_entry.message));
        log_entry.timestamp = xTaskGetTickCount();
        xQueueSend(log_queue, &log_entry, portMAX_DELAY);
        return -1;
    }
    strncpy(log_entry.message, "Log queue created successfully", sizeof(log_entry.message));
    log_entry.timestamp = xTaskGetTickCount();
    xQueueSend(log_queue, &log_entry, portMAX_DELAY);
    

    // Police department
    police->total_resources = POLICE_CARS;
    police->available_resources = POLICE_CARS;
    police->task_queue = xQueueCreate(10, sizeof(Event));                               // Create queue for inner department tasks
    police->resource_mutex = xSemaphoreCreateMutex();                                   // Mutex for lock
    police->resource_semaphore = xSemaphoreCreateCounting(POLICE_CARS, POLICE_CARS);    // (max res',entry res')
    
    if (police->task_queue == NULL || 
        police->resource_mutex == NULL || 
        police->resource_semaphore == NULL) {
        printf("Error: Failed to initialize police department resources\n");
        strncpy(log_entry.message, "Error initializing police department resources", sizeof(log_entry.message));
        log_entry.timestamp = xTaskGetTickCount();
        xQueueSend(log_queue, &log_entry, portMAX_DELAY);
        return -1;
    }
    strncpy(log_entry.message, "Successfully initialized police department resources", sizeof(log_entry.message));
    log_entry.timestamp = xTaskGetTickCount();
    xQueueSend(log_queue, &log_entry, portMAX_DELAY);

    // Ambulance department
    ambulance->total_resources = AMBULANCES;
    ambulance->available_resources = AMBULANCES;
    ambulance->task_queue = xQueueCreate(10, sizeof(Event));
    ambulance->resource_mutex = xSemaphoreCreateMutex();
    ambulance->resource_semaphore = xSemaphoreCreateCounting(AMBULANCES, AMBULANCES);
    
    if (ambulance->task_queue == NULL || 
        ambulance->resource_mutex == NULL || 
        ambulance->resource_semaphore == NULL) {
        printf("Error: Failed to initialize ambulance department resources\n");
        strncpy(log_entry.message, "Error initializing ambulance department resources", sizeof(log_entry.message));
        log_entry.timestamp = xTaskGetTickCount();
        xQueueSend(log_queue, &log_entry, portMAX_DELAY);
        return -1;
    }
    strncpy(log_entry.message, "Successfully initialized ambulance department resources", sizeof(log_entry.message));
    log_entry.timestamp = xTaskGetTickCount();
    xQueueSend(log_queue, &log_entry, portMAX_DELAY);

    // fire department
    fire->total_resources = FIRE_TRUCKS;
    fire->available_resources = FIRE_TRUCKS;
    fire->task_queue = xQueueCreate(10, sizeof(Event));
    fire->resource_mutex = xSemaphoreCreateMutex();
    fire->resource_semaphore = xSemaphoreCreateCounting(FIRE_TRUCKS, FIRE_TRUCKS);
    
    if (fire->task_queue == NULL || 
        fire->resource_mutex == NULL || 
        fire->resource_semaphore == NULL) {
        printf("Error: Failed to initialize fire department resources\n");
        strncpy(log_entry.message, "Error initializing fire department resources", sizeof(log_entry.message));
        log_entry.timestamp = xTaskGetTickCount();
        xQueueSend(log_queue, &log_entry, portMAX_DELAY);
        return -1;
    }
    strncpy(log_entry.message, "Successfully initialized fire department resources", sizeof(log_entry.message));
    log_entry.timestamp = xTaskGetTickCount();
    xQueueSend(log_queue, &log_entry, portMAX_DELAY);

    return 0;
}

// Generate random event
int generate_event(QueueHandle_t *event_queue) {
    

    LogMessage log_entry;      

    srand(time(NULL));

    while (1) { 
        // maybe i need here xQueuerecive for luck res'
        Event event;
        event.code = (rand() % 3) + 1;                                                      // Random event code between 1 to 3
        event.priority = rand() % 10 + 1;                                                   // Random priority between 1 to 10
        if (xQueueSend(*event_queue, &event, portMAX_DELAY) != pdPASS) {
            printf("Error: Failed to add event to queue\n");
            continue; // Skip to next iteration
        }

        UBaseType_t queue_size = uxQueueMessagesWaiting(*event_queue);

        char log_msg[128];
        snprintf(log_msg, sizeof(log_msg), 
                    "Generated Event: Code %d, Priority %d, Queue Status: %ld events in queue", 
                    event.code, event.priority, queue_size);
        printf("%s\n", log_msg); 

        strncpy(log_entry.message, log_msg, sizeof(log_entry.message));
        log_entry.timestamp = xTaskGetTickCount();

        // Send log message to queue
        BaseType_t queue_status = xQueueSend(log_queue, &log_entry, portMAX_DELAY);
        if (queue_status != pdPASS) {
            printf("Error: Failed to send log message to queue\n");
            // Log a failure message
            strncpy(log_entry.message, "Failed to send log message to queue", sizeof(log_entry.message));
            log_entry.timestamp = xTaskGetTickCount();
            return -1;
        }
        
        snprintf(log_msg, sizeof(log_msg), "[%d] Event added to queue: Code %d, Priority %d", log_entry.timestamp, event.code, event.priority);
        strncpy(log_entry.message, log_msg, sizeof(log_entry.message));
        xQueueSend(log_queue, &log_entry, portMAX_DELAY);

        xTaskNotifyGive(dispatcher_handle);                                                 // Signal event ready
     
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    return 0;
}

// Dispatcher task
int dispatcher_task(QueueHandle_t *event_queue, TaskHandle_t *dispatcher_handle) {
    LogMessage log_entry;

    while (1) {
        Event event;                                                                    // Save the event

        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);                                        // Wait in block state for event
        printf("test\n");
        if (uxQueueMessagesWaiting(*event_queue) > 0) {                                 // Check if there are any events in the queue
            printf("test2 \n");

            if (xQueueReceive(*event_queue, &event, portMAX_DELAY) != pdPASS) {         // If there are events in the queue, try to receive one
                // Handle the error if the event could not be received
                char log_msg[128];
                snprintf(log_msg, sizeof(log_msg), "Error: Failed to receive event from queue");

                strncpy(log_entry.message, log_msg, sizeof(log_entry.message));
                log_entry.timestamp = xTaskGetTickCount();
                xQueueSend(log_queue, &log_entry, 0);  

                printf("Error: Failed to receive event from queue\n");
                return -1;                                                              // Handle error by returning from the task
            }

            // If the event was received successfully
            char log_msg[128];
            snprintf(log_msg, sizeof(log_msg), "Dispatching Event: Code %d, Priority %d", event.code, event.priority);

            strncpy(log_entry.message, log_msg, sizeof(log_entry.message));
            log_entry.timestamp = xTaskGetTickCount();
            
            if (xQueueSend(log_queue, &log_entry, 0) != pdPASS) {
                printf("Error: Failed to send log message to queue\n");
                return -1;
            }

            //printf("Dispatching Event: Code %d, Priority %d\n", event.code, event.priority);

            // Dispatch event to the respective task queue
            if (event.code == POLICE_EVENT) {
                if (xQueueSend(police.task_queue, &event, portMAX_DELAY) != pdPASS) {
                    printf("Error: Failed to send event to police task queue\n");
                    return -1;
                }
            } else if (event.code == AMBULANCE_EVENT) {
                if (xQueueSend(ambulance.task_queue, &event, portMAX_DELAY) != pdPASS) {
                    printf("Error: Failed to send event to ambulance task queue\n");
                    return -1;
                }
            } else if (event.code == FIRE_EVENT) {
                if (xQueueSend(fire.task_queue, &event, portMAX_DELAY) != pdPASS) {
                    printf("Error: Failed to send event to fire task queue\n");
                    return -1;
                }
            }
        }                                                 
    }
    return 0;
}

// Department task
int department_task(Department *department) {                        
    //Event task_event;
    char dept_name[20];

    // Determine department name for logging
    if (department == &police) strcpy(dept_name, "Police");
    else if (department == &ambulance) strcpy(dept_name, "Ambulance");
    else strcpy(dept_name, "Fire");

    while (1) {
        Event task_event;

        if (xQueueReceive(department->task_queue, &task_event, portMAX_DELAY) != pdPASS) {
            printf("Error: Failed to receive event from department queue\n");
            return -1;
        }

        if (xSemaphoreTake(department->resource_semaphore, portMAX_DELAY) != pdPASS) {      // If data read succesfully from queue (with blocked state if empty queue)
            printf("Error: Failed to take semaphore\n");
            return -1;
        }

        if (xSemaphoreTake(department->resource_mutex, portMAX_DELAY) != pdPASS) {          // Lock if res' unavaible
            printf("Error: Failed to take mutex\n");
            return -1;
        }
      
        // Log task handling
        printf("[%s] Handling Event: Code %d, Priority %d\n", 
                dept_name, task_event.code, task_event.priority);

        // Process the event
        char log_msg[128];
        snprintf(log_msg, sizeof(log_msg), 
                    "%s Department handling Event: Code %d", 
                    dept_name, task_event.code);

        LogMessage log_entry;
        strncpy(log_entry.message, log_msg, sizeof(log_entry.message));
        log_entry.timestamp = xTaskGetTickCount();
        
        // Send log message to log_queue and check if it succeeded
        if (xQueueSend(log_queue, &log_entry, 0) != pdPASS) {
            printf("Error: Failed to send log message to queue\n");
            // Optionally, you can also break the loop or handle this differently
            return -1;
        }

        printf("%s\n", log_msg);
            
         // הוספת לוג שהמשימה טופלה בהצלחה
        snprintf(log_msg, sizeof(log_msg), "[%s] Task successfully handled and removed from the queue", dept_name);
        strncpy(log_entry.message, log_msg, sizeof(log_entry.message));
        log_entry.timestamp = xTaskGetTickCount();

        // Simulate task processing time
        vTaskDelay(pdMS_TO_TICKS((rand() % 5 + 1) * 100));

        // Release mutex and semaphore
        xSemaphoreGive(department->resource_mutex);
        xSemaphoreGive(department->resource_semaphore);
    }
    return 0;
}

void get_timestamp(char* buffer, size_t size) {
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", tm_info); // יצירת חותמת זמן בפורמט קריא
}

// Log task to write logs to file
int log_task(void* params) {
    LogMessage log_entry;
    FILE* log_file = fopen("emergency_dispatch.log", "w");

    if (!log_file) {
        printf("Error: Failed to open log file!\n");
        return -1;
    }

    while (1) {
        char timestamp[32];
        get_timestamp(timestamp, sizeof(timestamp));

        if (uxQueueMessagesWaiting(log_queue) > 0) {
            if (xQueueReceive(log_queue, &log_entry, portMAX_DELAY) != pdPASS) {
                printf("Error: Failed to receive log message from queue\n");
                return -1;
            }

            fprintf(log_file, "[Timestamp: %s] %s\n", timestamp, log_entry.message);

        } else {
            char log_msg[128];
            snprintf(log_msg, sizeof(log_msg), "Queue is empty");
            LogMessage empty_log_entry;
            strncpy(empty_log_entry.message, log_msg, sizeof(empty_log_entry.message));
            empty_log_entry.timestamp = xTaskGetTickCount();

            fprintf(log_file, "[Timestamp: %s] %s\n", timestamp, empty_log_entry.message);
            printf("%s\n", log_msg);

            vTaskDelay(pdMS_TO_TICKS(100));
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    fclose(log_file);
    return 0;
}

// Function to write logs to file at the end of simulation
int write_log_to_file(void) {
    FILE* log_file = fopen("emergency_dispatch.log", "a");
    LogMessage log_entry;

    if (!log_file) {
        printf("Error: Failed to open log file!\n");
        return -1;
    }

    while (xQueueReceive(log_queue, &log_entry, 0) == pdPASS) {
        fprintf(log_file, "[Timestamp: %d] %s\n", log_entry.timestamp, log_entry.message);
    }

    fclose(log_file);
    return 0;
}