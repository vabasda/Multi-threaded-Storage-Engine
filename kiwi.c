#include <string.h>

//Include db.h
#include "../engine/db.h"
#include "../engine/variant.h"

//Include bench.h
#include "bench.h"

//_write_test now takes a struct as arguments, in order to able to call it with pthread_create
void _write_test(void *arg)
{
        //Struct initialation 
        struct _data *d = (struct _data *) arg;
	
	int i;
	Variant sk, sv;

	char key[KSIZE + 1];
	char val[VSIZE + 1];
	char sbuf[1024];
	
	memset(key, 0, KSIZE + 1);
	memset(val, 0, VSIZE + 1);
	memset(sbuf, 0, 1024);

	for (i = 0; i < d->count; i++) {
		if (d->r)
			_random_key(key, KSIZE);
		else
			snprintf(key, KSIZE, "key-%d", i);
		fprintf(stderr, "%d adding %s\n", i, key);
		snprintf(val, VSIZE, "val-%d", i);

		sk.length = KSIZE;
		sk.mem = key;

	   
		sv.length = VSIZE;
		sv.mem = val;

		
		db_add(d->db, &sk, &sv);
		if ((i % 10000) == 0) {
			fprintf(stderr,"random write finished %d ops%30s\r", 
					i, 
					"");
			fflush(stderr);
		}
	}
}

//_read_test now takes a struct as arguments, in order to able to call it with pthread_create
void _read_test(void *arg)
{
        //Struct initialation 
        struct _data *d = (struct _data *) arg;
  
	int i;
	int ret;
	int found = 0;
	Variant sk;
	Variant sv;
	char key[KSIZE + 1];

	for (i = 0; i < d->count; i++) {
		memset(key, 0, KSIZE + 1);

		/* if you want to test random write, use the following */
		if (d->r)
			_random_key(key, KSIZE);
		else
			snprintf(key, KSIZE, "key-%d", i);
		fprintf(stderr, "%d searching %s\n", i, key);
		sk.length = KSIZE;
		sk.mem = key;
		ret = db_get(d->db, &sk, &sv);
		if (ret) {
		  //db_free_data(sv.mem);
			found++;
		} else {
		  INFO("not found key#%s",	sk.mem);
    	}

		if ((i % 10000) == 0) {
			fprintf(stderr,"random read finished %d ops%30s\r", 
					i, 
					"");

			fflush(stderr);
		}
	}
}

