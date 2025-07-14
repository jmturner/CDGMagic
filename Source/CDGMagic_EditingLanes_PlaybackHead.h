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

#ifndef CDGMAGIC_EDITINGLANES_PLAYBACKHEAD_H
#define CDGMAGIC_EDITINGLANES_PLAYBACKHEAD_H

#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Pixmap.H>

class CDGMagic_EditingLanes_PlaybackHead : public Fl_Widget
{
protected:
    int handle(int incoming_event);
    Fl_Pixmap *head_icon;

public:
    CDGMagic_EditingLanes_PlaybackHead(int x, int y, int h);
    int play_position(void);
    void play_position(int play_pos);

    void draw(void);
    void over_draw(void);
};

#endif
