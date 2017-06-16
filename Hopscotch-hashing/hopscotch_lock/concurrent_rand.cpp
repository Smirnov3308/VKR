
#include "hopscotch.hpp"
#include <stdlib.h> 
#include <sys/time.h>
using namespace std;

Hopscotch *obj;

typedef struct argsS {
	int* keys;
	int start, end;

	argsS(int* keys, int start, int end) {
		this->keys = keys;
		this->start = start;
		this->end = end;
	}
} args;


void *run(void* x) {
	int* keys = ((args*)x)->keys;
	int start = ((args*)x)->start;
	int end = ((args*)x)->end;
	delete (args*)x;
	int datas[1] = {1};
	int op;
    unsigned int seed = (unsigned int)(time(NULL));
	
	for(int i = start; i < end; i++){
		op = rand_r(&seed) % 100;
        if (op < 40 ) {
            obj->contains(&keys[i]);
        } else if (op > 70) {
            obj->add(&keys[i], datas);
        } else {
            obj->remove(&keys[i]);
        }
	}
	pthread_exit(NULL);
}


long runTest(const int numOfThreads) {

	int numberOfElem = 4000000;
	int* keys = new int[numberOfElem];
	int* start_keys = new int[2000000];
	struct timeval start, end;
    long mtime, seconds, useconds;
	int datas[1] = {1};
	srand(2661656);
	for(int i = 0; i < numberOfElem; i++) {
		keys[i] = rand();
	}
	obj = new Hopscotch();

	for(int i = 0; i < 2000000; i++) {
		start_keys[i] = rand();
		obj->add(&start_keys[i], datas);
	}

	
	gettimeofday(&start, NULL);

	pthread_t* threads = new pthread_t[numOfThreads];
	int rc;
	
	for(int t = 0; t < numOfThreads; t++) {
		args* arg_s = new argsS(keys, t*(numberOfElem/numOfThreads),  (t+1)*(numberOfElem/numOfThreads));
		rc = pthread_create(&threads[t], NULL, run, (void *)(arg_s));
		if(rc) {
			cout << "ERROR; return code from pthread_create() is " << rc << endl;
		}
	}

	for(int t = 0; t < numOfThreads; t++) {
		pthread_join(threads[t], NULL);
	}

	gettimeofday(&end, NULL);
	seconds = end.tv_sec - start.tv_sec;
	useconds = end.tv_usec - start.tv_usec;
	mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;
	delete obj;
	delete[] threads;
	return mtime;
}


int main(int argc, char** argv) {
	int numOfThreads = strtol(argv[1], NULL, 10);

	int numberOfTests = 1;
	long* results = new long[numberOfTests];

	for(int j = 0; j < numberOfTests; j++) {
		long totalTime = runTest(numOfThreads);
		results[j] = totalTime;
	}

	long avg = 0;
	for(int j = 0; j < numberOfTests; j++) {
		avg += results[j]/numberOfTests;
	}
	
	cout << avg << endl;
	delete[] results;
	return 0;
}

