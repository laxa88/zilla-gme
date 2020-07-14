// Game_Music_Emu 0.5.2. http://www.slack.net/~ant/

#include "Audio_Scope.h"

#include <assert.h>
#include <stdlib.h>

/* Copyright (C) 2005-2006 by Shay Green. Permission is hereby granted, free of
charge, to any person obtaining a copy of this software module and associated
documentation files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use, copy, modify,
merge, publish, distribute, sublicense, and/or sell copies of the Software, and
to permit persons to whom the Software is furnished to do so, subject to the
following conditions: The above copyright notice and this permission notice
shall be included in all copies or substantial portions of the Software. THE
SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

int const step_bits = 8;
int const step_unit = 1 << step_bits;
int const erase_color = 1;
int const draw_color = 2;

Audio_Scope::Audio_Scope()
{
	buf = 0;
}

Audio_Scope::~Audio_Scope()
{
	free( buf );
}

const char* Audio_Scope::init( int width, int height )
{
	assert( height <= 256 );
	assert( !buf ); // can only call init() once
	
	buf = (byte*) calloc( width * sizeof *buf, 1 );
	if ( !buf )
		return "Out of memory";
	
	low_y = 0;
	high_y = height;
	buf_size = width;
	
	for ( sample_shift = 6; sample_shift < 14; )
		if ( ((0x7FFFL * 2) >> sample_shift++) < height )
			break;

	return 0; // success
}

const char* Audio_Scope::draw( const short* in, long count, double step )
{
	render( in, count, (long) (step * step_unit) );

	return 0; // success
}

void Audio_Scope::render( short const* in, long count, long step )
{
	byte* old_pos = buf;
	int old_erase = *old_pos;
	int old_draw = 0;
	long in_pos = 0;

	int low_y = this->low_y;
	int high_y = this->high_y;
	int half_step = (step + step_unit / 2) >> (step_bits + 1);

	std::vector<ZL_Vector> p;
	p.push_back({ -1.f, (scalar)(ZLHEIGHT / 2) });
	p.push_back({ -1.f, 0.f });
	p.push_back({ (scalar)count+1.f, -1.f });
	p.push_back({ (scalar)count+1.f, (scalar)(ZLHEIGHT / 2)+1.f });

	while (count--)
	{
		int surface_pitch = 1;

		{
			int in_whole = in_pos >> step_bits;
			int sample = (0x7FFF * 2 - in [in_whole] - in [in_whole + half_step]) >> sample_shift;
			if ( !in_pos )
				old_draw = sample;
			in_pos += step;
			
			int delta = sample - old_draw;
			int offset = old_draw * surface_pitch;
			old_draw += delta;
			
			int next_line = surface_pitch;
			if ( delta < 0 )
			{
				delta = -delta;
				next_line = -surface_pitch;
			}
			
			*old_pos++ = sample;
			
			// min/max updating can be interleved anywhere
			
			if ( low_y > sample )
				low_y = sample;
			
			do
			{
				offset += next_line;
				p.push_back({ (scalar)count, (scalar)(offset + 1) });
			}
			while ( delta-- > 1 );

			if ( high_y < sample )
				high_y = sample;
		}
	}

	ZL_Display::ClearFill(ZL_Color::Black);
	ZL_Polygon(ZL_Polygon::BORDER).Add(p).Draw(ZL_Color::Green);
}
