#include "alut_backend.h"

#include "std_util.h"

#include <alut.h>

void oglt::alutBackendInit(int argc, char **argv)
{
	/* Initialise ALUT and eat any ALUT-specific commandline flags. */
	if (!alutInit(&argc, argv))
	{
		ALenum error = alutGetError();
		fprintf(stderr, "%s\n", alutGetErrorString(error));
		exit(EXIT_FAILURE);
	}
}

void oglt::alutBackendExit()
{
	if (!alutExit())
	{
		ALenum error = alutGetError();
		fprintf(stderr, "%s\n", alutGetErrorString(error));
		exit(EXIT_FAILURE);
	}
}

void oglt::alutBackendPlayFile(const char * fileName, bool looping)
{
	ALuint buffer;
	ALuint source;
	ALenum error;
	ALint status;

	/* Create an AL buffer from the given sound file. */
	buffer = alutCreateBufferFromFile(fileName);
	if (buffer == AL_NONE)
	{
		error = alutGetError();
		fprintf(stderr, "Error loading file: '%s'\n",
			alutGetErrorString(error));
		alutExit();
		exit(EXIT_FAILURE);
	}

	/* Generate a single source, attach the buffer to it and start playing. */
	alGenSources(1, &source);
	alSourcei(source, AL_BUFFER, buffer);
	if (looping) {
		alSourcei(source, AL_LOOPING, AL_TRUE);
	}
	alSourcePlay(source);

	/* Normally nothing should go wrong above, but one never knows... */
	error = alGetError();
	if (error != ALUT_ERROR_NO_ERROR)
	{
		fprintf(stderr, "%s\n", alGetString(error));
		alutExit();
		exit(EXIT_FAILURE);
	}
}
