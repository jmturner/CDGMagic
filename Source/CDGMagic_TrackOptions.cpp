/*
 *  This file is part of CD+Graphics Magic.
 *
 *  CD+Graphics Magic is free software: you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation, either version 2 of the
 *  License, or (at your option) any later version.
 *
 *  CD+Graphics Magic is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with CD+Graphics Magic. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "CDGMagic_TrackOptions.h"

CDGMagic_TrackOptions::CDGMagic_TrackOptions(int X, int Y, int W, int H, const char *L) : Fl_Group(X,Y,W,H,L)
{
    mask_edit_button = new Fl_Button(X, Y, W, H/2, "Mask");
    mask_edit_button->tooltip("Static Track Font Mask\n(Not yet implemented.)");
    mask_edit_button->deactivate();
//    mask_edit_button->callback(DoMaskEdit_callback_static, this);

    channel_counter = new Fl_Simple_Counter(X, Y+H/2, W, H/2);
    channel_counter->tooltip("Channel (0-15)\n"\
                             "Note: Not yet fully implemented!\n"\
                             "If placing content on channels other than 0,\n"\
                             "please verify the output using a third party player.");
    channel_counter->bounds(0.0, 15.0);
    channel_counter->step(1.0);
    channel_counter->value(0.0);

    internal_track = 0;

    end();
}

unsigned char CDGMagic_TrackOptions::channel()
{
    return static_cast<unsigned char>( channel_counter->value() );
}

void CDGMagic_TrackOptions::channel(unsigned char requested_channel)
{
    double temp_channel = static_cast<unsigned char>( requested_channel );
    if (temp_channel > 15.0)  { temp_channel = 15.0; };
    channel_counter->value( temp_channel );
    channel_counter->redraw();
}

unsigned char  CDGMagic_TrackOptions::track()  { return internal_track; }
void CDGMagic_TrackOptions::track(unsigned char requested_track)  { internal_track = requested_track; }
