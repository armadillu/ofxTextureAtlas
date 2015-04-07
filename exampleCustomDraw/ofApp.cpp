#include "ofApp.h"
#include "ofxTimeMeasurements.h"

float p;
void ofApp::setup(){

	ofSetFrameRate(60);
	ofSetVerticalSync(true);
	ofEnableAlphaBlending();
	ofBackground(22);

	TIME_SAMPLE_ENABLE();
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

	RUI_SETUP();
	RUI_SHARE_PARAM(p, -100, 100);
	RUI_LOAD_FROM_XML();
}

void ofApp::onAtlasesLoaded(bool &){
	ofLogNotice() << "atlases loaded!";
	atlasCreator.registerWithManager(atlasManager);
	altasLoaded = true;

	//get all the files that were in all the atlas when they were created
	filesToDraw = atlasCreator.getAllImagePaths();

	//shuffle them around for the debug view
	//std::random_shuffle(filesToDraw.begin(), filesToDraw.end());
}

void ofApp::update(){
	float dt = 1./60.;
	scale = ofLerp(scale, scaleTarget, 0.15);
	
}


void ofApp::draw(){

	if(!altasLoaded){

		ofDrawBitmapStringHighlight("load Progress: " + ofToString(atlasCreator.getPercentDone() * 100,1), 30, 50);

	}else{

		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

		//atlasManager.drawTexture("images/cats/00000247_027.jpg", ofRectangle(ofGetMouseX(), ofGetMouseY(), 100, 100));

		float s = 400 * scale;
		float padding = 10 * scale;
		float offsetX = padding * 10;
		float offsetY = padding ;
		atlasManager.beginBatchDraw();

		//i want to draw 2500 tiles, my file list only has N, so lets repeat
		int nTimes = 1; ceil(2500.0f / filesToDraw.size());

		float slant = s * SLANT;

		for(int i = 0; i < nTimes; i++){
			for(string file : filesToDraw){
				TextureAtlasDrawer::TextureDimensions td = atlasManager.getTextureDimensions(file);
				ofRectangle r = ofRectangle(offsetX , offsetY, s * td.aspectRatio - 2 * slant , s );
				TextureAtlasDrawer::TexQuad tq = getParalelogramForRect(r);

				atlasManager.drawTextureInBatch(file, tq);
				offsetX += s * td.aspectRatio - 2 * slant  + padding;
				if(offsetX > ofGetWidth()){
					offsetX = 0;
					offsetY += s + padding;
				}
			}
		}

		ofSetColor(255);
		int numCats = atlasManager.endBatchDraw(debug); //draws! returns num tiles drawn

		ofDrawBitmapStringHighlight("numCats: " + ofToString(numCats), 30, 50);
        
	}
}


TextureAtlasDrawer::TexQuad ofApp::getParalelogramForRect(const ofRectangle & r){

	float slant = r.height * SLANT;
	float ar = r.width / r.height;

	TextureAtlasDrawer::TexQuad quad;
	quad.verts.tl = ofVec3f(r.x + slant, r.y);
	quad.verts.tr = ofVec3f(r.x + r.width + slant, r.y);
	quad.verts.br = ofVec3f(r.x + r.width - slant, r.y + r.height);
	quad.verts.bl = ofVec3f(r.x - slant, r.y + r.height);

	quad.texCoords.tl = ofVec2f((slant + p) / (r.width), 0);
	quad.texCoords.tr = ofVec2f(1, 0);
	quad.texCoords.br = ofVec2f((r.width - (p + slant )) / (r.width), 1);
	quad.texCoords.bl = ofVec2f(0, 1);
	return quad;
}


void ofApp::keyPressed(int key){

	if(key=='f') ofToggleFullscreen();
	else debug ^= true;
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

