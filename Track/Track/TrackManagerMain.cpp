   


#include <iostream>
#include <fstream>
#include "fssimplewindow.h"
#include "TrackManager.h"

using namespace std;

int main()
{
	// set up track manager
	TrackManager theManager(900, 600);

	theManager.showMenu();

	while (theManager.manage()) {
		// actually display graphics
		FsSwapBuffers();

		// prepare for next loop
		FsSleep(25);
	}

	return 0;
}

