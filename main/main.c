#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "freertos/semphr.h"

#if CONFIG_FREERTOS_UNICORE
    static const BaseType_t app_cpu = 0; 
#else 
    static const BaseType_t app_cpu = 1; 
#endif

//Global Vars
static SemaphoreHandle_t sem;    
static uint8_t sCount = 3;         
static uint8_t flag = 0;                                  
static char* initStr = " -- Semaphore Demo --"; 
static char* strPtr = NULL; 

//Watcher Task: Task that passes value to be used in string generation then will use SemaphoreTake to watch and report once all tasks are completed
void taskOne(){
    {                                                                                                                   //Extra Scope [necessary] for proper cleanup using vTaskDelete()
        char* taskName = pcTaskGetName(NULL); 
        ESP_LOGW(taskName, "Generating string ..."); 
        uint8_t inChar; 
        uint8_t count = 0; 
        while(2 != flag){
            inChar = fgetc(stdin);                                                                                      //taking input must be inluded in loop; take until enter IN
            if (0 == flag){                                                                                             //Initialize str on stack 
                ESP_LOGI(taskName, "Enter Msg... "); 
                strPtr = (char *)pvPortMalloc(100*sizeof(char));                                                        //Default heap has space for 100 Chars
                flag = 1; 
            }   
            if ('\n' == inChar){                                                                                        //If enter IN
                strPtr[count] = inChar; 
                count = 0;    
                flag = 2; 
            } else if (0xFF == inChar){                                                                                 //Special Char to be ignored
                ; 
            } else {                                                                                                    //If anything other than [enter] IN
                strPtr[count] = inChar; 
                count++; 
            }
            vTaskDelay(50/portTICK_PERIOD_MS); 
        }
        for (size_t i = 0; i < sCount; i++){                                                                            //take sem equal to number max count 
            xSemaphoreTake(sem, portMAX_DELAY); 
        }
        ESP_LOGW(taskName, "All tasks have OUT msg. Generator task destruction ..."); 
    }
    
    vPortFree(strPtr);                                                                                                  //Free heap memory msg stored on
    flag = 3;                                                                       
    vTaskDelete(NULL);                                                                                                  //Watcher Deleting itself
    
}
//Generator Task: Will generate strings to be made based of passed value
void taskTwo(void *arg){
        char* taskName = pcTaskGetName(NULL); 
        while (1){                                                          
            if (2 == flag){                                                                                             //If message has been accepted from serial, print and return sem            
                uint8_t i = 0; 
                char checkChar = ' '; 
                ESP_LOGI(taskName, "Msg Received!");  
                if (NULL != strPtr){
                    while ('\n' != checkChar){
                        checkChar = strPtr[i]; 
                        printf("%c", strPtr[i]);
                        i++; 
                    } 
                    xSemaphoreGive(sem);  
                }
                vTaskDelay(100/portTICK_PERIOD_MS);
            } else if (3 == flag){                                                                                      //Post Message Recieved 
                ESP_LOGI(taskName, "waiting ..."); 
                vTaskDelay(10000/portTICK_PERIOD_MS); 
            } else {                                                                                                    //Pre msg recieved; to avoid watchdog error
                vTaskDelay(100/portTICK_PERIOD_MS); 
            }
        }
}

void setup(){
    vTaskDelay(1000/portTICK_PERIOD_MS); 
    char* taskName = pcTaskGetName(NULL);
    ESP_LOGI(taskName, "%s", initStr);
    sem = xSemaphoreCreateCounting(sCount, 0);                                                                          //Initialize counting semaphore
    xTaskCreatePinnedToCore(taskOne, "Watcher", 2048, NULL, 1, NULL, app_cpu); 
    xTaskCreatePinnedToCore(taskTwo, "Generator 1", 2048, NULL, 1, NULL, app_cpu); 
    xTaskCreatePinnedToCore(taskTwo, "Generator 2", 2048, NULL, 1, NULL, app_cpu); 
    xTaskCreatePinnedToCore(taskTwo, "Generator 3", 2048, NULL, 1, NULL, app_cpu); 
}

void app_main(void){
    setup(); 
}
