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

#include <string>

#include "CDGMagic_BMPClip_Window.h"

CDGMagic_BMPClip_Window::CDGMagic_BMPClip_Window(CDGMagic_BMPClip *clip_to_edit, int X, int Y) : Fl_Double_Window(0, 0, 0)
{
    const char *bmpclipwindow_title  = "BMP Clip Settings";
    const int   bmpclipwindow_width  = 600;
    const int   bmpclipwindow_height = 405;

    label( bmpclipwindow_title );
    size( bmpclipwindow_width, bmpclipwindow_height );
    position( X-w()/2, Y );

    preview_buffer = new unsigned char[300 * 216 * 3];

    begin();

        // Create a new 24bit image and set the pointer to the CDG screen data pointer.
        PreviewImage = new Fl_RGB_Image( preview_buffer, 300, 216, 3, 0 );
        // Create a new empty box and associate the image with it.
        PreviewBox = new Fl_Box(10, 10, 300+Fl::box_dw(FL_DOWN_BOX), 216+Fl::box_dh(FL_DOWN_BOX));
        PreviewBox->box(FL_DOWN_BOX);
        PreviewBox->image(PreviewImage);

        PreviewLabel = new Fl_Box(PreviewBox->x(), PreviewBox->y()+PreviewBox->h(), PreviewBox->w(), 20);
        PreviewLabel->label("Preview Image");
        PreviewLabel->box(FL_FLAT_BOX);
/*
        Fl_Choice *Transition_choice = new Fl_Choice(PreviewBox->x()+PreviewBox->w()+100, 10, 150, 30, "Transition:");
        Transition_choice->add("Left->Right->Top->Bottom", 0, NULL);
        Transition_choice->add("Right->Left->Top->Bottom", 0, NULL);
        Transition_choice->add("Left->Right->Bottom->Top", 0, NULL);
        Transition_choice->value(0);

        Fl_Choice *Border_choice = new Fl_Choice(PreviewBox->x()+PreviewBox->w()+100, 50, 150, 30, "Border:");
        Border_choice->add("None", 0, NULL);
        Border_choice->add("Transition", 0, NULL);
        Border_choice->add("First", 0, NULL);
        Border_choice->add("Last", 0, NULL);
        Border_choice->value(0);

        Fl_Choice *Remap_choice = new Fl_Choice(PreviewBox->x()+PreviewBox->w()+100, 90, 150, 30, "Color Remap:");
        Remap_choice->add("None", 0, NULL);
        Remap_choice->add("Add 4", 0, NULL);
        Remap_choice->add("Add 8", 0, NULL);
        Remap_choice->add("Add 10", 0, NULL);
        Remap_choice->add("Add 12", 0, NULL);
        Remap_choice->add("Rotate 4", 0, NULL);
        Remap_choice->add("Rotate 8", 0, NULL);
        Remap_choice->add("Reverse", 0, NULL);
        Remap_choice->value(0);
*/

        Fl_Button *OK_button = new Fl_Button(10, 365, 90, 30, "OK");
        OK_button->callback(DoOK_callback_static, this);

        Fl_Button *ApplyAll_button = new Fl_Button(120, 365, 80, 30, "Apply All");
        ApplyAll_button->tooltip("Apply the current display settings to all bitmaps in this sequence.\n"\
                                 "CTRL+Click: Apply the current X/Y offsets to all bitmaps in this sequence.");
        ApplyAll_button->callback(DoApplyAll_callback_static, this);

        Fl_Button *SetStart_button = new Fl_Button(220, 365, 80, 30, "Set Start");
        SetStart_button->tooltip("Advance to the next event, then set the start time to the current play position.");
        SetStart_button->callback(DoSetStart_callback_static, this);

        Fl_Button *Add_button = new Fl_Button(350, 365, 100, 30, "Add BMP...");
        Add_button->tooltip("Add another bitmap file to the end of this sequence.");
        Add_button->callback(DoAdd_callback_static, this);

        Fl_Button *Remove_button = new Fl_Button(470, 365, 100, 30, "Remove BMP");
        Remove_button->tooltip( "Remove the currently selected bitmap file from this sequence.\n"\
                                "Shortcut: Delete" );
        Remove_button->shortcut(FL_Delete);
        Remove_button->callback(DoRemove_callback_static, this);

        Trans_button = new Fl_Button(14, 255, 240, 25, "Load transition...");
        Trans_button->tooltip("Load a transition file.\n"\
                              "You can create a transition from a grayscale, 300x216 pixel gradient image via the tools menu.");
        Trans_button->callback(DoTransition_callback_static, this);

        Fl_Check_Button* animate_check = new Fl_Check_Button(294, 257, 20, 20, "Ani:");
        animate_check->align(FL_ALIGN_LEFT);
        animate_check->value( 0 );
        animate_check->tooltip( "Enable/disable transition \"animation\" preview.\n"\
                               "(Note: Animation speed is approximate, actual clip will differ.)" );
        animate_check->callback( animate_cb_static, this );

        Composite_choice = new Fl_Choice(50, 285, 100, 20, "Type:");
        Composite_choice->add("Opaque");
        Composite_choice->add("Replace");
        Composite_choice->add("Overlay");
        Composite_choice->value(0);
        Composite_choice->tooltip("Type of compositing used for the \"transparent\" color index.\n"\
                                  "Opaque: Replace the current layer, composite index is ignored.\n"\
                                  "Replace: Replace the current layer, composite index shows lower layers.\n"\
                                  "Overlay: Composite the current image into the current layer.");
        Composite_choice->callback( ShouldComp_cb_static, this );

        Screen_choice = new Fl_Choice(50, 310, 100, 20, "Clr:");
        Screen_choice->add("None");
        Screen_choice->value(0);
        Screen_choice->tooltip("Optional color index used to clear the screen prior to drawing bitmap.");
        Screen_choice->callback( ScreenIdx_cb_static, this );

        Border_choice = new Fl_Choice(50, 335, 100, 20, "Brdr:");
        Border_choice->add("None");
        Border_choice->value(0);
        Border_choice->tooltip("Optional color index used to set the border prior to drawing bitmap.");
        Border_choice->callback( BorderIdx_cb_static, this );

        // Set the additional drop down choice labels for color indices.
        char label_buff[16];
        for (int curr_idx = 0; curr_idx < 16; curr_idx++)
        {
            snprintf(label_buff, 16, "%02i", curr_idx);
            Screen_choice->add(label_buff);
            Border_choice->add(label_buff);
        };

        comp_idx_counter = new Fl_Counter(210, 285, 100, 20, "Comp:");
        comp_idx_counter->align(FL_ALIGN_LEFT);
        comp_idx_counter->step(1.0, 10.0);
        comp_idx_counter->bounds(0.0, 255.0);
        comp_idx_counter->value(0);
        comp_idx_counter->tooltip("Color index that will be \"transparent\" when compositing.");
        comp_idx_counter->callback( CompIdx_cb_static, this );

        fill_idx_counter = new Fl_Counter(210, 310, 100, 20, "Fill Idx:");
        fill_idx_counter->align(FL_ALIGN_LEFT);
        fill_idx_counter->step(1.0, 10.0);
        fill_idx_counter->bounds(0.0, 255.0);
        fill_idx_counter->value(0);
        fill_idx_counter->tooltip("Color index used to fill area outside of bitmap.");
        fill_idx_counter->callback( FillIdx_cb_static, this );

        update_pal_check = new Fl_Check_Button(251, 335, 20, 20, "Set Palette?");
        update_pal_check->align(FL_ALIGN_LEFT);
        update_pal_check->value( 1 );
        update_pal_check->tooltip( "Enable/disable setting of bitmap palette prior to display.\n"\
                                   "(A palette must be set via another clip, or the screen will remain black.)" );
        update_pal_check->callback( SetPal_cb_static, this );

        bmp_list_control = new Fl_Hold_Browser( 325, 10, 265, 320, "Image Sequence");
        bmp_list_control->tooltip( "Bitmap files in this sequence.\n"\
                                   "Click to select, then choose options at left.\n"\
                                   "CTRL+Up/Down: Move selected bitmap earlier/later." );
        bmp_list_control->callback(DoBMPBrowser_callback_static, this);
    end();

    current_clip = clip_to_edit;
    current_page = 0;
    animate_progress = -1; // Don't animate by default.

    update_current_values();
}

