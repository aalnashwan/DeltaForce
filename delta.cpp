#include "delta.h"
#include <math.h>

int hashFunction(const string& text, int start, int n, int hashSize)
{
	int hash = 0;

	//for all characters of the substring for which the hash is computed
	for (int i = start; i < start + n; i++)
	{
		//we construct the hash as follows:
		//hash("abc") = (97 * BASE^2) + (98 * BASE^1) + (99 * BASE^0) 
		//and then use the modulo so the hash isn't greater than the number of buckets of the hash table
		hash = ((hash * BASE) + text[i]) % hashSize;
	}

	return hash;
}

HashTable createHash(const string& text, int n)
{
	HashTable hashTable;
	hashTable.n = n;

	//number of N-character sequences in the text
	int numSeq = text.size() - n + 1;

	//create the hash table with size 1.5 * expected number of entries
	hashTable.table = vector< vector<int> >(int(numSeq * 1.5), vector<int>());

	//compute the power that we'll be subtracting from the hash in the rolling hash algorithm
	//it is BASE^N
	int power = 1;
	for (int i = 0; i < n; i++)
		power = (power * BASE) % hashTable.table.size();

	int hash = 0; //here we will be computing the hash value using the rolling hash algorithm

	//go through all the N-character sequences in the input text
	//the last N-character sequence begins at index text.size() - n
	for (unsigned int i = 0; i <= text.size() - n; i++)
	{
		//first sequence, compute the hash from the entire sequence
		if (i == 0)
		{
			hash = hashFunction(text, 0, n, hashTable.table.size());
		}
		//not a first sequence, compute the hash from the previous hash using the rolling hash algorithm
		else
		{
			//add a new character at the end and increase the power of the other characters
			hash = ((hash * BASE) + text[i + n - 1]) % hashTable.table.size();
			//subtract the first character
			hash -= power * text[i - 1] % hashTable.table.size();
			if (hash < 0) 
				hash += hashTable.table.size();
		}

		//insert the index where the sequence starts at the computed location in the hash table
		hashTable.table[hash].push_back(i);
	}

	return hashTable;
}

