#pragma once
/*
arpit sahni
*/

#include <map>

class Track2D;  // needed because Track.h may include SlideBox.h later

class SlideBox {
protected:
	// add whatever else you want
	// don't use (but don't delete) what you don't want 

	std::string label; // needed ?

	// using a map instead of individual member variables allows you to have
	// as many properties of type double as you want (width, height, initDist, etc)
	// see examples of how to use it below
	std::map<std::string, double> props;   

	double currDist, currVel, currAccel;
	bool showParameters = false;  // needed?

	Track2D* theTrack; // pointer so that many slidebox objects can point to same Track

	GLuint woodTextureID;

public:
	SlideBox();
	SlideBox(Track2D* aTrack) : SlideBox() { theTrack = aTrack; }

	double getProp(std::string wanted) {
		if (props.find(wanted) != props.end())
			return props.at(wanted);
		else
			return -INFINITY;
	}

	void reset() {
		currDist = props["initDist"];
		currVel = props["initVel"];
		currAccel = props["initAccel"];
	}

	void loadFromConsole();

	// obvious, but needed?
	void toggleShowParameters() { showParameters = !showParameters; }

	// moves the box by applying currVel for deltaT
	// recalculates currVel by applying currAccel for deltaT
	// recalculates currAccel based on dynamic equilibrium
	void move(double deltaT);

	// draws the box at current location and orientation
	void paint();

	// outputs a single line that describes the parameters of the slidebox
	// not required for PS09
	std::string getDynamicData();	
	std::string getStaticData();

	// outputs a single line that describes the static parameters of the slidebox
	// may be useful when asking users to add or delete boxes 
	friend std::ostream& operator<<(std::ostream& os, const SlideBox& aBox);
};