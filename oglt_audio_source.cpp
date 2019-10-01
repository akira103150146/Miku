#include "oglt_audio_source.h"

oglt::AudioSource::AudioSource()
{
	looping = false;
	playing = false;
}

oglt::AudioSource::~AudioSource()
{
}

bool oglt::AudioSource::load(const string & fileName)
{
	ALuint buffer;
	ALenum error;

	/* Create an AL buffer from the given sound file. */
	buffer = alutCreateBufferFromFile(fileName.c_str());
	if (buffer == AL_NONE)
	{
		error = alutGetError();
		fprintf(stderr, "Error loading file: '%s'\n",
			alutGetErrorString(error));
		alutExit();
		exit(EXIT_FAILURE);
	}

	/* Generate a single source, attach the buffer to it */
	alGenSources(1, &sourceId);
	alSourcei(sourceId, AL_BUFFER, buffer);

	sPath = fileName;
}

void oglt::AudioSource::play(bool looping)
{
	ALint status;
	alGetSourcei(sourceId, AL_SOURCE_STATE, &status);
	// if this source is playing ignore it
	if (status == AL_PLAYING) return;
	alSourcei(sourceId, AL_LOOPING, looping ? AL_TRUE : AL_FALSE);
	alSourcePlay(sourceId);
	playing = true;
}

void oglt::AudioSource::pause()
{
	ALint status;
	alGetSourcei(sourceId, AL_SOURCE_STATE, &status);
	// if this source is not playing ignore it
	if (status != AL_PLAYING) return;
	alSourcePause(sourceId);
}

void oglt::AudioSource::rewind()
{
	alSourceRewind(sourceId);
}

void oglt::AudioSource::attach(const AudioBufferObject & abo)
{
}

void oglt::AudioSource::set(AudioParam param, const glm::vec3 & value)
{
	switch (param) {
	case Audio_Position:
		alSource3f(sourceId, AL_POSITION, value[0], value[1], value[2]);
		position = value;
		break;
	default:
		break;
	}
}
