#pragma once
#include "../gui/framework.h"
#include "../globals.h"
#include "../sdks/sdk.h"

//Fonts
extern ImFont* mainfont;
extern ImFont* arrow;
extern ImDrawList* drawredef;

class cMenu
{
public:
	void DrawMenu();
};

void LoadSettings(int number);

char currentMessage[257]; // Create a separate variable to store the current message