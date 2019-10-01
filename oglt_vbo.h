#pragma once

#include "std_util.h"
#include "oglt_util.h"

namespace oglt {
	/********************************

	Class:		VertexBufferObject

	Purpose:	Wraps OpenGL vertex buffer
	object.

	********************************/

	class VertexBufferObject
	{
	public:
		void createVBO(int a_iSize = 0);
		void deleteVBO();

		void* mapBufferToMemory(int iUsageHint);
		void* mapSubBufferToMemory(int iUsageHint, uint uiOffset, uint uiLength);
		void unmapBuffer();

		void bindVBO(int a_iBufferType = GL_ARRAY_BUFFER);
		void uploadDataToGPU(int iUsageHint);

		void addData(void* ptrData, uint uiDataSize);

		void* getDataPointer();
		uint getBufferID();

		int getCurrentSize();

		VertexBufferObject();

	private:
		uint bufferId;
		uint size;
		uint currentSize;
		int bufferType;
		vector<GLubyte> data;

		bool bDataUploaded;
	};
}