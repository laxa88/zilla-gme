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

static void init()
{
	ZL_Display::Init("Zilla GME", scope_width, 256);
	ZL_Display::SetAA(false);
	ZL_Input::Init();

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

	// update track info

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

	ZL_String::format("%s: %d/%d %s (%ld:%02ld)",
		game, track, player->track_count(), player->track_info().song,
		seconds / 60, seconds % 60);

	// TODO display text in window
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
		//path = (argc > 1 ? argv[argc - 1] : "Data/test.nsf");
		path = (argc > 1 ? argv[argc - 1] : "Data/zelda.nsf");
		handle_error(player->load_file(path));
		start_track(1, path);
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
	}
} ZillaGME;


void handle_error(const char* error)
{
	if (error)
		ZL_LOG0("Error", "%s", error);
}
