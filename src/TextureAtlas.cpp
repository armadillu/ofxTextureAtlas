//
//  TextureAtlas.cpp
//  BaseApp
//
//  Created by Oriol Ferrer Mesià on 18/03/15.
//
//

#include "TextureAtlas.h"


TextureAtlas::TextureAtlas(){

	mipmapBias = 0.0f;
	loadXmlProgress = 0.0f;
	fboSizeInXml = numTexInXml = 0;
	dataReadyToLoadImage = false;
	loadingAtlas = false;
	xmlDataOK = imgDataOK = false;

}

void TextureAtlas::setup(int rectSize, float padding, GLint internalFormat_){

	internalFormat = internalFormat_;
	ofRectangle bounds = ofRectangle(0,0, rectSize, rectSize);
	packer = new ofRectanglePacker(bounds, padding);

	isFull = false;

	ofFbo::Settings fboSettings;
	fboSettings.width = rectSize;
	fboSettings.height = rectSize;
	fboSettings.useDepth = false;
	fboSettings.textureTarget = GL_TEXTURE_2D; //mipmaps!
	fboSettings.internalformat = internalFormat;
	fboSettings.numSamples = 0; //?  >0 wont draw the fbo. 0 seems to get u multisample anyway? TODO!

	atlasFbo.allocate(fboSettings);
	atlasFbo.begin();
	ofClear(0,0,0, (internalFormat_ == GL_RGBA ? 0 : 255));
	atlasFbo.end();

	xmlDataOK = imgDataOK = false;

}


bool TextureAtlas::addTexture(string file, float maxSize){

	if(isFull){
		ofLogError("TextureAtlas") << "This Atlas is Full! Can't add more textures";
		return false;
	}

	ofTexture tex;
	tex.enableMipmap();
	ofDisableArbTex();
	bool loadOK = ofLoadImage(tex, file);
	ofEnableArbTex();

	if(loadOK){
		ofRectangle rect;
		rect.width = tex.getWidth();
		rect.height = tex.getHeight();

		if(rect.width > rect.height){
			float scaleDown = maxSize / rect.width;
			if (maxSize < rect.width){ //dont upscale
				rect.width = maxSize;
				rect.height *= scaleDown;
			}else{
				ofLogNotice("TextureAtlas") << "not upscaling " << file;
			}
		}else{
			float scaleDown = maxSize / rect.height;
			if(maxSize < rect.height){
				rect.height = maxSize;
				rect.width *= scaleDown;
			}else{
				ofLogNotice("TextureAtlas") << "not upscaling " << file;
			}
		}
		rect.width = ceil(rect.width);
		rect.height = ceil(rect.height);

		bool didFit = packer->pack(rect);

		if(didFit){
			textureCrops[file] = rect;
			atlasFbo.begin();
				tex.draw(rect);
			atlasFbo.end();
			ofLogVerbose("TextureAtlas") << "Added Texture " << file << " to atlas";
		}else{
			isFull = true;
			delete packer;
			packer = NULL;
			//ofLogNotice("TextureAtlas") << "No Space for Texture " << file << " in atlas";
		}

		return didFit;
	}else{
		ofLogError("TextureAtlas") << "Error loading texture: " << file;
		return false;
	}
}

void TextureAtlas::generateMipMap(){
	atlasFbo.getTexture().generateMipmap();
	atlasFbo.getTexture().setTextureMinMagFilter(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
}

void TextureAtlas::setMipMapBias(float bias){
	atlasFbo.getTexture().bind();
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, bias);
	atlasFbo.getTexture().unbind();
}

bool TextureAtlas::saveToDisk(string imageFileName, string xmlFileName){

	ofLogNotice("TextureAtlas") << "Saving atlas To Disk " << imageFileName << " in atlas";
	ofPixels p;
	atlasFbo.readToPixels(p);
	ofImage img;
	img.setFromPixels(p);
	img.save(imageFileName, OF_IMAGE_QUALITY_BEST);

	ofXml xml;
	xml.addChild(atlasXmlRoot);
	xml.setTo(atlasXmlRoot);

	xml.addChild(atlasXmlProperties);
	xml.setTo(atlasXmlProperties);
		xml.addValue(atlasXmlSize, atlasFbo.getWidth());
		xml.addValue(atlasXmlNumTex, textureCrops.size());
	xml.setToParent();

	xml.addChild(atlasXmlTextureList);
	xml.setTo(atlasXmlTextureList);

	map<string, ofRectangle>::iterator it = textureCrops.begin();
	int c = 0;
	while(it != textureCrops.end()){

		string file = it->first;
		ofRectangle & r = it->second;

		string path = "tex[" + ofToString(c) + "]";
		xml.addValue("tex", file);
		xml.setTo(path);
		xml.setAttribute("x", ofToString(r.x));
		xml.setAttribute("y", ofToString(r.y));
		xml.setAttribute("width", ofToString(r.width));
		xml.setAttribute("height", ofToString(r.height));

		xml.setToParent();
		c++;
		++it;
	}
	xml.save(xmlFileName);
}

