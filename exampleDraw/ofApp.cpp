#include "ofApp.h"


void ofApp::setup(){

	ofSetFrameRate(60);
	ofSetVerticalSync(true);
	ofEnableAlphaBlending();
	ofBackground(22);

	scale = scaleTarget = 1.0;

	// LOAD ATLAS /////////////////////////////////////////////////

	altasLoaded = false;
	ofAddListener(atlasCreator.eventAllAtlasesLoaded, this, &ofApp::onAtlasesLoaded);
	atlasCreator.loadAtlasesFromDisk(GL_RGBA, //internal format
									 "textureCache", //dir
									 "png", //image format
									 true, //gen mipmaps
									 -0.9 //mipmap bias
									 );

	////////////////////////////////////////////////////////
}

void ofApp::onAtlasesLoaded(bool &){
	ofLogNotice() << "atlases loaded!";
	atlasCreator.registerWithManager(atlasManager);
	altasLoaded = true;

	//get all the files that were in all the atlas when they were created
	filesToDraw = atlasCreator.getAllImagePaths();

	//shuffle them around for the debug view
	std::random_shuffle(filesToDraw.begin(), filesToDraw.end());
}

void ofApp::update(){
	float dt = 1./60.;
	scale = ofLerp(scale, scaleTarget, 0.15);
}


void ofApp::draw(){

	if(!altasLoaded){

		ofDrawBitmapStringHighlight("load Progress: " + ofToString(atlasCreator.getPercentDone() * 100,1), 30, 50);

	}else{

		//atlasManager.drawTexture("images/cats/00000247_027.jpg", ofRectangle(ofGetMouseX(), ofGetMouseY(), 100, 100));

		float s = 250 * scale;
		float offsetX = 0;
		float offsetY = 0;
		atlasManager.beginBatchDraw();

		//i want to draw 2500 tiles, my file list only has N, so lets repeat
		int nTimes = ceil(2500.0f / filesToDraw.size());

		for(int i = 0; i < nTimes; i++){
			for(string file : filesToDraw){
				TextureAtlasDrawer::TextureDimensions td = atlasManager.getTextureDimensions(file);
				ofRectangle r = ofRectangle(offsetX , offsetY, s * td.aspectRatio, s );
				atlasManager.drawTextureInBatch(file, r);
				offsetX += s * td.aspectRatio;
				if(offsetX > ofGetWidth()){
					offsetX = 0;
					offsetY += s;
				}
			}
		}

		ofSetColor(255);
		int numCats = atlasManager.endBatchDraw(debug); //draws! returns num tiles drawn

		ofDrawBitmapStringHighlight("numCats: " + ofToString(numCats), 30, 50);
	}
}

void ofApp::keyPressed(int key){

	debug ^= true;
}

void ofApp::mouseScrolled( float x, float y ){

	scaleTarget += y * 0.02;
	scaleTarget = ofClamp(scaleTarget, 0.02, 1.0);
}

void ofApp::keyReleased(int key){

}


void ofApp::mouseMoved(int x, int y ){

}


void ofApp::mouseDragged(int x, int y, int button){

}


void ofApp::mousePressed(int x, int y, int button){

}


void ofApp::mouseReleased(int x, int y, int button){

}


void ofApp::windowResized(int w, int h){

}


void ofApp::gotMessage(ofMessage msg){

}


void ofApp::dragEvent(ofDragInfo dragInfo){
	
}

