#pragma once

#include "ofMain.h"
#include "TextureAtlasCreator.h"

class ofApp : public ofBaseApp{

public:
	void setup();
	void update();
	void draw();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y );
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseScrolled( float x, float y );
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);


	// APP CALLBACKS ////////////////////////////////////////

	void onAtlasCreationFinished(bool & arg);


	// APP SETUP ////////////////////////////////////////////

	ofRectangle *packer;

	vector<ofFbo> fbos;
	vector<ofRectangle>  rectangles;
	vector< vector<ofRectangle> > rectsPerFbo;

	TextureAtlasCreator atlasCreator;

	bool debug;
	float scale;

};
