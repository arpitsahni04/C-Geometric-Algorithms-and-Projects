
#pragma warning(disable:4996)
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

#include "fssimplewindow.h"
#include "yssimplesound.h"
#include "ysglfontdata.h"
#include "Maze.h"
#include "Entity.h"
#include "ButtonCollection.h"
#include "GraphicFont.h"
#include "StringPlus.h"

using namespace std;

void loadFile(Maze& theMaze);
void saveFile(Maze& theMaze);
void showMenu();
void createNewMaze(Maze& theMaze, Entity& theEntity);
void addButtons(ButtonCollection& someButtons, GraphicFont* aFont, int xLoc, int wid);

int main(void)
{
	Maze* theMaze = new Maze;  // moving storage from function stack to heap
	Entity* theEntity = new Entity; // moving storage to heap
	bool soundIsOn = true;
	bool pathIsFound = false;
	bool showVisited = false;
	char userChoice;
	int newRowSize, newColSize;
	int key = FSKEY_NULL;
	int mouseEvent, leftButton, middleButton, rightButton, locX, locY;

	FsOpenWindow(16, 16, (Maze::MAX_MAP_SIZE - 1) * Maze::BLOCK_SIZE + 130,
		(Maze::MAX_MAP_SIZE - 1) * Maze::BLOCK_SIZE, 1,
		"Maze Game Better(NG 2022)");

	// buttons
	int buttonKey = FSKEY_NULL;
	ButtonCollection* myButtons = new ButtonCollection; // put this AFTER FsOpenWindow()
	GraphicFont* buttonFont = new ComicSansFont;
	buttonFont->setColorRGB(0, 0, 0); // black

	addButtons(*myButtons, buttonFont, (Maze::MAX_MAP_SIZE - 1) * Maze::BLOCK_SIZE + 5, 100);

	// audio feedback on allowed/disallowed moves
	YsSoundPlayer theSoundPlayer;
	YsSoundPlayer::SoundData yesMoveSound;
	YsSoundPlayer::SoundData cantMoveSound;
	if (YSOK != yesMoveSound.LoadWav("click_x.wav"))
		cout << "Can't find yesMoveSound data" << endl;
	if (YSOK != cantMoveSound.LoadWav("gulp_x.wav"))
		cout << "Can't find cantMoveSound data" << endl;
	theSoundPlayer.Start();

	showMenu();
	theEntity->setMaze(*theMaze); // Task 4. Call this function one time (note addition of *)
	theEntity->reset();

	FsPollDevice();
	while (FSKEY_ESC != key)
	{
		FsPollDevice();
		key = FsInkey();
		mouseEvent = FsGetMouseEvent(leftButton, middleButton, rightButton, locX, locY);
		if (mouseEvent == FSMOUSEEVENT_LBUTTONDOWN || leftButton) // also do it if left button is held down
			theMaze->changeMapState(locX, locY, mouseEvent);  // Task 5

		// check if a button was clicked
		if (key == FSKEY_NULL && mouseEvent == FSMOUSEEVENT_LBUTTONDOWN) {
			buttonKey = myButtons->checkClick(locX, locY);

			if (buttonKey != FSKEY_NULL)
				key = buttonKey;  // pretend the user pressed a key 
		}

		// take care of window resizing
		int wid, hei;
		FsGetWindowSize(wid, hei);
		glViewport(0, 0, wid, hei);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0, (float)wid - 1, (float)hei - 1, 0, -1, 1);

		// draw all the stuff
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

		theMaze->paint(theEntity->reachedGoal());

		if (showVisited)
			theEntity->paintSearchedCells();
		theEntity->paintMotionTrack();
		theEntity->paint();

		if (theEntity->reachedGoal()) {
			theEntity->disableTorch(); // show everything
			theEntity->celebrateGoal();
		}

		myButtons->paint();
		myButtons->checkHover(locX, locY); // remove hover feedback for better performance ?

		FsSwapBuffers();

		// take care of user interactions
		switch (key) {
		case FSKEY_UP:
		case FSKEY_DOWN:
		case FSKEY_LEFT:
		case FSKEY_RIGHT:
			if (theEntity->move(key)) {
				if (soundIsOn) theSoundPlayer.PlayOneShot(yesMoveSound);
			}
			else
				if (soundIsOn) theSoundPlayer.PlayOneShot(cantMoveSound);
			break;
		case FSKEY_Q:
			soundIsOn = !soundIsOn;
			break;
		case FSKEY_L:
			loadFile(*theMaze); // note addition of asterisk
			theEntity->reset();
			showMenu();
			break;
		case FSKEY_R: theEntity->reset();
			break;
		case FSKEY_H:
			theEntity->toggleTorchShape();
			break;
		case FSKEY_PLUS:
			theEntity->increaseTorch();
			break;
		case FSKEY_MINUS:
			theEntity->decreaseTorch();
			break;
		case FSKEY_W:
			saveFile(*theMaze);  // similar to load file here and save file in PS04 (note addition of asterisk)
			showMenu();
			break;
		case FSKEY_T:
			theMaze->quarterTurn();
			if (FsGetKeyState(FSKEY_SHIFT)) {
				theMaze->quarterTurn();
				theMaze->quarterTurn();
			}
			theEntity->reset();
			break;
		case FSKEY_F:
			theMaze->mirrorLeftRight();
			theEntity->reset();
			break;
		case FSKEY_U:
			theMaze->mirrorUpDown();
			theEntity->reset();
			break;
		case FSKEY_M:
			theMaze->mirrorOnDiagonal();
			theEntity->reset();
			break;
		case FSKEY_C:
			createNewMaze(*theMaze, *theEntity);  // note asterisks
			break;
		case FSKEY_S:  // Task 5
		case FSKEY_E:  // Task 5
			theMaze->changeMapState(locX, locY, key);
			break;
		case FSKEY_G:  // Task 6
			if (pathIsFound)
				theEntity->clearShortestPath();
			else {
				theEntity->findShortestPath();
				showVisited = false;
			}
			pathIsFound = !pathIsFound;
			break;
		case FSKEY_V:  // A bit beyond Task 6
			showVisited = !showVisited;
			break;
		}
		FsSleep(20);
	}

	delete theMaze;
	delete theEntity;
	delete myButtons;
	delete buttonFont;
}