bool TextureAtlas::startLoadingFromDisk(GLint internalFormat_,
										string imageFileName_,
										string xmlFileName_,
										bool mipmaps,
										float mipmapBias_){

	if(textureCrops.size() > 0){
		ofLogError("TextureAtlas") << "Cant load; This atlas already has content!!";
		return false;
	}

	if (imageFileName_.size() == 0 || xmlFileName_.size() == 0){
		ofLogError("TextureAtlas") << "Cant load! provide an image and a xml!";
		return false;
	}

	loadingAtlas = true;
	xmlDataOK = imgDataOK = false;
	ofAddListener(ofEvents().update, this, &TextureAtlas::update);

	imageFileName = imageFileName_;
	xmlFileName = xmlFileName_;
	internalFormat = internalFormat_;
	loadWithMipmaps = mipmaps;
	mipmapBias = mipmapBias_;

	textureCrops.clear();

	ofLogNotice("TextureAtlas") << "loading from disk " << imageFileName << ", " << xmlFileName;

	startThread(); //load xml in thread
	return true;
}


bool TextureAtlas::loadXmlData(){

	ofXml xml;
	bool xmlOK = xml.load(xmlFileName);
	if(xmlOK){
		xml.setTo(atlasXmlRoot);
		xml.setTo(atlasXmlProperties);
		int fboSize = ofToInt(xml.getValue(atlasXmlSize));
		int numTex = ofToInt(xml.getValue(atlasXmlNumTex));
		xml.setToParent();

		xml.setTo(atlasXmlTextureList);

		loadXmlProgress = 0;
		for(int i = 0; i < numTex; i++){
			xml.setToChild(i);
			string fileName = xml.getValue();
			float x = ofToFloat(xml.getAttribute("x"));
			float y = ofToFloat(xml.getAttribute("y"));
			float width = ofToFloat(xml.getAttribute("width"));
			float height = ofToFloat(xml.getAttribute("height"));
			textureCrops[fileName] = ofRectangle(x, y, width, height);
			xml.setToParent();
			loadXmlProgress = i / float(numTex - 1);
		}
		numTexInXml = numTex;
		fboSizeInXml = fboSize;
	}else{
		ofLogError("TextureAtlas") << "Cant load xml! "<< xmlFileName;
		return false;
	}
	return true;
}


bool TextureAtlas::loadImageData(){

	ofImage img;
	bool ok = img.load(imageFileName);
	if (ok){

		ofFbo::Settings fboSettings;
		fboSettings.width = fboSizeInXml;
		fboSettings.height = fboSizeInXml;
		fboSettings.useDepth = false;
		fboSettings.textureTarget = GL_TEXTURE_2D; //mipmaps!
		fboSettings.internalformat = internalFormat;
		fboSettings.numSamples = 0; //?  >0 wont draw the fbo. 0 seems to get u multisample anyway? TODO!
		atlasFbo.allocate(fboSettings);
		atlasFbo.begin();
		ofClear(0,0,0,0);
		img.draw(0, 0);
		atlasFbo.end();

		if(loadWithMipmaps){
			generateMipMap();
			setMipMapBias(mipmapBias);
		}
	}else{
		ofLogError("TextureAtlas") << "Cant load image! "<< imageFileName;
		return false;
	}
	return true;
}

void TextureAtlas::update(ofEventArgs&){

	if(loadingAtlas && !isThreadRunning() && dataReadyToLoadImage){
		dataReadyToLoadImage = false;
		ofRemoveListener(ofEvents().update, this, &TextureAtlas::update);
		if(xmlDataOK){
			imgDataOK = loadImageData();
			if(!imgDataOK){
				ofLogError()<< "image data cant be loaded!" << imageFileName;
			}
		}else{
			ofLogError()<< "xml data cant be loaded!" << xmlFileName;
		}

		bool arg = imgDataOK && xmlDataOK;
		ofNotifyEvent(eventAtlasLoaded, arg, this);
	}
}

void TextureAtlas::threadedFunction(){

	dataReadyToLoadImage = false;
	bool ok = loadXmlData();
	dataReadyToLoadImage = true;
	xmlDataOK = ok;

}

int TextureAtlas::getNumTextures(){
	return textureCrops.size();
}


void TextureAtlas::drawDebug(int x, int y){

	ofPushMatrix();
	ofTranslate(x, y);
	atlasFbo.draw(0,0);
	ofNoFill();
	map<string, ofRectangle>::iterator it = textureCrops.begin();
	while(it != textureCrops.end()){
		ofSetColor(ofRandom(255), ofRandom(255), ofRandom(255));
		ofDrawRectangle(it->second);
		++it;
	}
	ofFill();
	ofSetColor(255);
	ofPopMatrix();
}