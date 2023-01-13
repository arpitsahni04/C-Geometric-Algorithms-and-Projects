#include <queue>
#include <iostream>
#include "fssimplewindow.h"
//#include "ysglfontdata.h"
#include "DrawingUtilNG.h"
#include "Maze.h"  // not included in Entity.h
#include "Entity.h"

void Entity::reset()
{
	// mark everything as not traveled and not visited and not "parented"
	for (int i = 1; i < Maze::MAX_MAP_SIZE; i++)
		for (int j = 1; j < Maze::MAX_MAP_SIZE; j++) {
			traveled[i][j] = false;
			visited[i][j] = false;     // could call clearShortestPath() here, but since I
			parent[i][j] = { -1,-1 };  // already have a loop running  . . .
		}

	// put entity back at start loc
	locRow = myMaze->getStart().row;
	locCol = myMaze->getStart().col;

	// reset celebrateCounter
	celebrateCounter = 0;
}

void Entity::clearShortestPath()
{
	for (int i = 1; i < Maze::MAX_MAP_SIZE; i++)
		for (int j = 1; j < Maze::MAX_MAP_SIZE; j++) {
			visited[i][j] = false; // closed
			parent[i][j] = { -1,-1 }; // for backtracking to get path
		}
}

bool Entity::move(int direction)
{
	int testCol = locCol, testRow = locRow;

	switch (direction) {
	case FSKEY_UP: testRow--;
		break;
	case FSKEY_DOWN: testRow++;
		break;
	case FSKEY_LEFT: testCol--;
		break;
	case FSKEY_RIGHT: testCol++;
		break;
	}
	if (myMaze->isNavigable(testRow, testCol)) {
		// record location where entity used to be
		traveled[locRow][locCol] = true;
		// change location of entity
		locCol = testCol;
		locRow = testRow;
		return true;
	}
	else
		return false;
}

void Entity::paintMotionTrack()
{
	int xCenter;
	int yCenter;

	glColor3ub(0, 0, 200); // blue

	// draw motion track
	for (int i = 1; i < Maze::MAX_MAP_SIZE; i++)
		for (int j = 1; j < Maze::MAX_MAP_SIZE; j++)
			if (traveled[i][j]) {
				xCenter = (j - 0.5) * Maze::BLOCK_SIZE;
				yCenter = (i - 0.5) * Maze::BLOCK_SIZE;
				DrawingUtilNG::drawRectangle(xCenter - 2, yCenter - 2, 4, 4, false);
			}
}
void Entity::paintSearchedCells()
{
	int xCenter;
	int yCenter;

	glColor3ub(220, 220, 120); // orange

	// shaded square where maze was visited in finding shortest path
	for (int i = 1; i < Maze::MAX_MAP_SIZE; i++)
		for (int j = 1; j < Maze::MAX_MAP_SIZE; j++)
			if (visited[i][j]) {
				xCenter = (j - 0.5) * Maze::BLOCK_SIZE;
				yCenter = (i - 0.5) * Maze::BLOCK_SIZE;
				DrawingUtilNG::drawRectangle(xCenter - 7, yCenter - 7, 14, 14, true);
			}
}

void Entity::paint()
{
	int xCenter = (locCol - 0.5) * Maze::BLOCK_SIZE;
	int yCenter = (locRow - 0.5) * Maze::BLOCK_SIZE;

	glColor3ub(0, 0, 255);  // blue

	// legs
	glBegin(GL_LINES);
	glVertex2i(xCenter - 8, yCenter - 5);
	glVertex2i(xCenter + 8, yCenter + 5);

	glVertex2i(xCenter + 8, yCenter - 5);
	glVertex2i(xCenter - 8, yCenter + 5);

	glVertex2i(xCenter + 8, yCenter - 0);
	glVertex2i(xCenter - 8, yCenter + 0);
	glEnd();

	// head
	glBegin(GL_LINE_LOOP);
	glVertex2i(xCenter, yCenter - 2);
	glVertex2i(xCenter - 3, yCenter - 6);
	glVertex2i(xCenter - 2, yCenter - 7);
	glVertex2i(xCenter + 2, yCenter - 7);
	glVertex2i(xCenter + 3, yCenter - 6);
	glEnd();

	// draw the shortest path using parent array, starting at end
	// and working back to beginning
	glColor3ub(150, 150, 0);  // umber
	Cell endCell = myMaze->getEnd();

	Cell currCell = parent[endCell.row][endCell.col];
	while (currCell.row != -1) {
		xCenter = (currCell.col - 0.5) * Maze::BLOCK_SIZE - 0;
		yCenter = (currCell.row - 0.5) * Maze::BLOCK_SIZE - 0;
		glTranslatef(xCenter, yCenter, 0);
		glRotatef(45.f, 0, 0, 1);
		DrawingUtilNG::drawRectangle(-5, -5, 10, 10, false);
		glRotatef(-45.f, 0, 0, 1);
		glTranslatef(-xCenter, -yCenter, 0);

		currCell = parent[currCell.row][currCell.col];
	}

	// map hiding
	if (theTorchShape != EVERYTHING) {

		// Draw a big grey blob, with a hole in the middle

		using namespace DrawingUtilNG;
		int wid, hei;
		FsGetWindowSize(wid, hei);  // get window size
		glColor3ub(200, 200, 200);

		if (theTorchShape == CIRCLE)
			drawCircle((locCol - 0.5) * Maze::BLOCK_SIZE, (locRow - 0.5) * Maze::BLOCK_SIZE,
				torchSize * Maze::BLOCK_SIZE, torchSize * Maze::BLOCK_SIZE,
				true, 2. * (wid + hei), 2. * (wid + hei));

		else if (theTorchShape == SQUARE)
			drawRectangle((locCol - torchSize / 2. - 0.5) * Maze::BLOCK_SIZE, (locRow - torchSize / 2. - 0.5) * Maze::BLOCK_SIZE,
				torchSize * Maze::BLOCK_SIZE, torchSize * Maze::BLOCK_SIZE,
				true, 2. * (wid + hei), 2. * (wid + hei));

	}
}

