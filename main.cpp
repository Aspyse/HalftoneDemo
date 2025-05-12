// Halftone Demo (dx11 + imgui)
// lets make something pretty

#include "engine.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
	Engine* engine;
	engine = new Engine;

	if (engine->Initialize())
	{
		engine->Run();
	}

	engine->Shutdown();
	delete engine;
	engine = 0;
	
	return 0;
}
