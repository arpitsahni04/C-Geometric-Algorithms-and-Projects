#include <iostream>
#include <string>
#include "fssimplewindow.h"
#include "ysglfontdata.h"
#include "yspng.h"
#include "StringPlus.h"
#include "DrawingUtilNG.h"

#include "Track2D.h"
#include "SlideBox.h"

const double GRAVITY = 9.80665;  // this implies units are meters and seconds

SlideBox::SlideBox()
{
	// some default parameters for quick testing
	props["width"] = 2.;
	props["height"] = 1.5;
	props["mass"] = 3;   // actually does not affect calculations (cancels out)
	props["mu"] = 0.1;   // coeff of friction b/w box and track
	props["initDist"] = 0;  // at start of track
	props["initVel"] = 0;   // starts from rest
	props["initAccel"] = 0; // no push

	theTrack = nullptr;
	// load texture data
	YsRawPngDecoder pngTemp;
	pngTemp.Decode("woodplank.png");
	//pngTemp.Decode("ComicSansFont01.png");

	glGenTextures(1, &woodTextureID);		// Reserve one texture identifier, DON'T LOSE THE TEXTURE ID
	glBindTexture(GL_TEXTURE_2D, woodTextureID);	// Making the texture identifier current (or bring it to the deck)

	// set up parameters for the current texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D
	(GL_TEXTURE_2D,
		0,	// Level of detail
		GL_RGBA,	// the "A" in RGBA will include the transparency
		pngTemp.wid,	// the image width and height
		pngTemp.hei,
		0,  // Border width, but not supported and needs to be 0.
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		pngTemp.rgba);

	reset();
}

void SlideBox::loadFromConsole()
{
	using namespace std;

	cout << endl << "Enter parameters for adding a slide box to model:" << endl;
	props["width"] =	StringPlus::getDouble(cin, "                               Box width (m) >> ");
	props["height"] =	StringPlus::getDouble(cin, "                              Box height (m) >> ");
	props["mass"] =		StringPlus::getDouble(cin, "                               Box mass (kg) >> ");
	props["mu"] =		StringPlus::getDouble(cin, "  Friction coeff b/w box & slider (unitless) >> ");

	double sliderLength = theTrack->getLength();
	if (sliderLength <= 0)
		props["initDist"] = StringPlus::getDouble(cin, "           Initial position along slider (m) >> ");
	else
		props["initDist"] = StringPlus::getDouble(cin, "Initial position along slider (0 to "
			+ StringPlus::sigFig(sliderLength, 4) + " m) >> ");

	props["initVel"] =		StringPlus::getDouble(cin, "                      Initial velocity (m/s) >> ");
	props["initAccel"] =	StringPlus::getDouble(cin, "                Initial acceleration (m/s^2) >> ");

	reset();
}

void SlideBox::move(double deltaT)
{
	if (theTrack != nullptr && theTrack->getLength() > 0) {
		// moves the box by applying currVel for deltaT
		// should I include contribution of accel to change in position?
		// sure, but the deltaT^2 term makes only a small difference
		currDist += currVel * deltaT + 0.5 * currAccel * deltaT * deltaT;

		// recalculates currVel by applying currAccel for deltaT
		currVel += currAccel * deltaT;

		// recalculates currAccel based on dynamic equilibrium
		// should I remove mass since it cancels out??? Probably
		float currAngle = theTrack->getAngle(currDist);
		if (currAngle > -INFINITY)
			currAngle *= atan(1.) / 45.; // converted to radians
		else
			currAngle = 0.;
		double normal = /*props["mass"] * */GRAVITY * cos(currAngle);
		double friction = props["mu"] * normal;
		if (currVel < 0)
			friction = -friction;

		// solve for "a": -friction – mg * sin (theta) = mass * a
		currAccel = -(friction + /*props["mass"] * */ GRAVITY * sin(currAngle)) /* / props["mass"] */;

		if (currDist > theTrack->getLength() || currDist < 0)
			reset();
	}
}