bool Entity::reachedGoal()
{
	if (myMaze != nullptr)
		return (myMaze->getEnd().row != -1
			&& locRow == myMaze->getEnd().row && locCol == myMaze->getEnd().col);
	else
		return false;

}

void Entity::celebrateGoal()
{
	using namespace DrawingUtilNG;

	int flashRate = 6;
	float radius = (celebrateCounter / flashRate + 2) * Maze::BLOCK_SIZE / 2;
	int xCenter = (locCol - 0.5) * Maze::BLOCK_SIZE; // the 0.5 centers on the block
	int yCenter = (locRow - 0.5) * Maze::BLOCK_SIZE;

	glColor3ub(255, 255, 0);
	glLineWidth(4);
	drawCircle(xCenter, yCenter, radius, radius * 1.2, false);
	drawCircle(xCenter, yCenter, radius * 1.5, radius * 1.8, false);
	glLineWidth(1);

	// increase counter and reset if needed
	if (++celebrateCounter > flashRate * 5)
		celebrateCounter = 0;
}

void Entity::findShortestPath()
{
	using namespace std;
	// parent array is a class member

	//bool visited[Maze::MAX_MAP_SIZE][Maze::MAX_MAP_SIZE]; // now a class variable so I can paint it
	Cell endCell = myMaze->getEnd();
	Cell currCell; // start location in current scene
	queue<Cell> processQueue; // open list

	// for cycling through adjacent cells
	int rowAdjust[] = { -1, 0, 1, 0 };
	int colAdjust[] = { 0, 1, 0, -1 };
	int testRow, testCol;

	clearShortestPath();
	//for (int i = 1; i < Maze::MAX_MAP_SIZE; i++)
	//	for (int j = 1; j < Maze::MAX_MAP_SIZE; j++) {
	//		visited[i][j] = false;
	//		parent[i][j] = { -1, -1 };
	//	}

	//Mark the root/start vertex as visited.
	visited[locRow][locCol] = true; // pushed in closed list, put in final path

	//	Add the root vertex to the process queue
	processQueue.push({ locRow,locCol });
	bool foundTheEnd = false;

	//	While the end vertex is yet to be discovered and there are vertices to process :
	while (processQueue.size() > 0 && !foundTheEnd) {
		//Get the current vertex at the head of the queue (removing it from the queue)
		currCell = processQueue.front();
		processQueue.pop();  // remove from queue

		//	If current vertex is the target vertex, we are done
		if (currCell.row == endCell.row && currCell.col == endCell.col)
			foundTheEnd = true;
		else {
			//	Else, for each of the adjacent vertices that have not been visited :

			for (int i = 0; i < 4; i++) {
				testRow = currCell.row + rowAdjust[i];
				testCol = currCell.col + colAdjust[i];

				// check visited and whether it is a valid neighbor
				if (!visited[testRow][testCol] && myMaze->isNavigable(testRow, testCol)) {
					//Set its parent to current vertex
					parent[testRow][testCol] = currCell;
					//	Mark it as visited
					visited[testRow][testCol] = true;
					//	Add it to the queue
					processQueue.push({ testRow,testCol });
				}
			}
		}
	}
	if (parent[endCell.row][endCell.col].row == -1)
		cout << "Could not find a path " << endl;


}


