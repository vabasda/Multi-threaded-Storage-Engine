#include "bench.h"

//Include pthread.h 
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

//Define a database to be used 
#define DATAS ("testdb")



void _random_key(char *key,int length) {
	int i;
	char salt[36]= "abcdefghijklmnopqrstuvwxyz0123456789";

	for (i = 0; i < length; i++)
		key[i] = salt[rand() % 36];
}

void _print_header(int count)
{
	double index_size = (double)((double)(KSIZE + 8 + 1) * count) / 1048576.0;
	double data_size = (double)((double)(VSIZE + 4) * count) / 1048576.0;

	printf("Keys:\t\t%d bytes each\n", 
			KSIZE);
	printf("Values: \t%d bytes each\n", 
			VSIZE);
	printf("Entries:\t%d\n", 
			count);
	printf("IndexSize:\t%.1f MB (estimated)\n",
			index_size);
	printf("DataSize:\t%.1f MB (estimated)\n",
			data_size);

	printf(LINE1);
}

void _print_environment()
{
	time_t now = time(NULL);

	printf("Date:\t\t%s", 
			(char*)ctime(&now));

	int num_cpus = 0;
	char cpu_type[256] = {0};
	char cache_size[256] = {0};

	FILE* cpuinfo = fopen("/proc/cpuinfo", "r");
	if (cpuinfo) {
		char line[1024] = {0};
		while (fgets(line, sizeof(line), cpuinfo) != NULL) {
			const char* sep = strchr(line, ':');
			if (sep == NULL || strlen(sep) < 10)
				continue;

			char key[1024] = {0};
			char val[1024] = {0};
			strncpy(key, line, sep-1-line);
			strncpy(val, sep+1, strlen(sep)-1);
			if (strcmp("model name", key) == 0) {
				num_cpus++;
				strcpy(cpu_type, val);
			}
			else if (strcmp("cache size", key) == 0)
				strncpy(cache_size, val + 1, strlen(val) - 1);	
		}

		fclose(cpuinfo);
		printf("CPU:\t\t%d * %s", 
				num_cpus, 
				cpu_type);

		printf("CPUCache:\t%s\n", 
				cache_size);
	}
}

//Initialize _write_test and _read_test, so that pthread_create can use them as arguments
void _write_test(void *arg);
void _read_test(void *arg);

int main(int argc,char** argv)
{
        //Variables for measuring time
        long long start,end;
	double cost;

	//Initializing the structs that we will be used
	struct _data add_data;
	struct _data get_data;
	struct _data get_data_extra;

	//Start measuring system time
       	start = get_ustime_sec();
	
	srand(time(NULL));
	if (argc < 3) {
		fprintf(stderr,"Usage: db-bench <write | read> <count>\n");
		exit(1);
	}
	
	if (strcmp(argv[1], "write") == 0) {

	        //Change add_data variables
		add_data.r = 0;
		add_data.count = atoi(argv[2]);
		
		//Open the database, since _write_test no longer executes db_open()
		DB* db = db_open(DATAS);
		add_data.db = db;
		
		_print_header(add_data.count);
		_print_environment();
		if (argc == 4)
		    add_data.r = 1;
		_write_test(&add_data);

		//Close the database, since _read_test no longer executes db_close()
		db_close(db);

		//Stop measuring time, calculate cost, and print results
	        end = get_ustime_sec();
	        cost = end - start;
		printf(LINE);
	        printf("|Random-Write	(done:%ld): %.6f sec/op; %.1f writes/sec(estimated); cost:%.3f(sec);\n"
		       ,add_data.count, (double)(cost / add_data.count)
		       ,(double)(add_data.count / cost)
		       ,cost);
	        
		
	} else if (strcmp(argv[1], "read") == 0) {

	        //Change get_data variables
		get_data.r = 0;
		get_data.count = atoi(argv[2]);

		//Open database, since _read_test no longer executes db_open()
		DB* db = db_open(DATAS);
		get_data.db = db;
		
		_print_header(get_data.count);
		_print_environment();
		if (argc == 4)
			get_data.r = 1;
		
		_read_test(&get_data);

		//Close database, since _read_test no longer executes db_close()
		db_close(db);

		//Stop measuring time, calculate cost, and print results
	        end = get_ustime_sec();
	        cost = end - start;
	        printf(LINE);
	        printf("|Random-Read	(done:%ld): %.6f sec/op; %.1f reads /sec(estimated); cost:%.3f(sec)\n",
	                get_data.count,
		        (double)(cost / get_data.count),
	                (double)(get_data.count / cost),
		        cost);
		
	} else if (strcmp(argv[1], "readwrite") == 0) {

	         //Change struct's variables
	         add_data.r = 0;
		 get_data.r = 0;
		 get_data_extra.r = 0;

		 //Store command line arguments from user
		 int percentageW = atoi(argv[3]);
		 int percentageR = atoi(argv[4]);
		 int get_threads_count = atoi(argv[5]);

		 //Open the database and update the database of the structs, so that write and read operations
		 //will be executed on the same database
	         DB* db = db_open(DATAS);
		 add_data.db = db;
		 get_data.db = db;
		 get_data_extra.db = db;

		 //Store how many operations to be executed
		 long int count = atoi(argv[2]);
		 
		 _print_header(count);
		 _print_environment();

		 //Calculate how many write and how many read operations will be executed
		 add_data.count = count*percentageW/100;
		 get_data.count = count*percentageR/100;

		 //Initialize the thread ids
		 pthread_t add_thread;
		 pthread_t get_thread[get_threads_count];

		 //Create the write thread, which will call _write_test with add_data struct as argument
		 pthread_create(&add_thread,NULL,(void *(*)(void *)) _write_test, (void *) &add_data);

		 //Chech if there will be more that one add threads
		 //If so, split the add operations fairly, to each thread
                 if(get_threads_count > 1){

		   //Get the integer part of the division
		   get_data.count = get_data.count / get_threads_count;

		   //Create every thread except one, where its one calls _read_test with get_data as argument,
		   //and its _read_test will execute the integer part of the get_data.count = get_data.count / get_threads_count division
		   for(int i = 0; i < get_threads_count-1; i++){
		      pthread_create(&get_thread[i],NULL,(void *(*)(void *)) _read_test, (void *) &get_data);
		   }

		   //Calculate the remaining adds, update get_data_extra struct, and create the last add thread
		   get_data_extra.count = count*percentageR/100 - (get_data.count*(get_threads_count-1));
		   pthread_create(&get_thread[get_threads_count-1],NULL,(void *(*)(void *)) _read_test, (void *) &get_data_extra);
		 }
		 
		 //If there is only one add thread, create it and call _read_test with get_data struct as argument
		 else{
		   pthread_create(&get_thread[0],NULL,(void *(*)(void *)) _read_test, (void *) &get_data);
		 }
		 
		 //Join all the threads, after they are done working 
		 pthread_join(add_thread,NULL);
		 for(int i = 0; i < get_threads_count; i++){
		    pthread_join(get_thread[i],NULL);
		 }

		 //Close the database
	         db_close(db);
		 
		 //Stop measuring time, calculate cost, and print results
	         end = get_ustime_sec();
	         cost = end - start;
	         printf(LINE);
	         printf("|Random-Read_Write	(done:%ld): %.6f sec/op; cost:%.3f(sec)\n",
		         count,
		         (double)(cost /count),
		         cost);
		 
		 }else {
		   fprintf(stderr,"Usage: db-bench <write | read> <count> <random>\n");
		    exit(1);
	         }
	
	return 1;
}
