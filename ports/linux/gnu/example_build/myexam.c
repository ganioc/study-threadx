#include <stdio.h>
#include "tx_api.h"

#define MY_BYTE_POOL_SIZE (8*1024)
#define MY_STACK_SIZE      1024

TX_BYTE_POOL byte_pool_0;

ULONG thread_0_counter;

TX_THREAD thread_0;
TX_THREAD thread_slow;
TX_TIMER  my_timer;
TX_EVENT_FLAGS_GROUP  my_event_group;
TX_QUEUE  my_queue;

void thread_0_entry(ULONG thread_input);
void thread_slow_entry(ULONG thread_input);
void my_timer0_function(ULONG timer_input);

TX_MUTEX   my_mutex;

int main(void){
    printf("hello myexam\n");

    tx_kernel_enter();

    return 0;
}
/* Define what the initial system looks like. */
void tx_application_define(void *first_unused_memory){
    CHAR* pointer = TX_NULL;
    ULONG status;

    // create a byte memory pool from which to allocate the thread 
    // stacks
    tx_byte_pool_create(&byte_pool_0, "byte_pool 0",
        first_unused_memory, MY_BYTE_POOL_SIZE);

    // Put system definition stuff in here, thread creates and 
    // other assorted create information
    // Allocate the stack for thread_0
    tx_byte_allocate(&byte_pool_0,(VOID **)&pointer,
        MY_STACK_SIZE, TX_NO_WAIT);
    
    // Create the main thread
    tx_thread_create(&thread_0, "thread_0", thread_0_entry, 0,
        pointer, MY_STACK_SIZE,
        1,1,TX_NO_TIME_SLICE, TX_AUTO_START);

    // Allocate the stack for thread_slow
    tx_byte_allocate(&byte_pool_0, (VOID**)&pointer, MY_STACK_SIZE,
        TX_NO_WAIT);

    // Create thread_slow 
    tx_thread_create(&thread_slow, "thread_slow",
        thread_slow_entry,1,
        pointer, MY_STACK_SIZE,
        15,15,TX_NO_TIME_SLICE, TX_AUTO_START);
    
    // create the mutex used by both threads, 0 & slow
    tx_mutex_create(&my_mutex, "my_mutex",TX_NO_INHERIT);

    // Create an application timer that executes,
    status = tx_timer_create(&my_timer, "my_timer0",
        my_timer0_function, 0x1234, 100, 25,
        TX_AUTO_ACTIVATE);

    // Activate an application timer,
    status = tx_timer_activate(&my_timer);

    // Create an event flags group
    status = tx_event_flags_create(&my_event_group,
        "my_event_group_name");
    
    // create a message queue,
    status = tx_queue_create(&my_queue, "my_queue0",
        TX_4_ULONG, 
        (VOID*)(first_unused_memory+MY_BYTE_POOL_SIZE),
        2000);
    
}

// thread_0 threads
void thread_0_entry(ULONG thread_input){
    UINT status;
    ULONG current_time;
    ULONG my_rx_message[4];

    while(1){
        thread_0_counter++;

        printf("thread_0 event sent: %lu\n", thread_0_counter);

        // 10ms per click
        tx_thread_sleep(2);

        status = tx_mutex_get(&my_mutex, TX_WAIT_FOREVER);
        if(status != TX_SUCCESS){
            break;
        }

        tx_thread_sleep(5);

        status = tx_mutex_put(&my_mutex);
        if(status != TX_SUCCESS){
            break;
        }

        tx_thread_sleep(4);

        status = tx_mutex_get(&my_mutex, TX_WAIT_FOREVER);
        if(status != TX_SUCCESS){
            break;
        }

        tx_thread_sleep(3);

        status = tx_mutex_put(&my_mutex);
        if(status != TX_SUCCESS){
            break;
        }    

        status = tx_queue_receive(&my_queue,
            my_rx_message,
            TX_WAIT_FOREVER);
        if(status == TX_SUCCESS){
            printf("%4lX %4lX\n", my_rx_message[0],
                my_rx_message[1]);
            printf("Thread 0 rx message OK!\n");
        }

        current_time = tx_time_get();
        printf("Current time: %lu Speedy_thread finished cycle ...\n",
            current_time);         
        
    }
}
// thread_slow
void thread_slow_entry(ULONG thread_input){
    UINT status;
    ULONG current_time;
    ULONG my_message[4];

    while(1){
        status=tx_mutex_get(&my_mutex,TX_WAIT_FOREVER);
        if(status != TX_SUCCESS) break;

        tx_thread_sleep(12);

        status = tx_mutex_put(&my_mutex);
        if(status != TX_SUCCESS) break;

        tx_thread_sleep(8);

        status=tx_mutex_get(&my_mutex,TX_WAIT_FOREVER);
        if(status != TX_SUCCESS) break;

        tx_thread_sleep(11);

        status = tx_mutex_put(&my_mutex);
        if(status != TX_SUCCESS) break;

        tx_thread_sleep(9);

        // send to message queue,
        my_message[0] = 10;
        my_message[1] = 11;
        my_message[2] = 12;
        my_message[3] = 13;
        status = tx_queue_send(&my_queue,
            my_message, TX_NO_WAIT);
        if(status == TX_SUCCESS){
            printf("Thread1 send msg OK!\n");
        }

        current_time= tx_time_get();
        printf("Current time: %lu Slow_thread finished cycle...\n",
            current_time);
    }
}
void my_timer0_function(ULONG timer_input){
    ULONG current_time;
    current_time = tx_time_get();
    printf("timer0 function: %ld\n", current_time);
}


