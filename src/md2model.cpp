/* Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above notice and this permission notice shall be included in all copies
 * or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
/* File for "Animation" lesson of the OpenGL tutorial on
 * www.videotutorialsrock.com
 */



#include <fstream>
#include <string.h>

#include "terrain/imageloader.h"
#include "md2model.h"

using namespace std;

namespace md2model {
	//Normals used in the MD2 file format
	float NORMALS[486] =
		{-0.525731f,  0.000000f,  0.850651f,
		 -0.442863f,  0.238856f,  0.864188f,
		 -0.295242f,  0.000000f,  0.955423f,
		 -0.309017f,  0.500000f,  0.809017f,
		 -0.162460f,  0.262866f,  0.951056f,
		  0.000000f,  0.000000f,  1.000000f,
		  0.000000f,  0.850651f,  0.525731f,
		 -0.147621f,  0.716567f,  0.681718f,
		  0.147621f,  0.716567f,  0.681718f,
		  0.000000f,  0.525731f,  0.850651f,
		  0.309017f,  0.500000f,  0.809017f,
		  0.525731f,  0.000000f,  0.850651f,
		  0.295242f,  0.000000f,  0.955423f,
		  0.442863f,  0.238856f,  0.864188f,
		  0.162460f,  0.262866f,  0.951056f,
		 -0.681718f,  0.147621f,  0.716567f,
		 -0.809017f,  0.309017f,  0.500000f,
		 -0.587785f,  0.425325f,  0.688191f,
		 -0.850651f,  0.525731f,  0.000000f,
		 -0.864188f,  0.442863f,  0.238856f,
		 -0.716567f,  0.681718f,  0.147621f,
		 -0.688191f,  0.587785f,  0.425325f,
		 -0.500000f,  0.809017f,  0.309017f,
		 -0.238856f,  0.864188f,  0.442863f,
		 -0.425325f,  0.688191f,  0.587785f,
		 -0.716567f,  0.681718f, -0.147621f,
		 -0.500000f,  0.809017f, -0.309017f,
		 -0.525731f,  0.850651f,  0.000000f,
		  0.000000f,  0.850651f, -0.525731f,
		 -0.238856f,  0.864188f, -0.442863f,
		  0.000000f,  0.955423f, -0.295242f,
		 -0.262866f,  0.951056f, -0.162460f,
		  0.000000f,  1.000000f,  0.000000f,
		  0.000000f,  0.955423f,  0.295242f,
		 -0.262866f,  0.951056f,  0.162460f,
		  0.238856f,  0.864188f,  0.442863f,
		  0.262866f,  0.951056f,  0.162460f,
		  0.500000f,  0.809017f,  0.309017f,
		  0.238856f,  0.864188f, -0.442863f,
		  0.262866f,  0.951056f, -0.162460f,
		  0.500000f,  0.809017f, -0.309017f,
		  0.850651f,  0.525731f,  0.000000f,
		  0.716567f,  0.681718f,  0.147621f,
		  0.716567f,  0.681718f, -0.147621f,
		  0.525731f,  0.850651f,  0.000000f,
		  0.425325f,  0.688191f,  0.587785f,
		  0.864188f,  0.442863f,  0.238856f,
		  0.688191f,  0.587785f,  0.425325f,
		  0.809017f,  0.309017f,  0.500000f,
		  0.681718f,  0.147621f,  0.716567f,
		  0.587785f,  0.425325f,  0.688191f,
		  0.955423f,  0.295242f,  0.000000f,
		  1.000000f,  0.000000f,  0.000000f,
		  0.951056f,  0.162460f,  0.262866f,
		  0.850651f, -0.525731f,  0.000000f,
		  0.955423f, -0.295242f,  0.000000f,
		  0.864188f, -0.442863f,  0.238856f,
		  0.951056f, -0.162460f,  0.262866f,
		  0.809017f, -0.309017f,  0.500000f,
		  0.681718f, -0.147621f,  0.716567f,
		  0.850651f,  0.000000f,  0.525731f,
		  0.864188f,  0.442863f, -0.238856f,
		  0.809017f,  0.309017f, -0.500000f,
		  0.951056f,  0.162460f, -0.262866f,
		  0.525731f,  0.000000f, -0.850651f,
		  0.681718f,  0.147621f, -0.716567f,
		  0.681718f, -0.147621f, -0.716567f,
		  0.850651f,  0.000000f, -0.525731f,
		  0.809017f, -0.309017f, -0.500000f,
		  0.864188f, -0.442863f, -0.238856f,
		  0.951056f, -0.162460f, -0.262866f,
		  0.147621f,  0.716567f, -0.681718f,
		  0.309017f,  0.500000f, -0.809017f,
		  0.425325f,  0.688191f, -0.587785f,
		  0.442863f,  0.238856f, -0.864188f,
		  0.587785f,  0.425325f, -0.688191f,
		  0.688191f,  0.587785f, -0.425325f,
		 -0.147621f,  0.716567f, -0.681718f,
		 -0.309017f,  0.500000f, -0.809017f,
		  0.000000f,  0.525731f, -0.850651f,
		 -0.525731f,  0.000000f, -0.850651f,
		 -0.442863f,  0.238856f, -0.864188f,
		 -0.295242f,  0.000000f, -0.955423f,
		 -0.162460f,  0.262866f, -0.951056f,
		  0.000000f,  0.000000f, -1.000000f,
		  0.295242f,  0.000000f, -0.955423f,
		  0.162460f,  0.262866f, -0.951056f,
		 -0.442863f, -0.238856f, -0.864188f,
		 -0.309017f, -0.500000f, -0.809017f,
		 -0.162460f, -0.262866f, -0.951056f,
		  0.000000f, -0.850651f, -0.525731f,
		 -0.147621f, -0.716567f, -0.681718f,
		  0.147621f, -0.716567f, -0.681718f,
		  0.000000f, -0.525731f, -0.850651f,
		  0.309017f, -0.500000f, -0.809017f,
		  0.442863f, -0.238856f, -0.864188f,
		  0.162460f, -0.262866f, -0.951056f,
		  0.238856f, -0.864188f, -0.442863f,
		  0.500000f, -0.809017f, -0.309017f,
		  0.425325f, -0.688191f, -0.587785f,
		  0.716567f, -0.681718f, -0.147621f,
		  0.688191f, -0.587785f, -0.425325f,
		  0.587785f, -0.425325f, -0.688191f,
		  0.000000f, -0.955423f, -0.295242f,
		  0.000000f, -1.000000f,  0.000000f,
		  0.262866f, -0.951056f, -0.162460f,
		  0.000000f, -0.850651f,  0.525731f,
		  0.000000f, -0.955423f,  0.295242f,
		  0.238856f, -0.864188f,  0.442863f,
		  0.262866f, -0.951056f,  0.162460f,
		  0.500000f, -0.809017f,  0.309017f,
		  0.716567f, -0.681718f,  0.147621f,
		  0.525731f, -0.850651f,  0.000000f,
		 -0.238856f, -0.864188f, -0.442863f,
		 -0.500000f, -0.809017f, -0.309017f,
		 -0.262866f, -0.951056f, -0.162460f,
		 -0.850651f, -0.525731f,  0.000000f,
		 -0.716567f, -0.681718f, -0.147621f,
		 -0.716567f, -0.681718f,  0.147621f,
		 -0.525731f, -0.850651f,  0.000000f,
		 -0.500000f, -0.809017f,  0.309017f,
		 -0.238856f, -0.864188f,  0.442863f,
		 -0.262866f, -0.951056f,  0.162460f,
		 -0.864188f, -0.442863f,  0.238856f,
		 -0.809017f, -0.309017f,  0.500000f,
		 -0.688191f, -0.587785f,  0.425325f,
		 -0.681718f, -0.147621f,  0.716567f,
		 -0.442863f, -0.238856f,  0.864188f,
		 -0.587785f, -0.425325f,  0.688191f,
		 -0.309017f, -0.500000f,  0.809017f,
		 -0.147621f, -0.716567f,  0.681718f,
		 -0.425325f, -0.688191f,  0.587785f,
		 -0.162460f, -0.262866f,  0.951056f,
		  0.442863f, -0.238856f,  0.864188f,
		  0.162460f, -0.262866f,  0.951056f,
		  0.309017f, -0.500000f,  0.809017f,
		  0.147621f, -0.716567f,  0.681718f,
		  0.000000f, -0.525731f,  0.850651f,
		  0.425325f, -0.688191f,  0.587785f,
		  0.587785f, -0.425325f,  0.688191f,
		  0.688191f, -0.587785f,  0.425325f,
		 -0.955423f,  0.295242f,  0.000000f,
		 -0.951056f,  0.162460f,  0.262866f,
		 -1.000000f,  0.000000f,  0.000000f,
		 -0.850651f,  0.000000f,  0.525731f,
		 -0.955423f, -0.295242f,  0.000000f,
		 -0.951056f, -0.162460f,  0.262866f,
		 -0.864188f,  0.442863f, -0.238856f,
		 -0.951056f,  0.162460f, -0.262866f,
		 -0.809017f,  0.309017f, -0.500000f,
		 -0.864188f, -0.442863f, -0.238856f,
		 -0.951056f, -0.162460f, -0.262866f,
		 -0.809017f, -0.309017f, -0.500000f,
		 -0.681718f,  0.147621f, -0.716567f,
		 -0.681718f, -0.147621f, -0.716567f,
		 -0.850651f,  0.000000f, -0.525731f,
		 -0.688191f,  0.587785f, -0.425325f,
		 -0.587785f,  0.425325f, -0.688191f,
		 -0.425325f,  0.688191f, -0.587785f,
		 -0.425325f, -0.688191f, -0.587785f,
		 -0.587785f, -0.425325f, -0.688191f,
		 -0.688191f, -0.587785f, -0.425325f};
	