CDGMagic_BMPClip_Window::~CDGMagic_BMPClip_Window()
{
    Fl::remove_timeout(animate_progress_static);
    current_clip->null_edit();
    delete[] preview_buffer;
}

void CDGMagic_BMPClip_Window::DoOK_callback(Fl_Button *CallingWidget)
{
    Fl::delete_widget(this);
}

int CDGMagic_BMPClip_Window::can_delete()
{
    show();
    return fl_choice("This clip has a window open for editing.\nDelete anyway?", "No (don't delete)", "Yes (delete this clip)", NULL);
}

void CDGMagic_BMPClip_Window::DoBMPBrowser_callback(Fl_Hold_Browser *CallingWidget)
{
    if (Fl::event_state() & FL_CTRL)
    {
        if ( (Fl::event_key() == FL_Up) && (bmp_list_control->value() >= 1) )
        {
            int curr_sel = bmp_list_control->value() - 1;
            std::iter_swap( current_clip->event_queue()->begin()+curr_sel+0,
                            current_clip->event_queue()->begin()+curr_sel+1 );
            update_browser_text(curr_sel+0);
            update_browser_text(curr_sel+1);
            return;
        };
        if ( (Fl::event_key() == FL_Down) && (bmp_list_control->value() <= bmp_list_control->size()) )
        {
            int curr_sel = bmp_list_control->value() - 1;
            std::iter_swap( current_clip->event_queue()->begin()+curr_sel+0,
                            current_clip->event_queue()->begin()+curr_sel-1 );
            update_browser_text(curr_sel+0);
            update_browser_text(curr_sel-1);
            return;
        };
    };
    DoPageChange_callback();
}

