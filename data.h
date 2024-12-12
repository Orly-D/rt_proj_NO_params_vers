#ifndef DATA_H
#define DATA_H

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

// Event codes
#define POLICE_EVENT 1
#define AMBULANCE_EVENT 2
#define FIRE_EVENT 3

// Resource limits
#define POLICE_CARS 3
#define AMBULANCES 4
#define FIRE_TRUCKS 2

// Logging queue size
#define LOG_QUEUE_SIZE 50


// Structure for an event
typedef struct {
    int code;       
    int priority;   
} Event;

// Structure for department resources
typedef struct {
    int total_resources;
    int available_resources;
    QueueHandle_t task_queue;
    SemaphoreHandle_t resource_mutex;                               // Lock for res' protection
    SemaphoreHandle_t resource_semaphore;                           // Tracks multiple res'

} Department;

// Log message structure
typedef struct {
    char message[128];
    int timestamp;
} LogMessage;

// Declaring global variables 
extern Department police;       
extern Department ambulance;    
extern Department fire;  
extern QueueHandle_t log_queue;
extern TaskHandle_t dispatcher_handle;

// Function prototypes
int init_departments(Department* police, Department* ambulance, Department* fire);
int generate_event(QueueHandle_t *event_queue);
int dispatcher_task(QueueHandle_t *event_queue, TaskHandle_t *dispatcher_handle);
int department_task(Department *department);
void get_timestamp(char* buffer, size_t size);
int log_task(void* params);
int write_log_to_file(void);

#endif // DATA_H
