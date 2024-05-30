#include "Game.h"
#include <cmath>

const int WALL_THICKNESS = 15;
const int WINDOW_W = 1024;
const int WINDOW_H = 768;
const int PADDLE_H = 100;
const float PADDLE_VEL = 400.0f;

Game::Game()
	:mWindow(nullptr),
	mRenderer(nullptr),
	mIsRunning(true),
	mPaddles{ 
		{
			{WALL_THICKNESS * 2.5f, static_cast<float>(WINDOW_H) / 2},
			0
		},
		{
			{static_cast<float>(WINDOW_W) - WALL_THICKNESS * 2.5f, static_cast<float>(WINDOW_H) / 2},
			0
		}
	},
	mBalls{ 
		{
			{static_cast<float>(WINDOW_W) / 2, static_cast<float>(WINDOW_H) / 2},
			{200.0f, 235.0f}, true
		},
		{
			{static_cast<float>(WINDOW_W) / 2, static_cast<float>(WINDOW_H) / 2},
			{100.0f, 200.0f}, true
		}
	},
	mTicksCount(0),
	mBallVelMult(1.0f),
	mPaddleVelMult(1.0f),
	mFailCounter(0)
{}

bool Game::Initialize() {
	int sdlResult = SDL_Init(SDL_INIT_VIDEO);
	if (sdlResult != 0) {
		SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
		return false;
	}
	
	mWindow = SDL_CreateWindow(
		"Pong FTW",	// name of the window
		250,		// top left x coordinate of the window
		50,			// top left y coordinate of the window
		WINDOW_W,	// width
		WINDOW_H,	// height
		0			// flags
	);
	if (!mWindow) {
		SDL_Log("Failed to create window: %s", SDL_GetError());
		return false;
	}

	mRenderer = SDL_CreateRenderer(
		mWindow,
		-1,
		SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
	);
	if (!mRenderer) {
		SDL_Log("Failed to create renderer: %s", SDL_GetError());
		return false;
	}

	return true;
}

void Game::Shutdown() {
	SDL_DestroyWindow(mWindow);
	SDL_DestroyRenderer(mRenderer);
	SDL_Quit();
}

void Game::RunLoop() {
	while(mIsRunning){
		ProcessInput();
		UpdateGame();
		GenerateOutput();
	}
}

void Game::ProcessInput() {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type){
		case SDL_QUIT:
			mIsRunning = false;
			break;
		default:
			break;
		} 
	}

	// array with the state of the keyboard
	const Uint8* keyboardState = SDL_GetKeyboardState(NULL);
	// exit key
	if (keyboardState[SDL_SCANCODE_ESCAPE]) {
		mIsRunning = false;
	}

	// controls player left
	if (keyboardState[SDL_SCANCODE_W]) {
		mPaddles[0].dir = -1;
	}
	if (keyboardState[SDL_SCANCODE_S]) {
		mPaddles[0].dir = 1;
	}

	// controls player right
	if (keyboardState[SDL_SCANCODE_I]) {
		mPaddles[1].dir = -1;
	}
	if (keyboardState[SDL_SCANCODE_K]) {
		mPaddles[1].dir = 1;
	}
}

void Game::GenerateOutput() {
	// set renderer color to draw
	SDL_SetRenderDrawColor(
		mRenderer,
		0,
		0,
		255,
		255
	);
	SDL_RenderClear(mRenderer);	// fill background main color

	SDL_SetRenderDrawColor(
		mRenderer,
		255,
		255,
		255,
		255
	);

	SDL_Rect upperWall = SDL_Rect{
		0,
		0,
		WINDOW_W,
		WALL_THICKNESS
	};
	SDL_RenderFillRect(mRenderer, &upperWall);

	SDL_Rect lowerWall = SDL_Rect{
		0,
		WINDOW_H - WALL_THICKNESS,
		WINDOW_W,
		WALL_THICKNESS
	};
	SDL_RenderFillRect(mRenderer, &lowerWall);

	SDL_Rect paddle1 = SDL_Rect{
		static_cast<int>(mPaddles[0].pos.x - WALL_THICKNESS / 2),
		static_cast<int>(mPaddles[0].pos.y - PADDLE_H / 2),
		WALL_THICKNESS,
		PADDLE_H
	};
	SDL_RenderFillRect(mRenderer, &paddle1);

	SDL_Rect paddle2 = SDL_Rect{
		static_cast<int>(mPaddles[1].pos.x - WALL_THICKNESS / 2),
		static_cast<int>(mPaddles[1].pos.y - PADDLE_H / 2),
		WALL_THICKNESS,
		PADDLE_H
	};
	SDL_RenderFillRect(mRenderer, &paddle2);

	for (Ball& ball : mBalls) {
		SDL_Rect cBall = SDL_Rect{
			static_cast<int>(ball.pos.x) - WALL_THICKNESS / 2,
			static_cast<int>(ball.pos.y) - WALL_THICKNESS / 2,
			WALL_THICKNESS,
			WALL_THICKNESS
		};
		SDL_RenderFillRect(mRenderer, &cBall);
	}

	SDL_RenderPresent(mRenderer);
}