void CDGMagic_BMPClip_Window::DoPageChange_callback()
{
    // Make sure we have at least one clip loaded.
    if (current_clip->event_queue()->size() < 1)  { return; };
    // Short-circuit the deselect. (Maybe this should happen in a subclass...)
    // Actually, it would be nice to implement a drag-and-drop swapper/reorganizer.
    if (bmp_list_control->value() < 1)  { bmp_list_control->value(current_page + 1); return; };
    // Get the requested page number. (Note: current_page is 0 based, most of the GUI controls are 1 based.)
    current_page = bmp_list_control->value() - 1;
    // Set it to the last available image if we would run past the buffer.
    // (This shouldn't be possible -- but maybe we could implement wrap/loop.
    if ( current_page >= current_clip->event_queue()->size() )  { current_page = current_clip->event_queue()->size()-1; };
    // Set the page. (This shouldn't really be necessary, either, but is if we clamped above.
    bmp_list_control->value( current_page+1 );
    // Update the clip info in the browser.
    update_browser_text(current_page);
    // Reload the current values.
    update_current_values();
}

void CDGMagic_BMPClip_Window::DoApplyAll_callback(Fl_Button *CallingWidget)
{
    // Copy the settings by default, or copy the offsets if the CTRL was held while clicking the button.
    int settings_or_offsets = (Fl::event_state() & FL_CTRL) ? 1 : 0;
    // Convenience variable for the original source event.
    CDGMagic_MediaEvent *source_event = current_clip->event_queue()->at(current_page);
    // Step through all of the current bitmaps in the sequence, copying the setting or positioning.
    for (unsigned int curr_event = 0; curr_event < current_clip->event_queue()->size(); curr_event++)
    {
        if (curr_event == current_page)  { continue; }; // Skip the source event, which is the current page.
        CDGMagic_MediaEvent *this_event = current_clip->event_queue()->at(curr_event);
        if (settings_or_offsets == 0)
        {
            this_event->border_index        = source_event->border_index;
            this_event->memory_preset_index = source_event->memory_preset_index;
            this_event->PALObject = (source_event->PALObject == NULL) ? NULL : this_event->BMPObject->PALObject();
            this_event->BMPObject->transition_file( source_event->BMPObject->transition_file() );
            this_event->BMPObject->fill_index(       source_event->BMPObject->fill_index()       );
            this_event->BMPObject->composite_index(  source_event->BMPObject->composite_index()  );
            this_event->BMPObject->should_composite( source_event->BMPObject->should_composite() );
        }
        else
        {
            this_event->BMPObject->x_offset( source_event->BMPObject->x_offset() );
            this_event->BMPObject->y_offset( source_event->BMPObject->y_offset() );
        };
    };
}

