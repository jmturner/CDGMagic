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

#include "CDGMagic_BMPClip.h"
#include "CDGMagic_BMPClip_Window.h"

#include "CDGMagic_EditingGroup.h"
#include "CDGMagic_EditingLanes.h"

CDGMagic_BMPClip::CDGMagic_BMPClip(int X, int Y, int W, int H) : CDGMagic_MediaClip(0, 0), CDGMagic_MovableClipBox(X,Y,W,H,0)
{
    box(FL_FLAT_BOX);
    color(FL_GRAY, FL_WHITE);
    is_movable = 1;
    internal_edit_window = NULL;
    label_width = 0;
    internal_picon_image = new unsigned char[72*48*3];
    duration( w() * 4 );
}

CDGMagic_BMPClip::~CDGMagic_BMPClip()
{
    // Delete the editing window, if one is present.
    if (internal_edit_window != NULL)  { Fl::delete_widget(internal_edit_window); internal_edit_window = NULL; };
    // Loop through and delete all the BMPObject pointers (event structs are deleted by MediaClip parent).
    for (unsigned int current_event = 0; current_event < event_queue()->size(); current_event++)
    {
        delete (event_queue()->at(current_event)->BMPObject);
    };
    // Delete the picon image pointer.
    delete[] internal_picon_image;
}

void CDGMagic_BMPClip::null_edit()
{
    internal_edit_window = NULL;
}

void CDGMagic_BMPClip::add_bmp_file()
{
    Fl_Native_File_Chooser file_chooser;
    file_chooser.type( Fl_Native_File_Chooser::BROWSE_MULTI_FILE );
    file_chooser.title("Open bitmap image file(s)...");
#ifdef WIN32
    file_chooser.filter("Bitmap Image(s) (*.bmp)\t*.bmp");
#else    // The parenthesis somehow break the non-Windows native file choosers?!...
    file_chooser.filter("Bitmap Image\t*.bmp\n");
#endif
    file_chooser.show();
    // Returns NULL if the hit cancel, so don't even attempt to make BitMap object in that case.
    if (file_chooser.count() < 1) { return; };

    // Try to create a BitMap Loader object from the given path/file.
    for (int file_num = 0; file_num < file_chooser.count(); file_num++)
    {
        add_bmp_file( file_chooser.filename(file_num) );
    };
}

