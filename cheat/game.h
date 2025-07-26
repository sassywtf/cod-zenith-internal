#pragma once

#include "../gui/framework.h"
#include "../gui/imgui/imgui.h"
#include "../gui/imgui/imgui_internal.h"
#include "../globals.h"
#include "../sdks/sdk.h"
#include "math.h"

namespace g_game
{
	void init(ImFont* font);
	void hookcheck();
}