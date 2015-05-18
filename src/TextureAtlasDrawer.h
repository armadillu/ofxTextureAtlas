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

	struct Quad{
		ofVec3f tl, tr, br, bl;
	};

	struct TexQuad{
		Quad verts;
		Quad texCoords;
	};


	TextureAtlasDrawer();

	//batch drawing; call begin, drawTexInBatch N times, then call end to finally draw.
	void beginBatchDraw();

	void drawTextureInBatch(const string& filePath, const ofRectangle& where);
	void drawTextureInBatch(const string& filePath, const TexQuad& quad);
	void drawTextureInBatch(const string& filePath, const TexQuad& quad, const ofFloatColor& c);

	int endBatchDraw(bool debug = false); //returns the # of tiles drawn

	//to draw a single texture in place
	void drawTexture(const string& filePath, const ofRectangle& where);

	TextureDimensions getTextureDimensions(const string& filePath);

	//utils
	static Quad toQuad(ofRectangle&r);

protected:

	void addContent(TextureAtlas* atlas);

	void addToMesh(ofMesh & mesh, const ofRectangle & where, const ofRectangle & texCoords);

	void addToMesh(ofMesh & mesh, const TexQuad& quad, const ofRectangle & texCoords);


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
