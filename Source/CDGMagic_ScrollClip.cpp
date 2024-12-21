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

#include "CDGMagic_ScrollClip.h"
#include "CDGMagic_ScrollClip_Window.h"

#include "CDGMagic_EditingGroup.h"
#include "CDGMagic_EditingLanes.h"

CDGMagic_ScrollClip::CDGMagic_ScrollClip(int X, int Y, int W, int H) : CDGMagic_MediaClip(0, 0), CDGMagic_MovableClipBox(X,Y,W,H,0)
{
    box(FL_FLAT_BOX);
    color(FL_GRAY, FL_WHITE);
    is_movable = 1;
    internal_edit_window = NULL;
    internal_picon_image = new unsigned char[72*48*3];
    label_width = 0;
    current_time = NULL;
    duration( w() * 4 );
};

CDGMagic_ScrollClip::~CDGMagic_ScrollClip()
{
    // Loop through and delete all the BMPObject pointers (event structs are deleted by MediaClip parent).
    for (unsigned int current_event = 0; current_event < event_queue()->size(); current_event++)
    {
        delete (event_queue()->at(current_event)->BMPObject);
    };
    // Delete the picon image pointer.
    delete[] internal_picon_image;
};

void CDGMagic_ScrollClip::null_edit()
{
    internal_edit_window = NULL;
};

void CDGMagic_ScrollClip::set_time_var(int *time)
{
   current_time = time;
}

int CDGMagic_ScrollClip::get_time()
{
    return *current_time;
};

void CDGMagic_ScrollClip::add_bmp_file()
{
    Fl_Native_File_Chooser file_chooser;
    file_chooser.title("Open bitmap image file...");
#ifdef WIN32
    file_chooser.filter("Bitmap Image (*.bmp)\t*.bmp");
#else    // The parenthesis somehow break the non-Windows native file choosers?!...
    file_chooser.filter("Bitmap Image\t*.bmp\n");
#endifv
    file_chooser.show();
    // Returns NULL if the hit cancel, so don't even attempt to make BitMap object in that case.
    if (file_chooser.count() != 1) { return; };
    // Try to create a BitMap Loader object from the given path/file.
    add_bmp_file( file_chooser.filename() );
};

