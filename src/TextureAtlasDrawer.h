//
//  TextureAtlasDrawer.h
//  BaseApp
//
//  Created by Oriol Ferrer Mesiˆ on 30/03/15.
//
//

#ifndef __BaseApp__TextureAtlasDrawer__
#define __BaseApp__TextureAtlasDrawer__
	
#include "ofMain.h"

class TextureAtlas;
class TextureAtlasCreator;

class TextureAtlasDrawer{

friend TextureAtlasCreator;

public:
	
	struct TextureDimensions{
		int width;			//tex size
		int height;
		float aspectRatio; //  w/h
	};

	TextureAtlasDrawer();

	void beginBatchDraw();

	void drawTextureInBatch(const string& filePath,
						  const ofRectangle& where);

	void drawTextureInBatch(const string& filePath,
						  const ofVec3f & topLeft,
						  const ofVec3f & topRight,
						  const ofVec3f & bottomRight,
						  const ofVec3f & bottomLeft);

	void drawTextureInBatch(const string& filePath,
						  const ofVec3f & topLeft,
						  const ofVec3f & topRight,
						  const ofVec3f & bottomRight,
						  const ofVec3f & bottomLeft,
							float tcx0, //tex coord x
							float tcy1, //tex coord y
							float tcxWidth, //tex coord width
							float tcxHeight); //tex coord height

	int endBatchDraw(bool debug = false); //returns the # of tiles drawn

	void drawTexture(const string& filePath, const ofRectangle& where);

	TextureDimensions getTextureDimensions(const string& filePath);


protected:

	void addContent(TextureAtlas* atlas);

	void addToMesh(ofMesh & mesh, const ofRectangle & where, const ofRectangle & texCoords);


	struct TextureInfo{
		TextureAtlas* atlas; //who holds the fbo
		ofTexture * tex;
		ofRectangle crop;	//in % of the whole fbo
		TextureDimensions dim;
	};

	map<string, TextureInfo> textures;

	bool batching;
	map<TextureAtlas*, ofMesh> currentBatch;

	vector<ofColor> debugColors;


};

#endif /* defined(__BaseApp__TextureAtlasDrawer__) */
