#pragma once

#include "oglt_util.h"
#include "std_util.h"
#include <glm\glm.hpp>

namespace oglt {
	struct Cursor {
		int x, y;
	};

	class IApp {
	public:
		void getViewport(uint& width, uint& height) { 
			width = this->viewportWidth;
		    height = this->viewportHeight;
		};

		glm::mat4* getProj() { return &proj; }
		glm::mat4* getOrth() { return &orth; }
		float getDeltaTime() { return deltaTime; }
		float getFrameDeltaTime() { return frameDeltaTime; }
		int getFps() { return fps; }
		int getUps() { return ups; }

		virtual void swapBuffers(){}

		virtual void setCursor(int x, int y){}
		virtual void setCursor(OGLT_CURSOR_TYPE cursorType) {}
		virtual void getCursor(int& x, int& y){}

		virtual bool key(int ikey) { return false; }
		virtual bool oneKey(int ikey) { return false; }

	protected:
		virtual void init() {
			deltaTime = 0;
			frameDeltaTime = 0;
			curFps = 0;
			fps = 0;
			curUps = 0;
			ups = 0;
			lastClock = clock();
			lastFrameClock = clock();
			frameClockInSecond = 0;
			clockInSecond = 0;
		}

		virtual void updateFrameTimer(){
			clock_t curClock = clock();
			frameDeltaTime = (float)(curClock - lastFrameClock) / (float)CLOCKS_PER_SEC;
			frameClockInSecond += (curClock - lastFrameClock);
			if (frameClockInSecond >= CLOCKS_PER_SEC) {
				frameClockInSecond -= CLOCKS_PER_SEC;
				fps = curFps;
				curFps = 0;
			}
			curFps++;
			lastFrameClock = curClock;
		}

		virtual void updateTimer() {
			clock_t curClock = clock();
			deltaTime = (float)(curClock - lastClock) / (float)CLOCKS_PER_SEC;
			clockInSecond += (curClock - lastClock);
			if (clockInSecond >= CLOCKS_PER_SEC) {
				clockInSecond -= CLOCKS_PER_SEC;
				ups = curUps;
				curUps = 0;
			}
			curUps++;
			lastClock = curClock;
		}

		uint viewportWidth, viewportHeight;
		glm::mat4 proj, orth;

		float deltaTime, frameDeltaTime;
		int fps, curFps;
		int ups, curUps;

		clock_t lastFrameClock, lastClock;
		clock_t frameClockInSecond, clockInSecond;

		static bool keyStates[256];
		static bool oneKeyStats[256];
		static Cursor cursor;
	};
}