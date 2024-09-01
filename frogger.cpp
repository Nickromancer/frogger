#include <iostream>
#include <chrono>
#include <random>
#include <map>
#include <tuple>
#include <string>

#include "ITUGames.h"

#pragma region Engine State

struct ScreenBuffer
{
    std::map<std::tuple<int, int>, char> buffer;
};

ScreenBuffer currentBuffer;
ScreenBuffer nextBuffer;

std::chrono::time_point<std::chrono::steady_clock> time_start;
std::chrono::time_point<std::chrono::steady_clock> time_end;
std::chrono::duration<double> time_elapsed;

// Time in update function
std::chrono::time_point<std::chrono::steady_clock> time_start_update;

// time after 1 second of ticks
long long time_elapsed_update;

// Positon of player
std::tuple<int, int> position;
// Start positon of player
std::tuple<int, int> start_position = std::make_tuple(ITUGames::Console::GetTerminalHeight() / 2, ITUGames::Console::GetTerminalWidth() - 10);

// Previous positon of player
std::tuple<int, int> last_position = std::make_tuple(-1, -1);

int finish_line_height = 5;
int finish_line_length;

// Bool for showing debugg log
bool bDebug = false;

// Bool for flipping player
bool bZero = true;

// Bool for obstacles triggering
bool bStep = false;

// Number of wins
int win_amount = 0;
// Number of deaths
int death_amount;

// Obstacle Array
std::vector<std::tuple<int, int>> obstacleArray;

// Game ticks per second
int ticks_per_second = 5;

// Rows in level
int rows = 3;

// Spacing of object rows
int spacing = 5;

// Offset from top
int offset = 10;

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
void Update(std::chrono::time_point<std::chrono::steady_clock> time);
void Render();

void SwapBuffersAndRender();
void ResolveConflicts(ScreenBuffer &buffer);
void WriteStringToBuffer(ScreenBuffer &buffer, int x, int y, const std::string &str);
void WriteCharToBuffer(ScreenBuffer &buffer, int x, int y, char ch);

int main()
{
    // setup
    std::srand(std::time(nullptr)); // initializes random generator
    ITUGames::Console::GotoTop();
    ITUGames::Console::HideCursor();
    finish_line_length = ITUGames::Console::GetTerminalHeight();
    ITUGames::Console::ClearScreen();

    // declare starting position
    position = std::make_tuple(20, 10);

    death_amount = 0;

    // for the update loop
    time_start_update = std::chrono::steady_clock::now();

    // Initialize obstacles into vector
    for (int i = offset; i < (rows * spacing) + offset; i += spacing)
    {
        for (int k = 0; k < finish_line_length; k += 10)
        {
            obstacleArray.push_back(std::make_tuple((k + i) % finish_line_length, i));
            obstacleArray.push_back(std::make_tuple((k + i) % finish_line_length + 1, i));
            obstacleArray.push_back(std::make_tuple((k + i) % finish_line_length + 2, i));
            obstacleArray.push_back(std::make_tuple((k + i) % finish_line_length + 3, i));
        }
    }

    while (true)
    {
        time_start = std::chrono::steady_clock::now();

        // LongComputation();
        ProcessEvents();
        Update(time_start_update);
        Render();
        ResolveConflicts(currentBuffer);
        SwapBuffersAndRender();

        time_end = std::chrono::steady_clock::now();
        time_elapsed = time_end - time_start;

        // time after computations have ended
        std::chrono::duration<double> time_computation(time_elapsed);

        // Convert the desired frame time to a duration
        std::chrono::duration<double> desired_frame_duration(desired_frame_time);

        // Calculate the remaining time to sleep
        std::chrono::duration<double> remaining_sleep_time = desired_frame_duration - time_elapsed;

        // Sleep for the remaining time if it's positive
        if (remaining_sleep_time.count() > 0)
        {
            ITUGames::Utils::PreciseSleep(remaining_sleep_time);
        }

        time_end = std::chrono::steady_clock::now();
        time_elapsed = time_end - time_start;

        // Print either debug log or stats
        ITUGames::Console::GotoTop();
        if (bDebug)
        {
            std::cout << "FPS                 : " << std::flush << static_cast<double>(1) / (time_elapsed.count()) << std::endl;
            std::cout << "Elapsed(ms)         : " << std::flush << time_elapsed.count() * static_cast<double>(1000) << std::endl;
            std::cout << "Computation(ms)     : " << std::flush << time_computation.count() * static_cast<double>(1000) << std::endl;
            std::cout << "Target(ms)          : " << std::flush << desired_frame_time * static_cast<double>(1000) << std::endl;
        }
        else
        {
            std::cout << "Wins   : " << std::flush << win_amount << std::endl;
            std::cout << "Deaths : " << std::flush << death_amount << std::endl;
        }
        // Print finish line
        ITUGames::Console::GotoCoords(0, finish_line_height);
        for (int i = 0; i < finish_line_length; i++)
        {
            std::cout << '#' << std::flush;
        }
    }

    return 0;
}

