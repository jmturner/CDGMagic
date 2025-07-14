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

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/fl_draw.H>

#include "CDGMagic_EditingGroup.h"
#include "CDGMagic_EditingLanes.h"
#include "CDGMagic_MovableClipBox.h"

CDGMagic_MovableClipBox::CDGMagic_MovableClipBox(int X, int Y, int W, int H, const char *L) : Fl_Box(X,Y,W,H,L)
{
    box(FL_FLAT_BOX);
    color(FL_GRAY, FL_WHITE);
    is_movable = 1;
    internal_playtime_var = NULL;
}

void CDGMagic_MovableClipBox::set_movable(int move_setting)
{
    is_movable = move_setting;
}

void CDGMagic_MovableClipBox::set_playtime_var(int *req_playtime_var)
{
    internal_playtime_var = req_playtime_var;
}

int CDGMagic_MovableClipBox::play_time()
{
    return *internal_playtime_var;
}

void CDGMagic_MovableClipBox::seek_and_play(int requested_seek_time)
{
    CDGMagic_EditingGroup *editing_group = dynamic_cast<CDGMagic_EditingGroup*>( parent()->parent() );
    editing_group->seek_and_play(requested_seek_time);
}

int CDGMagic_MovableClipBox::get_x_visible()
{
    CDGMagic_EditingLanes *editing_lanes = dynamic_cast<CDGMagic_EditingLanes*>( parent() );
    int return_value = x() - editing_lanes->xposition() + 300;// - editing_lanes->x());
    return return_value;
}

