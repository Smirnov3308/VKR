
////////////////////////////////////////////////////////////////////////////////
// Concurrent Hopscotch Hash Map
//
////////////////////////////////////////////////////////////////////////////////
//TERMS OF USAGE
//----------------------------------------------------------------------
//
//  Permission to use, copy, modify and distribute this software and
//  its documentation for any purpose is hereby granted without fee,
//  provided that due acknowledgements to the authors are provided and
//  this permission notice appears in all copies of the software.
//  The software is provided "as is". There is no warranty of any kind.
//
//Programmers:
//  Hila Goel
//  Tel-Aviv University
//  and
//  Maya Gershovitz
//  Tel-Aviv University
//  
//
//  Date: January, 2015.
////////////////////////////////////////////////////////////////////////////////
//
// This code was developed as part of "Workshop on Multicore Algorithms" 
// at Tel-Aviv university, under the guidance of Prof. Nir Shavit and Moshe Sulamy.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef HOPSCOTCH_HPP_
#define HOPSCOTCH_HPP_

using namespace std;

#include <iostream>
#include <pthread.h>
#include <malloc.h>

// This is one of eight implementations we created for the workshop.
// In this implementation we used a global lock method - 
// in every action that changes the data, the entire table is locked until completion.

class Hopscotch {
private:
	static const int HOP_RANGE = 32;
	static const int ADD_RANGE = 256;
	static const int MAX_SEGMENTS = 4194304; // Including neighbourhood for last hash location
	static const int MAX_TRIES = 2;
	int* BUSY;
	
	/*Bucket is the table object.
	Each bucket contains a key and data pairing (as in a usual hashmap),
	and an "hop_info" variable, containing the information 
	regarding all the keys that were initially mapped to the same bucket.*/
	struct Bucket {

		unsigned int _hop_info;
		int* _key;
		int* _data;

		Bucket() {
			_hop_info = 0;
			_key = NULL;
			_data = NULL;
		}
	};

	/*A pointer to the table*/
	Bucket* segments_arys;

public:
	Hopscotch();
	~Hopscotch();
	/*inline bool contains(int *key)
	Key - the key we'd like to search for in the table
	Returns true if the table contains the key, and false otherwise*/
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
	
	/*inline void trial()
	This is a method used for debugging purposes*/
	inline void trial() {
		Bucket* temp;
		int count = 0;
		for(int i = 0; i < MAX_SEGMENTS+256; i++) {
			temp = segments_arys + i;
			if(temp->_key != NULL) {
				count++;
			}
		}
		cout<< "Items in Hash = " << count << endl;
		cout<<"--------------------"<<endl;
	}
	
	bool add(int *key, int *data);
	int* remove(int* key);
	void find_closer_bucket(Bucket**,int*,int &);

};

#endif /* HOPSCOTCH_HPP_ */