void LongComputation()
{
    // get a random number between 0.01 and 0.016
    double sleep_time = (10 + std::rand() % 6) / 1000;

    ITUGames::Utils::PreciseSleep(std::chrono::duration<double>(sleep_time));
}

// Read Input from player and system
void ProcessEvents()
{
    // Check input from player
    char command = ITUGames::Console::GetCharacter(false);
    switch (command)
    {
    case ITUGames::Console::KEY_W:
        last_position = position;
        std::get<1>(position)--;
        break;
    case ITUGames::Console::KEY_S:
        last_position = position;
        std::get<1>(position)++;
        break;
    case ITUGames::Console::KEY_A:
        last_position = position;
        std::get<0>(position)--;
        break;
    case ITUGames::Console::KEY_D:
        last_position = position;
        std::get<0>(position)++;
        break;
    case ITUGames::Console::KEY_1:
        bDebug = !bDebug;
        ITUGames::Console::ClearScreen();
        break;
    default:
        break;
    }
}

// Update the game state and values
void Update(std::chrono::time_point<std::chrono::steady_clock> time_start)
{
    long long time_start_nanoseconds = time_start.time_since_epoch().count();
    long long time_now_nanoseconds = std::chrono::steady_clock::now().time_since_epoch().count();
    time_elapsed_update = time_now_nanoseconds - time_start_nanoseconds;

    // Set out player to flip after each half fps
    if ((time_elapsed_update % 1000000000) < 500000000)
    {
        bZero = false;
    }
    else
        bZero = true;

    // Set our obstacles to tick once after each half fps
    if (time_elapsed_update % (1000000000 / ticks_per_second) > (500000000 / ticks_per_second))
        bStep = true;

    // If the player reaches the finish line, get a point and reset position
    if (std::get<1>(position) == finish_line_height)
    {
        win_amount++;
        position = start_position;
    }

    // Moves all obstacles after one tick each half second
    if (bStep && time_elapsed_update % (1000000000 / ticks_per_second) < (500000000 / ticks_per_second))
    {
        for (std::tuple<int, int> &obstacle : obstacleArray)
        {
            std::get<0>(obstacle)++;
        }

        bStep = false;
        // Write each obstacle to buffer
        for (int i = 0; i < obstacleArray.size(); i += 4)
        {
            if (std::get<0>(obstacleArray[i]) > 4 && std::get<0>(obstacleArray[i]) < finish_line_length - 4)
            {
                WriteStringToBuffer(nextBuffer, std::get<0>(obstacleArray[i]), std::get<1>(obstacleArray[i]), "XXXX");
                WriteStringToBuffer(nextBuffer, (std::get<0>(obstacleArray[i]) - 1), std::get<1>(obstacleArray[i]), " ");
            }
        }
    }

    // Check if our obstacles reach the end, then move them to the start
    for (std::tuple<int, int> &obstacle : obstacleArray)
    {
        if (std::get<0>(obstacle) == finish_line_length)
        {
            std::get<0>(obstacle) = 0;
        }
    }

    // If the player touches a obstacle, die and reset position
    for (std::tuple<int, int> &obstacle : obstacleArray)
    {
        if (std::get<0>(position) == std::get<0>(obstacle) && std::get<1>(position) == std::get<1>(obstacle))
        {
            death_amount++;
            position = start_position;
        }
    }
}

