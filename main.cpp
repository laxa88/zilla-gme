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
	ZL_Font fnt;

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
		fnt = ZL_Font("Data/fntMain.png");

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

		RenderButtons();

		// Automatically go to next track when current one ends
		if (player->track_ended())
		{
			if (track < player->track_count())
				start_track(++track, path);
			else
				player->pause(paused = true);
		}
	}

	void PlayPrevTrack()
	{
		if (!paused && !--track)
			track = 1;
		start_track(track, path);
	}

	void PlayNextTrack()
	{
		if (track < player->track_count())
			start_track(++track, path);
	}

	void DecreaseTempo()
	{
		tempo -= 0.1;
		if (tempo < 0.1)
			tempo = 0.1;
		player->set_tempo(tempo);
	}

	void IncreaseTempo()
	{
		tempo += 0.1;
		if (tempo > 2.0)
			tempo = 2.0;
		player->set_tempo(tempo);
	}

	void TogglePause()
	{
		paused = !paused;
		player->pause(paused);
	}

	void ToggleEcho()
	{
		stereo_depth += 0.2;
		if (stereo_depth > 0.5)
			stereo_depth = 0;
		player->set_stereo_depth(stereo_depth);
	}

	void ResetSettings()
	{
		tempo = 1.0;
		muting_mask = 0;
		player->set_tempo(tempo);
		player->mute_voices(muting_mask);
	}

	void ToggleTrackChannel(int key)
	{
		if (ZLK_1 <= key && key <= ZLK_9) // toggle muting
		{
			muting_mask ^= 1 << (key - ZLK_1);
			player->mute_voices(muting_mask);
		}
	}

	void RenderButtons()
	{
		scalar buttonW = 70.f;
		scalar buttonH = 40.f;

		if (Button(ZL_Rectf::BySize({ 0.f, 0.f }, { buttonW, buttonH }), "<<"))
			PlayPrevTrack();

		if (Button(ZL_Rectf::BySize({ buttonW*1.f, 0.f }, { buttonW, buttonH }), ">>"))
			PlayNextTrack();

		if (Button(ZL_Rectf::BySize({ buttonW*2.f, 0.f }, { buttonW, buttonH }), "-Tempo"))
			DecreaseTempo();

		if (Button(ZL_Rectf::BySize({ buttonW*3.f, 0.f }, { buttonW, buttonH }), "+Tempo"))
			IncreaseTempo();

		

		if (Button(ZL_Rectf::BySize({ buttonW * 5.f, 0.f }, { buttonW, buttonH }), "Echo"))
			ToggleEcho();

		if (Button(ZL_Rectf::BySize({ buttonW * 6.f, 0.f }, { buttonW, buttonH }), paused ? "Play" : "Pause"))
			TogglePause();



		if (Button(ZL_Rectf::BySize({ 0.f, ZLHEIGHT-buttonH }, { buttonW, buttonH }), "CH1"))
			ToggleTrackChannel(ZLK_1);

		if (Button(ZL_Rectf::BySize({ buttonW * 1.f, ZLHEIGHT - buttonH }, { buttonW, buttonH }), "CH2"))
			ToggleTrackChannel(ZLK_2);

		if (Button(ZL_Rectf::BySize({ buttonW * 2.f, ZLHEIGHT - buttonH }, { buttonW, buttonH }), "CH3"))
			ToggleTrackChannel(ZLK_3);

		if (Button(ZL_Rectf::BySize({ buttonW * 3.f, ZLHEIGHT - buttonH }, { buttonW, buttonH }), "CH4"))
			ToggleTrackChannel(ZLK_4);



		if (Button(ZL_Rectf::BySize({ buttonW * 6.f, ZLHEIGHT-buttonH }, { buttonW, buttonH }), "Reset"))
			ResetSettings();
	}

	void OnKeyDown(ZL_KeyboardEvent& e)
	{
		switch (e.key)
		{
		case ZLK_ESCAPE:
			ZL_Application::Quit();
			break;

		case ZLK_LEFT:
			PlayPrevTrack();
			break;

		case ZLK_RIGHT:
			PlayNextTrack();
			break;

		case ZLK_MINUS:
			DecreaseTempo();
			break;

		case ZLK_EQUALS:
			IncreaseTempo();
			break;

		case ZLK_SPACE:
			TogglePause();
			break;

		case ZLK_E:
			ToggleEcho();
			break;

		case ZLK_0:
			ResetSettings();
			break;

		default:
			ToggleTrackChannel(e.key);
			break;
		}
	}

	bool Button(const ZL_Rectf& rec, const char* txt)
	{
		ZL_Display::DrawRect(rec, ZLALPHA(.8), ZLALPHA(ZL_Input::Held(rec) ? .6 : (ZL_Input::Hover(rec) ? .3 : .1)));
		fnt.Draw(rec.Center(), txt, ZL_Origin::Center);
		return (ZL_Input::Clicked(rec) != 0);
	}
} ZillaGME;


void handle_error(const char* error)
{
	if (error)
		ZL_LOG1("Error", "%s", error);
}