void CDGMagic_BMPClip::add_bmp_file(const char* requested_file)
{
    Fl_Native_File_Chooser file_chooser;
    const char *file_to_open = requested_file;
    CDGMagic_BMPLoader *current_image = NULL;

    while (current_image == NULL)
    {
        try
        {
            current_image = new CDGMagic_BMPLoader( file_to_open );
        }
        catch (int except)
        {
            // Allocate a temp buffer to store the user message.
            char *choice_message = new char[4096];
            // Format the message.
            snprintf(choice_message, 4096, "%s\n\nBMP Exception: %i\n%s\n\nWould you like to open a different file?", file_to_open, except, CDGMagic_BMPLoader::error_to_text(except) );
            // Ask the user if we should try again.
            int user_choice = fl_choice(choice_message, "No, move on to the next file.", "Yes, try to open a different file.", NULL);
            // Clean up the message buffer.
            delete[] choice_message;
            // If the user opted out, then just return, otherwise we'll go around again.
            if (user_choice == 0) { return; };
            // Try to open another file.
            file_chooser.type( Fl_Native_File_Chooser::BROWSE_FILE );
            file_chooser.title("Open bitmap image file...");
#ifdef WIN32
            file_chooser.filter("Bitmap Image (*.bmp)\t*.bmp");
#else    // The parenthesis somehow break the non-Windows native file choosers?!...
            file_chooser.filter("Bitmap Image\t*.bmp\n");
#endif
            file_chooser.show();
            // Returns NULL if the hit cancel, so we'll just assume they didn't want to try again after all.
            if (file_chooser.count() != 1) { return; };
            file_to_open = file_chooser.filename();
        };
    };

    // Now we have a bitmap object, so assign the various options.
    current_image->fill_index(16);
    current_image->composite_index(16);
    current_image->should_composite(1);
    // Center the image by default, only if smaller than the CD+G screen.
    // Images the same size or larger than the screen will be aligned to the top left.
    // The offsets will be (re)set later if loaded from a project.
    if ( current_image->width()  < 300 )  { current_image->x_offset( (300-current_image->width())  / 2 ); };
    if ( current_image->height() < 216 )  { current_image->y_offset( (216-current_image->height()) / 2 ); };
    // Assign the label to be the first file path in the sequence.
    if (event_queue()->size() < 1)
    {
        // We successully opened the file, so copy the label.
        copy_label( file_to_open );
        // Determine the label width.  (Returns a double... int is good enough for our measurement, though.)
        label_width = static_cast<int>( fl_width( file_to_open ) );
        // Set the picon image.
        current_image->bilinear_resize(internal_picon_image, 72, 48);
    };

    // Make a new event entry.
    CDGMagic_MediaEvent *current_event_entry = new CDGMagic_MediaEvent;
    // Set up the values.
    current_event_entry->start_offset = 0;
    current_event_entry->actual_start_offset = current_event_entry->start_offset;
    current_event_entry->duration     = 0;
    current_event_entry->actual_duration = current_event_entry->duration;
    current_event_entry->BMPObject = current_image;
    current_event_entry->PALObject = current_image->PALObject();
    current_event_entry->user_obj = NULL;
    current_event_entry->border_index = 16;
    current_event_entry->memory_preset_index = 16;
    current_event_entry->x_scroll = -1;
    current_event_entry->y_scroll = -1;
    // Add it to this clip's event queue.
    event_queue()->push_back(current_event_entry);
}

void CDGMagic_BMPClip::remove_bmp_file(unsigned int requested_clip_index)
{
    // Ignore if there's only one BMP file, or requested position is beyond the end of the queue.
    if ( (event_queue()->size() >= 2) && (requested_clip_index < event_queue()->size()) )
    {
        // Deallocate the BMP object.
        delete ( event_queue()->at(requested_clip_index)->BMPObject );
        // Deallocate the MediaEvent at the position.
        delete ( event_queue()->at(requested_clip_index) );
        // Erase the MediaEvent pointer from the queue.
        event_queue()->erase(event_queue()->begin() + requested_clip_index);

        // Reassign the label/picon if the first BMP was removed.
        if (requested_clip_index == 0)
        {
            CDGMagic_BMPObject* temp_bmp = event_queue()->at(0)->BMPObject;
            // We successully opened the file, so copy the label.
            copy_label( temp_bmp->file_path() );
            // Determine the label width.  (Returns a double... int is good enough for our measurement, though.)
            label_width = static_cast<int>( fl_width( temp_bmp->file_path() ) );
            // Set the picon image.
            temp_bmp->bilinear_resize(internal_picon_image, 72, 48);
        };
    };
}

void CDGMagic_BMPClip::set_transition_file(unsigned int requested_clip_index)
{
    // Ask to the user to select a file.
    Fl_Native_File_Chooser file_chooser;
    file_chooser.title("Open CD+G transition file...");
#ifdef WIN32
    file_chooser.filter("All Supported Transition Types\t*.{cmt,kbf}\nCD+Graphics Magic Transition (*.cmt)\t*.cmt\nKB Transition (*.kbf)\t*.kbf");
#else    // The parenthesis somehow break the non-Windows native file choosers?!...
    file_chooser.filter("All Supported Transition Types\t*.{cmt,kbf}\nCD+Graphics Magic Transition\t*.cmt\nKB Transition\t*.kbf\n");
#endif
    file_chooser.show();
    // Returns NULL if the hit cancel, so don't even attempt to set the transition in that case.
    if (file_chooser.count() != 1) { return; };
    // Try to set the transition file.
    if ( requested_clip_index < event_queue()->size() )
    {
        // Set the transition file.
        event_queue()->at(requested_clip_index)->BMPObject->transition_file( file_chooser.filename() );
    };
}