void createDelta(istream& oldf, istream& newf, ostream& deltaf)
{
	//read the entire contents of both files 
	std::string oldFile = string(std::istreambuf_iterator<char>(oldf), std::istreambuf_iterator<char>());
	std::string newFile = string(std::istreambuf_iterator<char>(newf), std::istreambuf_iterator<char>());

	//create a hash table from the old file
	HashTable hashTable = createHash(oldFile, N);

	//compute the power that we'll be subtracting from the hash in the rolling hash algorithm
	//it is BASE^N
	int power = 1;
	for (int i = 0; i < N; i++)
		power = (power * BASE) % hashTable.table.size();

	int hash = 0; //here we will be computing the hash value using the rolling hash algorithm

	//here we will accumulate characters for the add instruction 
	//so that they can be added all at once later
	string add; 
	//indicates if we skipped some characters in the old file because of a copy instruction
	//so that the hash needs to be computed again from the whole new sequence
	bool skip = false;

	//go through all the N-character sequences in the new file
	for (int newIndex = 0; newIndex < newFile.size(); newIndex++)
	{
		//the number of remaining characters in the new file is smaller than the sequence size
		//we don't compute hash, just make an add instruction for them
		//if there are some previous accumulated characters for an add instruction, add them too
		if (newIndex > newFile.size() - N)
		{
			deltaf << "A" << add.size() + newFile.size() - newIndex << ":" << add << newFile.substr(newIndex, newFile.size() - newIndex);
			return; //this si the end of the new file, we are done
		}
		//first sequence or we skipped some characters because of a copy instruction, 
		//compute the hash from the entire sequence
		else if (newIndex == 0 || skip == true)
		{
			hash = hashFunction(newFile, newIndex, N, hashTable.table.size());
			skip = false; //set skip back to false
		}
		//compute the hash from the previous hash using the rolling hash algorithm
		else
		{
			//add a new character at the end and increase the power of the other characters
			hash = ((hash * BASE) + newFile[newIndex + N - 1]) % hashTable.table.size();
			//subtract the first character
			hash -= power * newFile[newIndex - 1] % hashTable.table.size();
			if (hash < 0) 
				hash += hashTable.table.size();
		}
			
		//we search for the biggest matching sequence in the old file
		int biggestLength = 0; 
		//the index of the first character of the biggest matching sequence in the old file
		int biggestOldIndex = 0; 

		//search all the entries in the given bucket in the hash table
		for (int i = 0; i < hashTable.table[hash].size(); i++)
		{
			//get the index of the first character of the sequence from the old file
			int oldIndex = hashTable.table[hash][i];
			//length of the entire matching sequence (if they indeeds match) is at leat N
			int length = N;

			//check if the N-character sequences match
			if (oldFile.substr(oldIndex, N) == newFile.substr(newIndex, N))
			{
				//find out how far the match goes beyond the first N characters
				while (oldIndex + length < oldFile.size() && 
					   newIndex + length < newFile.size() && 
					   oldFile[oldIndex + length] == newFile[newIndex + length])
				{
					length++;
				}

				//if we found the longest match yet, update the relevant variables
				if (length > biggestLength)
				{
					biggestLength = length;
					biggestOldIndex = oldIndex;
				}
			}
		}
		
		//we have found a matching sequence for copying
		if (biggestLength != 0)
		{
			//there are some previous accumulated characters for an add instruction
			//create the add instruction first and clear the add vector
			if (!add.empty())
			{
				deltaf << "A" << add.size() << ":" << add;
				add.clear();
			}

			//create the copy instruction and advance the index in the new file accordingly
			deltaf << "C" << biggestLength << "," << biggestOldIndex;
			newIndex = newIndex + biggestLength - 1;
			//we have skipped characters in the new file, need to compute the next hash from the entire new sequence
			skip = true; 
		}
		//we have not found a matching sequence for copying
		else
		{
			//if this is the last sequence in the new file, create add instruction for it
			//if there are some previous accumulated characters for an add instruction
			//put it all into one add instruction
			if (newIndex == newFile.size() - N)
			{
				deltaf << "A" << add.size() + N << ":" << add << newFile.substr(newIndex, N);
				return; //this si the end of the new file, we are done
			}
			//this is not the end, add the current character to the characters
			//for which we will create an add instruction later
			else
			{
				add += newFile[newIndex];
			}
		}	
	}
}

bool getInt(istream& inf, int& n)
{
	char ch;
	if (!inf.get(ch) || !isascii(ch) || !isdigit(ch))
		return false;
	inf.unget();
	inf >> n;
	return true;
}

bool getCommand(istream& inf, char& cmd, int& length, int& offset)
{
	if (!inf.get(cmd) || (cmd == '\n'  &&  !inf.get(cmd)))
	{
		cmd = 'x';  // signals end of file
		return true;
	}
	char ch;
	switch (cmd)
	{
	case 'A':
		return getInt(inf, length) && inf.get(ch) && ch == ':';
	case 'C':
		return getInt(inf, length) && inf.get(ch) && ch == ',' && getInt(inf, offset);
	}
	return false;
}

bool applyDelta(istream& oldf, istream& deltaf, ostream& newf)
{
	char cmd;  
	int length; 
	int offset;
	string str;
	

	//while there is not error reading the delta file and we are not at the end of file
	while (getCommand(deltaf, cmd, length, offset) && cmd != 'x')
	{
		switch (cmd)
		{
		case 'A':
			//add command
			//read the given number of characters from the delta file and put them into the new file
			str = string(length, ' ');
			deltaf.read(&str[0], length);
			newf << str;
			break;
		case 'C':
			//copy command
			oldf.seekg(0); //return to the beginning of the old file
			oldf.ignore(offset); //move tp the given offset
			//read the given number of characters from the old file and put them into the new file
			str = string(length, ' ');
			oldf.read(&str[0], length);
			newf << str;
			break;
		default:
			break;
		}
	}

	if (cmd == 'x') //entire delta file was successfully processed
		return true;
	else //there was an error
	    return false;
}