
#include "hopscotch.hpp"

Hopscotch::Hopscotch() {
	int size = MAX_SEGMENTS+256;
	segments_arys = new Bucket[size];
	BUSY = (int*)malloc(sizeof(int));
	*BUSY = -1;
}

Hopscotch::~Hopscotch() {
	delete [] segments_arys;
	free(BUSY);
}

int* Hopscotch::remove(int *key) {
	unsigned int hash = ((*key)&(MAX_SEGMENTS-1));
	Bucket* start_bucket = segments_arys+hash;
	start_bucket->lock();

	unsigned int hop_info = start_bucket->_hop_info;
	unsigned int mask = 1;
	for(int i = 0; i < HOP_RANGE; ++i, mask <<= 1) {
		if(mask & hop_info) {
			Bucket* check_bucket = start_bucket+i;
			if (check_bucket->_key == NULL) {
				start_bucket->unlock();
				return NULL;
			}
			if(*key == *(check_bucket->_key)) {
				int* rc = check_bucket->_data;
				check_bucket->_key = NULL;
				check_bucket->_data = NULL;
				start_bucket->_hop_info &= ~(1<<i);
				start_bucket->unlock();
				return rc;
			}
		}
	}
	start_bucket->unlock();
	return NULL;
} 

void Hopscotch::find_closer_bucket(Bucket** free_bucket, int* free_distance, int &val) {
	Bucket* move_bucket = *free_bucket -(HOP_RANGE-1);
	for(int free_dist = (HOP_RANGE -1);free_dist>0;--free_dist) {
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
			move_bucket->lock();
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
				move_bucket->unlock();
				return;
			}
			move_bucket->unlock();
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
	Bucket* start_bucket = segments_arys + hash;
	start_bucket->lock();
	if(contains(key)) {
		start_bucket->unlock();
		return false;
	}

	Bucket* free_bucket = start_bucket;
	int free_distance = 0;
	for(; free_distance < ADD_RANGE; ++free_distance) {
		if(NULL == free_bucket->_key) {
			free_bucket->_key = BUSY;
			break;
		}
		++free_bucket;
	}

	if(free_distance < ADD_RANGE) {
		do{
			if(free_distance < HOP_RANGE) {
				start_bucket->_hop_info |= (1<<free_distance);
				free_bucket->_data = data;
				free_bucket->_key = key;
				start_bucket->unlock();
				return true;
			} else {
				find_closer_bucket(&free_bucket, &free_distance, val);
			}
		}while(0 != val);
	}
	start_bucket->unlock();
	return false;
}