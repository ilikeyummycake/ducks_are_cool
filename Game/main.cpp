//Q to fire lasers
//W to use sonic quack
//Collect ducks and kill frogs

#include "Application.h"

#include <stdexcept>
#include <iostream>


int main()
{
	try
	{
		Application app;
		app.run();
	}
	catch (std::exception& e)
	{
		std::cout << "\nEXCEPTION: " << e.what() << std::endl;
	}
}
