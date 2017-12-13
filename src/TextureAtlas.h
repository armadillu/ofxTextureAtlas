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

class TextureAtlas : public ofThread {

	const std::string atlasXmlRoot = "Atlas";
	const std::string atlasXmlProperties = "Properties";
	const std::string atlasXmlSize = "Size";
	const std::string atlasXmlNumTex = "NumTextures";
	const std::string atlasXmlInternalFormat = "internalFormat";
	const std::string atlasXmlTextureList = "TextureList";

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
	bool addTexture(std::string file, // tex file
					ofVec2f maxSize // biggest side, how big in atlas
					);

	void generateMipMap();
	void setMipMapBias(float bias);
	int getNumTextures();

	void drawDebug(int x, int y);

	void saveToDisk(std::string imageFileName, std::string xmlFileName);

	bool startLoadingFromDisk(GLint internalFormat_, std::string imageFileName, std::string xmlFileName, bool mipmaps,
							  float mipmapBias);

	ofEvent<TextureAtlas::AtlasLoadEventInfo> eventAtlasLoaded; // listen to this b4 startLoadingFromDisk()
	// and you will be notified when ready. You can query progress by calling getLoadXmlProgress()
	// once the xml is loaded, the texture atlas image will be loaded from the main thread,
	// and it will block the main thread for a few seconds! So only do this while on a loading
	// screen or similar

	void update(ofEventArgs &); // auto update
	float getLoadXmlProgress() { return loadXmlProgress; }

	const std::unordered_map<std::string, ofRectangle> &getTextureLocations() { return textureCrops; }
	const std::vector<std::string> getFiles();

  private:
	std::unordered_map<std::string, ofRectangle> textureCrops;
	ofRectanglePacker *packer;
	ofFbo atlasFbo;

	bool isFull;
	GLint internalFormat;

	void threadedFunction();

	// load data, for the thread to access
	bool loadingAtlas;
	bool xmlDataOK;
	bool imgDataOK;
	std::string xmlFileName;
	std::string imageFileName;
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
