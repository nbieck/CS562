////////////////////////////////////////////////////////////////////////////////////////////
//
//	Author: Niklas Bieck
//  Subject: CS350
//
////////////////////////////////////////////////////////////////////////////////////////////

#include "Application\Application.h"

int main(int argc, char **argv)
{
	try
	{
		CS350::Application app;

		app.Run();

		return 0;
	}
	catch (...)
	{
		return 1;
	}
}