int CDGMagic_MovableClipBox::handle(int incoming_event)
{
    static int orig_x = 0, orig_y = 0, new_x = 0, new_y = 0, offset_x = 0, offset_y = 0;//, orig_w = w(); // Was needed for duration adjustment.

    switch (incoming_event)
    {
        case FL_PUSH:
        {
            // Bail out if we didn't get left click.
            if (Fl::event_button() != FL_LEFT_MOUSE)  { break; };
            // Store the box origin points at the time of click.
            orig_x = x();
            orig_y = y();
            //orig_w = w(); // Was needed for duration adjustment.
            // Calculate the offset for dragging.
            offset_x = orig_x - Fl::event_x();
            offset_y = orig_y - Fl::event_y();
            // Take keyboard focus.
            take_focus();
            // Return non-zero because we used the event.
            return(1);
        };

        case FL_RELEASE:
        {
            // Bail out if we didn't get left click.
            if (Fl::event_button() != FL_LEFT_MOUSE)  { break; };
            // Fire the call back to update the new position.
            do_callback();
            // Return non-zero because we used the event.
            return 1;
        };
        case FL_DRAG:
        {
            // Do nothing (pass off, but don't handle, event) if the left mouse button was not pressed.
            if ( (Fl::event_button() != FL_LEFT_MOUSE) || (is_movable == 0) )  { break; };
            // Get a pointer to editing lanes object.
            CDGMagic_EditingLanes *editing_lanes = dynamic_cast<CDGMagic_EditingLanes*>( parent() );
            // Calculate the new X and Y position from the initial click position vs. the current "event" position.
            new_x = offset_x+Fl::event_x();
            new_y = offset_y+Fl::event_y();
            // Check that we're not at the beginning of the scroll area (and we probably aren't).
            // And if we're within 20 pixels of the left edge, then decrease scroll bar position.
            if (  (editing_lanes->xposition() > 0) && ( (Fl::event_x()-editing_lanes->x()) < 20)  )
            {
                int toscr = (100-(Fl::event_x()-editing_lanes->x()+80)) / 10;
                if (toscr < 0) { toscr = 0; };
                toscr = editing_lanes->xposition()-toscr;
                if (toscr < 0) { toscr = 0; };
                editing_lanes->scroll_to(toscr, editing_lanes->yposition());
            };
            // Check if the event X position is closer than 10 pixels from the right edge.
            if (Fl::event_x() > (editing_lanes->w()+editing_lanes->x()-50))
            {
                int toscr = (Fl::event_x()-(editing_lanes->w()+editing_lanes->x()-50)) / 10;
                if (toscr > 10) { toscr = 10; };
                // Then increase the scroll bar position.
                editing_lanes->scroll_to(editing_lanes->xposition()+toscr, editing_lanes->yposition());
            };
            // Check if the new X position is going to be before the start of the timeline.
            if ( (new_x+editing_lanes->xposition()-editing_lanes->x()) <= 0 )
            {
                // If so, then set the new X position to the minimum allowed value.
                new_x = editing_lanes->x() - editing_lanes->xposition() + 1;
            };
            // Constrain y values to multiples of lane_height + the start of first lane.
            new_y = ((Fl::event_y()-editing_lanes->y()-15)/(editing_lanes->lane_height()+1)) * (editing_lanes->lane_height()+1) + editing_lanes->y() + 15;
            // Make sure we aren't before the second lane.
            if (new_y < editing_lanes->y()+15+(editing_lanes->lane_height()+1)*2 ) { new_y = editing_lanes->y()+15+(editing_lanes->lane_height()+1)*2; };
            // Make sure we aren't the eighth lane (which is off the bottom of the timeline).
            if (new_y > editing_lanes->y()+15+(editing_lanes->lane_height()+1)*9 ) { new_y = editing_lanes->y()+15+(editing_lanes->lane_height()+1)*9; };
            // Determine if we must change the clip duration, and reflect that in the width.
            // NOTE: Commented out because this can't actually work anymore due to the draw() code...
            // The code to actually support automatic variable bandwidth (i.e. duration) isn't implmented in the encoder.
            // If it were, the draw method duration set could modified, and this reenabled.
            /*
            if (offset_x > -6)
            {
                int old_end = x()+w();
                int new_beg = old_end-new_x;
                w(new_beg);
            }
            else if  (offset_x < -(orig_w-7))
            {
                int orig_event = orig_x - offset_x;
                w(orig_w + (Fl::event_x() - orig_event));
                new_x = orig_x;
            };
            */
            // Set the newly calculated box position.
            position(new_x, new_y);
            // Redraw the box (brute forcing eliminates weird "droppings", but is obviously slower :-/).
            editing_lanes->redraw();
            // Return true because we handled the event.
            return 1;
        };
        // Change cursor type (if needed) while mousing over or leaving the box.
        // NOTE: The code to make duration setting actually work is not yet implemented in the encoder.
        // When/if it is, this along with w() dragging code above can be reenabled.
        /*
        case FL_MOVE:
        case FL_LEAVE:
        {
            int distance = Fl::event_x()-x();
            if (incoming_event == FL_MOVE && ((distance < 6) || (distance > (w()-7))) )
            {
                window()->cursor(FL_CURSOR_WE);
            }
            else
            {
                window()->cursor(FL_CURSOR_DEFAULT);
            };
            return 1;
        };
        */
        // Force a redraw when a box is selected/deselected (so the "highlight" color is redrawn).
        // Fallthrough IS INTENDED!... (I.E. Always redraw on focus change, only bring to top when coming in to focus.)
        case FL_FOCUS:      parent()->add(this);
        case FL_UNFOCUS:    redraw();  return 1;

        case FL_KEYBOARD:
        {
            CDGMagic_EditingLanes *editing_lanes = dynamic_cast<CDGMagic_EditingLanes*>( parent() );
            if (Fl::event_key() == FL_Left)
            {
                int new_x_pos = (Fl::event_state() & FL_CTRL) ? x()-10 : x()-1 ;
                // Check if the new X position is going to be before the start of the timeline.
                if ( (new_x_pos+editing_lanes->xposition()-editing_lanes->x()) <= 0 )
                {
                    // If so, then set the new X position to the minimum allowed value.
                    new_x_pos = editing_lanes->x() - editing_lanes->xposition() + 1;
                };
                position( new_x_pos, y() );
            };
            if (Fl::event_key() == FL_Right)
            {
                int new_x_pos = (Fl::event_state() & FL_CTRL) ? x()+10 : x()+1 ;
                // TODO: Somehow need to figure out whether we're past the end of the timeline.
                position( new_x_pos, y() );
            };
            if (Fl::event_key() == FL_Up)
            {
                int min_y_pos = (editing_lanes->y() + 15) + ((editing_lanes->lane_height()+1) * 2);
                int new_y_pos = (Fl::event_state() & FL_CTRL) ? min_y_pos : y()-editing_lanes->lane_height()-1;
                new_y_pos = (new_y_pos > min_y_pos) ? new_y_pos : min_y_pos;
                position( x(), new_y_pos );
            };
            if (Fl::event_key() == FL_Down)
            {
                int max_y_pos = (editing_lanes->y() + 15) + ((editing_lanes->lane_height()+1) * 9);
                int new_y_pos = (Fl::event_state() & FL_CTRL) ? max_y_pos : y()+editing_lanes->lane_height()+1;
                new_y_pos = (new_y_pos < max_y_pos) ? new_y_pos : max_y_pos;
                position( x(), new_y_pos );
            };
            if (Fl::event_key() == FL_Delete)
            {
                CDGMagic_EditingGroup *editing_group = dynamic_cast<CDGMagic_EditingGroup*>( parent()->parent() );
                editing_group->delete_clip(dynamic_cast<CDGMagic_MediaClip*>(this));
                Fl::delete_widget(this);
            };
            editing_lanes->redraw();
            return(1);
        };
    };

    return Fl_Box::handle(incoming_event);
}
