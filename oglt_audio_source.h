#pragma once

#include <alut.h>

#include "std_util.h"
#include "oglt_abo.h"
#include <glm\glm.hpp>

namespace oglt {
	enum AudioParam {
		Audio_Position
	};

	class AudioSource {
	public:
		AudioSource();
		~AudioSource();

		bool load(const string& fileName);
		void play(bool looping = false);
		void pause();
		void rewind();
		void attach(const AudioBufferObject& abo);

		void set(AudioParam param, const glm::vec3& value);
	private:
		ALuint sourceId;
		
		bool looping, playing;

		glm::vec3 position;

		string sPath;
	};
}