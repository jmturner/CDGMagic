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

// Library to extend Fl_Scroll with "media lanes" support.

#ifndef CDGMAGIC_EDITINGLANES_H
#define CDGMAGIC_EDITINGLANES_H

#include <FL/Fl_Scroll.H>

#include "CDGMagic_EditingLanes_PlaybackHead.h"

class CDGMagic_EditingLanes : public Fl_Scroll
{
#ifdef _CDGMAGIC_NOLANESHANDLEREDRAW_
private:
    int internal_handler_redraw;
#endif

protected:
    int handle(int incoming_message);

public:
    int current_lane_height;
    CDGMagic_EditingLanes_PlaybackHead *playback_head;

    CDGMagic_EditingLanes(int X, int Y, int W, int H, const char *L=0);
    CDGMagic_EditingLanes(int X, int Y);

    void draw();
    int lane_height();
    void lane_height(int height);
    void set_handler_redraw(int requested_redraw_mode);
};

#endif
