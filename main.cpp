
//#include "vulkanhp.h"
#include "vulcanico.hpp"
#include <iostream>
using namespace std;
int main()
{
	try {
	/*	csl::vulkan my_vk = csl::vulkan();
		cout << "Hello world" << endl;
		my_vk.run();*/
		HelloTriangleApplication app;
		app.run();
		
	}catch(std::exception &e)
	{
		cerr << e.what() << endl;
	}


	
}