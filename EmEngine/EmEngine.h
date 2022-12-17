#pragma once
#include <stdlib.h>
#include <iostream>
#include <windows.h>
#include <vector>
#include <chrono>
#include <thread>
#include <mutex>
#include <conio.h>
using namespace std;

class EmEngine {
public: // PUBLIC VARIABLES AND TYPES
    struct ScreenSegments {
        char** pixel;
        WORD** pixelColor;
    };
    mutex mtx;
    enum {
        BLACK = 0,
        DARKBLUE = FOREGROUND_BLUE,
        DARKGREEN = FOREGROUND_GREEN,
        DARKCYAN = FOREGROUND_GREEN | FOREGROUND_BLUE,
        DARKRED = FOREGROUND_RED,
        DARKMAGENTA = FOREGROUND_RED | FOREGROUND_BLUE,
        DARKYELLOW = FOREGROUND_RED | FOREGROUND_GREEN,
        DARKGRAY = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
        GRAY = FOREGROUND_INTENSITY,
        BLUE = FOREGROUND_INTENSITY | FOREGROUND_BLUE,
        GREEN = FOREGROUND_INTENSITY | FOREGROUND_GREEN,
        CYAN = FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE,
        RED = FOREGROUND_INTENSITY | FOREGROUND_RED,
        MAGENTA = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_BLUE,
        YELLOW = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN,
        WHITE = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
        // For black, don't add a BG color
        BGDARKBLUE = BACKGROUND_BLUE,
        BGDARKGREEN = BACKGROUND_GREEN,
        BGDARKCYAN = BACKGROUND_GREEN | BACKGROUND_BLUE,
        BGDARKRED = BACKGROUND_RED,
        BGDARKMAGENTA = BACKGROUND_RED | BACKGROUND_BLUE,
        BGDARKYELLOW = BACKGROUND_RED | BACKGROUND_GREEN,
        BGDARKGRAY = BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE,
        BGGRAY = BACKGROUND_INTENSITY,
        BGBLUE = BACKGROUND_INTENSITY | BACKGROUND_BLUE,
        BGGREEN = BACKGROUND_INTENSITY | BACKGROUND_GREEN,
        BGCYAN = BACKGROUND_INTENSITY | BACKGROUND_GREEN | BACKGROUND_BLUE,
        BGRED = BACKGROUND_INTENSITY | BACKGROUND_RED,
        BGMAGENTA = BACKGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_BLUE,
        BGYELLOW = BACKGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_GREEN,
        BGWHITE = BACKGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE,
    };
    // ScreenSegments: Though the main memory made from this struct is made private, the struct is made public for Build() and to hold returns from ReadScreen()
    // mtx: Mutex made available for locking PlayerUpdate based animations, WILL CAUSE CRASH IF USED IN FRAME UPDATE
    // enum: 16 bit color values stored as references
public: //PUBLIC CALLS
    // BASIC SET UP CALLS
    EmEngine() {
        running = false;
        consoleOut = 0;
        screen.pixel = new char* [0];
        screen.pixelColor = new WORD * [0];
    }
    void Build(int width, int height, int mapLimit, int fontw, int fonth, char standardChar, WORD standardColor) {
        srand(time(0));
        COORD coord = { (short)width, (short)height };
        consoleOut = GetStdHandle(STD_OUTPUT_HANDLE);
        SMALL_RECT rectWindow = { 0, 0, 1, 1 };
        SetConsoleWindowInfo(consoleOut, TRUE, &rectWindow);
        SetConsoleScreenBufferSize(consoleOut, coord);
        SetConsoleActiveScreenBuffer(consoleOut);
        HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_CURSOR_INFO info;
        info.dwSize = 100;
        info.bVisible = FALSE;
        SetConsoleCursorInfo(consoleHandle, &info);
        rectWindow = { 0, 0, (short)(width - 1), (short)(height - 1)};
        SetConsoleWindowInfo(consoleOut, TRUE, &rectWindow);
        //Font setup
        CONSOLE_FONT_INFOEX cfi;
        cfi.cbSize = sizeof(cfi);
        cfi.nFont = 0;
        cfi.dwFontSize.X = fontw;
        cfi.dwFontSize.Y = fonth;
        cfi.FontFamily = FF_DONTCARE;
        cfi.FontWeight = FW_NORMAL;
        SetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), FALSE, &cfi);
        //Pixel setup
        screen.pixel = new char* [height];
        screen.pixelColor = new WORD * [height];
        for (int i = 0; i < height; i++) {
            screen.pixel[i] = new char[width];
            screen.pixelColor[i] = new WORD[width];
        }
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width - 1; x++) {
                SetPixel(standardChar, x, y, standardColor);
            }
        }
    }
    void Start() {
        running = true;
        thread eThread = thread(&EmEngine::EngineThread, this);
        thread pThread = thread(&EmEngine::PlayerThread, this);
        eThread.join();
        pThread.join();
    }
    void Kill() {
        running = false;
    }
    // Build(): Sets up the console to fit the shape required by your game
    // Start(): Starts the threads for PlayerUpdate() and FrameUpdate()
    // Kill(): Ends the while loops that keep PlayerUpdate() and FrameUpdate() running, therefore ending the game on the engine side

    // UPDATE THREADS, BOTH NEED TO BE OVERWRITTEN
    virtual void PlayerUpdate(int input) = 0;
    virtual void FrameUpdate(float elapsedTime) = 0;
    // PlayerUpdate(): Caled from PlayerThread(), with input being the last heard keypress
    // FrameUpdate(): Called from EngineThread(), with elapsedTime referencing the amount of passed time since last frame

    // VISUAL ENGINE CALLS
    void SetPixel(char charPixel, int x, int y, WORD color) {
        screen.pixel[y][x] = charPixel;
        screen.pixelColor[y][x] = color;
        UpdateInsturctions temp;
        temp.x = x;
        temp.y = y;
        updates.push_back(temp);
    }
    ScreenSegments ReadScreen() {
        return screen;
    }
    // SetPixel(): Sets a pixels value in "screen" to be updated on display in EngineThread()
    // ReadScrean(): Returns a read only reference to the current value of every pixel on the screen

    // AUDIO ENGINE CALLS
    void AudioPlay(string file, bool loop) {
        wstring toPlay(file.begin(), file.end());
        if (loop) PlaySound(toPlay.c_str(), NULL, SND_FILENAME | SND_NODEFAULT | SND_ASYNC | SND_LOOP);
        else PlaySound(toPlay.c_str(), NULL, SND_FILENAME | SND_NODEFAULT | SND_ASYNC);
    }
    void KillAllAudio() {
        PlaySound(NULL, 0, 0);
    }
    // AudioPlay(): Plays a .wav file associated with the address given, silent on faulty address
    // KillAllAudio(): Kills all audio at once
