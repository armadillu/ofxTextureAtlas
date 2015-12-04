//
//  TextureAtlas.h
//  BaseApp
//
//  Created by Oriol Ferrer Mesià on 18/03/15.
//
//

#ifndef __BaseApp__TextureAtlas__
#define __BaseApp__TextureAtlas__

#include "ofMain.h"
#include "ofxRectangleUtils.h"

const string atlasXmlRoot = "Atlas";
const string atlasXmlProperties = "Properties";
const string atlasXmlSize = "Size";
const string atlasXmlNumTex = "NumTextures";
const string atlasXmlInternalFormat = "internalFormat";
const string atlasXmlTextureList = "TextureList";

class TextureAtlas : public ofThread {

	friend class TextureAtlasCreator;

  public:
	struct AtlasLoadEventInfo {
		bool ok;
		TextureAtlas *atlas;
	};

	TextureAtlas();

	void setup(int fboSize, float padding, GLint internalFormat);
	ofFbo &getFbo() { return atlasFbo; }

	/*return true if fits*/
	bool addTexture(string file, // tex file
					float maxSize // biggest side, how big in atlas
					);

	void generateMipMap();
	void setMipMapBias(float bias);
	int getNumTextures();

	void drawDebug(int x, int y);

	void saveToDisk(string imageFileName, string xmlFileName);

	bool startLoadingFromDisk(GLint internalFormat_, string imageFileName, string xmlFileName, bool mipmaps,
							  float mipmapBias);

	ofEvent<TextureAtlas::AtlasLoadEventInfo> eventAtlasLoaded; // listen to this b4 startLoadingFromDisk()
	// and you will be notified when ready. You can query progress by calling getLoadXmlProgress()
	// once the xml is loaded, the texture atlas image will be loaded from the main thread,
	// and it will block the main thread for a few seconds! So only do this while on a loading
	// screen or similar

	void update(ofEventArgs &); // auto update
	float getLoadXmlProgress() { return loadXmlProgress; }

	const map<string, ofRectangle> &getTextureLocations() { return textureCrops; }
	const vector<string> getFiles();

  private:
	map<string, ofRectangle> textureCrops;
	ofRectanglePacker *packer;
	ofFbo atlasFbo;

	bool isFull;
	GLint internalFormat;

	void threadedFunction();

	// load data, for the thread to access
	bool loadingAtlas;
	bool xmlDataOK;
	bool imgDataOK;
	string xmlFileName;
	string imageFileName;
	bool loadWithMipmaps;
	float mipmapBias;
	float loadXmlProgress;
	int fboSizeInXml;
	int numTexInXml;
	bool dataReadyToLoadImage;

	bool loadXmlData();
	bool loadImageData();
};

#endif /* defined(__BaseApp__TextureAtlas__) */