void CDGMagic_BMPClip_Window::DoAdd_callback(Fl_Button *CallingWidget)
{
    current_clip->add_bmp_file();
    current_page = current_clip->event_queue()->size() - 1; // current_page is 0-based.
    update_current_values();
}

void CDGMagic_BMPClip_Window::DoRemove_callback(Fl_Button *CallingWidget)
{
    if ( current_clip->event_queue()->size() <= 1 )
    {
        // TODO: Make this smarter...
        // Perhaps just disable the Remove button, or pass through to the clip delete function.
        fl_alert("This clip has only one bitmap file.\nPlease delete from the timeline instead.");
        return;
    };
    current_clip->remove_bmp_file(current_page);
    if ( current_page >= current_clip->event_queue()->size() )
    {
        current_page = current_clip->event_queue()->size() - 1; // current_page is 0-based.
    };
    update_current_values();
}

void CDGMagic_BMPClip_Window::DoTransition_callback(Fl_Button *CallingWidget)
{
    current_clip->set_transition_file(current_page);
    if ( current_bmp_object->transition_file() )
    {
        // Copy just the filename to the button label.
        CallingWidget->copy_label( fl_filename_name(  current_bmp_object->transition_file()  ) );
    }
    else
    {
        CallingWidget->copy_label( "Load transition..." );
    };
}

void CDGMagic_BMPClip_Window::DoSetStart_callback(Fl_Button *CallingWidget)
{
    // Subtract the clip start time from the current play time to the event offset.
    int new_event_offset = current_clip->play_time() - current_clip->start_pack();
    // Advance the page count by one.
    bmp_list_control->value( bmp_list_control->value() + 1.0 );
    // Update the preview image and controls.
    DoPageChange_callback();
    // Set the event time for current event.
    current_clip->event_queue()->at(current_page)->start_offset = new_event_offset;
    // Print out stupid debug stuff.
    printf("Event: %i, Time: %i, Start: %i, Offset: %i\n", current_page, current_clip->play_time(), current_clip->start_pack(), new_event_offset);
    // Force an update of the browser text.
    update_browser_text(current_page);
}

void CDGMagic_BMPClip_Window::update_browser_text(int requested_event_number)
{
    unsigned int evnt_idx = requested_event_number; // Was signed int, now unsigned so compiler doesn't warn on conditional below.
    int line_idx = requested_event_number + 1;

    if ( line_idx > bmp_list_control->size() )  { return; };
    if ( evnt_idx >= current_clip->event_queue()->size() )  { return; };

    CDGMagic_MediaEvent* temp_event = current_clip->event_queue()->at(evnt_idx);
    int actual_time = current_clip->start_pack() + temp_event->start_offset;

    char label_buffer[MAX_PATH];
    snprintf(label_buffer, MAX_PATH, "%03i :: %02i:%02i.%02i :: %s",
             line_idx,
             actual_time / 18000, (actual_time / 300) % 60, (actual_time % 300) / 4,
             fl_filename_name( current_clip->event_queue()->at(evnt_idx)->BMPObject->file_path()) );
    bmp_list_control->text(line_idx, label_buffer);
}