	//Returns whether the system is little-endian
	bool littleEndian() {
		//The short value 1 has bytes (1, 0) in little-endian and (0, 1) in
		//big-endian
		short s = 1;
		return (((char*)&s)[0]) == 1;
	}
	
	//Converts a four-character array to an integer, using little-endian form
	int toInt(const char* bytes) {
		return (int)(((unsigned char)bytes[3] << 24) |
					 ((unsigned char)bytes[2] << 16) |
					 ((unsigned char)bytes[1] << 8) |
					 (unsigned char)bytes[0]);
	}
	
	//Converts a two-character array to a short, using little-endian form
	short toShort(const char* bytes) {
		return (short)(((unsigned char)bytes[1] << 8) |
					   (unsigned char)bytes[0]);
	}
	
	//Converts a two-character array to an unsigned short, using little-endian
	//form
	unsigned short toUShort(const char* bytes) {
		return (unsigned short)(((unsigned char)bytes[1] << 8) |
								(unsigned char)bytes[0]);
	}
	
	//Converts a four-character array to a float, using little-endian form
	float toFloat(const char* bytes) {
		float f;
		if (littleEndian()) {
			((char*)&f)[0] = bytes[0];
			((char*)&f)[1] = bytes[1];
			((char*)&f)[2] = bytes[2];
			((char*)&f)[3] = bytes[3];
		}
		else {
			((char*)&f)[0] = bytes[3];
			((char*)&f)[1] = bytes[2];
			((char*)&f)[2] = bytes[1];
			((char*)&f)[3] = bytes[0];
		}
		return f;
	}
	
