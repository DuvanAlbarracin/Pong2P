#pragma once
#include <SDL.h>
#include <vector>

struct Vector2 {
	float x;
	float y;
};

struct Ball {
	Vector2 pos;
	Vector2 vel;
	bool active;
};

struct Paddle {
	Vector2 pos;
	int dir;
};

class Game {
public:
	Game();
	bool Initialize();
	void RunLoop();
	void Shutdown();

private:
	void ProcessInput();
	void UpdateGame();
	void GenerateOutput();

	SDL_Window* mWindow;
	bool mIsRunning;
	SDL_Renderer* mRenderer;

	std::vector<Paddle> mPaddles;
	std::vector<Ball> mBalls;
	
	Uint64 mTicksCount;

	float mPaddleVelMult;
	float mBallVelMult;

	int mFailCounter;
};