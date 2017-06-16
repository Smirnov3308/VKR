#ifndef HOPSCOTCH_HPP_
#define HOPSCOTCH_HPP_

using namespace std;

#include<iostream>
#include<pthread.h>
#include<malloc.h>

class Hopscotch {
private:
	static const int HOP_RANGE = 32;
	static const int ADD_RANGE = 256;
	static const int MAX_SEGMENTS = 4194304;
	static const int MAX_TRIES = 2;
	int* BUSY;
	
	bool volatile _lock;
	pthread_mutex_t lock_mutex;
	pthread_cond_t lock_cv;


	void lock() {
		pthread_mutex_lock(&lock_mutex);
		while(1) {
			if(_lock == false) {
				_lock = true;
				pthread_mutex_unlock(&lock_mutex);
				break;
			}
			pthread_cond_wait(&lock_cv, &lock_mutex);
		}
	}


	void unlock() {
		pthread_mutex_lock(&lock_mutex);
		_lock = false;
		pthread_cond_signal(&lock_cv);
		pthread_mutex_unlock(&lock_mutex);
	}
	struct Bucket {

		unsigned int volatile _hop_info;
		int* volatile _key;
		int* volatile _data;

		Bucket() {
			_hop_info = 0;
			_key = NULL;
			_data = NULL;
		}
	};

	Bucket* segments_arys;

public:
	Hopscotch();
	~Hopscotch();
	inline bool contains(int *key) {
		unsigned int hash = ((*key) & (MAX_SEGMENTS-1));
		Bucket* start_bucket = segments_arys+hash;
		unsigned int hop_info = start_bucket->_hop_info;
		unsigned int mask = 1;
		
		for(int i = 0; i < HOP_RANGE; ++i, mask <<= 1) {
			if(mask & hop_info) {
				Bucket* check_bucket = start_bucket+i;
				if(*key == *(check_bucket->_key)) {
					return true;
				}
			}
		}
		return false;
	}
	
	bool add(int *key, int *data);
	int* remove(int* key);
	void find_closer_bucket(Bucket**,int*,int &);

};

#endif /* HOPSCOTCH_HPP_ */
