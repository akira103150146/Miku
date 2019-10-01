#pragma once

#include "oglt_util.h"
#include "std_util.h"

namespace oglt {
	class UniformBufferObject
	{
	public:
		UniformBufferObject();
		~UniformBufferObject();
		void createUniformBuffer();
		void updateBuffer();
		void addData(void* data, GLuint size);
		void uploadBufferData(GLuint bindingIndex, GLenum mode);
		void deleteBuffer();

		GLuint getDataSize();
	private:
		GLuint bufferId;
		vector<void*> datas;
		vector<GLuint> sizes;

		GLuint dataSize;
	};
}