void Game::UpdateGame() {
	// wait until 16ms has passed since last frame
	const Uint64 timeout = mTicksCount + 16;
	while (SDL_GetTicks64() < timeout);

	// time diference in ticks from the last frame (in seconds)
	float deltaTime = (SDL_GetTicks64() - mTicksCount) / 1000.0f;
	mTicksCount = SDL_GetTicks64();

	// set max value if it too high due to breakpoint 
	// or similar situations
	if (deltaTime > 0.05f) {
		deltaTime = 0.05f;
	}

	for (Paddle& paddle : mPaddles) {
		// updates the position of the paddle based on the direction
		if (paddle.dir != 0) {
			paddle.pos.y += paddle.dir * PADDLE_VEL * mPaddleVelMult * deltaTime;
		}

		// paddle collition with walls
		if (paddle.pos.y + static_cast<float>(PADDLE_H) / 2 > WINDOW_H - WALL_THICKNESS) {
			paddle.pos.y = WINDOW_H - WALL_THICKNESS - static_cast<float>(PADDLE_H) / 2;
		}
		if (paddle.pos.y - static_cast<float>(PADDLE_H) / 2 < WALL_THICKNESS) {
			paddle.pos.y = WALL_THICKNESS + static_cast<float>(PADDLE_H) / 2;
		}
	}

	// updates ball position based on velocity vector
	for (Ball& ball : mBalls) {
		if (!ball.active) {
			continue;
		}

		ball.pos.x += ball.vel.x * deltaTime * mBallVelMult;
		ball.pos.y += ball.vel.y * deltaTime * mBallVelMult;

		// ball collition with walls
		if (ball.pos.y + static_cast<float>(WALL_THICKNESS) / 2 > WINDOW_H - WALL_THICKNESS) {
			// double check the ball direction, avoiding stuck ball
			if (ball.vel.y > 0.0f) {
				mBallVelMult += 0.1f;
				ball.vel.y *= -1;
			}
		}
		if (ball.pos.y - static_cast<float>(WALL_THICKNESS) / 2 < WALL_THICKNESS) {
			// double check the ball direction, avoiding stuck ball
			if (ball.vel.y < 0.0f) {
				mBallVelMult += 0.1f;
				ball.vel.y *= -1;
			}
		}

		// ball collition with paddle
		if (ball.pos.x - static_cast<float>(WALL_THICKNESS) / 2 < mPaddles[0].pos.x + static_cast<float>(WALL_THICKNESS) / 2
			&& std::abs(ball.pos.y - mPaddles[0].pos.y) < static_cast<float>(PADDLE_H) / 2) {
			// double check the ball direction, avoiding stuck ball
			if (ball.vel.x < 0.0f) {
				mBallVelMult += 0.2f;
				mPaddleVelMult += 0.1f;
				ball.vel.x *= -1;
			}
		}
		if (ball.pos.x + static_cast<float>(WALL_THICKNESS) / 2 > mPaddles[1].pos.x - static_cast<float>(WALL_THICKNESS) / 2
			&& std::abs(ball.pos.y - mPaddles[1].pos.y) < static_cast<float>(PADDLE_H) / 2) {
			// double check the ball direction, avoiding stuck ball
			if (ball.vel.x > 0.0f) {
				mBallVelMult += 0.2f;
				mPaddleVelMult += 0.1f;
				ball.vel.x *= -1;
			}
		}

		// counts the number of balls that crossed the limit
		if (ball.pos.x + static_cast<float>(WALL_THICKNESS) / 2 < 0.0f
			|| ball.pos.x - static_cast<float>(WALL_THICKNESS) / 2 > WINDOW_W) {
			mFailCounter += 1;
			ball.active = false;
		}
	}

	// end condition
	if (mFailCounter >= 2) {
		mIsRunning = false;
	}

	// reset the paddle's direction movement
	// otherwise it will move forever
	for (Paddle& paddle : mPaddles) {
		paddle.dir = 0;
	}

	// check for max value
	if (mBallVelMult >= 3.0f) {
		mBallVelMult = 3.0f;
	}
	if (mPaddleVelMult >= 2.0f) {
		mPaddleVelMult = 2.0f;
	}
}