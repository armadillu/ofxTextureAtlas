//
//  TextureAtlasDrawer.cpp
//  BaseApp
//
//  Created by Oriol Ferrer Mesiˆ on 30/03/15.
//
//

#include "TextureAtlasDrawer.h"
#include "TextureAtlas.h"

TextureAtlasDrawer::TextureAtlasDrawer(){

	batching = false;
	debugColors.push_back(ofColor::red);
	debugColors.push_back(ofColor::green);
	debugColors.push_back(ofColor::blue);
	debugColors.push_back(ofColor::yellow);
	debugColors.push_back(ofColor::orange);
	debugColors.push_back(ofColor::white);
	debugColors.push_back(ofColor::cyan);
	debugColors.push_back(ofColor::purple);
	debugColors.push_back(ofColor::lime);
	debugColors.push_back(ofColor::gray);
}

void TextureAtlasDrawer::addContent(TextureAtlas* atlas){

	map<string, ofRectangle> crops = atlas->getTextureLocations();
	map<string, ofRectangle>::iterator it = crops.begin();

	float fboSize = atlas->getFbo().getWidth();
	TextureInfo ti;
	ti.atlas = atlas;
	ti.tex = &atlas->getFbo().getTexture();

	while(it != crops.end()){
		ti.crop = it->second;

		ti.dim.width = ti.crop.width;
		ti.dim.height = ti.crop.height;
		ti.dim.aspectRatio = ti.dim.width / float(ti.dim.height);

		//we know this is GL_TEXTURE_2D, so we convert units to percentage (atlas stores rects in pixels)
		ti.crop.x = ti.crop.x / fboSize;
		ti.crop.y = ti.crop.y / fboSize;
		ti.crop.width = ti.crop.width / fboSize;
		ti.crop.height = ti.crop.height / fboSize;
		textures[it->first] = ti;
		ofLogNotice("TextureAtlasDrawer") << "adding atlas file reference: " << it->first;
		++it;
	}
}

void TextureAtlasDrawer::drawTexture(const string& filePath,
									  const ofRectangle& where){

	ofMesh mesh;
	mesh.setMode(OF_PRIMITIVE_TRIANGLE_FAN);

	TextureInfo & ti = textures[filePath];
	TextureAtlas * atlas = ti.atlas;
	ofRectangle & texCoords = ti.crop;

	if(atlas){

		mesh.addVertex(ofVec2f(where.x, where.y));
		mesh.addTexCoord(ofVec2f(texCoords.x, texCoords.y)); //vflip

		mesh.addVertex(ofVec2f(where.x + where.width, where.y));
		mesh.addTexCoord(ofVec2f(texCoords.x + texCoords.width, texCoords.y)); //vflip

		mesh.addVertex(ofVec2f(where.x + where.width, where.y + where.height));
		mesh.addTexCoord(ofVec2f(texCoords.x + texCoords.width, texCoords.y + texCoords.height)); //vflip

		mesh.addVertex(ofVec2f(where.x , where.y + where.height));
		mesh.addTexCoord(ofVec2f(texCoords.x, texCoords.y + texCoords.height)); //vflip

		ti.tex->bind();
		mesh.draw();
		ti.tex->unbind();
	}
}

TextureAtlasDrawer::TextureDimensions
TextureAtlasDrawer::getTextureDimensions(const string& filePath){

	TextureDimensions ti;
	map<string, TextureInfo>::iterator it = textures.find(filePath);
	if(it != textures.end()){
		return it->second.dim;
	}
	ofLogError("TextureAtlasDrawer") << "No texture exists at that path?";
	return ti;
}

void TextureAtlasDrawer::addToMesh(ofMesh & mesh, const ofRectangle & where, const ofRectangle & texCoords){

	int num = mesh.getVertices().size();

	mesh.addVertex(ofVec2f(where.x, where.y));
	mesh.addTexCoord(ofVec2f(texCoords.x, texCoords.y)); //vflip

	mesh.addVertex(ofVec2f(where.x + where.width, where.y));
	mesh.addTexCoord(ofVec2f(texCoords.x + texCoords.width, texCoords.y)); //vflip

	mesh.addVertex(ofVec2f(where.x + where.width, where.y + where.height));
	mesh.addTexCoord(ofVec2f(texCoords.x + texCoords.width, texCoords.y + texCoords.height)); //vflip

	mesh.addVertex(ofVec2f(where.x , where.y + where.height));
	mesh.addTexCoord(ofVec2f(texCoords.x, texCoords.y + texCoords.height)); //vflip

	mesh.addIndex(num + 0);
	mesh.addIndex(num + 1);
	mesh.addIndex(num + 2);

	mesh.addIndex(num + 0);
	mesh.addIndex(num + 3);
	mesh.addIndex(num + 2);
}

