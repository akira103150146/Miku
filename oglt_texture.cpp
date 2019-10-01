#include "oglt_texture.h"

#include <FreeImage.h>

#pragma comment(lib, "FreeImage.lib")

using namespace oglt;

Texture::Texture()
{
	bMipMapsGenerated = false;
	textureType = TEXTURE_2D;
}

/*-----------------------------------------------

Name:	CreateEmptyTexture

Params:	a_iWidth, a_iHeight - dimensions
format - format of data

Result:	Creates texture from provided data.

/*---------------------------------------------*/

void Texture::createEmptyTexture(int a_iWidth, int a_iHeight, GLenum format)
{
	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_2D, textureId);
	if (format == GL_RGBA || format == GL_BGRA)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, a_iWidth, a_iHeight, 0, format, GL_UNSIGNED_BYTE, NULL);
	// We must handle this because of internal format parameter
	else if (format == GL_RGB || format == GL_BGR)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, a_iWidth, a_iHeight, 0, format, GL_UNSIGNED_BYTE, NULL);
	else
		glTexImage2D(GL_TEXTURE_2D, 0, format, a_iWidth, a_iHeight, 0, format, GL_UNSIGNED_BYTE, NULL);

	glGenSamplers(1, &samplerId);
}

/*-----------------------------------------------

Name:	CreateFromData

Params:	a_sPath - path to the texture
format - format of data
bGenerateMipMaps - whether to create mipmaps

Result:	Creates texture from provided data.

/*---------------------------------------------*/

void Texture::createFromData(GLubyte* bData, int a_iWidth, int a_iHeight, int a_iBPP, GLenum format, bool bGenerateMipMaps)
{
	// Generate an OpenGL texture ID for this texture
	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_2D, textureId);
	if (format == GL_RGBA || format == GL_BGRA)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, a_iWidth, a_iHeight, 0, format, GL_UNSIGNED_BYTE, bData);
	// We must handle this because of internal format parameter
	else if (format == GL_RGB || format == GL_BGR)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, a_iWidth, a_iHeight, 0, format, GL_UNSIGNED_BYTE, bData);
	else
		glTexImage2D(GL_TEXTURE_2D, 0, format, a_iWidth, a_iHeight, 0, format, GL_UNSIGNED_BYTE, bData);
	if (bGenerateMipMaps)glGenerateMipmap(GL_TEXTURE_2D);
	glGenSamplers(1, &samplerId);

	sPath = "";
	bMipMapsGenerated = bGenerateMipMaps;
	width = a_iWidth;
	height = a_iHeight;
	iBPP = a_iBPP;
}

/*-----------------------------------------------

Name:	LoadTexture2D

Params:	a_sPath - path to the texture
bGenerateMipMaps - whether to create mipmaps

Result:	Loads texture from a file, supports most
graphics formats.

/*---------------------------------------------*/

bool Texture::loadTexture2D(string a_sPath, bool bGenerateMipMaps)
{
	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
	FIBITMAP* dib(0);

	fif = FreeImage_GetFileType(a_sPath.c_str(), 0); // Check the file signature and deduce its format

	if (fif == FIF_UNKNOWN) // If still unknown, try to guess the file format from the file extension
		fif = FreeImage_GetFIFFromFilename(a_sPath.c_str());

	if (fif == FIF_UNKNOWN) // If still unknown, return failure
		return false;

	if (FreeImage_FIFSupportsReading(fif)) // Check if the plugin has reading capabilities and load the file
		dib = FreeImage_Load(fif, a_sPath.c_str());
	if (!dib)
		return false;

	GLubyte* bDataPointer = (GLubyte*)FreeImage_GetBits(dib); // Retrieve the image data

												 // If somehow one of these failed (they shouldn't), return failure
	if (bDataPointer == NULL || FreeImage_GetWidth(dib) == 0 || FreeImage_GetHeight(dib) == 0)
		return false;

	GLenum format;
	int bada = FreeImage_GetBPP(dib);
	if (FreeImage_GetBPP(dib) == 32)format = GL_BGRA;
	if (FreeImage_GetBPP(dib) == 24)format = GL_BGR;
	if (FreeImage_GetBPP(dib) == 8)format = GL_LUMINANCE;
	createFromData(bDataPointer, FreeImage_GetWidth(dib), FreeImage_GetHeight(dib), FreeImage_GetBPP(dib), format, bGenerateMipMaps);

	FreeImage_Unload(dib);

	sPath = a_sPath;

	textureType = TEXTURE_2D;

	return true; // Success
}

