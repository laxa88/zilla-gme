#include "include.h"

int const scope_width = 512;

#include "Music_Player.h"
#include "Audio_Scope.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void handle_error(const char*);

static bool paused;
static Audio_Scope* scope;
static Music_Player* player;
static short scope_buf[scope_width * 2];

//bool mixer(short* buffer, unsigned int samples, bool need_mix)
//{
//	int len = (samples << 1);
//
//	for (int i = 0; i < len; i++) {
//		buffer[i] = RAND_RANGE(INT8_MIN, INT8_MAX);
//	}
//
//	return need_mix;
//}

static void init()
{
	ZL_Display::Init("Zilla GME", 854, 480);
	ZL_Input::Init();

	// Start SDL
	/*if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
		exit(EXIT_FAILURE);
	atexit(SDL_Quit);
	SDL_EnableKeyRepeat(500, 80);*/

	// Init scope
	scope = new Audio_Scope;
	if (!scope)
		handle_error("Out of memory");
	if (scope->init(scope_width, 256))
		handle_error("Couldn't initialize scope");
	memset(scope_buf, 0, sizeof scope_buf);

	// Create player
	player = new Music_Player;
	if (!player)
		handle_error("Out of memory");
	handle_error(player->init());
	player->set_scope_buffer(scope_buf, scope_width * 2);
}

static void start_track(int track, const char* path)
{
	paused = false;
	handle_error(player->start_track(track - 1));

	// update window title with track info

	long seconds = player->track_info().length / 1000;
	const char* game = player->track_info().game;
	if (!*game)
	{
		// extract filename
		game = strrchr(path, '\\'); // DOS
		if (!game)
			game = strrchr(path, '/'); // UNIX
		if (!game)
			game = path;
		else
			game++; // skip path separator
	}

	char title[512];
	sprintf(title, "%s: %d/%d %s (%ld:%02ld)",
		game, track, player->track_count(), player->track_info().song,
		seconds / 60, seconds % 60);
	//SDL_WM_SetCaption(title, title);
}

static struct sZillaGME : public ZL_Application
{
	ZL_Surface srfBuffer;

	int track = 1;
	double tempo = 1.0;
	bool running = true;
	double stereo_depth = 0.0;
	int muting_mask = 0;
	const char* path;

	sZillaGME() : ZL_Application(60) { }

	void Load(int argc, char *argv[])
	{
		init();
		
		// Load file
		path = (argc > 1 ? argv[argc - 1] : "Data/test.nsf");
		handle_error(player->load_file(path));
		start_track(1, path);

		//srfBuffer = ZL_Surface(256, 256); //initialize a 256x256 render target texture
		//srfBuffer.RenderToBegin(); //start drawing onto the render target texture
		//ZL_Display::ClearFill(ZL_Color::Red); //clear the texture to fully red
		//ZL_Surface srfLogo("Data/ZILLALIB.png"); //load another surface texture
		//srfLogo.DrawTo(10.0f, 10.0f, 90.0f, 90.0f); //draw the newly loaded surface texture into the buffer
		//srfLogo.DrawTo(70.0f, 70.0f, 150.0f, 150.0f); //again
		//srfLogo.DrawTo(130.0f, 130.0f, 210.0f, 210.0f); //again
		//srfBuffer.RenderToEnd(); //end drawing to the texture

		/*ZL_Audio::Init();
		ZL_Audio::HookAudioMix(&mixer);*/
	}

	void AfterFrame()
	{
		// Update scope
		scope->draw(scope_buf, scope_width, 2);

		// Automatically go to next track when current one ends
		if (player->track_ended())
		{
			if (track < player->track_count())
				start_track(++track, path);
			else
				player->pause(paused = true);
		}






		//ZL_Vector SurfacePosition(299.0f, 112.0f);
		//if (ZL_Input::Held())
		//{
		//	//Draw into the buffer while the mouse is pressed, draw in 1 pixel steps from the old mouse position to the current position
		//	ZL_Vector MousePosInImage = ZL_Input::Pointer() - SurfacePosition, MouseMoveDir = ZL_Input::PointerDelta().VecNorm();
		//	scalar MouseMoveTotal = (ZL_Input::Down() ? 0.0f : ZL_Math::Max(ZL_Input::PointerDelta().GetLength() - 1.0f, 0.0f));
		//	srfBuffer.RenderToBegin();
		//	for (scalar i = 0; i <= MouseMoveTotal; i++) {
		//		//ZL_Display::FillCircle(MousePosInImage - MouseMoveDir * i, 1.0f, ZL_Color::Blue);
		//		
		//	}
		//	ZL_Display::DrawLine(MousePosInImage, MousePosInImage - ZL_Input::PointerDelta(), ZL_Color::White);
		//	srfBuffer.RenderToEnd();
		//}

		//ZL_Display::ClearFill(ZL_Color::Black); //clear whole screen
		//srfBuffer.Draw(SurfacePosition);
	}
} ZillaGME;


void handle_error(const char* error)
{
	if (error)
	{
		// put error in window title
		char str[256];
		sprintf(str, "Error: %s", error);
		fprintf(stderr, str);
		//SDL_WM_SetCaption(str, str);

		// wait for keyboard or mouse activity
		/*SDL_Event e;
		do
		{
			while (!SDL_PollEvent(&e)) {}
		} while (e.type != SDL_QUIT && e.type != SDL_KEYDOWN && e.type != SDL_MOUSEBUTTONDOWN);*/

		//exit(EXIT_FAILURE);
	}
}
