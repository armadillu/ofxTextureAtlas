//
//  TextureAtlas.cpp
//  BaseApp
//
//  Created by Oriol Ferrer Mesià on 18/03/15.
//
//

#include "TextureAtlas.h"

using namespace std;

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


bool TextureAtlas::addTexture(string file, ofVec2f maxSize){

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
		
		ofRectangle maxRect = ofRectangle(0,0,maxSize.x, maxSize.y);

		if(rect.width > rect.height){
			float scaleDown = maxSize.x / rect.width;
			if (maxSize.x < rect.width){ //dont upscale
				rect.width = maxSize.x;
				rect.height *= scaleDown;
			}else{
				ofLogNotice("TextureAtlas") << "not upscaling \"" << file << "\" [" << tex.getWidth() << " x " << tex.getHeight() << "]";
			}
		}else{
			float scaleDown = maxSize.y / rect.height;
			if(maxSize.y < rect.height){
				rect.height = maxSize.y;
				rect.width *= scaleDown;
			}else{
				ofLogNotice("TextureAtlas") << "not upscaling \"" << file << "\" [" << tex.getWidth() << " x " << tex.getHeight() << "]";
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

void TextureAtlas::saveToDisk(string imageFileName, string xmlFileName){

	ofLogNotice("TextureAtlas") << "Saving atlas To Disk " << imageFileName << " in atlas";
	ofPixels p;
	atlasFbo.readToPixels(p);
	ofImage img;
	img.setFromPixels(p);
	img.save(imageFileName, OF_IMAGE_QUALITY_BEST);

	ofXml xml;
	auto ptr = xml.appendChild(atlasXmlRoot);	//create 	<Atlas>
	ptr = ptr.appendChild(atlasXmlProperties); // create	<Properties>

	ptr.appendChild(atlasXmlSize).set(atlasFbo.getWidth());
	ptr.appendChild(atlasXmlNumTex).set(textureCrops.size());

	ptr = xml.findFirst(atlasXmlRoot).appendChild(atlasXmlTextureList); //create 	<TextureList>

	map<string, ofRectangle>::iterator it = textureCrops.begin();

	while(it != textureCrops.end()){

		string file = it->first;
		ofRectangle & r = it->second;

		auto ptr2 = ptr.appendChild("tex");
		ptr2.set(file);
		ptr2.setAttribute("x", ofToString(r.x));
		ptr2.setAttribute("y", ofToString(r.y));
		ptr2.setAttribute("width", ofToString(r.width));
		ptr2.setAttribute("height", ofToString(r.height));
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
		auto ptr = xml.findFirst(atlasXmlRoot + "/" + atlasXmlProperties);
		int fboSize = ptr.getChild(atlasXmlSize).getIntValue();
		int numTex = ptr.getChild(atlasXmlNumTex).getIntValue();

		ptr = xml.findFirst(atlasXmlRoot + "/" + atlasXmlTextureList);

		loadXmlProgress = 0;

		auto texs = ptr.getChildren("tex");
		int i = 0;
		for(auto & tex: texs){
			string filename = tex.getValue();
			auto x = tex.getAttribute("x").getIntValue();
			auto y = tex.getAttribute("y").getIntValue();
			auto width = tex.getAttribute("width").getIntValue();
			auto height = tex.getAttribute("height").getIntValue();
			textureCrops[filename] = ofRectangle(x, y, width, height);
			loadXmlProgress = i / float(numTex - 1);
			i++;
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

		AtlasLoadEventInfo arg;
		arg.ok = imgDataOK && xmlDataOK;
		arg.atlas = this;
		ofNotifyEvent(eventAtlasLoaded, arg, this);
	}
}

const vector<string> TextureAtlas::getFiles(){

	vector<string> files;
	map<string, ofRectangle>::iterator it = textureCrops.begin();
	while(it != textureCrops.end()){
		files.push_back(it->first);
		++it;
	}
	return files;
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
