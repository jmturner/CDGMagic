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

#ifndef CDGMAGIC_TRACKOPTIONS_H
#define CDGMAGIC_TRACKOPTIONS_H

#include <FL/Fl_Group.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Simple_Counter.H>

class CDGMagic_TrackOptions : public Fl_Group
{
protected:
    Fl_Button         *mask_edit_button;
    Fl_Simple_Counter *channel_counter;
    unsigned char      internal_track;


public:
    CDGMagic_TrackOptions(int X, int Y, int W, int H, const char *L=0);
    ~CDGMagic_TrackOptions() {};

    unsigned char  track();
    void track(unsigned char requested_track);

    unsigned char  channel();
    void channel(unsigned char requested_channel);
};

#endif
