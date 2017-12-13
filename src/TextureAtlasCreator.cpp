//
//  TextureAtlasCreator.cpp
//  BaseApp
//
//  Created by Oriol Ferrer Mesià on 18/03/15.
//
//

#include "TextureAtlasCreator.h"
#include "TextureAtlasDrawer.h"

using namespace std;

TextureAtlasCreator::TextureAtlasCreator(){

	state = IDLE;
	currentAtlas = NULL;
	loadingAtlas = 0;
	numImagesPerUpdate = 2;
}


float TextureAtlasCreator::getPercentDone(){

	float ret = 0.0f;
	if(state == CREATING){
		return float(currentFile) / float(fileList.size());
	}else
	if (state == LOADING){
		for(int i = 0; i < atlases.size(); i++){
			ret += atlases[i]->getLoadXmlProgress() / float(atlases.size());
		}
	}
	return ret;
}


string TextureAtlasCreator::getCurrentCreatingFileName(){
	if(state == CREATING){
		if(currentFile >= 0 && currentFile < fileList.size()){
			return fileList[currentFile];
		}
	}
	return "";
}


bool TextureAtlasCreator::registerWithManager(TextureAtlasDrawer& manager){

	if (state == IDLE){
		for(int i = 0; i < atlases.size(); i++){
			manager.addContent(atlases[i]);
		}
		return true;
	}else{
		ofLogError("TextureAtlasCreator") << "cant do now, busy!";
	}
	return false;
}


vector<string> TextureAtlasCreator::getAllImagePaths(){

	vector<string> list;
	if (state == IDLE){
		for(int i = 0; i < atlases.size(); i++){
			auto all = atlases[i]->getTextureLocations();
			auto it = all.begin();
			while(it != all.end()){
				list.push_back(it->first);
				++it;
			}
		}
	}
	return list;
}


ofTexture * TextureAtlasCreator::getCurrentlyCreatedAtlas(){
	if(state == CREATING){
		if(currentAtlas){
			return &currentAtlas->getFbo().getTexture();
		}
	}
	return NULL;
}

void TextureAtlasCreator::createAtlases(vector<string> fileList_,
										int fboSize_,
										GLint internalFormat_,
										ofVec2f maxImageSize_,
										float padding_,
										bool generateMipMaps,
										float mipmapBias_){

	if(state == IDLE){

		ofAddListener(ofEvents().update, this, &TextureAtlasCreator::update);
		fboSize = fboSize_;
		internalFormat = internalFormat_;
		maxItemSideSize = maxImageSize_;
		padding = padding_;
		makeMipMaps = generateMipMaps;
		mipmapBias = mipmapBias_;

		fileList = fileList_;

		state = CREATING;
		currentFile = 0;

		ofLogNotice("TextureAtlasCreator") << "Starting Atlases for " << fileList.size() <<
		" images in FBOs of " << fboSize << " x " << fboSize << "\nand resizing images to a max size of " <<
		maxItemSideSize << " and padding of " << padding << " px.";

		currentAtlas = new TextureAtlas();
		currentAtlas->setup(fboSize, padding, internalFormat);
		atlases.push_back(currentAtlas);

	}else{
		ofLogError("TextureAtlasCreator") << "I am busy now, cant do that!";
	}
}

