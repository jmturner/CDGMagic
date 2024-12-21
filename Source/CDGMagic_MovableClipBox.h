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

// Library to extend immobile Fl_Box to moveable.

#ifndef CDGMAGIC_MOVABLECLIPBOX_H
#define CDGMAGIC_MOVABLECLIPBOX_H

#include <FL/Fl_Box.H>

class CDGMagic_MovableClipBox : public Fl_Box
{
protected:
    int handle(int incoming_event);
    int is_movable;
    int *internal_playtime_var;

public:
    CDGMagic_MovableClipBox(int X, int Y, int W, int H, const char *L=0);
    void set_movable(int move_setting);
    void set_playtime_var(int *req_playtime_var);
    void seek_and_play(int requested_seek_time);
    int play_time();
    int get_x_visible();
};



#endif
