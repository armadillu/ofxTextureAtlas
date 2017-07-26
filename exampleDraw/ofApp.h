#pragma once

#include "ofMain.h"
#include "TextureAtlasCreator.h"
#include "TextureAtlasDrawer.h"

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
	void mouseScrolled( int x, int y, float scrollX, float scrollY );
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);


	// APP CALLBACKS ////////////////////////////////////////

	void onAtlasesLoaded(bool &);


	// APP SETUP ////////////////////////////////////////////


	TextureAtlasCreator atlasCreator;
	TextureAtlasDrawer atlasManager;

	bool debug;
	float scale;
	float scaleTarget;

	bool altasLoaded;

	vector<string> filesToDraw;

};
