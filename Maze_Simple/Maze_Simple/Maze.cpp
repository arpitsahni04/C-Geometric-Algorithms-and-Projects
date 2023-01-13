#include <sstream>
#include <fstream>
#include <iostream>
#include "fssimplewindow.h"
#include "ysglfontdata.h"
#include "DrawingUtilNG.h"
#include "Maze.h"

using namespace std;

void Maze::readFile(std::ifstream& input)
{
	string wholeLineString; // used to read file one whole line at a time 
	stringstream lineStream; // used to easily get numbers from part of line
	int colonLocation; // used to store location of colon in line 

	while (!input.eof()) {
		lineStream.clear();// just in case 

		getline(input, wholeLineString); // read the whole line into a string

		// if there is a colon, set up lineStream to start just after colon 
		// also, remember colonLocation, just in case 
		if ((colonLocation = wholeLineString.find(":")) != string::npos)
			lineStream.str(wholeLineString.substr(colonLocation + 1));
		else {
			colonLocation = -1;
			lineStream.str(wholeLineString);
		}

		// check for keywords in the line
		if (wholeLineString.find("Start:") != string::npos) // find() returns npos when not found 
			lineStream >> startRow >> startCol; // put values into class variables 

		else if (wholeLineString.find("End:") != string::npos) // find() returns npos when not found 
			lineStream >> endRow >> endCol; // put values into class variables 

		else if (wholeLineString.find("Begin:") != string::npos) {
			lineStream >> colSize;
			bool continueReadingMap = true;
			rowSize = 0;
			while (continueReadingMap && !input.eof()) { // note extra protection for end-of-file
				getline(input, wholeLineString); // read the whole line into a string
				if (wholeLineString.find("End:") != string::npos)
					continueReadingMap = false;
				else {
					// convert string into a stringstream and read whole thing into the map
					lineStream.clear();// just in case 
					lineStream.str(wholeLineString);
					rowSize++;
					// read each 0 or 1 on the line directly into map array
					for (int j = 1; j <= colSize; j++)  // using j due to tradition of j as column index
						lineStream >> map[rowSize][j];

					// alternatively, we could read without knowing the column size, but it can
					// give bad results if there is extra stuff after the zeroes and ones
					//colSize = 0;
					//while (!lineStream.eof())
					//	lineStream >> map[rowSize][++colSize];

				}
			}
		}
	}
}

void Maze::paint(bool useFancyBackground)
{
	using namespace DrawingUtilNG;

	// gray background for whole screen
	glColor3ub(100, 100, 100);
	drawRectangle(0, 0, (MAX_MAP_SIZE - 1) * BLOCK_SIZE, (MAX_MAP_SIZE - 1) * BLOCK_SIZE, true);

	if (useFancyBackground) {
		// technicolor background for maze

		glShadeModel(GL_SMOOTH); // set up smooth shading
		glBegin(GL_QUADS);

		glColor3ub(240, 0, 240); // red
		glVertex2i(0, 0); // top left

		glColor3ub(0, 240, 240); // green
		glVertex2i(colSize * BLOCK_SIZE, 0); // top right

		glColor3ub(0, 0, 240); // blue
		glVertex2i(colSize * BLOCK_SIZE, rowSize * BLOCK_SIZE); // bot right

		glColor3ub(240, 240, 0); // yellow
		glVertex2i(0, rowSize * BLOCK_SIZE); // bot left

		glEnd();
		glShadeModel(GL_FLAT); // stop smooth shading

	}
	else {
		// black background for maze
		glColor3ub(0, 0, 0);
		drawRectangle(0, 0, colSize * BLOCK_SIZE, rowSize * BLOCK_SIZE, true);
	}

	// white boxes for navigable squares
	glColor3ub(240, 240, 240); // not quite white
	for (int i = 1; i <= rowSize; i++) {
		for (int j = 1; j <= colSize; j++) {
			if (!map[i][j]) {
				drawRectangle((j - 1) * BLOCK_SIZE, (i - 1) * BLOCK_SIZE,
					BLOCK_SIZE, BLOCK_SIZE, true);
			}
		}
	}

	// start and end blocks
	glColor3ub(0, 255, 0);  // green
	drawRectangle((startCol - 1) * BLOCK_SIZE, (startRow - 1) * BLOCK_SIZE,
		BLOCK_SIZE, BLOCK_SIZE, true);
	glColor3ub(255, 0, 0);  // red
	drawRectangle((endCol - 1) * BLOCK_SIZE, (endRow - 1) * BLOCK_SIZE,
		BLOCK_SIZE, BLOCK_SIZE, true);

}

void Maze::quarterTurn()
{
	// cannot do it "in place" without copying
	bool temp[MAX_MAP_SIZE][MAX_MAP_SIZE];
	for (int i = 1; i < MAX_MAP_SIZE; i++)
		for (int j = 1; j < MAX_MAP_SIZE; j++)
			temp[i][j] = map[i][j];

	swap(rowSize, colSize);
	for (int i = 1; i <= rowSize; i++)
		for (int j = 1; j <= colSize; j++) {
			map[i][j] = temp[colSize - j + 1][i];
		}

	if (startRow != -1 && endRow != -1) {
		int newStartCol = colSize - startRow + 1;
		int newEndCol = colSize - endRow + 1;
		startRow = startCol;
		startCol = newStartCol;
		endRow = endCol;
		endCol = newEndCol;
	}
}

void Maze::mirrorOnDiagonal()
{
	// can do it "in place" without copying
	for (int i = 1; i < MAX_MAP_SIZE; i++)
		for (int j = i; j < MAX_MAP_SIZE; j++) {
			swap(map[i][j], map[j][i]);
		}

	if (startRow != -1 && endRow != -1) {
		swap(rowSize, colSize);
		swap(startRow, startCol);
		swap(endRow, endCol);
	}
}

void Maze::mirrorLeftRight()
{
	// can do it "in place" without copying
	for (int i = 1; i <= rowSize; i++)
		for (int j = 1; j <= colSize / 2; j++) {
			swap(map[i][j], map[i][colSize - j + 1]);
		}

	if (startRow != -1 && endRow != -1) {
		startCol = colSize - startCol + 1;
		endCol = colSize - endCol + 1;
	}
}