void TextureAtlasDrawer::addToMesh(ofMesh & mesh, const TexQuad& quad, const ofRectangle & texCoords){

	ofVec2f tc00 = texCoords.getTopLeft();
	ofVec2f tc10 = texCoords.getTopRight();
	ofVec2f tc11 = texCoords.getBottomRight();
	ofVec2f tc01 = texCoords.getBottomLeft();

	int num = mesh.getVertices().size();
	mesh.addVertex(quad.verts.tl);
	mesh.addTexCoord(tc00 + (tc10 - tc00) * quad.texCoords.tl);

	mesh.addVertex(quad.verts.tr);
	mesh.addTexCoord(tc00 + (tc10 - tc00) * quad.texCoords.tr);

	mesh.addVertex(quad.verts.br);
	mesh.addTexCoord(tc01 + (tc11 - tc01) * quad.texCoords.br);

	mesh.addVertex(quad.verts.bl);
	mesh.addTexCoord(tc01 + (tc11 - tc01) * quad.texCoords.bl);

	mesh.addIndex(num + 0); //tri 1
	mesh.addIndex(num + 1);
	mesh.addIndex(num + 2);

	mesh.addIndex(num + 0); //tri 2
	mesh.addIndex(num + 3);
	mesh.addIndex(num + 2);
}



void TextureAtlasDrawer::beginBatchDraw(){
	currentBatch.clear();
	batching = true;
}


int TextureAtlasDrawer::endBatchDraw(bool debug){

	map<TextureAtlas*, ofMesh>::iterator it = currentBatch.begin();

	int c = 0;
	//walk through all atlases involved in this batch, draw its mesh
	int mc = 0;
	while(it != currentBatch.end()){
		if(it->first){
			it->first->getFbo().getTexture().bind();
			if(debug){
				ofSetColor(debugColors[mc%debugColors.size()]);
				mc++;
			}
			it->second.draw();
			c += it->second.getVertices().size()/4;
			it->first->getFbo().getTexture().unbind();
		}else{
			ofLogError("TextureAtlasDrawer") << "NULL Atlas?";
		}
		++it;
	}
	batching = false;
	return c;
}


void TextureAtlasDrawer::drawTextureInBatch(const string& filePath, const ofRectangle& where){

	TextureInfo & ti = textures[filePath];
	TextureAtlas* atlas = ti.atlas;

	map<TextureAtlas*, ofMesh>::iterator it = currentBatch.find(atlas);
	if(it == currentBatch.end()){
		currentBatch[atlas] = ofMesh();
		currentBatch[atlas].setMode(OF_PRIMITIVE_TRIANGLES);
	}

	ofMesh & mesh = currentBatch[atlas];
	addToMesh(mesh, where, ti.crop);
}



void TextureAtlasDrawer::drawTextureInBatch(const string& filePath, const TexQuad& quad){

	TextureInfo & ti = textures[filePath];
	TextureAtlas* atlas = ti.atlas;

	map<TextureAtlas*, ofMesh>::iterator it = currentBatch.find(atlas);
	if(it == currentBatch.end()){
		currentBatch[atlas] = ofMesh();
		currentBatch[atlas].setMode(OF_PRIMITIVE_TRIANGLES);
	}

	ofMesh & mesh = currentBatch[atlas];
	addToMesh(mesh, quad, ti.crop);
}

void TextureAtlasDrawer::drawTextureInBatch(const string& filePath, const TexQuad& quad, const ofFloatColor & col){

	TextureInfo & ti = textures[filePath];
	TextureAtlas* atlas = ti.atlas;

	map<TextureAtlas*, ofMesh>::iterator it = currentBatch.find(atlas);
	if(it == currentBatch.end()){
		currentBatch[atlas] = ofMesh();
		currentBatch[atlas].setMode(OF_PRIMITIVE_TRIANGLES);
	}

	ofMesh & mesh = currentBatch[atlas];
	addToMesh(mesh, quad, ti.crop);

	mesh.addColor(col);
	mesh.addColor(col);
	mesh.addColor(col);
	mesh.addColor(col);
}


