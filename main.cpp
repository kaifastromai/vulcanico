
#include "vulkanhp.h"
#include <iostream>
using namespace std;
int main()
{
	try {
		csl::vulkan my_vk = csl::vulkan();
		cout << "Hello world" << endl;
	}catch(std::exception &e)
	{
		cerr << e.what() << endl;
	}


	
}