void Maze::mirrorUpDown()
{
	// can do it "in place" without copying
	for (int i = 1; i <= rowSize / 2; i++)
		for (int j = 1; j <= colSize; j++) {
			swap(map[i][j], map[rowSize - i + 1][j]);
		}

	if (startRow != -1 && endRow != -1) {
		startRow = rowSize - startRow + 1;
		endRow = rowSize - endRow + 1;
	}
}

int Maze::sumOfMap()
{
	int navigableCount = 0;
	for (int i = 1; i <= rowSize; i++)
		for (int j = 1; j <= colSize; j++)
			if (!map[i][j]) navigableCount++;
	return navigableCount;
}

// mouseX and mouseY are the pixel coordinates of a left click
// key is either FSMOUSEEVENT_LBUTTONDOWN, FSKEY_S, or FSKEY_E
// mouse click toggles obstacle state of cell/block below mouse location
// unless block is start or end (start and end MUST be navigable)
// if block is navigable, S and E will set it to either start or end
// note, start or end may become undefined (-1, -1)

void Maze::changeMapState(int mouseX, int mouseY, int key)
{
	// get cell/block from mouseX and mouseY
	int selectedRow = mouseY / BLOCK_SIZE + 1; // use integer division
	int selectedCol = mouseX / BLOCK_SIZE + 1;

	if (1 <= selectedRow && selectedRow <= rowSize && 1 <= selectedCol && selectedCol <= colSize) {
		if (key == FSKEY_S && !map[selectedRow][selectedCol]) { // reset start if selected cell is navigable
			startRow = selectedRow;
			startCol = selectedCol;
		}
		else if (key == FSKEY_E && !map[selectedRow][selectedCol]) { // reset end if  cell is navigable
			endRow = selectedRow;
			endCol = selectedCol;
		}
		else if ((selectedRow != startRow && selectedRow != endRow)   // if not at start or at end
			|| (selectedCol != startCol && selectedCol != endCol)) {
			if (key == FSMOUSEEVENT_LBUTTONDOWN   // user just clicked OR is holding mouse down with request different from last
				|| prevChangedRow != selectedRow || prevChangedCol != selectedCol) { 
				
				map[selectedRow][selectedCol] = !map[selectedRow][selectedCol];
				prevChangedRow = selectedRow; // prevChangedRow and prevChangedCol are class integer variables
				prevChangedCol = selectedCol; // initialized to -1 (used to prevent "blinking" of square)
			}
		}
	}
}

void Maze::clear(int newRowSize, int newColSize)
{
	if (1 < newRowSize && newRowSize < MAX_MAP_SIZE
		&& 1 < newColSize && newColSize < MAX_MAP_SIZE) {

		// reset size
		rowSize = newRowSize;
		colSize = newColSize;

		// turn off start and end;
		startRow = startCol = endRow = endCol = -1;

		// make whole maze obstacles
		for (int i = 1; i <= rowSize; i++)
			for (int j = 1; j <= colSize; j++)
				map[i][j] = true;
	}
}

bool Maze::isNavigable(int row, int col)
{
	// note order of conditions 
	// (I don't check map[row][col] until I am sure that row and col are valid)
	return (0 < row && row <= rowSize
		&& 0 < col && col <= colSize
		&& !map[row][col]);
}

std::ostream& operator<<(std::ostream& os, const Maze& aMaze)
{
	os << "Start: " << aMaze.startRow << " " << aMaze.startCol << endl;
	os << "End: " << aMaze.endRow << " " << aMaze.endCol << endl;
	os << "Map Begin: " << aMaze.colSize << endl;
	for (int i = 1; i <= aMaze.rowSize; i++) {
		for (int j = 1; j <= aMaze.colSize; j++)
			os << aMaze.map[i][j] << " ";
		os << endl;
	}
	os << "Map End:" << endl;
	return os;
}


