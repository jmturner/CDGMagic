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

#include <cstdio>

#include "CDGMagic_TimeOutput.h"

CDGMagic_TimeOutput::CDGMagic_TimeOutput(int X, int Y, int W, int H, const char *L) : Fl_Box(X,Y,W,H,L)
{
    // Allocate text format buffer.
    internal_time_text = new char[16];
    // Set the desired default FLTK options.
    tooltip("Time");
    box(FL_ROUNDED_BOX);
    color(FL_BLACK);
    // Initialize the time to 0.
    set_time(0);
}

CDGMagic_TimeOutput::~CDGMagic_TimeOutput()
{
    delete[] internal_time_text;
}


void CDGMagic_TimeOutput::set_time(int requested_time)
{
    sprintf( internal_time_text,
             "%02i:%02i.%02i",
             (requested_time/(300*60))%60,  // Minutes
             (requested_time/300)%60,       // Seconds
             (requested_time/4)%75 );       // Sectors (or frames?)
    redraw();
}

void CDGMagic_TimeOutput::draw(void)
{
    // Have the parent draw its box.
    Fl_Box::draw();
    // Set the font, size, and color.
    fl_font(FL_SCREEN, 24);  fl_color(FL_GREEN);
    // Draw the (centered) text.
    fl_draw( internal_time_text, x(), y(), w(), h(), FL_ALIGN_CENTER );
}
