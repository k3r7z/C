#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include "../TUI/tui.h"


typedef struct{
    int thread_number;
    bool *array;
    int from, to;
    bool *found;
} ThreadData;


void *find_bit(void *arg){
    ThreadData *data = (ThreadData*) arg;

    int i = data->from;
    while( *data->found == false && i < data->to ){
	if( data->array[i] == true ){
	    tui_print_success("(Thread %d) Bit found in the position %d", data->thread_number, i);
	    *data->found = true;
	    return NULL;
	}
	++i;
    }

    if( *data->found == true)
	tui_print_info("(Thread %d) Stopping because bit was found in other thread", data->thread_number);
    else
	tui_print_error("(Thread %d) Couldn't find the bit in the assignated range", data->thread_number);
    
    return NULL;
}


static void print_help(){
    printf("-n <number of threads> (default 4) -s <size of the array> (default 1 000 000)\n");
}

int main(int argc, char **argv){
    int opt;
    int array_size = 1000000;
    int number_of_threads = 4;

    while( (opt = getopt(argc, argv, "n:s:h")) != -1){
	switch(opt){
	case 'h':
	    print_help();
	    return 0;
	case 'n':
	    number_of_threads = atoi(optarg);
	    break;
	case 's':
	    array_size = atoi(optarg);
	    break;
	case '?':
	    return 1;
	}
    }
    
    time_t start = clock();
    srand(time(NULL));
    bool *array = malloc(array_size * sizeof(bool));
    for(int i = 0; i < array_size; ++i)
	array[i] = false;
    int random_position = rand() % array_size;
    array[random_position] = true;
    tui_print_info("The 1 is on the %d position", random_position);
    

    bool *found = malloc(sizeof(bool));
    int range_division = array_size / number_of_threads;
    pthread_t threads[number_of_threads];
    ThreadData data[number_of_threads];
    for(int i = 0; i < number_of_threads; ++i){
	data[i].thread_number = i + 1;
	data[i].array = array;
	data[i].from = range_division * i;
	data[i].to = range_division * (i + 1);
	data[i].found = found;
	pthread_create(&threads[i], NULL, find_bit, &data[i]);
    }

    for(int i = 0; i < number_of_threads; ++i )
	pthread_join(threads[i], NULL);

    free(found);
    free(array);
    array = NULL;

    double execution_time = (double) ( clock() - start ) / CLOCKS_PER_SEC;
    tui_print_info("Program finished in %.3f seconds", execution_time);
    return 0;
}