void CDGMagic_BMPClip::draw()
{
    int proper_width = ((duration()/4) >= 16) ? (duration()/4) : 16;
    if ( w() != proper_width )  {  size( proper_width, h() );  };
    // Set the box color depending on whether we're in focus or not.
    // [TODO: How could this be altered to allow multiple selections?]
    Fl_Color current_color = (Fl::focus() == this) ? selection_color() : color();
    // Draw the box with the current selection color.
    draw_box(box(), current_color);
    // Check if there's a picon image to be drawn.
    if (internal_picon_image)
    {
        // Draw the left side image if we have room.
        if (w() >=  74)  { fl_draw_image( internal_picon_image, x()+1, y()+1, 72, 48, 3, 0); }
        // Draw the right side image if we have room.
        if (w() >= 150)  { fl_draw_image( internal_picon_image, x()+w()-73, y()+1, 72, 48, 3, 0); };
    }
    // Only draw the label if it won't overrun the box.
    if (label_width < w()-10) { draw_label(); };
}

int CDGMagic_BMPClip::handle(int incoming_event)
{
    switch ( incoming_event )
    {
        case FL_RELEASE:
            if (Fl::event_button() != FL_LEFT_MOUSE) { break; };
            if (Fl::event_clicks() == 1)
            {
                do_edit(Fl::event_x_root(), Fl::event_y_root());
                return 1;
            };
        case FL_KEYBOARD:
        {
            // Delete the clip when the Delete key is pressed, asking first if an edit window is open.
            if ( (Fl::event_key() == FL_Delete)
            && ((internal_edit_window == NULL) || (internal_edit_window->can_delete() == 1)) )
            {
                // Upcast pointers to edit group/lanes (this is not ideal :-/)...
                CDGMagic_EditingGroup *editing_group = dynamic_cast<CDGMagic_EditingGroup*>( parent()->parent() );
                CDGMagic_EditingLanes *editing_lanes = dynamic_cast<CDGMagic_EditingLanes*>( parent() );
                // Delete the clip EDITING window (which will close automatically).
                Fl::delete_widget(internal_edit_window); internal_edit_window = NULL;
                // Remove the clip from the editing deque.
                editing_group->delete_clip(dynamic_cast<CDGMagic_MediaClip*>(this));
                // Delete the actual clip widget.
                Fl::delete_widget(this);
                // Redraw the edit lanes to reflect the deletion.
                editing_lanes->redraw();
                // We consumed the event, so return 1.
                return(1);
            };
            // Edit the clip if Enter is pressed (same as double-click).
            if (Fl::event_key() == FL_Enter)
            {
                do_edit(Fl::event_x_root(), Fl::event_y_root());
                return 1;
            };
        };
    };

    return CDGMagic_MovableClipBox::handle(incoming_event);
}

void CDGMagic_BMPClip::do_edit(int win_x, int win_y)
{
    // First, check if we need to make an edit window for *this* clip.
    if ( internal_edit_window == NULL )
    {
        internal_edit_window = new CDGMagic_BMPClip_Window(this, win_x, win_y);
        internal_edit_window->show();
    }
    // Otherwise, we just bring the existing edit window for *this* clip to the front.
    else
    {
        internal_edit_window->position(win_x-internal_edit_window->w()/2, win_y);
        internal_edit_window->show();
    };
}