void SlideBox::paint()
{
	Point2D currPos;
	float currAngle;
	double Ax, Ay, Bx, By, Cx, Cy, Dx, Dy;

	if (theTrack == nullptr || theTrack->getLength() <= 0) {
		currPos.x = 5;
		currPos.y = 15;
		currAngle = 0;
	}
	else {
		currPos = theTrack->getCoords(currDist);
		currAngle = theTrack->getAngle(currDist);
	}

	// transform so that I can paint the box in "local" coordinates
	glTranslatef(currPos.x, currPos.y, 0);
	glRotatef(currAngle, 0, 0, 1);

	double width = props["width"], height = props["height"]; // not faster, just more convenient for code

	// do the texture thing here
	// enable textures
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glEnable(GL_TEXTURE_2D);

	// bind the texture ID
	glBindTexture(GL_TEXTURE_2D, woodTextureID);

	// set color "tint"
	//	glColor4d(1.0, 1.0, 1.0, 1.0);  // this color will "tint" the image

	// draw a quad with the texture coords too
	glBegin(GL_QUADS);

	// For each vertex, assign texture coordinate before vertex coordinate.
	glTexCoord2d(0.0, 0.0);  // these are "percentage" of the image
	glVertex2f(-width / 2, height);

	glTexCoord2d(1.0, 0.0);
	glVertex2f(width / 2, height);

	glTexCoord2d(1.0, 1.0);
	glVertex2f(width / 2, 0);

	glTexCoord2d(0.0, 1.0);
	glVertex2f(-width / 2, 0);

	glEnd();

	// disable textures
	glDisable(GL_TEXTURE_2D);

	// draw in local coords
	glColor3ub(0, 0, 0); // black
	glLineWidth(3);
	DrawingUtilNG::drawRectangle(-width / 2., 0, width, height, false);
	glBegin(GL_LINES);
	glVertex2d(-width / 2, 0.);
	glVertex2d(width / 2, height);  // diagonal
	glVertex2d(width / 2, 0.);
	glVertex2d(-width / 2, height); // diagonal
	glEnd();
	glLineWidth(1);

	//transform back
	glRotatef(-currAngle, 0, 0, 1);
	glTranslatef(-currPos.x, -currPos.y, 0);

}

string SlideBox::getDynamicData()
{
	string theData;
	theData = "Pos: " + StringPlus::sigFig(currDist, 4);
	theData += "   Vel: " + StringPlus::sigFig(currVel, 4);
	theData += "   Accel: " + StringPlus::sigFig(currAccel, 4);
	return theData;
}

string SlideBox::getStaticData()
{
	string theData;
	theData = "Friction: " + StringPlus::sigFig(props["mu"], 4);
	theData += "   InitPos: " + StringPlus::sigFig(props["initDist"], 4) + "m";
	theData += "   InitVel: " + StringPlus::sigFig(props["initVel"], 4) + "m/s";
	theData += "   InitAccel: " + StringPlus::sigFig(props["initAccel"], 4) + "m/s^2";
	return theData;
}

std::ostream& operator<<(std::ostream& os, const SlideBox& aBox)
{
	auto oldPrecision = os.precision();
	os.precision(3);

	std::string spacer;
	if (&os == &std::cout) 
		spacer = ", "; // if outputting to console, use commas and keep on same line
	else
		spacer = "\n"; // if outputting to file, put each property on separate line
	
	// print properties in prescribed order
	std::vector<std::string> printOrder =
	{ "width", "height", "mass", "mu", "initDist", "initVel", "initAccel" };

	for (int i = 0; i < aBox.props.size(); i++) {
		if (aBox.props.find(printOrder[i]) != aBox.props.end()) { // if property is in map

			if (i > 0) os << spacer;
			os << printOrder[i] << "=" << aBox.props.at(printOrder[i]);// [] doesn't work here
		}
	}

	// print properties in alphabetical order (directly from map)
	// int i = 0;
	//for (auto& item : aBox.props) {
	//  if (i++>0) os << spacer;
	//	os << item.first << "=" << item.second;
	//}

	os.precision(oldPrecision);
	return os;
}
