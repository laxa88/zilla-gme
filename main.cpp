#include "include.h"

static struct sZillaGME : public ZL_Application
{
	sZillaGME() : ZL_Application(60) { }

	virtual void Load(int argc, char *argv[])
	{
		if (!ZL_Application::LoadReleaseDesktopDataBundle()) return;
		if (!ZL_Display::Init("Zilla Game Music Player", 1280, 720, ZL_DISPLAY_ALLOWRESIZEHORIZONTAL)) return;
		ZL_Display::ClearFill(ZL_Color::White);
		ZL_Display::SetAA(true);
		ZL_Audio::Init();
		ZL_Application::SettingsInit("ZillaGME");
		
		ZL_SceneManager::Init(SCENE_GAME);
	}
} ZillaGME;
