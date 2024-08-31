#include <iostream>
#include <chrono>
#include <random>

#include "ITUGames.h"

#pragma region Engine State

std::chrono::time_point<std::chrono::steady_clock> time_start;
std::chrono::time_point<std::chrono::steady_clock> time_end;
std::chrono::duration<double> time_elapsed;

// Time in update function
std::chrono::time_point<std::chrono::steady_clock> time_start_update;

// Positon of player
std::tuple<int, int> position;

// Calculate the desired time per frame in milliseconds
double desired_frame_time = 1.0 / 60.0;

#pragma endregion // Engine State

#pragma region Game State

/**
 * If you are unsure how to organize the additional variables you will need for the exercise, just dump them here.We will address this in future lectures
 * In the meantime, a good approach is to sort them and put the ones that relate to the same thing togheter
 * - is this variable part of the game? Of the engine? ...
 * - is it about input? Player state? Logging? ...
 * And so on. Are some of those questions conflicting with each other? Yep, architecturing code is hard, but we'll get a hang of it.
 */

#pragma endregion // Game State

void LongComputation();
void ProcessEvents();
void Update();
void Render(std::chrono::time_point<std::chrono::steady_clock> time);

int main()
{
	// setup
	std::srand(std::time(nullptr)); // initializes random generator
	ITUGames::Console::ClearScreen();
	ITUGames::Console::GotoTop();
	ITUGames::Console::HideCursor();

	// declare starting position
	position = std::make_tuple(20, 10);

	// for the update loop
	time_start_update = std::chrono::steady_clock::now();
	time_end = std::chrono::steady_clock::now();

	while (true)
	{
		time_start = std::chrono::steady_clock::now();
		time_elapsed = time_start - time_end;

		LongComputation();

		ProcessEvents();
		Update();
		Render(time_start_update);

		// time after computations have ended
		std::chrono::duration<double> time_computation(time_elapsed);

		// // Convert the desired frame time to a duration
		// std::chrono::duration<double> desired_frame_duration(desired_frame_time);

		// // Calculate the remaining time to sleep
		// std::chrono::duration<double> remaining_sleep_time = desired_frame_duration - time_elapsed;

		// // Sleep for the remaining time if it's positive
		// if (remaining_sleep_time.count() > 0)
		// {
		// 	ITUGames::Utils::PreciseSleep(remaining_sleep_time);
		// }

		time_end = time_start;

		ITUGames::Console::GotoTop();
		std::cout << "FPS                 : " << static_cast<double>(1) / (time_elapsed.count()) << std::endl;
		std::cout << "Elapsed(ms)         : " << time_elapsed.count() * static_cast<double>(1000) << std::endl;
		std::cout << "Computation(ms)     : " << time_computation.count() * static_cast<double>(1000) << std::endl;
		std::cout << "Target(ms)          : " << desired_frame_time * static_cast<double>(1000) << std::endl;
	}

	return 0;
}

void LongComputation()
{
	// get a random number between 0.01 and 0.016
	double sleep_time = (10 + std::rand() % 6) / 1000;

	ITUGames::Utils::PreciseSleep(std::chrono::duration<double>(sleep_time));
}

void ProcessEvents()
{
	ITUGames::Console::ClearScreen();
	ITUGames::Console::GotoTop();

	char command = ITUGames::Console::GetCharacter(false);

	switch (command)
	{
	case ITUGames::Console::KEY_W:
		std::get<1>(position)--;
		break;
	case ITUGames::Console::KEY_S:
		std::get<1>(position)++;
		break;
	case ITUGames::Console::KEY_A:
		std::get<0>(position)--;
		break;
	case ITUGames::Console::KEY_D:
		std::get<0>(position)++;
		break;
	default:
		break;
	}
}

void Update()
{
	ITUGames::Console::GotoCoords(std::get<0>(position), std::get<1>(position));
}

void Render(std::chrono::time_point<std::chrono::steady_clock> time_start)
{
	long long time_start_nanoseconds = time_start.time_since_epoch().count();
	long long time_now_nanoseconds = std::chrono::steady_clock::now().time_since_epoch().count();
	long long time_elapsed_update = time_now_nanoseconds - time_start_nanoseconds;

	if ((time_elapsed_update % 1000000000) < 500000000)
	{
		ITUGames::Console::RenderCharacter('0');
	}
	else
		ITUGames::Console::RenderCharacter('O');
}