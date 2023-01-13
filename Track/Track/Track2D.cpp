#include "DrawingUtilNG.h"
#include "fssimplewindow.h"
#include "Track2D.h"


void Track2D::paint(bool closed, bool filled, bool showPoints, float colorOverride)
{
	if (thePoints.size() >= 2) {
		double red, green, blue;
		DrawingUtilNG::hsv2rgb(colorHue, 1.f, 1.f, red, green, blue);

		glColor3d(red, green, blue);

		glLineWidth(4);
		glBegin(GL_LINES);
		glVertex2f(splinePoints.front().x, splinePoints.front().y);

		for (auto currPnt : splinePoints) {
			glVertex2f(currPnt.x, currPnt.y);
			glVertex2f(currPnt.x, currPnt.y);
		}
		glEnd();

		glLineWidth(1);

		// now draw the points (maybe)
		if (showPoints) {
			float pntSize;
			if (thePoints.size() <= 1)
				pntSize = 4.f;
			else
				// set pntSize to 10% of the average line segment length
				pntSize = 0.05 * perimeter() / thePoints.size();

			for (int i = 1; i <= thePoints.size(); i++) {
				paintPoint(i, pntSize);
			}
		}
	}
}

Point2D Track2D::getCoords(float givenLength)
{
	int backIndex;
	float fraction;
	getBackIndex(givenLength, backIndex, fraction);
	if (0 <= backIndex && backIndex < splinePoints.size() - 1) {
		return Line2D::scale(splinePoints[backIndex], splinePoints[backIndex + 1], fraction);
	}
	else 
		return { -INFINITY, -INFINITY };

}

float Track2D::getAngle(float givenLength)
{
	int backIndex;
	float fraction;
	getBackIndex(givenLength, backIndex, fraction);
	if (0 <= backIndex && backIndex < splinePoints.size() - 1) {
		return Line2D::getAngle(splinePoints[backIndex], splinePoints[backIndex + 1]);
	}
	else 
		return -INFINITY;

}

bool Track2D::addPoint(int index, float ratio)
{
	if (0. >= ratio || ratio >= 1.0 || index < 2 || index >= thePoints.size())
		return false;
	else {
		// insert a point on the spline some splinePoints back from
		int pointsBack = splineSubdivisions / 2;
		int i = 0;
		auto targetPnt = thePoints.at(index - 1);
		double tolerance = 1e-5;
		while (i < splinePoints.size() && (abs(splinePoints.at(i).x - targetPnt.x) > tolerance
			|| abs(splinePoints.at(i).y - targetPnt.y) > tolerance) ) {
			i+= splineSubdivisions;
		}
		if (pointsBack < i && i <= splinePoints.size()) {
			Shape2D::addPoint(splinePoints.at(i - pointsBack), index);
			//recalcShape();  // addPoint will do it anyway
			return true;
		}
		else
			return false;
	}
	return false;
}

void Track2D::getBackIndex(float givenLength, int& backIndex, float& fraction)
{
	if (splinePoints.size() > 1) {
		float lengthAtBackPoint = 0;
		float lengthAtForwardPoint = Line2D::getLength(splinePoints[0], splinePoints[1]);
		lengthAtBackPoint = 0;
		backIndex = 0;
		while (lengthAtForwardPoint < givenLength && backIndex < splinePoints.size()-1) {
			lengthAtBackPoint = lengthAtForwardPoint;
			lengthAtForwardPoint += Line2D::getLength(splinePoints[backIndex], splinePoints[backIndex + 1]);
			backIndex++;
		}
		fraction = (givenLength - lengthAtBackPoint) / (lengthAtForwardPoint - lengthAtBackPoint);
	}
	else {
		backIndex = -1;
		fraction = -1;
	}
}

void Track2D::recalcShape()
{
	Shape2D::recalcShape();
	perim -= Line2D::getLength(thePoints.front(), thePoints.back());
	recalcSpline();
}

void Track2D::recalcSpline()
{
	splinePoints.clear();

	// now get the distances
	if (thePoints.size() > 2) {

		if (thePoints.size() >= 4) {

			splinePoints.push_back(thePoints.at(1));
			// first and last guide points are for sloping the first part of curve
			for (int i = 1; i < thePoints.size() - 2; i++) {
				// generate spline points
				segmentPoints(i);
			}

			// now get the length
			length = 0;
			for (int i = 1; i < splinePoints.size(); i++) 
				length += Line2D::getLength(splinePoints[i], splinePoints[i - 1]);
		}
	}
}

void Track2D::segmentPoints(int segIndex)
{
	float c = splineTension; // spline tension (0.0 means no spline at all)

	auto prev = thePoints[segIndex - 1];
	auto curr = thePoints[segIndex];
	auto next = thePoints[segIndex + 1];
	auto nextNext = thePoints[segIndex + 2];


	float xa = -c * prev.x + (2. - c) * curr.x
		+ (c - 2.) * next.x + c * nextNext.x;
	float xb = 2. * c * prev.x + (c - 3.) * curr.x
		+ (3. - 2. * c) * next.x - c * nextNext.x;
	float xc = -c * prev.x + c * next.x;
	float xd = curr.x;

	float ya = -c * prev.y + (2. - c) * curr.y
		+ (c - 2.) * next.y + c * nextNext.y;
	float yb = 2. * c * prev.y + (c - 3.) * curr.y
		+ (3. - 2. * c) * next.y - c * nextNext.y;
	float yc = -c * prev.y + c * next.y;
	float yd = curr.y;

	// add the spline points

	//splinePoints.push_back(curr);
	float t, x, y;
	for (int k = 1; k <= splineSubdivisions; k++) {
		t = float(k) / splineSubdivisions;  // parameter
		x = xa * t * t * t + xb * t * t + xc * t + xd;
		y = ya * t * t * t + yb * t * t + yc * t + yd;

		splinePoints.push_back({ x, y });
	}

}
