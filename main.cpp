
#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
#include "delta.h"
using namespace std;

bool runtest(string oldname, string newname, string deltaname, string newname2)
{
	ifstream oldfile(oldname);
	if (!oldfile)
	{
		cerr << "Cannot open " << oldname << endl;
		return false;
	}
	ifstream newfile(newname);
	if (!newfile)
	{
		cerr << "Cannot open " << newname << endl;
		return false;
	}
	ofstream deltafile(deltaname);
	if (!deltafile)
	{
		cerr << "Cannot create " << deltaname << endl;
		return false;
	}
	createDelta(oldfile, newfile, deltafile);
	deltafile.close();

	oldfile.clear();   // clear the end of file condition
	oldfile.seekg(0);  // reset back to beginning of the file
	ifstream deltafile2(deltaname);
	if (!deltafile2)
	{
		cerr << "Cannot read the " << deltaname << " that was just created!" << endl;
		return false;
	}
	ofstream newfile2(newname2);
	if (!newfile2)
	{
		cerr << "Cannot create " << newname2 << endl;
		return false;
	}
	assert(applyDelta(oldfile, deltafile2, newfile2));
	cout << "You must compare " << newname << " and " << newname2 << endl;
	cout << "If they are not identical, you failed this test." << endl;
	return true;
}

int main()
{
	assert(runtest("smallmart1.txt", "smallmart2.txt", "mydeltafile.txt", "mynewfile2.txt"));
	//createHash("abcdefghijklmnopqrstuvwxyz123456789", 8);

	system("PAUSE");

	return 0;
}