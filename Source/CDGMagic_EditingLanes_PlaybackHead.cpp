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

#include "CDGMagic_EditingLanes.h"
#include "CDGMagic_EditingLanes_PlaybackHead.h"

#include "playback_head_xpm.h"

CDGMagic_EditingLanes_PlaybackHead::CDGMagic_EditingLanes_PlaybackHead(int X, int Y, int H) : Fl_Widget(X,Y,11,H,0)
{
    head_icon = new Fl_Pixmap(playback_head_xpm);
}

int CDGMagic_EditingLanes_PlaybackHead::play_position(void)
{
    CDGMagic_EditingLanes *scrtemp = (CDGMagic_EditingLanes*) parent();
    return (x()+scrtemp->xposition()+5-scrtemp->x()) * 4;
}

void CDGMagic_EditingLanes_PlaybackHead::play_position(int play_pos)
{
    CDGMagic_EditingLanes *scrtemp = (CDGMagic_EditingLanes*) parent();
    int new_scroll_pos = play_pos/4 - scrtemp->w()/2;
    if (new_scroll_pos < 0) {new_scroll_pos = 0;};
    scrtemp->scroll_to( new_scroll_pos, scrtemp->yposition() );
    if (new_scroll_pos)
    {
        x( scrtemp->x()+scrtemp->w()/2-5 );
    }
    else
    {
        x( scrtemp->x()+play_pos/4-5 );
    };
    parent()->redraw();
}

void CDGMagic_EditingLanes_PlaybackHead::draw(void)
{
    // Short circuit default draw method, so we always draw on top of EVERYTHING.
    // [TODO: See if there's a better way to do this, perhaps by fiddling with add() or the child() array.]
    return;
}

void CDGMagic_EditingLanes_PlaybackHead::over_draw(void)
{
    // Draw the "head" triangle icon at the widget position.
    head_icon->draw(x(), y());
    // Set the color to a light blue-ish.
    fl_color(0x0080FFFF);
    // Draw a vertical line down from the play head to the bottom of the widget.
    fl_line(x()+5, y()+6, x()+5, y()+h());
}

int CDGMagic_EditingLanes_PlaybackHead::handle(int incoming_event)
{
    // Print some information about this event to the console for debugging.
#ifdef _DEBUG_PLAYBACKHEAD_EVENTS_
    printf("EdtPHEAD_e:%i,X:%i,Y:%i,EvX:%i,EvY:%i,Push:%i,Below:%i\n", incoming_event, x(), y(), Fl::event_x(), Fl::event_y(), Fl::pushed(), Fl::belowmouse() );
#endif
    // Set up the variable to store positional data between call.
    static int origin_x = 0, offset_x = 0, new_x = 0;
    // Switch based on the event type.
    switch (incoming_event)
    {
        case FL_PUSH:
        {
            // If we didn't a left button event, we ignore and pass it through.
            if (Fl::event_button() != FL_LEFT_MOUSE) { break; } ;
            // Store the origin X position of the playback head.
            origin_x = x();
            // Calculate the offset origin X position of the event, relative to this Widget.
            offset_x = x() - Fl::event_x();
            // Take focus to update the selection color and grab positioning keyboard keys.
            take_focus();
            // Return non-zero to stop additional processing of the current event.
            return 1;
        };

        case FL_DRAG:
        {
            // If we didn't a left button event, we ignore and pass it through.
            if (Fl::event_button() != FL_LEFT_MOUSE) { break; } ;
            // Calculate the new X position of this Widget.
            new_x = offset_x + Fl::event_x();
            // Get a pointer to parent CDGMagic_EditingLanes object. [TODO: The cast and silliness is very messy/evil... Find a better way to do this!]
            CDGMagic_EditingLanes *editing_lanes = (CDGMagic_EditingLanes*) parent();
            // Keep the head away from the left side of window. [TODO: Determine how to set up clipping so it doesn't leave "droppings".]
            if (new_x < editing_lanes->x()+5)
            {
                // Constrain the playback head to five pixels from the lanes to prevent overwriting/droppings.
                new_x = editing_lanes->x()+5;
                // If the scroll position is not at the beginning, then scroll a bit.
                if ( editing_lanes->xposition() > 0 )  { editing_lanes->scroll_to(editing_lanes->xposition()-10, editing_lanes->yposition() ); };
            }
            // Keep the head away from the right side of window. [TODO: Determine how to set up clipping so it doesn't leave "droppings".]
            else if (new_x > editing_lanes->x()+editing_lanes->w()-15 )
            {
                // Constrain the playback head from the lanes to prevent overwriting/droppings.
                new_x = editing_lanes->x()+editing_lanes->w()-15;
                // TODO: if ( editing_lanes->xposition() < max_length )
                // If the scroll position is not at the end, then scroll a bit.
                editing_lanes->scroll_to(editing_lanes->xposition()+10, editing_lanes->yposition() );
            };
            // Set the new Widget X position.
            x(new_x);
            // If the X position changed, then fire the callback to update play position.
            if (x() != origin_x)  { do_callback(); };
            // Redraw the parent container (wonder if I can just set some kind of damage thing here?).
            parent()->redraw();
            // Return non-zero to stop additional processing of the current event.
            return 1;
        };
        // Set the cursor to hand when it's over the play head, and back to the default when not.
        case FL_ENTER:  {  window()->cursor(FL_CURSOR_HAND);  return 1;  };
        case FL_LEAVE:  {  window()->cursor(FL_CURSOR_DEFAULT);  return 1;  };
        // Force a redraw when losing/grabbing focus.  [TODO: Again, find a better way than brute force for this.]
        case FL_FOCUS:
        case FL_UNFOCUS:    redraw();  return 1;

        case FL_RELEASE:
        {
            // This is messy, but just fire the callback if right click -- the callback checks again for right-click, fires the renders preview callback.
            if (Fl::event_button() == FL_RIGHT_MOUSE) { do_callback(); return 1; } ;
        };
    };
    // Pass the event off to other widgets for possible processing.
    return Fl_Widget::handle(incoming_event);
}

