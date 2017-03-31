#include "Application.hpp"

void main(int argc, char *argv[]) {
	CApplication Application;
	if (!Application.bootStrap(argc, argv) || !Application.Run())
		Application.showUsage();
}