void CDGMagic_BMPClip_Window::update_current_values()
{
    unsigned int current_list_size = bmp_list_control->size();  // Unsigned so compiler doesn't warn on conditional below.
    if ( current_list_size != current_clip->event_queue()->size() )
    {
        bmp_list_control->clear();
        for (unsigned int curr_evnt = 0; curr_evnt < current_clip->event_queue()->size(); curr_evnt++)
        {
            bmp_list_control->add(" ");  update_browser_text(curr_evnt);
        };
    };
    // Set the selection.
    bmp_list_control->value(current_page + 1);
    // Set the temp values to the requested page's pointers.
    CDGMagic_MediaEvent* temp_event = current_clip->event_queue()->at(current_page);
    current_bmp_object = temp_event->BMPObject;
    // Update the controls.
    fill_idx_counter->value( current_bmp_object->fill_index() );
    comp_idx_counter->value( current_bmp_object->composite_index() );
    Composite_choice->value( current_bmp_object->should_composite() );
    Screen_choice->value( (temp_event->memory_preset_index < 16) ? temp_event->memory_preset_index + 1 : 0 );
    Border_choice->value( (temp_event->border_index < 16) ? temp_event->border_index + 1 : 0 );
    update_pal_check->value( (temp_event->PALObject != NULL) ? 1 : 0 );
    // Set the transition button label to either the transition file, or a prompt to load a new transition.
    if ( current_bmp_object->transition_file() )
    {
        Trans_button->copy_label( fl_filename_name(  current_bmp_object->transition_file()  ) );
    }
    else
    {
        Trans_button->copy_label( "Load transition..." );
    };
    // Update the preview display.
    update_preview();
}

