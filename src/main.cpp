#include "globals.h"
#include <iostream>
#include "Skie.h"
#include "sk_text.h"
#include <vulkan/vulkan.hpp>
using namespace std;
int main()
{
	try {
		sk::text::test();
	}catch(std::exception &e)
	{
		cerr << e.what() << endl;
		
	}



	
}