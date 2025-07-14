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

#ifndef CDGMAGIC_SCROLLCLIP_H
#define CDGMAGIC_SCROLLCLIP_H

#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Native_File_Chooser.H>

#include "CDGMagic_MediaClip.h"
#include "CDGMagic_BMPLoader.h"
#include "CDGMagic_MovableClipBox.h"

class CDGMagic_ScrollClip_Window; // Silly forward declaration due to circular header inclusion crap.

class CDGMagic_ScrollClip : public CDGMagic_MediaClip, public CDGMagic_MovableClipBox
{
private:
    int *current_time;

protected:
    int handle(int incoming_event);
    void do_edit(int win_x, int win_y);

    CDGMagic_ScrollClip_Window* internal_edit_window;
    unsigned char* internal_picon_image;
    int label_width;

public:
    CDGMagic_ScrollClip(int X, int Y, int W, int H);
    ~CDGMagic_ScrollClip();

    int serialize(char_vector* incoming_save_vector);
    int deserialize(char* incoming_save_data);

    void set_time_var(int *time);
    int  get_time();

    void draw();
    void add_bmp_file();
    void add_bmp_file(const char* requested_file);
    void null_edit();
    void set_transition_file(unsigned int requested_clip_index);
};

#endif
