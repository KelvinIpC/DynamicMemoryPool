#include <iostream>

#include "MemoryPool.h"
using namespace std;

int main(void)
{
	cout << "Test" << endl;
	MemoryPool mPool(128);
	MemoryBlock mb = mPool.MemAlloc(20);
	cout << mb.Size() << endl;
	return 0;
}
