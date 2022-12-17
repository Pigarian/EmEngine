#include "EmEngine.h"

using namespace std;

class Example : public EmEngine
{
public: // Unique code, only needed to the purposes of this example
	Example() {
		Build(width, height, height - statSpace, 24, 24, ' ', WHITE);
		pc.icon = '@';
		pc.color = BLACK + BGDARKGRAY; // 2 layer 16 bit color system, with the main pixel colors begining with BG and the char colors begining being standard
		pc.x = rangeRand(2, width - 2);
		pc.y = rangeRand(2, height - 2);
		SetPixel(pc.icon, pc.x, pc.y, pc.color);
	}

	int rangeRand(int min, int max) {
		return (rand() % (max - min + 1)) + min;
	}

	void movePlayer(int xChange, int yChange) {
		SetPixel(pc.icon, xChange, yChange, pc.color);
		SetPixel(' ', pc.x, pc.y, WHITE);
		pc.x = xChange;
		pc.y = yChange;
	}
public: // Overwritten
	void PlayerUpdate(int input) override { // Updates every time a player inputs a command, even between logic frames
		//The code here is unique to this example, simply handles movement
		if (input == 119 && pc.y - 1 >= 1) movePlayer(pc.x, pc.y - 1); // w
		else if (input == 97 && pc.x - 1 >= 1) movePlayer(pc.x - 1, pc.y); // a
		else if (input == 115 && pc.y + 1 < height - statSpace) movePlayer(pc.x, pc.y + 1); // s
		else if (input == 100 && pc.x + 1 < width - 1) movePlayer(pc.x + 1, pc.y); // d
		else if (input == 107) Kill(); // k, ends the game
	}

	void FrameUpdate(float elapsedTime) override { // Updates every frame at ~60fps
		//Both functions must be overwitten, but neither *has* to be used, though not using them defeats the point of the engine
	}
private:
	const int width = 40;
	const int height = 40;
	const int statSpace = 5;

	struct playerInfo {
		char icon;
		int color;
		int x;
		int y;
	};
	playerInfo pc;
};

int main() {
	Example thisGame;
	thisGame.Start();
	return 0;
}//*/