void Texture::setSamplerParameter(GLenum parameter, GLenum value)
{
	glSamplerParameteri(samplerId, parameter, value);
}

/*-----------------------------------------------

Name:	SetFiltering

Params:	tfMagnification - mag. filter, must be from
ETextureFiltering enum
tfMinification - min. filter, must be from
ETextureFiltering enum

Result:	Sets magnification and minification
texture filter.

/*---------------------------------------------*/

void Texture::setFiltering(int a_tfMagnification, int a_tfMinification)
{
	glBindSampler(0, samplerId);

	// Set magnification filter
	if (a_tfMagnification == TEXTURE_FILTER_MAG_NEAREST)
		glSamplerParameteri(samplerId, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	else if (a_tfMagnification == TEXTURE_FILTER_MAG_BILINEAR)
		glSamplerParameteri(samplerId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Set minification filter
	if (a_tfMinification == TEXTURE_FILTER_MIN_NEAREST)
		glSamplerParameteri(samplerId, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	else if (a_tfMinification == TEXTURE_FILTER_MIN_BILINEAR)
		glSamplerParameteri(samplerId, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	else if (a_tfMinification == TEXTURE_FILTER_MIN_NEAREST_MIPMAP)
		glSamplerParameteri(samplerId, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	else if (a_tfMinification == TEXTURE_FILTER_MIN_BILINEAR_MIPMAP)
		glSamplerParameteri(samplerId, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	else if (a_tfMinification == TEXTURE_FILTER_MIN_TRILINEAR)
		glSamplerParameteri(samplerId, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	tfMinification = a_tfMinification;
	tfMagnification = a_tfMagnification;
}

/*-----------------------------------------------

Name:	BindTexture

Params:	iTextureUnit - texture unit to bind texture to

Result:	Guess what it does :)

/*---------------------------------------------*/
void Texture::bindTexture(int iTextureUnit)
{
	glActiveTexture(GL_TEXTURE0 + iTextureUnit);
	glBindTexture(GL_TEXTURE_2D, textureId);
	glBindSampler(iTextureUnit, samplerId);
	switch (textureType)
	{
	case TEXTURE_2D:
		glActiveTexture(GL_TEXTURE0 + iTextureUnit);
		glBindTexture(GL_TEXTURE_2D, textureId);
		glBindSampler(iTextureUnit, samplerId);
		break;
	case CUBE_MAP:
		glActiveTexture(GL_TEXTURE0 + iTextureUnit);
		glBindTexture(GL_TEXTURE_CUBE_MAP, textureId);
		break;
	default:
		break;
	}
}

/*-----------------------------------------------

Name:	DeleteTexture

Params:	none

Result:	Frees all memory used by texture.

/*---------------------------------------------*/

void Texture::deleteTexture()
{
	glDeleteSamplers(1, &samplerId);
	glDeleteTextures(1, &textureId);
}

/*-----------------------------------------------

Name:	Getters

Params:	none

Result:	... They get something :D

/*---------------------------------------------*/

int Texture::getMinificationFilter()
{
	return tfMinification;
}

int Texture::getMagnificationFilter()
{
	return tfMagnification;
}

int Texture::getWidth()
{
	return width;
}

int Texture::getHeight()
{
	return height;
}

int Texture::getBPP()
{
	return iBPP;
}

uint Texture::getTextureID()
{
	return textureId;
}

string Texture::getPath()
{
	return sPath;
}

Texture_Type Texture::getType()
{
	return textureType;
}

bool Texture::reloadTexture()
{
	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
	FIBITMAP* dib(0);

	fif = FreeImage_GetFileType(sPath.c_str(), 0); // Check the file signature and deduce its format

	if (fif == FIF_UNKNOWN) // If still unknown, try to guess the file format from the file extension
		fif = FreeImage_GetFIFFromFilename(sPath.c_str());

	if (fif == FIF_UNKNOWN) // If still unknown, return failure
		return false;

	if (FreeImage_FIFSupportsReading(fif)) // Check if the plugin has reading capabilities and load the file
		dib = FreeImage_Load(fif, sPath.c_str());
	if (!dib)
		return false;

	GLubyte* bDataPointer = (GLubyte*)FreeImage_GetBits(dib); // Retrieve the image data

												 // If somehow one of these failed (they shouldn't), return failure
	if (bDataPointer == NULL || FreeImage_GetWidth(dib) == 0 || FreeImage_GetHeight(dib) == 0)
		return false;

	GLenum format;
	int bada = FreeImage_GetBPP(dib);
	if (FreeImage_GetBPP(dib) == 32)format = GL_RGBA;
	if (FreeImage_GetBPP(dib) == 24)format = GL_BGR;
	if (FreeImage_GetBPP(dib) == 8)format = GL_LUMINANCE;

	glBindTexture(GL_TEXTURE_2D, textureId);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, format, GL_UNSIGNED_BYTE, bDataPointer);
	if (bMipMapsGenerated)glGenerateMipmap(GL_TEXTURE_2D);

	FreeImage_Unload(dib);

	return true; // Success
}

bool Texture::createCubeMapFromData(string FilePath, int i)
{
	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
	FIBITMAP* dib(0);
	fif = FreeImage_GetFileType(FilePath.c_str(), 0);

	if (fif == FIF_UNKNOWN) // If still unknown, try to guess the file format from the file extension
		fif = FreeImage_GetFIFFromFilename(FilePath.c_str());

	if (fif == FIF_UNKNOWN) // If still unknown, return failure
		return false;

	if (FreeImage_FIFSupportsReading(fif)) // Check if the plugin has reading capabilities and load the file
		dib = FreeImage_Load(fif, FilePath.c_str());

	if (!dib)
		return false;

	GLubyte* bDataPointer = (GLubyte*)FreeImage_GetBits(dib); // Retrieve the image data

	if (bDataPointer == NULL || FreeImage_GetWidth(dib) == 0 || FreeImage_GetHeight(dib) == 0)
		return false;
	int W, H;
	GLenum format;
	W = FreeImage_GetWidth(dib);
	H = FreeImage_GetHeight(dib);
	if (FreeImage_GetBPP(dib) == 32)
	{
		format = GL_BGRA;
	}
	if (FreeImage_GetBPP(dib) == 24)
	{
		format = GL_BGR;
	}if (FreeImage_GetBPP(dib) == 8)
	{
		format = GL_LUMINANCE;
	}
	if (format == GL_RGBA || format == GL_BGRA)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i
			, 0, GL_RGBA, W, H, 0, format, GL_UNSIGNED_BYTE, bDataPointer);
	}

	else if (format == GL_RGB || format == GL_BGR) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i
			, 0, GL_RGB, W, H, 0, format, GL_UNSIGNED_BYTE, bDataPointer);
	}
	else
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i
			, 0, format, W, H, 0, format, GL_UNSIGNED_BYTE, bDataPointer);
	}

	FreeImage_Unload(dib);

	return true;

}
bool Texture::loadTexture(string front, string back, string right, string left, string up, string down) {
	std::vector<std::string> faces;
	faces.push_back(right);
	faces.push_back(left);
	faces.push_back(up);
	faces.push_back(down);
	faces.push_back(back);
	faces.push_back(front);

	glGenTextures(1, &this->textureId);

	glBindTexture(GL_TEXTURE_CUBE_MAP, this->textureId);

	for (int i = 0; i < faces.size(); i++)
	{
		createCubeMapFromData(faces[i], i);
	}


	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	textureType = CUBE_MAP;

	sPath = front + back + right + left + up + down;

	return true;
}