void Render()
{

    // Print player
    if (bZero)
    {
        WriteCharToBuffer(nextBuffer, std::get<0>(position), std::get<1>(position), '0');
    }
    else
        WriteCharToBuffer(nextBuffer, std::get<0>(position), std::get<1>(position), 'O');

    // Clear previous player position
    WriteCharToBuffer(nextBuffer, std::get<0>(last_position), std::get<1>(last_position), ' ');
}

void WriteCharToBuffer(ScreenBuffer &buffer, int x, int y, char ch)
{
    buffer.buffer[std::make_tuple(x, y)] = ch;
}

void WriteStringToBuffer(ScreenBuffer &buffer, int x, int y, const std::string &str)
{
    for (size_t i = 0; i < str.size(); ++i)
    {
        buffer.buffer[std::make_tuple(x + i, y)] = str[i];
    }
}

void ResolveConflicts(ScreenBuffer &buffer)
{
    // Implement any conflict resolution logic if needed
    // For simplicity, we keep the last written character

    // Remove duplicate entries in the buffer
    for (auto it = buffer.buffer.begin(); it != buffer.buffer.end();)
    {
        int x = std::get<0>(it->first);
        int y = std::get<1>(it->first);
        char ch = it->second;

        // Check if the current position in the buffer is the same as the current position in the current buffer
        if (currentBuffer.buffer.count(std::make_tuple(x, y)) > 0 && currentBuffer.buffer[std::make_tuple(x, y)] == ch)
        {
            // Remove the entry from the buffer
            it = buffer.buffer.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void SwapBuffersAndRender()
{
    // Clear the console screen
    // ITUGames::Console::ClearScreen();

    // Render the next buffer to the console
    for (const auto &entry : nextBuffer.buffer)
    {
        int x = std::get<0>(entry.first);
        int y = std::get<1>(entry.first);
        char ch = entry.second;
        ITUGames::Console::GotoCoords(x, y);
        std::cout << ch << std::flush;
    }

    // Swap buffers
    currentBuffer = nextBuffer;
    nextBuffer.buffer.clear();
}

// Render the game state
/* void Render()
{
    // Clear each row
    if (time_elapsed_update % (1000000000 / ticks_per_second) < (500000000 / ticks_per_second))
    {
        for (int i = 0; i < rows; i++)
            ITUGames::Console::ClearLine(offset + (spacing * i));
    }
    // Print finish line
    ITUGames::Console::GotoCoords(0, finish_line_height);
    for (int i = 0; i < finish_line_length; i++)
    {
        std::cout << '#' << std::flush;
    }

    // Print player
    ITUGames::Console::GotoCoords(std::get<0>(position), std::get<1>(position));
    if (bZero)
    {
        ITUGames::Console::RenderCharacter('0');
    }
    else
        ITUGames::Console::RenderCharacter('O');

    // Clear previous player position
    ITUGames::Console::GotoCoords(std::get<0>(last_position), std::get<1>(last_position));
    ITUGames::Console::RenderCharacter(' ');

    // Print each obstacle
    if (time_elapsed_update % (1000000000 / ticks_per_second) < (500000000 / ticks_per_second))
    {
        for (int i = 0; i < obstacleArray.size(); i += 4)
        {
            if (std::get<0>(obstacleArray[i]) > 4 && std::get<0>(obstacleArray[i]) < finish_line_length - 4)
            {
                ITUGames::Console::GotoCoords(std::get<0>(obstacleArray[i]), std::get<1>(obstacleArray[i]));
                std::cout << "XXXX" << std::flush << "\r";
            }
        }
    }
} */