void CDGMagic_ScrollClip::add_bmp_file(const char* requested_file)
{
    // Try to create a BitMap Loader object from the given path/file.
  if (requested_file != NULL)
  {
    try
    {
        CDGMagic_BMPLoader *current_image = new CDGMagic_BMPLoader( requested_file );
        current_image->fill_index(16);
        current_image->composite_index(16);
        current_image->should_composite(1);

        // If needed, create a new MediaEvent entry.
        if (event_queue()->size() < 1)
        {
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
        // Otherwise, delete the current bitmap object, so we can add the new one.
        else
        {
            delete (event_queue()->at(0)->BMPObject);
            event_queue()->front()->BMPObject = current_image;
            event_queue()->front()->PALObject = current_image->PALObject();
        };

        // We successully opened the file, so copy the label.
        copy_label( requested_file );
        // Determine the label width.  (Returns a double... int is good enough for our measurement, though.)
        label_width = static_cast<int>( fl_width( requested_file ) );
        // Set the picon image.
        current_image->bilinear_resize(internal_picon_image, 72, 48);


        ////////// temp code for testing //////////
        for (unsigned int curr_evnt = 1; curr_evnt < event_queue()->size(); curr_evnt++)
        {
            // Deallocate the MediaEvent at the position.
            delete ( event_queue()->at(curr_evnt) );
            // Erase the MediaEvent pointer from the queue.
            event_queue()->erase(event_queue()->begin() + curr_evnt);
        };

        int curr_offset = 300 * 5;
        for (int inc = 0; inc < 300; inc++)
        {
            // Make a new event entry.
            CDGMagic_MediaEvent *current_event_entry = new CDGMagic_MediaEvent;
            // Set up the values.
            current_event_entry->start_offset = curr_offset;
            current_event_entry->actual_start_offset = current_event_entry->start_offset;
            current_event_entry->duration     = 0;
            current_event_entry->actual_duration = current_event_entry->duration;
            current_event_entry->BMPObject = NULL;
            current_event_entry->PALObject = NULL;
            current_event_entry->user_obj = NULL;
            current_event_entry->border_index = 16;
            current_event_entry->memory_preset_index = 16;
            current_event_entry->x_scroll = -1;
            current_event_entry->y_scroll = (inc % 11);
            if ( (current_event_entry->y_scroll == 0) && (inc > 0) ) { current_event_entry->y_scroll |= 0x20; };
            // Add it to this clip's event queue.
            event_queue()->push_back(current_event_entry);
            curr_offset += 10;
        };
        ////////// temp test code end //////////
    }
    // Just put up an alert with the error and bail if there was a problem.
    catch (int except)
    {
        switch (except)
        {
            case                 CDGMagic_BMPLoader::NO_PATH: fl_alert("BMP Exception: %i\nNo path was given!", except);
                                                              break;
            case               CDGMagic_BMPLoader::OPEN_FAIL: fl_alert("BMP Exception: %i\nCould not open file!", except);
                                                              break;
            case                CDGMagic_BMPLoader::TO_LARGE: fl_alert("BMP Exception: %i\nFile is larger than arbitrary 10MB limit!", except);
                                                              break;
            case                CDGMagic_BMPLoader::TO_SMALL: fl_alert("BMP Exception: %i\nFile is smaller than arbitrary 100B limit!", except);
                                                              break;
            case                 CDGMagic_BMPLoader::NOT_BMP: fl_alert("BMP Exception: %i\nThis file does not start with bitmap magic BM!", except);
                                                              break;
            case                CDGMagic_BMPLoader::BAD_SIZE: fl_alert("BMP Exception: %i\nThis file's header size and actual size are inconsistent!", except);
                                                              break;
            case              CDGMagic_BMPLoader::BAD_OFFSET: fl_alert("BMP Exception: %i\nThis file's data offset is beyond the end of file!", except);
                                                              break;
            case      CDGMagic_BMPLoader::UNSUPPORTED_HEADER: fl_alert("BMP Exception: %i\nThis file uses an unsupported header version!", except);
                                                              break;
            case                CDGMagic_BMPLoader::TOO_WIDE: fl_alert("BMP Exception: %i\nThis file is wider than the arbitrary limit of 16K pixels!", except);
                                                              break;
            case                CDGMagic_BMPLoader::TOO_TALL: fl_alert("BMP Exception: %i\nThis file is taller than the arbitrary limit of 16K pixels!", except);
                                                              break;
            case     CDGMagic_BMPLoader::INVALID_COLORPLANES: fl_alert("BMP Exception: %i\nThis file uses an unsupported number of color planes!", except);
                                                              break;
            case  CDGMagic_BMPLoader::UNSUPPORTED_COLORDEPTH: fl_alert("BMP Exception: %i\nThis file is not 16 (4 bit) or 256 (8 bit) indexed color!", except);
                                                              break;
            case CDGMagic_BMPLoader::UNSUPPORTED_COMPRESSION: fl_alert("BMP Exception: %i\nThis file is compressed, only uncompressed BMPs are supported!", except);
                                                              break;
            case     CDGMagic_BMPLoader::UNSUPPORTED_PALETTE: fl_alert("BMP Exception: %i\nThis file uses an unsupported number of palette entries!", except);
                                                              break;
            case       CDGMagic_BMPLoader::INSUFFICIENT_DATA: fl_alert("BMP Exception: %i\nInsufficient data, this file may have been truncated!", except);
                                                              break;
            default: fl_alert("BMP Exception: %i", except);
                     break;
        };
    };
  } else { printf("CDGMagic_ScrollClip::add_bmp_file was passed NULL!\n"); };
};

void CDGMagic_ScrollClip::set_transition_file(unsigned int requested_clip_index)
{
    // Ask to the user to select a file.
    Fl_Native_File_Chooser file_chooser;
    file_chooser.title("Open CD+Graphics transition file...");
#ifdef WIN32
    file_chooser.filter("CD+Graphics Magic Transition (*.cmt)\t*.cmt\nKB Transition (*.kbf)\t*.kbf");
#else    // The parenthesis somehow break the non-Windows native file choosers?!...
    file_chooser.filter("CD+Graphics Magic Transition \t*.cmt\nKB Transition \t*.kbf\n");
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
};

void CDGMagic_ScrollClip::draw()
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
};

int CDGMagic_ScrollClip::handle(int incoming_event)
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
        };
    };

    return CDGMagic_MovableClipBox::handle(incoming_event);
};

void CDGMagic_ScrollClip::do_edit(int win_x, int win_y)
{
    // First, check if we need to make an edit window for *this* clip.
    if ( internal_edit_window == NULL )
    {
        internal_edit_window = new CDGMagic_ScrollClip_Window(this, win_x, win_y);
        internal_edit_window->show();
    }
    // Otherwise, we just bring the existing edit window for *this* clip to the front.
    else
    {
        internal_edit_window->position(win_x-internal_edit_window->w()/2, win_y);
        internal_edit_window->show();
    };
};

