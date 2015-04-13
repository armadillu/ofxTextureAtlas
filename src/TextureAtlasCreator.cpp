//
//  TextureAtlasCreator.cpp
//  BaseApp
//
//  Created by Oriol Ferrer Mesià on 18/03/15.
//
//

#include "TextureAtlasCreator.h"
#include "TextureAtlasDrawer.h"

TextureAtlasCreator::TextureAtlasCreator(){

	state = IDLE;
	currentAtlas = NULL;
	loadingAtlas = 0;
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
			map<string, ofRectangle> all = atlases[i]->getTextureLocations();
			map<string, ofRectangle>::iterator it = all.begin();
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
										float maxItemSideSize_,
										float padding_,
										bool generateMipMaps,
										float mipmapBias_){

	if(state == IDLE){

		ofAddListener(ofEvents().update, this, &TextureAtlasCreator::update);
		fboSize = fboSize_;
		internalFormat = internalFormat_;
		maxItemSideSize = maxItemSideSize_;
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


void TextureAtlasCreator::update(ofEventArgs&){

	if(state == CREATING){

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
			ofRemoveListener(ofEvents().update, this, &TextureAtlasCreator::update);

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
	}
}


void TextureAtlasCreator::loadAtlasesFromDisk(GLint internalFormat,
											  string directory,
											  string imgFormat /*png | jpg*/,
											  bool generateMipMaps,
											  float mipmapBias){

	if(state == IDLE){
		ofLogNotice("TextureAtlasCreator") << "Load Atlases From Disk - " << directory;
		state = LOADING;
		ofDirectory d;
		d.allowExt("xml");
		d.listDir(directory);
		loadingAtlas = d.numFiles();
		for(int i = 0; i < d.numFiles(); i++){
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
	}
}


void TextureAtlasCreator::onAtlasLoaded(bool & ok){

	if(state == LOADING){
		loadingAtlas--;
		if(loadingAtlas == 0){
			state = IDLE;
			bool ok = true;
			ofNotifyEvent(eventAllAtlasesLoaded, ok, this);
			for(int i = 0; i < atlases.size(); i++){
				ofRemoveListener(atlases[i]->eventAtlasLoaded, this, &TextureAtlasCreator::onAtlasLoaded);
			}
		}
	}else{
		ofLogError("TextureAtlasCreator") << "Hmmm not busy but getting loaded events?";
	}
}


void TextureAtlasCreator::saveToDisk(string directory, string imgFormat /*png | jpg*/){

	if(state == IDLE){

		state = SAVING;
		if(imgFormat != "png" && imgFormat != "jpg" ){
			ofLogError("TextureAtlasCreator") << "img format must be 'png' or 'jpg'";
		}

		directory = ofFilePath::addTrailingSlash(directory);
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