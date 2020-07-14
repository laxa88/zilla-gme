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

	~sZillaGME() {
		delete player;
		delete scope;
	}

	void Load(int argc, char *argv[])
	{
		ZL_Display::Init("Zilla GME", scope_width, 256);
		ZL_Display::SetAA(false);
		ZL_Input::Init();
		ZL_Display::sigKeyDown.connect(this, &sZillaGME::OnKeyDown);

		init();
		
		// Load file
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

	void OnKeyDown(ZL_KeyboardEvent& e)
	{
		switch (e.key)
		{
		case ZLK_ESCAPE:
			ZL_Application::Quit();
			break;

		case ZLK_LEFT:
			if (!paused && !--track)
				track = 1;
			start_track(track, path);
			break;

		case ZLK_RIGHT:
			if (track < player->track_count())
				start_track(++track, path);
			break;

		case ZLK_MINUS:
			tempo -= 0.1;
			if (tempo < 0.1)
				tempo = 0.1;
			player->set_tempo(tempo);
			break;

		case ZLK_EQUALS:
			tempo += 0.1;
			if (tempo > 2.0)
				tempo = 2.0;
			player->set_tempo(tempo);
			break;

		case ZLK_SPACE:
			paused = !paused;
			player->pause(paused);
			break;

		case ZLK_E:
			stereo_depth += 0.2;
			if (stereo_depth > 0.5)
				stereo_depth = 0;
			player->set_stereo_depth(stereo_depth);
			break;

		case ZLK_0:
			tempo = 1.0;
			muting_mask = 0;
			player->set_tempo(tempo);
			player->mute_voices(muting_mask);
			break;

		default:
			if (ZLK_1 <= e.key && e.key <= ZLK_9) // toggle muting
			{
				muting_mask ^= 1 << (e.key - ZLK_1);
				player->mute_voices(muting_mask);
			}
			break;
		}
	}
} ZillaGME;


void handle_error(const char* error)
{
	if (error)
		ZL_LOG1("Error", "%s", error);
}
