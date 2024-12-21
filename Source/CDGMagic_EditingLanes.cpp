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

#include <FL/Fl.H>
#include <FL/fl_ask.H>
#include <FL/fl_draw.H>

#include "CDGMagic_EditingLanes.h"

CDGMagic_EditingLanes::CDGMagic_EditingLanes(int X, int Y, int W, int H, const char *L) : Fl_Scroll(X,Y,W,H,L)
{
    // Set the default lane height.
    current_lane_height = 50;
#ifdef _CDGMAGIC_NOLANESHANDLEREDRAW_
    internal_handler_redraw = 0;
#endif
    // We want a basic, opaque but borderless box.
    box(FL_FLAT_BOX);
    // Neutral gray color for background.
    color(46);
    // Always show the horizontal scroll bar, never vertical (maybe we should add a vertical option?).
    type(HORIZONTAL_ALWAYS);
    // Create the playback head widget.
    playback_head = new CDGMagic_EditingLanes_PlaybackHead(X, Y+1, H-2-16);
}


int CDGMagic_EditingLanes::lane_height()  {  return current_lane_height;  }

void CDGMagic_EditingLanes::lane_height(int height)  {  current_lane_height = height;  }

int CDGMagic_EditingLanes::handle(int incoming_event)
{
    // Print out some debug info about the incoming event.
#ifdef _DEBUG_EDITINGLANES_EVENTS_
    printf("EdtLANES_e:%i,X:%i,Y:%i,EvX:%i,EvY:%i,Push:%i,Below:%i\n", incoming_event, x(), y(), Fl::event_x(), Fl::event_y(), Fl::pushed(), Fl::belowmouse() );
    if (incoming_event == FL_DND_ENTER) {return 1;};
    if (incoming_event == FL_DND_DRAG) {return 1;};
    if (incoming_event == FL_DND_RELEASE) {return 1;};
    if (incoming_event == FL_PASTE)
    {
        printf("Paste: %s\n", Fl::event_text() );
        printf("Length: %i\n", Fl::event_length() );
        return 1;
    };
#endif
    // I'm not even going to attempt to understand this...
    // The redraw() here consumes maybe 50% CPU while the program is idle...
    // BUT, removing the redraw() causes all sorts of crashes(?!?!) in Fl_Group while loading projects...
    // So, the conditional here will be enabled when doing so IF compiled with the ifdef conditional.
    // (Unfortunately, it doesn't seem to prevent ALL crashes under X... And it isn't necessary at all under Windows. Hence the compile time directive.)
    // OSX hasn't been tested either way.
#ifdef _CDGMAGIC_NOLANESHANDLEREDRAW_
    if (internal_handler_redraw > 0)
#endif
    {
        // Resize the playback head bar. [TODO: Find a better way to do this than from every call back of the parent(!!)]
        playback_head->size( playback_head->w(), h()-2-16);
        // Force a redraw of the Lanes. [TODO: Again, this shouldn't be required... Find out what's needed to get proper redrawing without brute force.]
        redraw();
    };
    // Check if the event was a left button push.
    if ( (incoming_event == FL_PUSH) && (Fl::event_button() == FL_LEFT_MOUSE) )
    {
        // Check if it occured within the top positioning portion of the lanes scroller. [TODO: Make some constansts (or variables) instead of hard coding these.]
        if ( ((Fl::event_y() < y()+14)) && (Fl::event_x() > x()+10) && (Fl::event_x() < x()+w()-10) )
        {
            // Pass focus to the playback head widget/object.
            Fl::pushed(playback_head);
            // Update the playback head position to the event position.
            playback_head->position( Fl::event_x()-5, playback_head->y() );
            // Which then requires a redraw of the lanes...
            redraw();
            // Fire the playback head callback (to update the various shown timeline positions).
            playback_head->do_callback();
            // We used the event, so return non-zero.
            return 1;
        };
    };
    // Pass the event off to other widgets for possible processing.
    return Fl_Scroll::handle(incoming_event);
}

void CDGMagic_EditingLanes::draw()
{
    // Set FL_DAMAGE_ALL to force a full redraw (ugh...).
    damage(FL_DAMAGE_ALL);
    // Invoke the parent class' original draw method.
    Fl_Scroll::draw();
    // Set the current drawing foreground color to black.
    fl_color(FL_BLACK);

    // Draw all of our lanes/lines.
    fl_line(x(), y(), x(), y()+h()-1);                // Left line.
    fl_line(x()+w()-1, y(), x()+w()-1, y()+h()-1);    // Right line.

    fl_line(x(), y(), x()+w()-1, y());                // Position scrub (top) line begin.
    fl_line(x(), y()+ 7, x()+w()-1, y()+ 7);        // Time ruler line begin.

    char tmp_buff[16];
    fl_font(FL_SCREEN, 8);
    for (int line_x = (75-xposition())%75; line_x < w(); line_x += 75)  // Time (vertical lines) units.
    {
        if (line_x > 0)
        {
            int curr_frame = line_x + xposition();
            int min = curr_frame / ( 75 * 60 );
            int sec = (curr_frame / 75) % 60;
            sprintf(tmp_buff, "%02i:%02i", min, sec);
            fl_color(FL_BLACK);
            fl_line(x()+line_x, y()+7, x()+line_x, y()+14);
            fl_color(FL_GREEN);
#ifdef WIN32
            fl_draw(tmp_buff, x()+line_x+5, y()+fl_height()+6);
#else
            fl_draw(tmp_buff, x()+line_x+5, y()+fl_height());
#endif
        };
    };

    fl_color(FL_BLACK);
    for (int line_y = 14; line_y < hscrollbar.y()-y(); line_y += current_lane_height+1)  // Media lanes begin.
    {
        fl_line(x(), y()+line_y, x()+w()-1, y()+line_y);
    };

    // Draw the playback head.
    playback_head->over_draw();
}

#ifdef _CDGMAGIC_NOLANESHANDLEREDRAW_
void CDGMagic_EditingLanes::set_handler_redraw(int requested_redraw_mode)  { internal_handler_redraw = requested_redraw_mode; }
#endif