void loadFile(Maze& theMaze)
{
	bool continueAsking = true;
	string fileToOpen;

	while (continueAsking) {
		//cout << "Enter file name to load >> ";
		//cin >> fileToOpen;
		fileToOpen = DrawingUtilNG::getStringFromScreen("Enter file name of maze to load.",
			"Press ENTER when done, ESC to cancel.");

		if (fileToOpen.length() > 0) {
			// so I don't have to type so much during testing
			if (fileToOpen == "a")
				fileToOpen = "mazeA_10x10.map";
			else if (fileToOpen == "b")
				fileToOpen = "mazeB_25x25.map";
			else if (fileToOpen == "c")
				fileToOpen = "mazeC_40x20.map";

			ifstream inFile(fileToOpen);
			if (inFile.is_open()) {
				theMaze.readFile(inFile);
				continueAsking = false;
				inFile.close();
			}
			else {
				cout << "Could not open file. ";
			}
		}
		else // blank entry for filename so cancel load file
			continueAsking = false;
	}
}

void showMenu()
{
	cout << endl;
	cout << "Instructions: " << endl;
	cout << "    Arrows to move entity." << endl;
	cout << "    'R' to Reset entity to start" << endl;
	cout << "    'L' to Load a maze file" << endl;
	cout << "    'W' to write maze to file" << endl;
	cout << "    'H' to Hide/unHide map" << endl;
	cout << "       '+/-' to increase/decrease torch size" << endl;
	cout << "    'Q' to toggle sound Quiet/loud" << endl;
	cout << endl;
	cout << "  Maze manipulation" << endl;
	cout << "     'T' to Turn maze: 90 degrees CW, Shift+T for CCW" << endl;
	cout << "     'F' to Flip maze left-right" << endl;
	cout << "     'U' to turn maze Upsidedown" << endl;
	cout << "     'M' to mirror maze about its diagonal" << endl;
	cout << "     'C' to create a new maze" << endl;
	cout << endl;
	cout << "  Letter on a square to set start/end (if navigable)" << endl;
	cout << "     'S' for start" << endl;
	cout << "     'E' for end" << endl;
	cout << "     Click mouse on cell to toggle navigable/obstacle" << endl;
	cout << endl;
	cout << "  Path Finding (press letter again to hide)" << endl;
	cout << "    'G' use BFS to find shortest path from entity to end." << endl;
	//	cout << "    'A' use A* to find shortest path from entity to end." << endl;
	//	cout << "    'K' to set cost of traversing an obstacle." << endl;
	cout << "    'V' show Visited squares in path-finding" << endl;
	cout << endl;
}

