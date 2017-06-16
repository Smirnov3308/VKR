

#include "hopscotch.hpp"


Hopscotch::Hopscotch() {
	segments_arys = new Bucket[MAX_SEGMENTS+256];
	BUSY=(int *)malloc(sizeof(int));
	*BUSY = -1;
	_lock = false;
	pthread_mutex_init(&lock_mutex,NULL);
	pthread_cond_init(&lock_cv, NULL);
}


Hopscotch::~Hopscotch() {
	delete [] segments_arys;
	free(BUSY);
	pthread_mutex_destroy(&lock_mutex);
	pthread_cond_destroy(&lock_cv);
}


int* Hopscotch::remove(int *key) {
	unsigned int hash = ((*key) & (MAX_SEGMENTS-1));
	Bucket* start_bucket = segments_arys+hash;
	this->lock();

	unsigned int hop_info = start_bucket->_hop_info;
	unsigned int mask = 1;
	for(int i = 0; i < HOP_RANGE; ++i, mask <<= 1) {
		if(mask & hop_info) {
			Bucket* check_bucket = start_bucket+i;
			if(*key == *(check_bucket->_key)) {
				int* rc = check_bucket->_data;
				check_bucket->_key = NULL;
				check_bucket->_data = NULL;
				start_bucket->_hop_info &= ~(1<<i);
				this->unlock();
				return rc;
			}
		}
	}
	this->unlock();
	return NULL;
}


void Hopscotch::find_closer_bucket(Bucket** free_bucket, int* free_distance, int& val) {
	Bucket* move_bucket = *free_bucket -(HOP_RANGE-1);
	int max_free_dist = (HOP_RANGE - 1) > *free_distance ? *free_distance : (HOP_RANGE - 1);
		
	for(int free_dist = max_free_dist; free_dist > 0; --free_dist) {
		unsigned int start_hop_info = move_bucket->_hop_info;
		int move_free_distance = -1;
		unsigned int mask = 1;
		for(int i = 0; i < free_dist; ++i, mask <<= 1) {
			if(mask & start_hop_info) {
				move_free_distance = i;
				break;
			}		
		}
		if(-1 != move_free_distance) {
			if(start_hop_info == move_bucket->_hop_info) {
				Bucket* new_free_bucket = move_bucket + move_free_distance;
				move_bucket->_hop_info |= (1 << free_dist);
				(*free_bucket)->_data = new_free_bucket->_data;
				(*free_bucket)->_key = new_free_bucket->_key;
				new_free_bucket->_key = BUSY;
				new_free_bucket->_data = BUSY;
				move_bucket->_hop_info &= ~(1<<move_free_distance);
				*free_bucket = new_free_bucket;
				*free_distance = *free_distance - free_dist + move_free_distance;
				return;
			}
		}
		++move_bucket;
	}
	(*free_bucket)->_key = NULL;
	val = 0;
	*free_distance = 0;
}


bool Hopscotch::add(int *key,int *data) {
	int val = 1;
	unsigned int hash = ((*key) & (MAX_SEGMENTS-1));
	Bucket* start_bucket = segments_arys+hash;
	this->lock();
	if(contains(key)) {
		this->unlock();
		return false;
	}
	int free_bucket_index = hash;
	Bucket* free_bucket = start_bucket;
	int free_distance = 0;
	for(; free_distance < ADD_RANGE; ++free_distance) {
		if(NULL == free_bucket->_key) {
			free_bucket->_key = BUSY;
			break;
		}
		free_bucket_index++;
		++free_bucket;
	}
	if(free_distance < ADD_RANGE) {
		do {
			if(free_distance < HOP_RANGE) {
				start_bucket->_hop_info |= (1<<free_distance);
				free_bucket->_data = data;
				free_bucket->_key = key;
				this->unlock();
				return true;
			} else {
				find_closer_bucket(&free_bucket, &free_distance, val);
			}
		}while(0 != val);
	}
	this->unlock();
	return false;
}


