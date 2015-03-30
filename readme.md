#What is this

Requires ofxXmlSettings and my fork of [ofxRectangleUtils](https://github.com/armadillu/ofxRectangleUtils).

![cats atlas](https://farm8.staticflickr.com/7586/16983466005_7091e373f2_z_d.jpg)

This will allow you to draw lots of different images on screen with a minimal number of texture binds. First, you must create the atlases. Supply a list of images you want to be able to draw later, and create atlases for them. You can specify the atlas size, the max image size, interal format (with alpha, etc), and if you want mipmaps or not.

The addon accepts any image aspect ratio, so images can be all of different aspect ratios on the same atlas (ie not only sprite sheets). It does rectangle packing to maximize the atlas space.

Each atlas is created in an fbo + a data structure, which can be saved to disk for later use. An atlas is saved to disk as an image file (png / jpg) and an XML. The xml holds a list of the images files that are contained inside the atlas, the atlas size, and each of the rects that defines where the image is in the atlas.

You can easly load atlases from disk (multi threaded, as loading from XML is very slow) and use the TextureAtlasDrawer class to draw any images you need, regardless of which atlas they fell into. It will create an ofMesh on the fly for each atlas that holds an image that happens to be in the list of images you want to draw, minimizing the number of texture binds().


#Examples

To run the examples, first create some atlases. Open up exampleCreate, add lots of images into bin/data/images/cats and run it; it will create some atlases into bin/data/textureCache

Then, to draw using the atlas, try the other example, exampleDraw. Copy the atlases created in bin/data/textureCache with exampleCreate into the bin/data/textureCache of exampleDraw, and run it. Use the mouse wheel to zoom in and out.
