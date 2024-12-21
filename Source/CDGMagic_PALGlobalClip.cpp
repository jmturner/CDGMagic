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

#include "CDGMagic_PALGlobalClip.h"
#include "CDGMagic_PALGlobalClip_Window.h"

#include "CDGMagic_PALObject.h"

#include "CDGMagic_EditingGroup.h"
#include "CDGMagic_EditingLanes.h"

CDGMagic_PALGlobalClip::CDGMagic_PALGlobalClip(int X, int Y, int W, int H) : CDGMagic_MediaClip(0, 0), CDGMagic_MovableClipBox(X,Y,W,H,0)
{
    box(FL_FLAT_BOX);
    color(FL_GRAY);
    is_movable = 0;
    internal_edit_window = NULL;
    duration( w() * 4 );
    copy_label("this is the global graphics commands clip");

    // Make a new event entry.
    CDGMagic_MediaEvent *current_event_entry = new CDGMagic_MediaEvent;
    // Set up the values.
    current_event_entry->start_offset = 250; // About 833ms from the beginning of the track.
    current_event_entry->actual_start_offset = current_event_entry->start_offset;
    current_event_entry->duration     = 0;
    current_event_entry->actual_duration = current_event_entry->duration;
    current_event_entry->BMPObject = NULL;
    current_event_entry->PALObject = NULL;
    current_event_entry->user_obj = NULL;
    current_event_entry->border_index = 16;         // Leave border alone.
    current_event_entry->memory_preset_index = 16;  // Don't clear screen.
    // This just sets the scroll offsets to 0, in case there's still offsets in the player's buffer.
    // Otherwise, a small amount of image data at the edge of the screen could be cut off.
    // It would be good to set transparency to opaque for all indices as well,
    // but the transparency object isn't implemented yet. (Maybe it should be part of the PALObject?)
    current_event_entry->x_scroll = 0; // Set X offset to 0 pixels.
    current_event_entry->y_scroll = 0; // Sey Y offset to 0 pixels.
    // Add it to this clip's event queue.
    event_queue()->push_back(current_event_entry);
}

CDGMagic_PALGlobalClip::~CDGMagic_PALGlobalClip()
{
    // Loop through and delete all the PALObject pointers (event structs are deleted by MediaClip parent).
    for (unsigned int current_event = 0; current_event < event_queue()->size(); current_event++)
    {
        delete (event_queue()->at(current_event)->PALObject);
    };
}

void CDGMagic_PALGlobalClip::null_edit()
{
    internal_edit_window = NULL;
}

void CDGMagic_PALGlobalClip::draw()
{
    CDGMagic_MovableClipBox::draw(); // Placeholder for our own extra drawing.
}

int CDGMagic_PALGlobalClip::handle(int incoming_event)
{
    if ( (incoming_event == FL_RELEASE) && (Fl::event_button() == FL_LEFT_MOUSE) )
    {
        if (Fl::event_clicks() == 1)
        {
            // Disabled for now, due to the global clip editing window not having any controls implemented yet.
            //do_edit(Fl::event_x_root(), Fl::event_y_root());
            // Just show a simple alert instead.
            fl_message("This *should* be an edit window to allow arbitrary or timed placement of\n"\
                       "palette, screen/border preset, and transparency commands...\n\n"\
                       "Unfortunately, it hasn't been implemented yet.");
            return 1;
        };
    };
    return CDGMagic_MovableClipBox::handle(incoming_event);
}

void CDGMagic_PALGlobalClip::do_edit(int win_x, int win_y)
{
    // First, check if we need to make an edit window for *this* clip.
    if ( internal_edit_window == NULL )
    {
        internal_edit_window = new CDGMagic_PALGlobalClip_Window(this, win_x, win_y);
        internal_edit_window->show();
    }
    // Otherwise, we just bring the existing edit window for *this* clip to the front.
    else
    {
        internal_edit_window->position(win_x-internal_edit_window->w()/2, win_y);
        internal_edit_window->show();
    };
}

int CDGMagic_PALGlobalClip::serialize(char_vector* incoming_save_vector)
{
    return event_queue()->size();  // Just a placeholder for now.
}

int CDGMagic_PALGlobalClip::deserialize(char* incoming_save_data)
{
    return 0; // Just a placeholder for now.
}