void CDGMagic_BMPClip_Window::update_preview()
{
    int x_offset = current_bmp_object->x_offset();
    int y_offset = current_bmp_object->y_offset();

    char temp_label[32];
    snprintf(temp_label, 32, "X: %i, Y: %i", x_offset, y_offset);
    PreviewLabel->copy_label(temp_label);

    unsigned int tmp_color = 0x00;
    unsigned int src_x = 0, src_y = 0, dst_pxl = 0;

    int mem_idx = (Screen_choice->value() > 0) ? Screen_choice->value()-1 : 0;
    int brd_idx = (Border_choice->value() > 0) ? Border_choice->value()-1 : 0;
    unsigned int mem_rgb = current_bmp_object->PALObject()->color( mem_idx );
    unsigned int brd_rgb = current_bmp_object->PALObject()->color( brd_idx );

    if (animate_progress == -1)
    {
        for (int the_px = 0; the_px < 300*216; the_px++)
        {
            dst_pxl = the_px * 3;
            preview_buffer[dst_pxl+0] = (brd_rgb >> 030) & 0xFF;
            preview_buffer[dst_pxl+1] = (brd_rgb >> 020) & 0xFF;
            preview_buffer[dst_pxl+2] = (brd_rgb >> 010) & 0xFF;
        };
        for (int y_inc = 12; y_inc < 204; y_inc++)
        {
            src_y = (y_inc - y_offset);
            for (int x_inc = 6; x_inc < 294; x_inc++)
            {
                src_x = (x_inc - x_offset);
                dst_pxl = (x_inc + y_inc * 300) * 3;
                tmp_color = current_bmp_object->get_rgb_pixel(src_x, src_y);
                preview_buffer[dst_pxl+0] = (tmp_color >> 030) & 0xFF;
                preview_buffer[dst_pxl+1] = (tmp_color >> 020) & 0xFF;
                preview_buffer[dst_pxl+2] = (tmp_color >> 010) & 0xFF;
            };
        };
    }
    else
    {
        // First, clear the screen.
        // Also, should add a way to temporal reverse and spatial flip/mirror the transition blocks.
        for (int the_px = 0; the_px < 300*216; the_px++)
        {
            dst_pxl = the_px * 3;
            preview_buffer[dst_pxl+0] = (mem_rgb >> 030) & 0xFF;
            preview_buffer[dst_pxl+1] = (mem_rgb >> 020) & 0xFF;
            preview_buffer[dst_pxl+2] = (mem_rgb >> 010) & 0xFF;
        };
        for (int the_px = 0; the_px < 300*12; the_px++)
        {
            dst_pxl = the_px * 3;
            preview_buffer[dst_pxl+0] = (brd_rgb >> 030) & 0xFF;
            preview_buffer[dst_pxl+1] = (brd_rgb >> 020) & 0xFF;
            preview_buffer[dst_pxl+2] = (brd_rgb >> 010) & 0xFF;
        };
        for (int the_px = 300*204; the_px < 300*216; the_px++)
        {
            dst_pxl = the_px * 3;
            preview_buffer[dst_pxl+0] = (brd_rgb >> 030) & 0xFF;
            preview_buffer[dst_pxl+1] = (brd_rgb >> 020) & 0xFF;
            preview_buffer[dst_pxl+2] = (brd_rgb >> 010) & 0xFF;
        };
        for (int the_ln = 300*12; the_ln < 300*204; the_ln+=300)
        {
            for (int the_px = the_ln; the_px < the_ln+6; the_px++)
            {
                dst_pxl = the_px * 3;
                preview_buffer[dst_pxl+0] = (brd_rgb >> 030) & 0xFF;
                preview_buffer[dst_pxl+1] = (brd_rgb >> 020) & 0xFF;
                preview_buffer[dst_pxl+2] = (brd_rgb >> 010) & 0xFF;
            };
            for (int the_px = the_ln+294; the_px < the_ln+300; the_px++)
            {
                dst_pxl = the_px * 3;
                preview_buffer[dst_pxl+0] = (brd_rgb >> 030) & 0xFF;
                preview_buffer[dst_pxl+1] = (brd_rgb >> 020) & 0xFF;
                preview_buffer[dst_pxl+2] = (brd_rgb >> 010) & 0xFF;
            };
        };
        // Then step through the transition blocks, until we hit the current progress.
        for (int xy_block = 0; xy_block < animate_progress; xy_block++)
        {
            int x_min = current_bmp_object->transition_block(xy_block, 0) *  6;
            int x_max = x_min+6;
            int y_min = current_bmp_object->transition_block(xy_block, 1) * 12;
            int y_max = y_min+12;
            for (int y_inc = y_min; y_inc < y_max; y_inc++)
            {
                src_y = (y_inc - y_offset);
                for (int x_inc = x_min; x_inc < x_max; x_inc++)
                {
                    src_x = (x_inc - x_offset);
                    dst_pxl = (x_inc + y_inc * 300) * 3;
                    tmp_color = current_bmp_object->get_rgb_pixel(src_x, src_y);
                    preview_buffer[dst_pxl+0] = (tmp_color >> 030) & 0xFF;
                    preview_buffer[dst_pxl+1] = (tmp_color >> 020) & 0xFF;
                    preview_buffer[dst_pxl+2] = (tmp_color >> 010) & 0xFF;
                };
            };
        };
    };
    PreviewImage->uncache();
    PreviewBox->redraw();
}

