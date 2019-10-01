#include "oglt_vbo.h"

using namespace oglt;

VertexBufferObject::VertexBufferObject()
{
	bDataUploaded = false;
	bufferId = 0;
}

/*-----------------------------------------------

Name:	createVBO

Params:	a_iSize - initial size of buffer

Result:	Creates vertex buffer object.

/*---------------------------------------------*/

void VertexBufferObject::createVBO(int a_iSize)
{
	glGenBuffers(1, &bufferId);
	data.reserve(a_iSize);
	size = a_iSize;
	currentSize = 0;
}

/*-----------------------------------------------

Name:	deleteVBO

Params:	none

Result:	Releases VBO and frees all memory.

/*---------------------------------------------*/

void VertexBufferObject::deleteVBO()
{
	glDeleteBuffers(1, &bufferId);
	bDataUploaded = false;
	data.clear();
}

/*-----------------------------------------------

Name:	mapBufferToMemory

Params:	iUsageHint - GL_READ_ONLY, GL_WRITE_ONLY...

Result:	Maps whole buffer data to memory and
returns pointer to data.

/*---------------------------------------------*/

void* VertexBufferObject::mapBufferToMemory(int iUsageHint)
{
	if (!bDataUploaded)return NULL;
	void* ptrRes = glMapBuffer(bufferType, iUsageHint);
	return ptrRes;
}

/*-----------------------------------------------

Name:	mapSubBufferToMemory

Params:	iUsageHint - GL_READ_ONLY, GL_WRITE_ONLY...
uiOffset - data offset (from where should
data be mapped).
uiLength - length of data

Result:	Maps specified part of buffer to memory.

/*---------------------------------------------*/

void* VertexBufferObject::mapSubBufferToMemory(int iUsageHint, uint uiOffset, uint uiLength)
{
	if (!bDataUploaded)return NULL;
	void* ptrRes = glMapBufferRange(bufferType, uiOffset, uiLength, iUsageHint);
	return ptrRes;
}

/*-----------------------------------------------

Name:	unmapBuffer

Params:	none

Result:	Unmaps previously mapped buffer.

/*---------------------------------------------*/

void VertexBufferObject::unmapBuffer()
{
	glUnmapBuffer(bufferType);
}

/*-----------------------------------------------

Name:	bindVBO

Params:	a_iBufferType - buffer type (GL_ARRAY_BUFFER, ...)

Result:	Binds this VBO.

/*---------------------------------------------*/

void VertexBufferObject::bindVBO(int a_iBufferType)
{
	bufferType = a_iBufferType;
	glBindBuffer(bufferType, bufferId);
}

/*-----------------------------------------------

Name:	uploadDataToGPU

Params:	iUsageHint - GL_STATIC_DRAW, GL_DYNAMIC_DRAW...

Result:	Sends data to GPU.

/*---------------------------------------------*/

void VertexBufferObject::uploadDataToGPU(int iDrawingHint)
{
	glBufferData(bufferType, data.size(), &data[0], iDrawingHint);
	bDataUploaded = true;
	data.clear();
}

/*-----------------------------------------------

Name:	addData

Params:	ptrData - pointer to arbitrary data
uiDataSize - data size in GLbytes

Result:	Adds arbitrary data to VBO.

/*---------------------------------------------*/

void VertexBufferObject::addData(void* ptrData, uint uiDataSize)
{
	data.insert(data.end(), (GLubyte*)ptrData, (GLubyte*)ptrData + uiDataSize);
	currentSize += uiDataSize;
}

/*-----------------------------------------------

Name:	GetDataPointer

Params:	none

Result:	Returns data pointer (only before uploading).

/*---------------------------------------------*/

void* VertexBufferObject::getDataPointer()
{
	if (bDataUploaded)return NULL;
	return (void*)data[0];
}

/*-----------------------------------------------

Name:	GetBufferID

Params:	none

Result:	Returns VBO ID.

/*---------------------------------------------*/

uint VertexBufferObject::getBufferID()
{
	return bufferId;
}

/*-----------------------------------------------

Name:	GetCurrentSize

Params:	none

Result: Returns size of data that has been added to
the buffer object.

/*---------------------------------------------*/

int VertexBufferObject::getCurrentSize()
{
	return currentSize;
}