int CDGMagic_BMPClip::serialize(char_vector* incoming_save_vector)
{
    add_text(incoming_save_vector, "CDGMagic_BMPClip::"  );
    add_char(incoming_save_vector, static_cast<char>( track_options()->track()   ) );
    add_int(incoming_save_vector, start_pack()          );
    add_int(incoming_save_vector, duration()            );
    add_int(incoming_save_vector, static_cast<int>( event_queue()->size() ) );
    for (unsigned int evnt = 0; evnt < event_queue()->size(); evnt++ )
    {
        add_int(incoming_save_vector, event_queue()->at(evnt)->start_offset );
        add_int(incoming_save_vector, event_queue()->at(evnt)->duration );
        if ( event_queue()->at(evnt)->BMPObject )
        {
            CDGMagic_MediaEvent *tmp_event = event_queue()->at(evnt);
            CDGMagic_BMPObject    *tmp_bmp = tmp_event->BMPObject;
            int upd_pal = (tmp_event->PALObject == NULL) ? 0 : 1;
            add_text(incoming_save_vector, (char*) tmp_bmp->file_path() );
            add_int(incoming_save_vector, tmp_bmp->width()             );
            add_int(incoming_save_vector, tmp_bmp->height()            );
            add_int(incoming_save_vector, tmp_bmp->x_offset()          );
            add_int(incoming_save_vector, tmp_bmp->y_offset()          );
            add_char(incoming_save_vector, static_cast<char>( tmp_bmp->fill_index() )       );
            add_char(incoming_save_vector, static_cast<char>( tmp_bmp->composite_index() )  );
            add_int(incoming_save_vector, tmp_bmp->should_composite()  );
            add_char(incoming_save_vector, tmp_event->border_index        );
            add_char(incoming_save_vector, tmp_event->memory_preset_index );
            add_int(incoming_save_vector, upd_pal );
            add_text(incoming_save_vector, (char*) tmp_bmp->transition_file()   );
            add_short(incoming_save_vector, tmp_bmp->transition_length() );
        };
    };
    return event_queue()->size();
}

