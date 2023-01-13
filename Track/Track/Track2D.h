#pragma once
/*
arpit sahni
*/


#include "Shape2D.h"

class Track2D : public Shape2D {

protected:
	std::vector<Point2D> splinePoints;
	int splineSubdivisions = 20;
	float splineTension = 0.8;
	float length = 0;

public:
	Track2D() {};

	Track2D(std::ifstream inFile) : Shape2D(inFile) {};

	// over-ride
	void paint(bool closed = true, bool filled = false,
		bool showPoints = false, float colorOverride = -1.f);

	float getLength() { return length; }

	// measured along spline
	Point2D getCoords(float givenLength); 
	
	// measured from back point to forward point
	float getAngle(float givenLength);

	// over-ride (don't need to over-ride the other version of this function)
	bool isContained(float testX, float testY, bool isInclusive = true) {
		return true;
	}

	// over-ride
	bool addPoint(int index, float ratio);

protected:
	// returns the index of the point that is just before the givenLength
	// and the length along track for that back point
	void getBackIndex(float givenLength, int &backIndex, float& lengthAtBackPoint);
	
	// over-ride
	bool selfIntersects() { return false; };

	// over-ride
	void recalcShape();

	void recalcSpline();
	void segmentPoints(int segIndex);
};