	//Reads the next four bytes as an integer, using little-endian form
	int readInt(ifstream &input) {
		char buffer[4];
		input.read(buffer, 4);
		return toInt(buffer);
	}
	
	//Reads the next two bytes as a short, using little-endian form
	short readShort(ifstream &input) {
		char buffer[2];
		input.read(buffer, 2);
		return toShort(buffer);
	}
	
	//Reads the next two bytes as an unsigned short, using little-endian form
	unsigned short readUShort(ifstream &input) {
		char buffer[2];
		input.read(buffer, 2);
		return toUShort(buffer);
	}
	
	//Reads the next four bytes as a float, using little-endian form
	float readFloat(ifstream &input) {
		char buffer[4];
		input.read(buffer, 4);
		return toFloat(buffer);
	}
	
	//Calls readFloat three times and returns the results as a Vec3f object
	Vec3f readVec3f(ifstream &input) {
		float x = readFloat(input);
		float y = readFloat(input);
		float z = readFloat(input);
		return Vec3f(x, y, z);
	}
	
	//Makes the image into a texture, and returns the id of the texture
	GLuint loadTexture(Image *image) {
		GLuint textureId;
		glGenTextures(1, &textureId);
		glBindTexture(GL_TEXTURE_2D, textureId);
		glTexImage2D(GL_TEXTURE_2D,
					 0,
					 GL_RGB,
					 image->width, image->height,
					 0,
					 GL_RGB,
					 GL_UNSIGNED_BYTE,
					 image->pixels);
		return textureId;
	}
}

