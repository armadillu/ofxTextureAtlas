#include "ofApp.h"


void ofApp::setup(){

	ofSetFrameRate(60);
	ofSetVerticalSync(true);
	ofEnableAlphaBlending();
	ofBackground(22);

	scale = 1.0;

	// LISTENERS
	ofAddListener(atlasCreator.eventAtlasCreationFinished, this, &ofApp::onAtlasCreationFinished);

	vector<string> fileList;
	ofDirectory d;
	d.allowExt("jpg");
	d.listDir("images/cats"); //you are expected to put a ton of images in there.
	d.sort();
	for(int i = 0; i < d.numFiles(); i++){
		fileList.push_back(d.getPath(i));
	}

	fileList.resize(MIN(500, fileList.size())); //lets limit the file list to 500


	// CREATE ATLAS (+ callback to save) //////////////////////////

	//create as many atlases as required to fit all those images in atlases of 4096 x 4096
	//each image being 256 at the most.
	atlasCreator.createAtlases(	fileList,
							   	4096, 	//fbo/atlas size
							   	GL_RGB, //internal format
							   	ofVec2f(400,400), //maxItemSideSize
								2.0,	//padding
							    true, //mipmaps
							   	-0.9 //mipmap bias
							   );

}

void ofApp::onAtlasCreationFinished(bool & arg){
	//once atlases are created, save them to disk, so that we can load them next time
	//without having to recreate them
	atlasCreator.saveToDisk("textureCache", "png"); //save in a folder named "textureCache", in png format
}


void ofApp::update(){
	float dt = 1./60.;

}

void ofApp::draw(){

	ofDrawBitmapString("Progress: " + ofToString(atlasCreator.getPercentDone() * 100,1), 30, 50);

	if(atlasCreator.isCreating()){
		//atlass construction process
		ofTexture * atlas = atlasCreator.getCurrentlyCreatedAtlas();
		atlas->draw(0,0, ofGetHeight(), ofGetHeight());
		ofDrawBitmapStringHighlight("Creating Atlas: " + ofToString(atlasCreator.getPercentDone(), 2) +
									"\n" + atlasCreator.getCurrentCreatingFileName(),
									30, 30
									);

	}
	if(atlasCreator.isIdle() ){
		int x = 0;
		for(int i = 0;  i < atlasCreator.getNumAtlases(); i++){
			ofFbo & fbo = atlasCreator.getAtlasAtIndex(i)->getFbo();
			fbo.draw(ofGetMouseX() + x, ofGetMouseY(), fbo.getWidth() * scale, fbo.getHeight() * scale);
			x += fbo.getWidth() * scale;
		}
	}

	if(debug && atlasCreator.isIdle()){
		ofClear(0);
		int x = 0;
		for(int i = 0;  i < atlasCreator.getNumAtlases(); i++){
			atlasCreator.getAtlasAtIndex(i)->drawDebug(ofGetMouseX() + x,ofGetMouseY());
			x += atlasCreator.getAtlasAtIndex(i)->getFbo().getWidth();
		}
	}


}

void ofApp::keyPressed(int key){

	debug ^= true;
}

void ofApp::mouseScrolled( float x, float y ){

	scale += y * 0.02;
	scale = ofClamp(scale, 0.02, 1.0);
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