string TextureAtlasCreator::getMemStats(){

	uint64_t pixelsInGPU = 0;

	for(int i = 0; i < atlases.size(); i++){
		ofFbo & fbo = atlases[i]->getFbo();
		if(atlases[i]->getFbo().isAllocated()){
			int bpp = 3;
			#if OF_VERSION_MINOR < 9
			switch (fbo.getTextureReference().getTextureData().glTypeInternal) {
			#else
			switch (fbo.getTexture().getTextureData().glInternalFormat) {
			#endif
				case GL_RGBA: bpp = 4; break;
				case GL_RGB: bpp = 3; break;
				case GL_RGBA8: bpp = 4; break;
				case GL_RGB8: bpp = 3; break;
				case GL_LUMINANCE: bpp = 1; break;
				default: ofLogError("TextureAtlasCreator") << "unknown glTypeInternal when gathering stats!";break;
			}

			pixelsInGPU += fbo.getWidth() * fbo.getHeight() * bpp * float(atlases[i]->loadWithMipmaps ? 1.333f : 1.0f);
		}
	}
	string msg = "Used VRAM: " + bytesToHumanReadable(pixelsInGPU,2);
	return msg;
}


string TextureAtlasCreator::bytesToHumanReadable(long long bytes, int decimalPrecision){
	string ret;
	if (bytes < 1024 ){ //if in bytes range
		ret = ofToString(bytes) + " bytes";
	}else{
		if (bytes < 1024 * 1024){ //if in kb range
			ret = ofToString(bytes / float(1024), decimalPrecision) + " KB";
		}else{
			if (bytes < (1024 * 1024 * 1024)){ //if in Mb range
				ret = ofToString(bytes / float(1024 * 1024), decimalPrecision) + " MB";
			}else{
				ret = ofToString(bytes / float(1024 * 1024 * 1024), decimalPrecision) + " GB";
			}
		}
	}
	return ret;
}


void TextureAtlasCreator::update(ofEventArgs&){

	if(state == CREATING){

		int c = 0;
		bool done = false;
		
		while( !done && c < numImagesPerUpdate ){
			if(currentFile >= fileList.size()){

				bool ok = true;
				state = IDLE;
				ofNotifyEvent(eventAtlasCreationFinished, ok, this);
				ofLogNotice("TextureAtlasCreator") << "Done!";
				if(makeMipMaps){
					currentAtlas->generateMipMap();
					currentAtlas->setMipMapBias(mipmapBias);
				}
				currentAtlas = NULL;
				ofLogWarning("TextureAtlasCreator") << getMemStats();
				ofRemoveListener(ofEvents().update, this, &TextureAtlasCreator::update);
				done = true;
			}else{

				string file = fileList[currentFile];
				bool didFit = currentAtlas->addTexture(file, maxItemSideSize);
				if(!didFit){
					if(makeMipMaps){
						currentAtlas->generateMipMap();
						currentAtlas->setMipMapBias(mipmapBias);
					}
					currentAtlas = new TextureAtlas();
					currentAtlas->setup(fboSize, padding, internalFormat );
					atlases.push_back(currentAtlas);
					currentFile--;
					ofLogNotice("TextureAtlasCreator") << "Creating a new TextureAtlas (" << atlases.size() << ")";
				}
				currentFile++;
			}
			c++;
		}
	}
}


bool TextureAtlasCreator::loadAtlasesFromDisk(GLint internalFormat,
											  string directory,
											  string imgFormat /*png | jpg*/,
											  bool generateMipMaps,
											  float mipmapBias){

	if(state == IDLE){
		ofLogNotice("TextureAtlasCreator") << "Load Atlases From Disk - " << directory;
		bool exists = ofDirectory::doesDirectoryExist(directory);
		bool empty = true;
		if (exists) empty = ofDirectory::isDirectoryEmpty(directory);

		if (empty | !exists){
			ofLogError("TextureAtlasCreator") << "can't find the directory (or its empty) to load from: " << directory;
			return false;
		}
		state = LOADING;
		ofDirectory d;
		d.allowExt("xml");
		d.listDir(directory);
		loadingAtlas = d.numFiles();
		for(int i = 0; i < loadingAtlas; i++){
			TextureAtlas * atlas = new TextureAtlas();
			string xmlName = d.getPath(i);
			string imgName = ofFilePath::removeExt(xmlName);
			imgName += "." + imgFormat;
			ofAddListener(atlas->eventAtlasLoaded, this, &TextureAtlasCreator::onAtlasLoaded);
			bool loadOK = atlas->startLoadingFromDisk(internalFormat, imgName, xmlName, generateMipMaps, mipmapBias);

			if (loadOK){
				ofLogNotice("TextureAtlasCreator") << "loaded Atlas OK - " << xmlName;
				atlases.push_back(atlas);
			}else{
				ofLogError("TextureAtlasCreator") << "Failed to load Atlas! - " << xmlName;
			}
		}
	}else{
		ofLogError("TextureAtlasCreator") << "I am busy now, cant do that!";
		return false;
	}
	return true;
}


	void TextureAtlasCreator::onAtlasLoaded(TextureAtlas::AtlasLoadEventInfo & info){

	if(state == LOADING){
		vector<string> files = info.atlas->getFiles();
		fileList.insert(fileList.end(),files.begin(), files.end());
		loadingAtlas--;
		if(loadingAtlas == 0){
			state = IDLE;
			bool ok = true;
			ofNotifyEvent(eventAllAtlasesLoaded, ok, this);
			for(int i = 0; i < atlases.size(); i++){
				ofRemoveListener(atlases[i]->eventAtlasLoaded, this, &TextureAtlasCreator::onAtlasLoaded);
			}
			ofLogWarning("TextureAtlasCreator") << getMemStats();
		}
	}else{
		ofLogError("TextureAtlasCreator") << "Hmmm not busy but getting loaded events?";
	}
}


void TextureAtlasCreator::saveToDisk(string directory, string imgFormat /*png | jpg | jpeg*/){

	if(state == IDLE){

		state = SAVING;
		if(imgFormat != "png" && imgFormat != "jpg" && imgFormat != "jpeg" ){
			ofLogError("TextureAtlasCreator") << "img format must be 'png' or 'jpg' or 'jpeg'";
		}

		directory = ofFilePath::addTrailingSlash(directory);
		ofDirectory::removeDirectory(directory, true); //remove all old atlases
		ofDirectory::createDirectory(directory, true, true);

		for(int i = 0; i < atlases.size(); i++){
			string imgName = directory + ofToString(i) + "." + imgFormat;
			string xmlName = directory + ofToString(i) + ".xml";
			atlases[i]->saveToDisk( imgName, xmlName );
			ofLogNotice() << "saving atlas as '" << imgName << "' and '" << xmlName << "'";
		}
		state = IDLE;
	}else{
		ofLogError("TextureAtlasCreator") << "I am busy now, cant do that!";
	}
}