MD2Model::~MD2Model() {
	if (frames != NULL) {
		for(int i = 0; i < numFrames; i++) {
			delete[] frames[i].vertices;
		}
		delete[] frames;
	}
	
	if (texCoords != NULL) {
		delete[] texCoords;
	}
	if (triangles != NULL) {
		delete[] triangles;
	}
}

MD2Model::MD2Model() {
	frames = NULL;
	texCoords = NULL;
	triangles = NULL;
	time = 0;
}

//Loads the MD2 model
MD2Model* MD2Model::loadModel(const char* filename) {
	ifstream input;
	input.open(filename, istream::binary);
	
	char buffer[64];
	input.read(buffer, 4); //Should be "IPD2", if this is an MD2 file
	if (buffer[0] != 'I' || buffer[1] != 'D' ||
		buffer[2] != 'P' || buffer[3] != '2') {
		printf ("This is not a MD2 file %s\n",filename);
		return NULL;
	}
    if (md2model::readInt(input) != 8) { //The version number
		printf ("Version number is not right\n");
		return NULL;
	}
	
    int textureWidth = md2model::readInt(input);   //The width of the textures
    int textureHeight = md2model::readInt(input);  //The height of the textures
    md2model::readInt(input);                      //The number of bytes per frame
    int numTextures = md2model::readInt(input);    //The number of textures
	if (numTextures != 1) {
        //printf ("There are more than one texture %d\n", numTextures);
		//return NULL;
	}
    int numVertices = md2model::readInt(input);    //The number of vertices
    int numTexCoords = md2model::readInt(input);   //The number of texture coordinates
    int numTriangles = md2model::readInt(input);   //The number of triangles
    md2model::readInt(input);                      //The number of OpenGL commands
    int numFrames = md2model::readInt(input);      //The number of frames
	
	//Offsets (number of bytes after the beginning of the file to the beginning
	//of where certain data appear)
    int textureOffset = md2model::readInt(input);  //The offset to the textures
    int texCoordOffset = md2model::readInt(input); //The offset to the texture coordinates
    int triangleOffset = md2model::readInt(input); //The offset to the triangles
    int frameOffset = md2model::readInt(input);    //The offset to the frames
    md2model::readInt(input);                      //The offset to the OpenGL commands
    md2model::readInt(input);                      //The offset to the end of the file
	
	//Load the texture
	input.seekg(textureOffset, ios_base::beg);
	input.read(buffer, 64);
	if (strlen(buffer) < 5 ||
		strcmp(buffer + strlen(buffer) - 4, ".bmp") != 0) {
        //printf ("Texture is no a bmp file.\n");
		//return NULL;
	}

    // @FIXME: It should be incorporated to the texture model of the engine.
    //strcpy ( buffer, "vtr.bmp");
    //Image* image = loadBMP(buffer);
    //GLuint textureId = md2model::loadTexture(image);
    //delete image;
	MD2Model* model = new MD2Model();
    //model->textureId = textureId;
	
	//Load the texture coordinates
	input.seekg(texCoordOffset, ios_base::beg);
	model->texCoords = new MD2TexCoord[numTexCoords];
	for(int i = 0; i < numTexCoords; i++) {
		MD2TexCoord* texCoord = model->texCoords + i;
        texCoord->texCoordX = (float)md2model::readShort(input) / textureWidth;
        texCoord->texCoordY = 1 - (float)md2model::readShort(input) / textureHeight;
	}
	
	//Load the triangles
	input.seekg(triangleOffset, ios_base::beg);
	model->triangles = new MD2Triangle[numTriangles];
	model->numTriangles = numTriangles;
	for(int i = 0; i < numTriangles; i++) {
		MD2Triangle* triangle = model->triangles + i;
		for(int j = 0; j < 3; j++) {
            triangle->vertices[j] = md2model::readUShort(input);
		}
		for(int j = 0; j < 3; j++) {
            triangle->texCoords[j] = md2model::readUShort(input);
		}
	}
	
	//Load the frames
	input.seekg(frameOffset, ios_base::beg);
	model->frames = new MD2Frame[numFrames];
	model->numFrames = numFrames;
	for(int i = 0; i < numFrames; i++) {
		MD2Frame* frame = model->frames + i;
		frame->vertices = new MD2Vertex[numVertices];
        Vec3f scale = md2model::readVec3f(input);
        Vec3f translation = md2model::readVec3f(input);
		input.read(frame->name, 16);
		
		for(int j = 0; j < numVertices; j++) {
			MD2Vertex* vertex = frame->vertices + j;
			input.read(buffer, 3);
			Vec3f v((unsigned char)buffer[0],
					(unsigned char)buffer[1],
					(unsigned char)buffer[2]);
			vertex->pos = translation + Vec3f(scale[0] * v[0],
											  scale[1] * v[1],
											  scale[2] * v[2]);
			input.read(buffer, 1);
			int normalIndex = (int)((unsigned char)buffer[0]);
            vertex->normal = Vec3f(md2model::NORMALS[3 * normalIndex],
                                   md2model::NORMALS[3 * normalIndex + 1],
                                   md2model::NORMALS[3 * normalIndex + 2]);
		}
	}
	
	model->startFrame = 0;
	model->endFrame = numFrames - 1;
	return model;
}