int CDGMagic_ScrollClip::serialize(char_vector* incoming_save_vector)
{
    add_text(incoming_save_vector, "CDGMagic_ScrollClip::"  );
    add_char(incoming_save_vector, static_cast<char>( track_options()->track()   ) );
    add_int(incoming_save_vector, start_pack()          );
    add_int(incoming_save_vector, duration()            );
    add_int(incoming_save_vector, static_cast<int>( event_queue()->size() ) );
    for (unsigned int evnt = 0; evnt < event_queue()->size(); evnt++ )
    {
        add_int(incoming_save_vector, event_queue()->at(evnt)->start_offset );
        add_int(incoming_save_vector, event_queue()->at(evnt)->duration );
//        if ( internal_queue->at(evnt)->PALObject )  { internal_queue->at(evnt)->PALObject->serialize(my_data); };
        if ( event_queue()->at(evnt)->BMPObject )
        {
            CDGMagic_BMPObject *tmp_bmp = event_queue()->at(evnt)->BMPObject;
            add_text(incoming_save_vector, (char*) tmp_bmp->file_path() );
            add_int(incoming_save_vector, tmp_bmp->width()             );
            add_int(incoming_save_vector, tmp_bmp->height()            );
            add_int(incoming_save_vector, tmp_bmp->x_offset()          );
            add_int(incoming_save_vector, tmp_bmp->y_offset()          );
            add_char(incoming_save_vector, static_cast<char>( tmp_bmp->fill_index() )       );
            add_char(incoming_save_vector, static_cast<char>( tmp_bmp->composite_index() )  );
            add_int(incoming_save_vector, tmp_bmp->should_composite()  );
            add_text(incoming_save_vector, (char*) tmp_bmp->transition_file()   );
            add_short(incoming_save_vector, tmp_bmp->transition_length() );
        };
    };
    return event_queue()->size();
};

int CDGMagic_ScrollClip::deserialize(char* incoming_save_data)
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
        int clip_time_offset = 0;
        int clip_time_duration = 0;
        pos += get_int( &incoming_save_data[pos], tmp_int);
        printf("Setting clip offset: %i, Offset: %i\n", tmp_int, pos);
        pos += get_int( &incoming_save_data[pos], tmp_int);
        printf("Setting clip duration: %i, Offset: %i\n", tmp_int, pos);

        printf("Trying to open: %s\n", &incoming_save_data[pos] );
        add_bmp_file( &incoming_save_data[pos] );
        pos += strlen( &incoming_save_data[pos] ) + 1;
        printf("prj_offset: %i\n", pos);

        event_queue()->at(evnt_num)->start_offset = clip_time_offset;
        event_queue()->at(evnt_num)->duration = clip_time_duration;

        CDGMagic_BMPObject *tmp_bmp = event_queue()->at(evnt_num)->BMPObject;

        pos += get_int(&incoming_save_data[pos], tmp_int);  // Height
        pos += get_int(&incoming_save_data[pos], tmp_int);  // Width

        pos += get_int(&incoming_save_data[pos], tmp_int);  // X Offset
        printf("Setting x_offset %i\n", tmp_int);
        tmp_bmp->x_offset(tmp_int);

        pos += get_int(&incoming_save_data[pos], tmp_int);  // Y Offset
        printf("Setting y_offset %i\n", tmp_int);
        tmp_bmp->y_offset(tmp_int);

        pos += get_char(&incoming_save_data[pos], tmp_char);// Fill Index
        printf("Setting fill_index %i\n", tmp_char);
        tmp_bmp->fill_index(tmp_char);

        pos += get_char(&incoming_save_data[pos], tmp_char);// Composite Index
        printf("Setting composite_index %i\n", tmp_char);
        tmp_bmp->composite_index(tmp_char);

        pos += get_int(&incoming_save_data[pos], tmp_int);  // Composite Type
        printf("Setting should_composite %i\n", tmp_int);
        tmp_bmp->should_composite(tmp_int);

        if ( incoming_save_data[pos] )
        {
            tmp_bmp->transition_file( &incoming_save_data[pos] );// Transition File
        }
        pos += strlen( &incoming_save_data[pos] ) + 1;
        printf("prj_offset: %i\n", pos);

        pos += get_short(&incoming_save_data[pos], tmp_short);  // Transition length
    };

    return pos;
};