int CDGMagic_BMPClip::deserialize(char* incoming_save_data)
{
    CDGMagic_EditingLanes* editing_lanes = dynamic_cast<CDGMagic_EditingLanes*>( parent() );
    int clip_x = 0, clip_y = 0;

    int pos = 0;
    char tmp_char = 0;
    short tmp_short = 0;
    int  tmp_int  = 0;

    pos += get_char( &incoming_save_data[pos], tmp_char );
    printf("Setting track: %i, Offset: %i\n", tmp_char, pos);
    clip_y = editing_lanes->y()+15+((editing_lanes->lane_height()+1)*(tmp_char+2));

    pos += get_int( &incoming_save_data[pos], tmp_int);
    printf("Setting start: %i, Offset: %i\n", tmp_int, pos);
    start_pack( tmp_int );
    clip_x = (start_pack()/4)+(editing_lanes->x()+1)+editing_lanes->xposition();

    pos += get_int( &incoming_save_data[pos], tmp_int);
    printf("Setting duration: %i, Offset: %i\n", tmp_int, pos);
    duration(   tmp_int );

    position(clip_x, clip_y);
    int proper_width = ((duration()/4) >= 16) ? (duration()/4) : 16;
    size( proper_width, h() );

    int total_events = 0;
    pos += get_int( &incoming_save_data[pos], total_events );
    for (int evnt_num = 0; evnt_num < total_events; evnt_num++)
    {
        unsigned int initial_events = event_queue()->size();
        int  clip_time_offset       = 0;
        int  clip_time_duration     = 0;
        int  clip_x_offset          = 0;
        int  clip_y_offset          = 0;
        char clip_fill_index        = 0;
        char clip_composite_index   = 0;
        int  clip_should_composite  = 0;
        char clip_border_index      = 0;
        char clip_screen_index      = 0;
        int  clip_should_palette    = 0;


        pos += get_int( &incoming_save_data[pos], clip_time_offset);
        pos += get_int( &incoming_save_data[pos], clip_time_duration);

        const char *bmp_file_path = &incoming_save_data[pos];
        printf("Trying to open: %s\n", bmp_file_path );
        add_bmp_file( bmp_file_path );
        pos += strlen( &incoming_save_data[pos] ) + 1;
        printf("prj_offset: %i\n", pos);

        pos += get_int(&incoming_save_data[pos], tmp_int);               // Height
        pos += get_int(&incoming_save_data[pos], tmp_int);               // Width
        pos += get_int(&incoming_save_data[pos], clip_x_offset);         // X Offset
        pos += get_int(&incoming_save_data[pos], clip_y_offset);         // Y Offset
        pos += get_char(&incoming_save_data[pos], clip_fill_index);      // Fill Index
        pos += get_char(&incoming_save_data[pos], clip_composite_index); // Composite Index
        pos += get_int(&incoming_save_data[pos], clip_should_composite); // Composite Type
        pos += get_char(&incoming_save_data[pos], clip_border_index);    // Border Index
        pos += get_char(&incoming_save_data[pos], clip_screen_index);    // Screen Index
        pos += get_int(&incoming_save_data[pos], clip_should_palette);   // Palette Update

        const char *trans_file_path = &incoming_save_data[pos];
        pos += strlen( &incoming_save_data[pos] ) + 1;  // Transition File
        printf("prj_offset: %i\n", pos);

        pos += get_short(&incoming_save_data[pos], tmp_short);  // Transition length

        if ( event_queue()->size() > initial_events )
        {
#ifdef _CDGMAGIC_BMPCLIP_DEBUG_
            printf("Setting clip offset: %i\n", clip_time_offset);
#endif
            event_queue()->back()->start_offset = clip_time_offset;
#ifdef _CDGMAGIC_BMPCLIP_DEBUG_
            printf("Setting clip duration: %i\n", clip_time_duration);
#endif
            event_queue()->back()->duration = clip_time_duration;

            CDGMagic_MediaEvent *tmp_event = event_queue()->back();
            CDGMagic_BMPObject    *tmp_bmp = tmp_event->BMPObject;

#ifdef _CDGMAGIC_BMPCLIP_DEBUG_
            printf("Setting x_offset %i\n", clip_x_offset);
#endif
            tmp_bmp->x_offset(clip_x_offset);
#ifdef _CDGMAGIC_BMPCLIP_DEBUG_
            printf("Setting y_offset %i\n", clip_y_offset);
#endif
            tmp_bmp->y_offset(clip_y_offset);

#ifdef _CDGMAGIC_BMPCLIP_DEBUG_
            printf("Setting fill_index %i\n", clip_fill_index);
#endif
            tmp_bmp->fill_index(clip_fill_index);
#ifdef _CDGMAGIC_BMPCLIP_DEBUG_
            printf("Setting composite_index %i\n", clip_composite_index);
#endif
            tmp_bmp->composite_index(clip_composite_index);
#ifdef _CDGMAGIC_BMPCLIP_DEBUG_
            printf("Setting should_composite %i\n", clip_should_composite);
#endif
            tmp_bmp->should_composite(clip_should_composite);

#ifdef _CDGMAGIC_BMPCLIP_DEBUG_
            printf("Setting border:%i, screen:%i, palette:%i\n", clip_border_index, clip_screen_index, clip_should_palette);
#endif
            tmp_event->border_index        = clip_border_index;
            tmp_event->memory_preset_index = clip_screen_index;
            tmp_event->PALObject = (clip_should_palette == 1) ? tmp_event->BMPObject->PALObject() : NULL;

            //if ( strlen(trans_file_path) > 0 ) // Was...
            if ( *trans_file_path != '\0' )      // Style recommended by CppCheck.
            {
                int trans_error = tmp_bmp->transition_file( trans_file_path );
                if (trans_error != 0) { fl_alert("There was an error loading transition file:\n%s", trans_file_path); };
            };
        }
        else
        {
            fl_alert("There was a problem loading bitmap file:\n%s", bmp_file_path);
        };
    };

    return pos;
}
