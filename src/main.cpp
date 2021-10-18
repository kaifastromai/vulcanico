
#include <iostream>
#include "Skie.h"
#include <vulkan/vulkan.hpp>
using namespace std;
int main()
{
	vk::Extent2D f;
	try {

	auto sk = sk::Skie();
	sk.run();



		
	}catch(std::exception &e)
	{
		cerr << e.what() << endl;
		
	}



	
}