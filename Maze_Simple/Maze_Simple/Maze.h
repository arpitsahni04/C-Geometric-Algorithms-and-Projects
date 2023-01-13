/*
  ===============================
 || XXX   XXXX        XX    XXXX||
 ||   X X XXXX XXXX X    XX XXXX||  
 ||XX   X XXX    XX XXXX   XXXXX||  
 ||XX XXXX XXXX XXX XXXX      XX||  
 ||XX XXXX X    XXX XXXX XXXX XX||
 ||XX XXXX X XXXXXX XXXX X      ||
 ||XX XXXXXX XXXXXX XXXX XXXXXX ||
 || X  XXXXX       XXXXX XXXXXX ||  Maze navigation using keyboard to
 || XX X     XX XX  XXXX X    X ||  create simple interactive game.
 || XX X XXX XX XXX XXX XXeXX X ||  With maze editing.
 || XX X   X XX X X XXX XXXXX XX||
 ||    XXX X XX X X XXX       XX||
 ||XXX     X    X   XXXXXXXXXXXX||
  ===============================

*/
#pragma once
#include <fstream>
//#include "Entity.h"  // not needed

// trying to avoid global, non-context constants
//#define MAX_MAP_SIZE  41
//#define BLOCK_SIZE 20

struct Cell {
	int row, col;
};

class Maze {
public:
	static const int BLOCK_SIZE = 20; // static means that I could access without an object
	static const int MAX_MAP_SIZE = 41;  // also, there is only one copy for whole class

private:
	int startRow = -1, startCol = -1;
	int endRow = -1, endCol = -1;
	int rowSize = -1, colSize = -1;

	// true (1) means obstacle, false (0) means navigable 
	// ignore zero row and zero column (can use indices 1 to 40)
	// initialize to all obstacles
	//bool map[MAX_MAP_SIZE][MAX_MAP_SIZE] = { true }; 
	bool map[100][100] = { true };

	// Task 5 (to allow changing just by dragging
	int prevChangedRow = -1, prevChangedCol = -1;
public:

	Maze() { };

	// reads a file from permanent storage
	void readFile(std::ifstream& input);

	// draws whole maze on screen, including start and end locations
	// the fancy background option is used in case we want to "celebrate"
	void paint(bool useFancyBackground = false);

	// outpus maze data to cout or to permanent storage
	friend std::ostream& operator<<(std::ostream& os, const Maze& aMaze);

	// some get and sets
	int getRowSize() { return rowSize; }
	int getColSize() { return colSize; }
	Cell getStart() { return { startRow, startCol }; }
	Cell getEnd() { return { endRow, endCol }; }

	// checks both obstacles and out-of-bounds
	bool isNavigable(int row, int col);

	// Nestor's cute ideas for maze editing

	// turns the maze 90 degrees clockwise (the actual data, not just the view)
	void quarterTurn();

	// mirrors maze
	void mirrorOnDiagonal();

	// mirrors maze
	void mirrorLeftRight();

	// mirrors maze
	void mirrorUpDown();

	// returns number of navigable cells in maze
	int sumOfMap();

	// Task 5
	// mouseX and mouseY are the pixel coordinates of a left click
	// key is either FSMOUSEEVENT_LBUTTONDOWN, FSKEY_S, or FSKEY_E
	// mouse click toggles obstacle state of cell/block below mouse location
	// unless block is start or end (start and end MUST be navigable
	// if block is navigable, S and E will set it to either start or end
	// note, start or end may become undefined (-1, -1)
	void changeMapState(int mouseX, int mouseY, int key);

	// A bit beyond Task 5
	void clear(int newRowSize, int newColSize);
	
	// used by Nestor to create the init() hardwired functions
	//void outputHardwire();

private:

	// get rid of these once readFile() is working
	//void init10x10();
	//void init25x25();
	//void init40x20();
};