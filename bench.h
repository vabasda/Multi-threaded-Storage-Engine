#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "../engine/db.h"

#define KSIZE (16)
#define VSIZE (1000)

#define LINE "+-----------------------------+----------------+------------------------------+-------------------+\n"
#define LINE1 "---------------------------------------------------------------------------------------------------\n"

//We define a struct, containing the arguments of _write_test and _read_test
typedef struct _data{
  long int count;
  int r;
  DB* db;
}data;


long long get_ustime_sec(void);
void _random_key(char *key,int length);