int CDGMagic_BMPClip_Window::handle(int incoming_event)
{
    static int orig_x = 0, orig_y = 0;
    static int x_off_a = 0, y_off_a = 0;

    if (Fl::belowmouse() == PreviewBox)
    {
        switch ( incoming_event )
        {
            case FL_PUSH:
                if (Fl::event_button() != FL_LEFT_MOUSE) { break; };
                orig_x = Fl::event_x();
                orig_y = Fl::event_y();
                x_off_a = current_bmp_object->x_offset();
                y_off_a = current_bmp_object->y_offset();
                // Center on double-click.
                if (Fl::event_clicks() == 1)
                {
                    current_bmp_object->x_offset( (300-current_bmp_object->width())  / 2 );
                    current_bmp_object->y_offset( (216-current_bmp_object->height()) / 2 );
                    update_preview();
                };
                return 1;

            case FL_DRAG:
                if (Fl::event_button() != FL_LEFT_MOUSE) { break; };
                current_bmp_object->x_offset( x_off_a + Fl::event_x() - orig_x );
                current_bmp_object->y_offset( y_off_a + Fl::event_y() - orig_y );
                update_preview();
                return 1;
        };
    };

    // Crazy hack to free the editing window resources if the user closes the window via the window manager ("X", TitleBaseMenu->Close, etc.) instead of the OK button.
    // FLTK provides no direct functionality (that I'm aware of) to determine the difference between the minimize (hide) and close events.
    static int previous_event = -1;
    if ( (incoming_event == FL_HIDE) && (previous_event != FL_UNFOCUS) )
    {
        printf("Deleting CDGMagic_BMPClip_Window object!\n");
        Fl::delete_widget( this );
        return 1;
    };
    previous_event = incoming_event;

    return Fl_Double_Window::handle(incoming_event);
}

void CDGMagic_BMPClip_Window::FillIdx_cb_static(Fl_Widget *CallingWidget, void *IncomingObject)
{
    CDGMagic_BMPClip_Window*  this_win = static_cast<CDGMagic_BMPClip_Window*>( IncomingObject );
    Fl_Counter*            idx_counter = dynamic_cast<Fl_Counter*>( CallingWidget );
    this_win->current_bmp_object->fill_index( static_cast<int>(idx_counter->value()) );
    this_win->update_preview();
}

void CDGMagic_BMPClip_Window::CompIdx_cb_static(Fl_Widget *CallingWidget, void *IncomingObject)
{
    CDGMagic_BMPClip_Window* this_win = static_cast<CDGMagic_BMPClip_Window*>( IncomingObject );
    Fl_Counter*           idx_counter = dynamic_cast<Fl_Counter*>( CallingWidget );
    this_win->current_bmp_object->composite_index( static_cast<int>(idx_counter->value()) );
}

void CDGMagic_BMPClip_Window::ShouldComp_cb_static(Fl_Widget *CallingWidget, void *IncomingObject)
{
    CDGMagic_BMPClip_Window*  this_win = static_cast<CDGMagic_BMPClip_Window*>( IncomingObject );
    Fl_Choice*             type_choice = dynamic_cast<Fl_Choice*>( CallingWidget );
    this_win->current_bmp_object->should_composite( type_choice->value() );
}

void CDGMagic_BMPClip_Window::ScreenIdx_cb_static(Fl_Widget *CallingWidget, void *IncomingObject)
{
    CDGMagic_BMPClip_Window*  this_win = static_cast<CDGMagic_BMPClip_Window*>( IncomingObject );
    Fl_Choice*           screen_choice = dynamic_cast<Fl_Choice*>( CallingWidget );
    CDGMagic_MediaEvent* temp_event = this_win->current_clip->event_queue()->at(this_win->current_page);
    temp_event->memory_preset_index = (screen_choice->value() > 0) ? screen_choice->value() - 1 : 16;
}

void CDGMagic_BMPClip_Window::BorderIdx_cb_static(Fl_Widget *CallingWidget, void *IncomingObject)
{
    CDGMagic_BMPClip_Window*  this_win = static_cast<CDGMagic_BMPClip_Window*>( IncomingObject );
    Fl_Choice*           border_choice = dynamic_cast<Fl_Choice*>( CallingWidget );
    CDGMagic_MediaEvent* temp_event = this_win->current_clip->event_queue()->at(this_win->current_page);
    temp_event->border_index = (border_choice->value() > 0) ? border_choice->value() - 1 : 16;
    this_win->update_preview();
}

