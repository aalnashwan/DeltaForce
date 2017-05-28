#ifndef DELTA_H
#define DELTA_H

#include <iostream>
#include <vector>
#include <list>
#include <string>
using namespace std;

const int BASE = 7;
const int N = 8;

//hash table that stores all the consecutive N-character sequences from a string
struct HashTable
{
	int n; //length of the character sequences stored in this hash table

    //vector of buckets, each bucket can contain a list of several entries
	//an entry is an index into the string from which this hash table was created on which the given sequence starts
	vector<vector<int>> table; 

};

//computes a hash value for a given consecutive N-character sequence of the given string, using rolling hash
//text: the string from which the N-character sequence is taken
//start: the index of the first character of the N-character sequence
//n: the length of the N-character sequence
//hashSize: the number of buckets in the hash table
int hashFunction(const string& text, int start, int n, int hashSize);

//creates a hash table from all the consecutive N-character sequences from the given text string 
//text: the text string from which a hash table is created
//n: the length of the character sequences that are saved into the hash table
HashTable createHash(const string& text, int n);

//This function takes the contents of two files, A and A', 
//and produces a delta containing instructions for converting A into A'.
//oldf: an already-opened input source for the A file
//newf: an already-opened input source for the A' file
//deltaf: an already-opened output destination to which the delta is written
void createDelta(istream& oldf, istream& newf, ostream& deltaf);

//This function takes the content of a file A and a delta file, 
//and will apply the instructions in the delta file to produce a new file A'.
//oldf: an already-opened input source for the A file
//deltaf: an already-opened input source for the delta file
//newf: an already-opened output destination to which the A' file is written
//The function returns true if the operation succeeds. 
//If it fails because the delta file is malformed, the function returns false. 
//If the function returns false, the caller can make no assumption about what may or may not have been written to the output destination.
bool applyDelta(istream& oldf, istream& deltaf, ostream& newf);

#endif
