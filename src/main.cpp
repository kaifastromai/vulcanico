
#include <iostream>
#include "Skie.h"
using namespace std;
int main()
{
	try {

	auto sk = sk::Skie();
	sk.run();
		
	}catch(std::exception &e)
	{
		cerr << e.what() << endl;
		
	}


	
}