void saveFile(Maze& theMaze)
{
	string fileToSave;

	//cout << "Enter file name to save >> ";
	//cin >> fileToSave;
	fileToSave = DrawingUtilNG::getStringFromScreen("Enter name of file to save.",
		"Press ENTER when done, ESC to cancel.");

	if (fileToSave.length() > 0) {
		ofstream outFile(fileToSave);
		if (outFile.is_open()) {
			outFile << theMaze;
			outFile.close();
		}
		else {
			cout << "Could not save file. ";
		}
	}
}

void createNewMaze(Maze& theMaze, Entity& theEntity)
{
	int newRowSize, newColSize;

	// on the console
	//char userChoice;
	//cout << "Are you sure you want to clear whole maze (Y/N) >> ";
	//cin >> userChoice;
	//if (userChoice == 'Y' || userChoice == 'y') {
	//	cout << "    Please enter number of rows and number of columns (space separated) >> ";
	//	cin >> newRowSize >> newColSize;
	//	theMaze.clear(newRowSize, newColSize);
	//	theEntity.reset();
	//}
	//showMenu(); // So that it is "fresh"

	// on the screen
	string inputString = DrawingUtilNG::getStringFromScreen(
		"If you are sure you want to erase current maze and create ",
		"a new maze, enter new row size and column size (max 40),", 
		"separated by space. Press Esc to Cancel");
	inputString = StringPlus::trim(inputString);
	if (inputString.length() > 1) {
		stringstream inputStream;
		inputStream.str(inputString);
		inputStream >> newRowSize >> newColSize;
		theMaze.clear(newRowSize, newColSize);
		theEntity.reset();
	}

}

void addButtons(ButtonCollection& someButtons, GraphicFont* aFont, int xLoc, int wid)
{
	int hei = 30;
	int spacing = 10;

	int currY = 10;
	someButtons.add(xLoc, currY, wid, hei, FSKEY_L, "Load", aFont,
		"Load a maze from a file");

	currY += hei + spacing;
	someButtons.add(xLoc, currY, wid, hei, FSKEY_W, "Write", aFont,
		"Save current maze to a file");

	currY += hei + spacing;
	someButtons.add(xLoc, currY, wid, hei, FSKEY_H, "Hide", aFont,
		"Hides map except for area of torch. Use +/- to control size of torch. Click again to change shape");

	currY += hei + spacing;
	someButtons.add(xLoc, currY, wid, hei, FSKEY_Q, "Sound", aFont,
		"Toggle sound feedback on/off");

	currY += hei + spacing;
	someButtons.add(xLoc, currY, wid, hei, FSKEY_R, "Reset", aFont,
		"Clear path travelled and put entity back at start");

	currY += hei + spacing * 3;
	someButtons.add(xLoc, currY, wid, hei, FSKEY_T, "Rotate", aFont,
		"Rotate maze 90deg clockwise");

	currY += hei + spacing;
	someButtons.add(xLoc, currY, wid, hei, FSKEY_F, "Flip L/R", aFont,
		"Mirror maze left/right (about vert centerline)");

	currY += hei + spacing;
	someButtons.add(xLoc, currY, wid, hei, FSKEY_U, "Flip U/D", aFont,
		"Mirror maze up/down (about horz centerline)");

	currY += hei + spacing;
	someButtons.add(xLoc, currY, wid, hei, FSKEY_M, "Mirror", aFont,
		"Mirror maze about diagonal line (both L/R and U/D)");

	currY += hei + spacing;
	someButtons.add(xLoc, currY, wid, hei, FSKEY_C, "Create", aFont,
		"Erase current maze and start an all-obstacle maze of given size");

	currY += hei + spacing * 3;
	someButtons.add(xLoc, currY, wid, hei, FSKEY_G, "Path BFS", aFont,
		"Find shortest path using breadth-first search. Click again to hide.");

	currY += hei + spacing;
	someButtons.add(xLoc, currY, wid, hei, FSKEY_A, "Path A*", aFont,
		"Find shortest path using A* heuristic search. Click again to hide");

	currY += hei + spacing;
	someButtons.add(xLoc, currY, wid, hei, FSKEY_V, "Searched", aFont,
		"Show squares that were visited during search. Click again to hide");


	// to disable a button (will gray out and won't return its value)
	someButtons.disableButton(FSKEY_A);
}