void MD2Model::setAnimation(const char* name) {
	/* The names of frames normally begin with the name of the animation in
	 * which they are, e.g. "run", and are followed by a non-alphabetical
	 * character.  Normally, they indicate their frame number in the animation,
	 * e.g. "run_1", "run_2", etc.
	 */
     startFrame = 1; endFrame = 23;
	bool found = false;
	for(int i = 0; i < numFrames; i++) {
		MD2Frame* frame = frames + i;
		if (strlen(frame->name) > strlen(name) &&
			strncmp(frame->name, name, strlen(name)) == 0 &&
			!isalpha(frame->name[strlen(name)])) {
			if (!found) {
				found = true;
				startFrame = i;
			}
			else {
				endFrame = i;
			}
		}
		else if (found) {
			break;
		}
	}

}

void MD2Model::advance(float dt) {
	if (dt < 0) {
		return;
	}
	
	time += dt;
	if (time < 1000000000) {
		time -= (int)time;
	}
	else {
		time = 0;
	}
    
    //printf ("Time: %f\n", time);
}

void MD2Model::setTexture(GLuint texture)
{
    textureId = texture;
}
void MD2Model::draw(GLuint texture)
{
    assert( 0 || !"Should not be executed.");
}
void MD2Model::draw() {
    //glEnable(GL_TEXTURE_2D);
    //glBindTexture(GL_TEXTURE_2D, textureId);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glColor3f(1.0f, 1.0f, 1.0f);
	
	//Figure out the two frames between which we are interpolating
	int frameIndex1 = (int)(time * (endFrame - startFrame + 1)) + startFrame;
	if (frameIndex1 > endFrame) {
		frameIndex1 = startFrame;
	}
	
	int frameIndex2;
	if (frameIndex1 < endFrame) {
		frameIndex2 = frameIndex1 + 1;
	}
	else {
		frameIndex2 = startFrame;
	}
	
	MD2Frame* frame1 = frames + frameIndex1;
	MD2Frame* frame2 = frames + frameIndex2;
	
	//Figure out the fraction that we are between the two frames
	float frac =
		(time - (float)(frameIndex1 - startFrame) /
		 (float)(endFrame - startFrame + 1)) * (endFrame - startFrame + 1);
	
	//Draw the model as an interpolation between the two frames

	glBegin(GL_TRIANGLES);
	for(int i = 0; i < numTriangles; i++) {
		MD2Triangle* triangle = triangles + i;
        glColor3f(i/((float)numTriangles),i/((float)numTriangles), i/((float)numTriangles));
		for(int j = 0; j < 3; j++) {
			MD2Vertex* v1 = frame1->vertices + triangle->vertices[j];
			MD2Vertex* v2 = frame2->vertices + triangle->vertices[j];
			Vec3f pos = v1->pos * (1 - frac) + v2->pos * frac;
			Vec3f normal = v1->normal * (1 - frac) + v2->normal * frac;
			if (normal[0] == 0 && normal[1] == 0 && normal[2] == 0) {
				normal = Vec3f(0, 0, 1);
			}
			glNormal3f(normal[0], normal[1], normal[2]);
			
            //@FIXME The model has no tex coordinates so it wont work here.
			MD2TexCoord* texCoord = texCoords + triangle->texCoords[j];
			glTexCoord2f(texCoord->texCoordX, texCoord->texCoordY);
			glVertex3f(pos[0], pos[1], pos[2]);
		}
	}
    //glDisable(GL_TEXTURE_2D);
	glEnd();
}









