//
//  TextureAtlasCreator.h
//  BaseApp
//
//  Created by Oriol Ferrer Mesià on 18/03/15.
//
//

#ifndef __BaseApp__TextureAtlasCreator__
#define __BaseApp__TextureAtlasCreator__

#include "ofMain.h"
#include "ofxRectangleUtils.h"
#include "TextureAtlas.h"

class TextureAtlasDrawer;

class TextureAtlasCreator{

public:

	TextureAtlasCreator();

	void createAtlases(	vector<string> fileList,
						int fboSize,
					   	GLint internalFormat,
					   	float maxItemSideSize,
					   	float padding,
					    bool generateMipMaps,
					    float mipmapBias
					   );

	void saveToDisk(string directory, string imgFormat /*png | jpg*/);
	bool loadAtlasesFromDisk(GLint internalFormat,
							 string directory,
							 string imgFormat, /*png | jpg*/
							 bool generateMipMaps,
							 float mipmapBias);

	bool registerWithManager(TextureAtlasDrawer& manager);

	int getNumAtlases(){return atlases.size();};
	TextureAtlas* getAtlasAtIndex(int index){return atlases[index];}
	
	//when creating an atlas, how many to add per atlas per update call
	void setNumImagesPerUpdate(int n){numImagesPerUpdate = n;}


	bool isIdle(){return state == IDLE;}
	bool isLoading(){return state == LOADING;}
	bool isCreating(){return state == CREATING;}
	bool isSaving(){return state == SAVING;}

	float getPercentDone(); //while creating OR loading
	string getCurrentCreatingFileName(); //only while creating
	ofTexture * getCurrentlyCreatedAtlas(); //only while creating

	vector<string> getAllImagePaths();

	ofEvent<bool> eventAtlasCreationFinished;
	ofEvent<bool> eventAllAtlasesLoaded;

	//for each individual atlas
	void onAtlasLoaded(TextureAtlas::AtlasLoadEventInfo & info);

	void update(ofEventArgs&); //dont call this
	string getMemStats(); //call only when loaded / created
	vector<string> getImageFileList(){return fileList;}

private:

	enum State{
		IDLE = 0,
		LOADING,
		CREATING,
		SAVING
	};

	State state;
	int loadingAtlas;
	int currentFile;

	int numImagesPerUpdate;
	
	int fboSize;
	float maxItemSideSize;
	float padding;
	bool makeMipMaps;
	float mipmapBias;

	TextureAtlas * currentAtlas;

	vector<TextureAtlas*> atlases; //fbos
	map<string, TextureAtlas*> fileToAtlas;

	vector<string> fileList;
	GLint internalFormat;

	string bytesToHumanReadable(long long bytes, int decimalPrecision);
};

#endif /* defined(__BaseApp__TextureAtlasCreator__) */
