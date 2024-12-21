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

#ifndef CDGMAGIC_PALGLOBALCLIP_H
#define CDGMAGIC_PALGLOBALCLIP_H

#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/fl_ask.H>

#include "CDGMagic_MediaClip.h"
#include "CDGMagic_MovableClipBox.h"

class CDGMagic_PALGlobalClip_Window; // Silly forward declaration due to circular header inclusion crap.

class CDGMagic_PALGlobalClip : public CDGMagic_MediaClip, public CDGMagic_MovableClipBox
{
protected:
    int handle(int incoming_event);
    void do_edit(int win_x, int win_y);

    CDGMagic_PALGlobalClip_Window*  internal_edit_window;

public:
    CDGMagic_PALGlobalClip(int X, int Y, int W, int H);
    ~CDGMagic_PALGlobalClip();

    int serialize(char_vector* incoming_save_vector);
    int deserialize(char* incoming_save_data);

    void draw();
    void null_edit();
};

#endif