void CDGMagic_BMPClip_Window::SetPal_cb_static(Fl_Widget *CallingWidget, void *IncomingObject)
{
    CDGMagic_BMPClip_Window*  this_win = static_cast<CDGMagic_BMPClip_Window*>( IncomingObject );
    Fl_Check_Button*         pal_check = dynamic_cast<Fl_Check_Button*>( CallingWidget );
    CDGMagic_MediaEvent* temp_event = this_win->current_clip->event_queue()->at(this_win->current_page);
    temp_event->PALObject = (pal_check->value() == 1) ? temp_event->BMPObject->PALObject() : NULL;
    pal_check->redraw(); // Force a redraw, because the check isn't always shown right away for some reason...
}

void CDGMagic_BMPClip_Window::animate_cb_static(Fl_Widget *CallingWidget, void *IncomingObject)
{
    CDGMagic_BMPClip_Window*  this_win = static_cast<CDGMagic_BMPClip_Window*>( IncomingObject );
    Fl_Check_Button*     animate_check = dynamic_cast<Fl_Check_Button*>( CallingWidget );
    if (animate_check->value())
    {
        this_win->animate_progress = 0;
        this_win->update_preview();
        CDGMagic_BMPClip_Window::animate_progress_static(IncomingObject);
    }
    else
    {
        Fl::remove_timeout(animate_progress_static);
        this_win->animate_progress = -1;
        this_win->update_preview();
    };
}

void CDGMagic_BMPClip_Window::animate_progress_static(void *IncomingObject)
{
    CDGMagic_BMPClip_Window*  this_win = static_cast<CDGMagic_BMPClip_Window*>( IncomingObject );
    this_win->animate_progress += 5;
    int max_progress = static_cast<int>(static_cast<float>(this_win->current_bmp_object->transition_length()) * 1.2);
    if ( this_win->animate_progress >= max_progress )
    {
        this_win->animate_progress = 0;
    };
    this_win->update_preview();
    Fl::repeat_timeout(0.03, animate_progress_static, IncomingObject);
}

//##### Static Callbacks Below this Line #####//

void CDGMagic_BMPClip_Window::DoOK_callback_static(Fl_Widget *CallingWidget, void *IncomingObject)
{
    ((CDGMagic_BMPClip_Window*)IncomingObject)->DoOK_callback( dynamic_cast<Fl_Button*>(CallingWidget) );
}

void CDGMagic_BMPClip_Window::DoApplyAll_callback_static(Fl_Widget *CallingWidget, void *IncomingObject)
{
    ((CDGMagic_BMPClip_Window*)IncomingObject)->DoApplyAll_callback( dynamic_cast<Fl_Button*>(CallingWidget) );
}

void CDGMagic_BMPClip_Window::DoAdd_callback_static(Fl_Widget *CallingWidget, void *IncomingObject)
{
    ((CDGMagic_BMPClip_Window*)IncomingObject)->DoAdd_callback( dynamic_cast<Fl_Button*>(CallingWidget) );
}

void CDGMagic_BMPClip_Window::DoRemove_callback_static(Fl_Widget *CallingWidget, void *IncomingObject)
{
    ((CDGMagic_BMPClip_Window*)IncomingObject)->DoRemove_callback( dynamic_cast<Fl_Button*>(CallingWidget) );
}

void CDGMagic_BMPClip_Window::DoSetStart_callback_static(Fl_Widget *CallingWidget, void *IncomingObject)
{
    ((CDGMagic_BMPClip_Window*)IncomingObject)->DoSetStart_callback( dynamic_cast<Fl_Button*>(CallingWidget) );
}

void CDGMagic_BMPClip_Window::DoTransition_callback_static(Fl_Widget *CallingWidget, void *IncomingObject)
{
    ((CDGMagic_BMPClip_Window*)IncomingObject)->DoTransition_callback( dynamic_cast<Fl_Button*>(CallingWidget) );
}

void CDGMagic_BMPClip_Window::DoBMPBrowser_callback_static(Fl_Widget *CallingWidget, void *IncomingObject)
{
    ((CDGMagic_BMPClip_Window*)IncomingObject)->DoBMPBrowser_callback( dynamic_cast<Fl_Hold_Browser*>(CallingWidget) );
}