private: // PRIVATE VARIABLES AND TYPES
    bool running;
    HANDLE consoleOut;
    ScreenSegments screen;
    struct UpdateInsturctions {
        int x;
        int y;
    };
    vector<UpdateInsturctions> updates;
    // running: The bool that keeps EngineThread() and PlayerThread() running
    // consoleOut: The reference to the console
    // screen: Contains a reference to every pixel on the screen
    // UpdateInstructions: A struct to hold a set of coordinates
    // updates: Hold every update instruction in order of when they were called, to be updated in that order on a call by EngineThread() and then cleared
private: // PRIVATE CALLS
    // THREAD CONTROL
    void EngineThread() {
        auto tp1 = chrono::system_clock::now();
        auto tp2 = chrono::system_clock::now();
        while (running)
        {
            mtx.lock();
            tp2 = chrono::system_clock::now();
            chrono::duration<float> elapsedTime = tp2 - tp1;
            tp1 = tp2;
            float fElapsedTime = elapsedTime.count();
            FrameUpdate(fElapsedTime);
            if (updates.size() > 0) SetScreen();
            mtx.unlock();
            this_thread::sleep_for(chrono::microseconds(16)); // 60fps
        }
    }
    void PlayerThread() {
        while (running) {
            int input = _getch();
            PlayerUpdate(input);
        }
    }
    // EngineThread(): Keeps track of time between frames, running at maximum 60fps. Also responsible for calling visual updates on the engine side
    // PlayerThread(): Awaits a keyboard input, then calls PlayerInput with the ASCII value

    // SCREEN UPDATES
    void SetScreen() {
        for (int i = 0; i < updates.size(); i++) HardSetPixel(updates[i]);
        updates.clear();
    }
    void HardSetPixel(UpdateInsturctions updateInfo) {
        DWORD tempMem;
        COORD tempCoord;
        tempCoord.X = updateInfo.x;
        tempCoord.Y = updateInfo.y;
        WriteConsoleOutputAttribute(consoleOut, &screen.pixelColor[updateInfo.y][updateInfo.x], 1, tempCoord, &tempMem);
        WriteConsoleOutputCharacterA(consoleOut, &screen.pixel[updateInfo.y][updateInfo.x], 1, tempCoord, &tempMem);
    }
    // SetScreen(): Calls HardSetPixel() for every coordinate in "updates"
    // HardSetPixel(): Updates the screen to match the record in "screen"
};

/*HOW TO SET UP:
Paste the following into main/source.cpp:

#include "EmEngine.h"

class Example : public EmEngine
{
public: // Unique code for your project
    Example() {
        //Build(width, height, height - (desired stat space), 24, 24, ' ', 0);
    }
public: // Overwritten
    void PlayerUpdate(int input) override {

    }

    void FrameUpdate(float elapsedTime) override {

    }
};

int main() {
    Example thisGame;
    thisGame.Start();
    return 0;
}
*/