**EmEngine**
A windows based C++ in-terminal game engine meant to ease the creation of classic games like Rogue and Dwarf Fortress

Simply include the EmEnhine.h file in your project, and then make a derived class in your main.cpp as follows:
#include "EmEngine.h"

class Example : public EmEngine
{
public: // Unique code for your project
    
public: // Overwritten
    void PlayerUpdate(int input) override {

    }

    void FrameUpdate(float elapsedTime) override {

    }
};
