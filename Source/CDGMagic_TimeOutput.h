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

#include <FL/Fl.H>
//#include <FL/Fl_Widget.H>
#include <FL/Fl_Box.H>
#include <FL/fl_draw.H>

class CDGMagic_TimeOutput : public Fl_Box//Fl_Widget
{
private:
    char* internal_time_text;

public:
    CDGMagic_TimeOutput(int X, int Y, int W, int H, const char *L=0);
    CDGMagic_TimeOutput(int X, int Y);
    ~CDGMagic_TimeOutput();

    void set_time(int requested_time);
    void draw(void);
};