/*
void Maze::outputHardwire()
{
	cout << "rowSize = " << rowSize << "; colSize = " << colSize << ";" << endl;
	cout << "startRow = " << startRow << "; startCol = " << startCol << ";" << endl;
	cout << "endRow = " << endRow << "; endCol = " << endCol << ";" << endl;
	for (int i = 1; i <= rowSize; i++) {
		for (int j = 1; j <= colSize; j++) {
			cout << "map[" << i << "][" << j << "] = " << int(map[i][j]) << "; ";
		}
		cout << endl;
	}
}

void Maze::init10x10()
{
	rowSize = 10; colSize = 10;
	startRow = 1; startCol = 4;
	endRow = 7; endCol = 6;
	map[1][1] = 1; map[1][2] = 1; map[1][3] = 1; map[1][4] = 0; map[1][5] = 1; map[1][6] = 0; map[1][7] = 0; map[1][8] = 0; map[1][9] = 1; map[1][10] = 0;
	map[2][1] = 0; map[2][2] = 1; map[2][3] = 0; map[2][4] = 0; map[2][5] = 1; map[2][6] = 0; map[2][7] = 1; map[2][8] = 0; map[2][9] = 1; map[2][10] = 0;
	map[3][1] = 0; map[3][2] = 1; map[3][3] = 0; map[3][4] = 1; map[3][5] = 1; map[3][6] = 0; map[3][7] = 1; map[3][8] = 0; map[3][9] = 1; map[3][10] = 1;
	map[4][1] = 0; map[4][2] = 1; map[4][3] = 0; map[4][4] = 1; map[4][5] = 0; map[4][6] = 0; map[4][7] = 1; map[4][8] = 0; map[4][9] = 0; map[4][10] = 1;
	map[5][1] = 0; map[5][2] = 1; map[5][3] = 0; map[5][4] = 0; map[5][5] = 0; map[5][6] = 1; map[5][7] = 1; map[5][8] = 1; map[5][9] = 0; map[5][10] = 0;
	map[6][1] = 0; map[6][2] = 1; map[6][3] = 0; map[6][4] = 1; map[6][5] = 1; map[6][6] = 1; map[6][7] = 0; map[6][8] = 1; map[6][9] = 1; map[6][10] = 1;
	map[7][1] = 0; map[7][2] = 0; map[7][3] = 0; map[7][4] = 0; map[7][5] = 1; map[7][6] = 0; map[7][7] = 0; map[7][8] = 0; map[7][9] = 1; map[7][10] = 0;
	map[8][1] = 1; map[8][2] = 1; map[8][3] = 1; map[8][4] = 0; map[8][5] = 1; map[8][6] = 1; map[8][7] = 1; map[8][8] = 0; map[8][9] = 1; map[8][10] = 1;
	map[9][1] = 1; map[9][2] = 0; map[9][3] = 1; map[9][4] = 0; map[9][5] = 0; map[9][6] = 1; map[9][7] = 1; map[9][8] = 0; map[9][9] = 0; map[9][10] = 1;
	map[10][1] = 1; map[10][2] = 1; map[10][3] = 1; map[10][4] = 1; map[10][5] = 0; map[10][6] = 0; map[10][7] = 0; map[10][8] = 0; map[10][9] = 1; map[10][10] = 1;
}

void Maze::init25x25()
{
	rowSize = 25; colSize = 25;
	startRow = 7; startCol = 10;
	endRow = 22; endCol = 25;
	map[1][1] = 1; map[1][2] = 1; map[1][3] = 1; map[1][4] = 0; map[1][5] = 1; map[1][6] = 0; map[1][7] = 0; map[1][8] = 0; map[1][9] = 1; map[1][10] = 0; map[1][11] = 1; map[1][12] = 1; map[1][13] = 1; map[1][14] = 1; map[1][15] = 1; map[1][16] = 0; map[1][17] = 0; map[1][18] = 0; map[1][19] = 1; map[1][20] = 1; map[1][21] = 1; map[1][22] = 0; map[1][23] = 0; map[1][24] = 0; map[1][25] = 1;
	map[2][1] = 0; map[2][2] = 1; map[2][3] = 0; map[2][4] = 0; map[2][5] = 1; map[2][6] = 0; map[2][7] = 1; map[2][8] = 0; map[2][9] = 1; map[2][10] = 0; map[2][11] = 0; map[2][12] = 1; map[2][13] = 1; map[2][14] = 0; map[2][15] = 0; map[2][16] = 0; map[2][17] = 1; map[2][18] = 0; map[2][19] = 1; map[2][20] = 1; map[2][21] = 1; map[2][22] = 0; map[2][23] = 1; map[2][24] = 0; map[2][25] = 0;
	map[3][1] = 0; map[3][2] = 1; map[3][3] = 0; map[3][4] = 1; map[3][5] = 1; map[3][6] = 0; map[3][7] = 1; map[3][8] = 0; map[3][9] = 1; map[3][10] = 1; map[3][11] = 0; map[3][12] = 1; map[3][13] = 1; map[3][14] = 0; map[3][15] = 1; map[3][16] = 1; map[3][17] = 1; map[3][18] = 0; map[3][19] = 1; map[3][20] = 0; map[3][21] = 0; map[3][22] = 0; map[3][23] = 1; map[3][24] = 1; map[3][25] = 0;
	map[4][1] = 0; map[4][2] = 1; map[4][3] = 0; map[4][4] = 1; map[4][5] = 0; map[4][6] = 0; map[4][7] = 1; map[4][8] = 0; map[4][9] = 0; map[4][10] = 1; map[4][11] = 0; map[4][12] = 1; map[4][13] = 0; map[4][14] = 0; map[4][15] = 0; map[4][16] = 1; map[4][17] = 1; map[4][18] = 0; map[4][19] = 0; map[4][20] = 0; map[4][21] = 1; map[4][22] = 1; map[4][23] = 1; map[4][24] = 1; map[4][25] = 0;
	map[5][1] = 0; map[5][2] = 1; map[5][3] = 0; map[5][4] = 0; map[5][5] = 0; map[5][6] = 1; map[5][7] = 1; map[5][8] = 1; map[5][9] = 0; map[5][10] = 0; map[5][11] = 0; map[5][12] = 1; map[5][13] = 0; map[5][14] = 1; map[5][15] = 0; map[5][16] = 1; map[5][17] = 0; map[5][18] = 0; map[5][19] = 1; map[5][20] = 1; map[5][21] = 1; map[5][22] = 0; map[5][23] = 0; map[5][24] = 0; map[5][25] = 0;
	map[6][1] = 0; map[6][2] = 1; map[6][3] = 0; map[6][4] = 1; map[6][5] = 1; map[6][6] = 1; map[6][7] = 0; map[6][8] = 1; map[6][9] = 1; map[6][10] = 1; map[6][11] = 0; map[6][12] = 1; map[6][13] = 0; map[6][14] = 1; map[6][15] = 0; map[6][16] = 1; map[6][17] = 1; map[6][18] = 1; map[6][19] = 1; map[6][20] = 0; map[6][21] = 1; map[6][22] = 0; map[6][23] = 1; map[6][24] = 1; map[6][25] = 1;
	map[7][1] = 0; map[7][2] = 0; map[7][3] = 0; map[7][4] = 0; map[7][5] = 1; map[7][6] = 0; map[7][7] = 0; map[7][8] = 0; map[7][9] = 1; map[7][10] = 0; map[7][11] = 0; map[7][12] = 1; map[7][13] = 0; map[7][14] = 1; map[7][15] = 0; map[7][16] = 1; map[7][17] = 0; map[7][18] = 0; map[7][19] = 0; map[7][20] = 0; map[7][21] = 0; map[7][22] = 0; map[7][23] = 1; map[7][24] = 0; map[7][25] = 0;
	map[8][1] = 1; map[8][2] = 1; map[8][3] = 1; map[8][4] = 0; map[8][5] = 1; map[8][6] = 1; map[8][7] = 1; map[8][8] = 0; map[8][9] = 1; map[8][10] = 1; map[8][11] = 1; map[8][12] = 1; map[8][13] = 0; map[8][14] = 1; map[8][15] = 0; map[8][16] = 1; map[8][17] = 0; map[8][18] = 1; map[8][19] = 1; map[8][20] = 1; map[8][21] = 1; map[8][22] = 1; map[8][23] = 1; map[8][24] = 1; map[8][25] = 1;
	map[9][1] = 1; map[9][2] = 0; map[9][3] = 1; map[9][4] = 0; map[9][5] = 0; map[9][6] = 1; map[9][7] = 1; map[9][8] = 0; map[9][9] = 0; map[9][10] = 1; map[9][11] = 0; map[9][12] = 0; map[9][13] = 0; map[9][14] = 1; map[9][15] = 0; map[9][16] = 1; map[9][17] = 0; map[9][18] = 1; map[9][19] = 1; map[9][20] = 0; map[9][21] = 0; map[9][22] = 0; map[9][23] = 0; map[9][24] = 1; map[9][25] = 1;
	map[10][1] = 1; map[10][2] = 1; map[10][3] = 1; map[10][4] = 1; map[10][5] = 0; map[10][6] = 1; map[10][7] = 1; map[10][8] = 0; map[10][9] = 1; map[10][10] = 1; map[10][11] = 0; map[10][12] = 1; map[10][13] = 1; map[10][14] = 1; map[10][15] = 0; map[10][16] = 1; map[10][17] = 0; map[10][18] = 0; map[10][19] = 0; map[10][20] = 0; map[10][21] = 1; map[10][22] = 1; map[10][23] = 0; map[10][24] = 0; map[10][25] = 0;
	map[11][1] = 0; map[11][2] = 0; map[11][3] = 0; map[11][4] = 1; map[11][5] = 0; map[11][6] = 1; map[11][7] = 0; map[11][8] = 0; map[11][9] = 1; map[11][10] = 0; map[11][11] = 0; map[11][12] = 1; map[11][13] = 0; map[11][14] = 0; map[11][15] = 0; map[11][16] = 1; map[11][17] = 0; map[11][18] = 1; map[11][19] = 1; map[11][20] = 1; map[11][21] = 1; map[11][22] = 1; map[11][23] = 1; map[11][24] = 1; map[11][25] = 0;
	map[12][1] = 0; map[12][2] = 1; map[12][3] = 0; map[12][4] = 0; map[12][5] = 0; map[12][6] = 1; map[12][7] = 0; map[12][8] = 1; map[12][9] = 1; map[12][10] = 0; map[12][11] = 1; map[12][12] = 1; map[12][13] = 1; map[12][14] = 1; map[12][15] = 1; map[12][16] = 1; map[12][17] = 0; map[12][18] = 1; map[12][19] = 0; map[12][20] = 1; map[12][21] = 0; map[12][22] = 0; map[12][23] = 0; map[12][24] = 1; map[12][25] = 0;
	map[13][1] = 0; map[13][2] = 1; map[13][3] = 1; map[13][4] = 1; map[13][5] = 1; map[13][6] = 1; map[13][7] = 0; map[13][8] = 1; map[13][9] = 0; map[13][10] = 0; map[13][11] = 1; map[13][12] = 0; map[13][13] = 0; map[13][14] = 0; map[13][15] = 0; map[13][16] = 0; map[13][17] = 0; map[13][18] = 1; map[13][19] = 0; map[13][20] = 1; map[13][21] = 0; map[13][22] = 1; map[13][23] = 0; map[13][24] = 0; map[13][25] = 0;
	map[14][1] = 0; map[14][2] = 0; map[14][3] = 0; map[14][4] = 0; map[14][5] = 0; map[14][6] = 0; map[14][7] = 0; map[14][8] = 1; map[14][9] = 0; map[14][10] = 1; map[14][11] = 1; map[14][12] = 0; map[14][13] = 1; map[14][14] = 1; map[14][15] = 1; map[14][16] = 1; map[14][17] = 1; map[14][18] = 1; map[14][19] = 0; map[14][20] = 1; map[14][21] = 0; map[14][22] = 1; map[14][23] = 1; map[14][24] = 1; map[14][25] = 1;
	map[15][1] = 1; map[15][2] = 1; map[15][3] = 1; map[15][4] = 1; map[15][5] = 1; map[15][6] = 1; map[15][7] = 0; map[15][8] = 1; map[15][9] = 0; map[15][10] = 1; map[15][11] = 0; map[15][12] = 0; map[15][13] = 1; map[15][14] = 1; map[15][15] = 0; map[15][16] = 0; map[15][17] = 0; map[15][18] = 0; map[15][19] = 0; map[15][20] = 1; map[15][21] = 0; map[15][22] = 0; map[15][23] = 0; map[15][24] = 0; map[15][25] = 1;
	map[16][1] = 1; map[16][2] = 1; map[16][3] = 0; map[16][4] = 1; map[16][5] = 0; map[16][6] = 0; map[16][7] = 0; map[16][8] = 0; map[16][9] = 0; map[16][10] = 1; map[16][11] = 1; map[16][12] = 0; map[16][13] = 0; map[16][14] = 1; map[16][15] = 0; map[16][16] = 1; map[16][17] = 1; map[16][18] = 1; map[16][19] = 1; map[16][20] = 1; map[16][21] = 0; map[16][22] = 1; map[16][23] = 1; map[16][24] = 0; map[16][25] = 0;
	map[17][1] = 1; map[17][2] = 1; map[17][3] = 0; map[17][4] = 1; map[17][5] = 0; map[17][6] = 1; map[17][7] = 1; map[17][8] = 1; map[17][9] = 1; map[17][10] = 1; map[17][11] = 1; map[17][12] = 1; map[17][13] = 0; map[17][14] = 1; map[17][15] = 0; map[17][16] = 1; map[17][17] = 0; map[17][18] = 0; map[17][19] = 0; map[17][20] = 0; map[17][21] = 0; map[17][22] = 1; map[17][23] = 1; map[17][24] = 1; map[17][25] = 0;
	map[18][1] = 0; map[18][2] = 0; map[18][3] = 0; map[18][4] = 1; map[18][5] = 0; map[18][6] = 0; map[18][7] = 0; map[18][8] = 1; map[18][9] = 0; map[18][10] = 0; map[18][11] = 0; map[18][12] = 1; map[18][13] = 0; map[18][14] = 1; map[18][15] = 0; map[18][16] = 1; map[18][17] = 0; map[18][18] = 1; map[18][19] = 1; map[18][20] = 1; map[18][21] = 1; map[18][22] = 1; map[18][23] = 0; map[18][24] = 0; map[18][25] = 0;
	map[19][1] = 0; map[19][2] = 1; map[19][3] = 0; map[19][4] = 1; map[19][5] = 1; map[19][6] = 1; map[19][7] = 0; map[19][8] = 1; map[19][9] = 1; map[19][10] = 1; map[19][11] = 0; map[19][12] = 1; map[19][13] = 0; map[19][14] = 1; map[19][15] = 0; map[19][16] = 1; map[19][17] = 0; map[19][18] = 1; map[19][19] = 1; map[19][20] = 1; map[19][21] = 1; map[19][22] = 1; map[19][23] = 1; map[19][24] = 1; map[19][25] = 1;
	map[20][1] = 0; map[20][2] = 1; map[20][3] = 0; map[20][4] = 0; map[20][5] = 0; map[20][6] = 0; map[20][7] = 0; map[20][8] = 1; map[20][9] = 1; map[20][10] = 1; map[20][11] = 0; map[20][12] = 1; map[20][13] = 0; map[20][14] = 0; map[20][15] = 0; map[20][16] = 1; map[20][17] = 0; map[20][18] = 1; map[20][19] = 0; map[20][20] = 0; map[20][21] = 0; map[20][22] = 0; map[20][23] = 0; map[20][24] = 0; map[20][25] = 0;
	map[21][1] = 0; map[21][2] = 1; map[21][3] = 1; map[21][4] = 1; map[21][5] = 1; map[21][6] = 1; map[21][7] = 1; map[21][8] = 1; map[21][9] = 0; map[21][10] = 0; map[21][11] = 0; map[21][12] = 1; map[21][13] = 1; map[21][14] = 1; map[21][15] = 1; map[21][16] = 1; map[21][17] = 0; map[21][18] = 1; map[21][19] = 0; map[21][20] = 1; map[21][21] = 1; map[21][22] = 1; map[21][23] = 1; map[21][24] = 1; map[21][25] = 0;
	map[22][1] = 0; map[22][2] = 0; map[22][3] = 0; map[22][4] = 0; map[22][5] = 0; map[22][6] = 0; map[22][7] = 0; map[22][8] = 0; map[22][9] = 0; map[22][10] = 1; map[22][11] = 1; map[22][12] = 1; map[22][13] = 1; map[22][14] = 1; map[22][15] = 1; map[22][16] = 0; map[22][17] = 0; map[22][18] = 1; map[22][19] = 0; map[22][20] = 0; map[22][21] = 0; map[22][22] = 0; map[22][23] = 1; map[22][24] = 1; map[22][25] = 0;
	map[23][1] = 1; map[23][2] = 1; map[23][3] = 1; map[23][4] = 1; map[23][5] = 1; map[23][6] = 1; map[23][7] = 1; map[23][8] = 1; map[23][9] = 1; map[23][10] = 1; map[23][11] = 0; map[23][12] = 0; map[23][13] = 0; map[23][14] = 0; map[23][15] = 0; map[23][16] = 0; map[23][17] = 1; map[23][18] = 1; map[23][19] = 0; map[23][20] = 0; map[23][21] = 1; map[23][22] = 0; map[23][23] = 1; map[23][24] = 1; map[23][25] = 0;
	map[24][1] = 0; map[24][2] = 0; map[24][3] = 0; map[24][4] = 1; map[24][5] = 1; map[24][6] = 0; map[24][7] = 0; map[24][8] = 0; map[24][9] = 0; map[24][10] = 0; map[24][11] = 0; map[24][12] = 1; map[24][13] = 0; map[24][14] = 1; map[24][15] = 1; map[24][16] = 1; map[24][17] = 1; map[24][18] = 0; map[24][19] = 0; map[24][20] = 1; map[24][21] = 1; map[24][22] = 0; map[24][23] = 1; map[24][24] = 1; map[24][25] = 0;
	map[25][1] = 1; map[25][2] = 1; map[25][3] = 0; map[25][4] = 0; map[25][5] = 0; map[25][6] = 0; map[25][7] = 1; map[25][8] = 1; map[25][9] = 1; map[25][10] = 1; map[25][11] = 1; map[25][12] = 1; map[25][13] = 0; map[25][14] = 0; map[25][15] = 0; map[25][16] = 0; map[25][17] = 0; map[25][18] = 0; map[25][19] = 1; map[25][20] = 1; map[25][21] = 0; map[25][22] = 0; map[25][23] = 0; map[25][24] = 0; map[25][25] = 0;
}

void Maze::init40x20()
{
	rowSize = 20; colSize = 40;
	startRow = 7; startCol = 4;
	endRow = 18; endCol = 38;
	map[1][1] = 1; map[1][2] = 1; map[1][3] = 1; map[1][4] = 0; map[1][5] = 1; map[1][6] = 0; map[1][7] = 0; map[1][8] = 0; map[1][9] = 1; map[1][10] = 0; map[1][11] = 1; map[1][12] = 1; map[1][13] = 1; map[1][14] = 1; map[1][15] = 1; map[1][16] = 0; map[1][17] = 0; map[1][18] = 0; map[1][19] = 1; map[1][20] = 1; map[1][21] = 1; map[1][22] = 1; map[1][23] = 1; map[1][24] = 0; map[1][25] = 0; map[1][26] = 0; map[1][27] = 0; map[1][28] = 1; map[1][29] = 0; map[1][30] = 1; map[1][31] = 1; map[1][32] = 1; map[1][33] = 1; map[1][34] = 1; map[1][35] = 1; map[1][36] = 1; map[1][37] = 1; map[1][38] = 1; map[1][39] = 1; map[1][40] = 1;
	map[2][1] = 0; map[2][2] = 1; map[2][3] = 0; map[2][4] = 0; map[2][5] = 1; map[2][6] = 0; map[2][7] = 1; map[2][8] = 0; map[2][9] = 1; map[2][10] = 0; map[2][11] = 0; map[2][12] = 1; map[2][13] = 1; map[2][14] = 0; map[2][15] = 0; map[2][16] = 0; map[2][17] = 1; map[2][18] = 0; map[2][19] = 0; map[2][20] = 0; map[2][21] = 0; map[2][22] = 0; map[2][23] = 1; map[2][24] = 0; map[2][25] = 1; map[2][26] = 1; map[2][27] = 0; map[2][28] = 1; map[2][29] = 0; map[2][30] = 0; map[2][31] = 0; map[2][32] = 0; map[2][33] = 0; map[2][34] = 0; map[2][35] = 0; map[2][36] = 0; map[2][37] = 0; map[2][38] = 0; map[2][39] = 0; map[2][40] = 0;
	map[3][1] = 0; map[3][2] = 1; map[3][3] = 0; map[3][4] = 1; map[3][5] = 1; map[3][6] = 0; map[3][7] = 1; map[3][8] = 0; map[3][9] = 1; map[3][10] = 1; map[3][11] = 0; map[3][12] = 1; map[3][13] = 1; map[3][14] = 0; map[3][15] = 1; map[3][16] = 1; map[3][17] = 1; map[3][18] = 0; map[3][19] = 1; map[3][20] = 1; map[3][21] = 0; map[3][22] = 1; map[3][23] = 1; map[3][24] = 1; map[3][25] = 1; map[3][26] = 0; map[3][27] = 0; map[3][28] = 0; map[3][29] = 0; map[3][30] = 1; map[3][31] = 0; map[3][32] = 1; map[3][33] = 1; map[3][34] = 1; map[3][35] = 1; map[3][36] = 1; map[3][37] = 0; map[3][38] = 1; map[3][39] = 1; map[3][40] = 1;
	map[4][1] = 0; map[4][2] = 1; map[4][3] = 0; map[4][4] = 1; map[4][5] = 0; map[4][6] = 0; map[4][7] = 1; map[4][8] = 0; map[4][9] = 0; map[4][10] = 1; map[4][11] = 0; map[4][12] = 1; map[4][13] = 0; map[4][14] = 0; map[4][15] = 0; map[4][16] = 1; map[4][17] = 0; map[4][18] = 0; map[4][19] = 0; map[4][20] = 1; map[4][21] = 0; map[4][22] = 0; map[4][23] = 1; map[4][24] = 0; map[4][25] = 0; map[4][26] = 0; map[4][27] = 1; map[4][28] = 1; map[4][29] = 1; map[4][30] = 1; map[4][31] = 0; map[4][32] = 1; map[4][33] = 0; map[4][34] = 0; map[4][35] = 0; map[4][36] = 1; map[4][37] = 0; map[4][38] = 0; map[4][39] = 0; map[4][40] = 1;
	map[5][1] = 0; map[5][2] = 1; map[5][3] = 0; map[5][4] = 0; map[5][5] = 0; map[5][6] = 1; map[5][7] = 1; map[5][8] = 1; map[5][9] = 0; map[5][10] = 0; map[5][11] = 0; map[5][12] = 1; map[5][13] = 0; map[5][14] = 1; map[5][15] = 0; map[5][16] = 1; map[5][17] = 0; map[5][18] = 1; map[5][19] = 0; map[5][20] = 1; map[5][21] = 0; map[5][22] = 1; map[5][23] = 1; map[5][24] = 0; map[5][25] = 1; map[5][26] = 0; map[5][27] = 1; map[5][28] = 0; map[5][29] = 0; map[5][30] = 0; map[5][31] = 0; map[5][32] = 1; map[5][33] = 0; map[5][34] = 1; map[5][35] = 0; map[5][36] = 0; map[5][37] = 0; map[5][38] = 1; map[5][39] = 0; map[5][40] = 1;
	map[6][1] = 0; map[6][2] = 1; map[6][3] = 0; map[6][4] = 1; map[6][5] = 1; map[6][6] = 1; map[6][7] = 0; map[6][8] = 1; map[6][9] = 1; map[6][10] = 1; map[6][11] = 0; map[6][12] = 1; map[6][13] = 0; map[6][14] = 1; map[6][15] = 0; map[6][16] = 1; map[6][17] = 0; map[6][18] = 1; map[6][19] = 1; map[6][20] = 1; map[6][21] = 0; map[6][22] = 0; map[6][23] = 0; map[6][24] = 0; map[6][25] = 1; map[6][26] = 0; map[6][27] = 1; map[6][28] = 1; map[6][29] = 1; map[6][30] = 0; map[6][31] = 0; map[6][32] = 1; map[6][33] = 0; map[6][34] = 0; map[6][35] = 1; map[6][36] = 0; map[6][37] = 1; map[6][38] = 0; map[6][39] = 0; map[6][40] = 1;
	map[7][1] = 0; map[7][2] = 0; map[7][3] = 0; map[7][4] = 0; map[7][5] = 1; map[7][6] = 0; map[7][7] = 0; map[7][8] = 0; map[7][9] = 1; map[7][10] = 0; map[7][11] = 0; map[7][12] = 1; map[7][13] = 1; map[7][14] = 1; map[7][15] = 0; map[7][16] = 1; map[7][17] = 0; map[7][18] = 0; map[7][19] = 0; map[7][20] = 0; map[7][21] = 0; map[7][22] = 1; map[7][23] = 1; map[7][24] = 0; map[7][25] = 1; map[7][26] = 0; map[7][27] = 0; map[7][28] = 0; map[7][29] = 0; map[7][30] = 0; map[7][31] = 1; map[7][32] = 1; map[7][33] = 1; map[7][34] = 0; map[7][35] = 0; map[7][36] = 1; map[7][37] = 0; map[7][38] = 0; map[7][39] = 1; map[7][40] = 1;
	map[8][1] = 1; map[8][2] = 0; map[8][3] = 1; map[8][4] = 0; map[8][5] = 1; map[8][6] = 1; map[8][7] = 1; map[8][8] = 0; map[8][9] = 1; map[8][10] = 1; map[8][11] = 1; map[8][12] = 1; map[8][13] = 0; map[8][14] = 0; map[8][15] = 0; map[8][16] = 1; map[8][17] = 0; map[8][18] = 1; map[8][19] = 1; map[8][20] = 1; map[8][21] = 1; map[8][22] = 0; map[8][23] = 0; map[8][24] = 0; map[8][25] = 1; map[8][26] = 1; map[8][27] = 1; map[8][28] = 0; map[8][29] = 1; map[8][30] = 0; map[8][31] = 0; map[8][32] = 1; map[8][33] = 0; map[8][34] = 0; map[8][35] = 1; map[8][36] = 0; map[8][37] = 1; map[8][38] = 0; map[8][39] = 0; map[8][40] = 1;
	map[9][1] = 1; map[9][2] = 0; map[9][3] = 1; map[9][4] = 0; map[9][5] = 0; map[9][6] = 1; map[9][7] = 1; map[9][8] = 0; map[9][9] = 0; map[9][10] = 1; map[9][11] = 0; map[9][12] = 0; map[9][13] = 0; map[9][14] = 1; map[9][15] = 0; map[9][16] = 1; map[9][17] = 0; map[9][18] = 1; map[9][19] = 0; map[9][20] = 0; map[9][21] = 1; map[9][22] = 0; map[9][23] = 1; map[9][24] = 0; map[9][25] = 1; map[9][26] = 1; map[9][27] = 0; map[9][28] = 0; map[9][29] = 1; map[9][30] = 0; map[9][31] = 1; map[9][32] = 1; map[9][33] = 0; map[9][34] = 1; map[9][35] = 0; map[9][36] = 0; map[9][37] = 0; map[9][38] = 1; map[9][39] = 0; map[9][40] = 1;
	map[10][1] = 1; map[10][2] = 0; map[10][3] = 1; map[10][4] = 1; map[10][5] = 0; map[10][6] = 1; map[10][7] = 1; map[10][8] = 0; map[10][9] = 1; map[10][10] = 1; map[10][11] = 0; map[10][12] = 1; map[10][13] = 1; map[10][14] = 1; map[10][15] = 0; map[10][16] = 1; map[10][17] = 0; map[10][18] = 0; map[10][19] = 0; map[10][20] = 0; map[10][21] = 0; map[10][22] = 0; map[10][23] = 1; map[10][24] = 0; map[10][25] = 1; map[10][26] = 0; map[10][27] = 0; map[10][28] = 1; map[10][29] = 0; map[10][30] = 0; map[10][31] = 0; map[10][32] = 0; map[10][33] = 0; map[10][34] = 0; map[10][35] = 0; map[10][36] = 1; map[10][37] = 0; map[10][38] = 0; map[10][39] = 0; map[10][40] = 1;
	map[11][1] = 0; map[11][2] = 0; map[11][3] = 0; map[11][4] = 1; map[11][5] = 1; map[11][6] = 1; map[11][7] = 0; map[11][8] = 0; map[11][9] = 1; map[11][10] = 0; map[11][11] = 0; map[11][12] = 1; map[11][13] = 0; map[11][14] = 0; map[11][15] = 0; map[11][16] = 1; map[11][17] = 0; map[11][18] = 1; map[11][19] = 1; map[11][20] = 0; map[11][21] = 1; map[11][22] = 1; map[11][23] = 1; map[11][24] = 1; map[11][25] = 1; map[11][26] = 0; map[11][27] = 1; map[11][28] = 1; map[11][29] = 0; map[11][30] = 1; map[11][31] = 0; map[11][32] = 1; map[11][33] = 1; map[11][34] = 0; map[11][35] = 0; map[11][36] = 0; map[11][37] = 0; map[11][38] = 0; map[11][39] = 1; map[11][40] = 1;
	map[12][1] = 0; map[12][2] = 1; map[12][3] = 0; map[12][4] = 0; map[12][5] = 0; map[12][6] = 1; map[12][7] = 0; map[12][8] = 1; map[12][9] = 1; map[12][10] = 0; map[12][11] = 1; map[12][12] = 1; map[12][13] = 0; map[12][14] = 1; map[12][15] = 1; map[12][16] = 1; map[12][17] = 0; map[12][18] = 1; map[12][19] = 1; map[12][20] = 0; map[12][21] = 1; map[12][22] = 1; map[12][23] = 0; map[12][24] = 0; map[12][25] = 0; map[12][26] = 0; map[12][27] = 1; map[12][28] = 0; map[12][29] = 0; map[12][30] = 1; map[12][31] = 0; map[12][32] = 0; map[12][33] = 0; map[12][34] = 0; map[12][35] = 1; map[12][36] = 1; map[12][37] = 1; map[12][38] = 0; map[12][39] = 0; map[12][40] = 1;
	map[13][1] = 0; map[13][2] = 1; map[13][3] = 1; map[13][4] = 1; map[13][5] = 1; map[13][6] = 1; map[13][7] = 0; map[13][8] = 1; map[13][9] = 0; map[13][10] = 0; map[13][11] = 1; map[13][12] = 0; map[13][13] = 0; map[13][14] = 0; map[13][15] = 0; map[13][16] = 0; map[13][17] = 0; map[13][18] = 1; map[13][19] = 1; map[13][20] = 0; map[13][21] = 0; map[13][22] = 1; map[13][23] = 1; map[13][24] = 0; map[13][25] = 0; map[13][26] = 1; map[13][27] = 1; map[13][28] = 0; map[13][29] = 1; map[13][30] = 1; map[13][31] = 0; map[13][32] = 1; map[13][33] = 0; map[13][34] = 1; map[13][35] = 0; map[13][36] = 0; map[13][37] = 0; map[13][38] = 1; map[13][39] = 0; map[13][40] = 1;
	map[14][1] = 0; map[14][2] = 0; map[14][3] = 0; map[14][4] = 0; map[14][5] = 0; map[14][6] = 0; map[14][7] = 0; map[14][8] = 1; map[14][9] = 0; map[14][10] = 1; map[14][11] = 1; map[14][12] = 0; map[14][13] = 1; map[14][14] = 1; map[14][15] = 1; map[14][16] = 1; map[14][17] = 0; map[14][18] = 0; map[14][19] = 1; map[14][20] = 1; map[14][21] = 0; map[14][22] = 0; map[14][23] = 1; map[14][24] = 1; map[14][25] = 0; map[14][26] = 1; map[14][27] = 1; map[14][28] = 0; map[14][29] = 1; map[14][30] = 0; map[14][31] = 0; map[14][32] = 1; map[14][33] = 0; map[14][34] = 1; map[14][35] = 0; map[14][36] = 1; map[14][37] = 0; map[14][38] = 1; map[14][39] = 0; map[14][40] = 1;
	map[15][1] = 1; map[15][2] = 1; map[15][3] = 0; map[15][4] = 1; map[15][5] = 1; map[15][6] = 1; map[15][7] = 0; map[15][8] = 1; map[15][9] = 0; map[15][10] = 1; map[15][11] = 0; map[15][12] = 0; map[15][13] = 1; map[15][14] = 1; map[15][15] = 0; map[15][16] = 0; map[15][17] = 0; map[15][18] = 0; map[15][19] = 0; map[15][20] = 1; map[15][21] = 1; map[15][22] = 0; map[15][23] = 0; map[15][24] = 1; map[15][25] = 0; map[15][26] = 0; map[15][27] = 1; map[15][28] = 0; map[15][29] = 1; map[15][30] = 1; map[15][31] = 1; map[15][32] = 1; map[15][33] = 0; map[15][34] = 1; map[15][35] = 0; map[15][36] = 0; map[15][37] = 0; map[15][38] = 1; map[15][39] = 0; map[15][40] = 1;
	map[16][1] = 1; map[16][2] = 1; map[16][3] = 0; map[16][4] = 1; map[16][5] = 0; map[16][6] = 1; map[16][7] = 0; map[16][8] = 0; map[16][9] = 0; map[16][10] = 1; map[16][11] = 1; map[16][12] = 0; map[16][13] = 0; map[16][14] = 1; map[16][15] = 0; map[16][16] = 1; map[16][17] = 0; map[16][18] = 1; map[16][19] = 0; map[16][20] = 0; map[16][21] = 1; map[16][22] = 1; map[16][23] = 0; map[16][24] = 1; map[16][25] = 1; map[16][26] = 0; map[16][27] = 1; map[16][28] = 0; map[16][29] = 0; map[16][30] = 0; map[16][31] = 0; map[16][32] = 1; map[16][33] = 0; map[16][34] = 0; map[16][35] = 1; map[16][36] = 1; map[16][37] = 1; map[16][38] = 0; map[16][39] = 0; map[16][40] = 1;
	map[17][1] = 1; map[17][2] = 1; map[17][3] = 0; map[17][4] = 1; map[17][5] = 0; map[17][6] = 1; map[17][7] = 1; map[17][8] = 1; map[17][9] = 1; map[17][10] = 1; map[17][11] = 1; map[17][12] = 1; map[17][13] = 0; map[17][14] = 1; map[17][15] = 0; map[17][16] = 1; map[17][17] = 0; map[17][18] = 1; map[17][19] = 1; map[17][20] = 0; map[17][21] = 0; map[17][22] = 1; map[17][23] = 0; map[17][24] = 0; map[17][25] = 0; map[17][26] = 0; map[17][27] = 1; map[17][28] = 0; map[17][29] = 1; map[17][30] = 0; map[17][31] = 1; map[17][32] = 1; map[17][33] = 1; map[17][34] = 0; map[17][35] = 1; map[17][36] = 0; map[17][37] = 1; map[17][38] = 1; map[17][39] = 1; map[17][40] = 1;
	map[18][1] = 0; map[18][2] = 0; map[18][3] = 0; map[18][4] = 1; map[18][5] = 0; map[18][6] = 0; map[18][7] = 0; map[18][8] = 1; map[18][9] = 0; map[18][10] = 0; map[18][11] = 0; map[18][12] = 1; map[18][13] = 0; map[18][14] = 1; map[18][15] = 0; map[18][16] = 1; map[18][17] = 0; map[18][18] = 1; map[18][19] = 1; map[18][20] = 1; map[18][21] = 0; map[18][22] = 1; map[18][23] = 1; map[18][24] = 1; map[18][25] = 1; map[18][26] = 1; map[18][27] = 1; map[18][28] = 0; map[18][29] = 1; map[18][30] = 0; map[18][31] = 0; map[18][32] = 0; map[18][33] = 0; map[18][34] = 0; map[18][35] = 1; map[18][36] = 1; map[18][37] = 1; map[18][38] = 0; map[18][39] = 0; map[18][40] = 0;
	map[19][1] = 0; map[19][2] = 1; map[19][3] = 0; map[19][4] = 1; map[19][5] = 1; map[19][6] = 1; map[19][7] = 0; map[19][8] = 1; map[19][9] = 1; map[19][10] = 1; map[19][11] = 0; map[19][12] = 1; map[19][13] = 0; map[19][14] = 1; map[19][15] = 0; map[19][16] = 1; map[19][17] = 0; map[19][18] = 0; map[19][19] = 0; map[19][20] = 1; map[19][21] = 0; map[19][22] = 0; map[19][23] = 1; map[19][24] = 1; map[19][25] = 0; map[19][26] = 1; map[19][27] = 1; map[19][28] = 0; map[19][29] = 1; map[19][30] = 1; map[19][31] = 1; map[19][32] = 0; map[19][33] = 1; map[19][34] = 0; map[19][35] = 0; map[19][36] = 0; map[19][37] = 1; map[19][38] = 1; map[19][39] = 1; map[19][40] = 0;
	map[20][1] = 0; map[20][2] = 1; map[20][3] = 0; map[20][4] = 0; map[20][5] = 0; map[20][6] = 0; map[20][7] = 0; map[20][8] = 1; map[20][9] = 1; map[20][10] = 1; map[20][11] = 0; map[20][12] = 1; map[20][13] = 0; map[20][14] = 0; map[20][15] = 0; map[20][16] = 1; map[20][17] = 1; map[20][18] = 1; map[20][19] = 0; map[20][20] = 1; map[20][21] = 1; map[20][22] = 0; map[20][23] = 0; map[20][24] = 0; map[20][25] = 0; map[20][26] = 1; map[20][27] = 1; map[20][28] = 0; map[20][29] = 0; map[20][30] = 0; map[20][31] = 0; map[20][32] = 0; map[20][33] = 1; map[20][34] = 1; map[20][35] = 1; map[20][36] = 0; map[20][37] = 0; map[20][38] = 0; map[20][39] = 0; map[20][40] = 0;
}
*/

