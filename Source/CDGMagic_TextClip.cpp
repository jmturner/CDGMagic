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

#include "CDGMagic_TextClip.h"
#include "CDGMagic_TextClip_Window.h"

#include "CDGMagic_EditingGroup.h"
#include "CDGMagic_EditingLanes.h"

#include <cmath>  // Used for value of M_PI in rounded outlines.

CDGMagic_TextClip::CDGMagic_TextClip(int X, int Y, int W, int H) : CDGMagic_MediaClip(0, 0), CDGMagic_MovableClipBox(X,Y,W,H,0)
{
    box(FL_FLAT_BOX);
    color(FL_GRAY, FL_WHITE);
    is_movable = 1;
    align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);

    internal_edit_window = NULL;

    internal_karaoke_mode     =  KAR_MODE__TITLES;
    internal_lines_per_page   =  0;  // Gets set by karaoke_mode(...);
    internal_page_mode        =  0;  // Whether to clear by line or page.
    internal_highlight_mode   =  1;  // 0x01 = Text, 0x02 = Outline, 0x03 = Both.
    internal_square_size      =  0;
    internal_round_size       =  0;
    internal_outline_index    =  1;
    internal_box_index        =  0;
    internal_frame_index      =  4;
    internal_foreground_index =  2;
    internal_background_index =  0;
    internal_fill_index       = 16;
    internal_composite_index  = 16;
    internal_should_composite =  1;
    internal_xor_bandwidth    =  4;
    internal_palette_number   =  1;
    internal_antialias_mode   =  0;

    internal_font_index  =  0;
    internal_font_size   = 20;
    internal_text_buffer = new Fl_Text_Buffer();

    render_text_to_bmp();
    copy_label("This is a synchronized text or karaoke clip... Double-click to edit content, style, and timing.");

    duration( w() * 4 );
}

CDGMagic_TextClip::~CDGMagic_TextClip()
{
    // Delete the editing window, if one is present.
    if (internal_edit_window != NULL)  {  Fl::delete_widget(internal_edit_window); internal_edit_window = NULL;  };
    // Delete the text buffer.
    delete internal_text_buffer;
    // Loop through and delete all the pointers (event structs are deleted by MediaClip parent).
    for (unsigned int current_event = 0; current_event < event_queue()->size(); current_event++)
    {
        delete (event_queue()->at(current_event)->BMPObject);
        delete ( static_cast<CDGMagic_TextEvent_Info*>(event_queue()->at(current_event)->user_obj) );
    };
}

void CDGMagic_TextClip::null_edit()
{
    internal_edit_window = NULL;
}

void CDGMagic_TextClip::render_text_to_bmp()
{
    // Karaoke timing/text is handled by an alternate function, for now at least.
    // This should probably either be combined into one "smart" function,
    // or another "render_text" function added to do the basic setup, then
    // dispatch the title or karaoke functions as necessary.
    if ( internal_karaoke_mode >= KAR_MODE__5TLINE )  { render_karaoke_to_bmp(); return; }

    CDGMagic_MediaEvent *current_event_entry;
    CDGMagic_BMPObject *current_image;
    const int line_width  = 288;
    //const int line_height =  24;
    // Determine the larger of the two border sizes.
    int max_border_size = (internal_square_size > internal_round_size) ? internal_square_size : internal_round_size;
    // Then add the border height to the text height.
    int actual_text_height = internal_font_size + max_border_size * 2;
    // Now, do a ceiling division for the minimum number of 12 pixel high font blocks that will take.
    int blk_height = static_cast<int>( (static_cast<double>(actual_text_height) + 11.0) / 12.0 );
    // Then set the line height to the pixel height of that amount of blocks.
    const int line_height = blk_height * 12;
    // Set the lines per page based on line height in page/titles mode.
    if ((internal_karaoke_mode == KAR_MODE__TITLES) && (line_height > 0))  { internal_lines_per_page = 192/line_height; };
    // Create a temporary buffer to copy the offscreen buffer into.
    unsigned char *temp_buffer = new unsigned char[line_width*line_height*3];
    // Create a temporary offscreen buffer for text writing functions.
    Fl_Offscreen offscreen_image = fl_create_offscreen(line_width, line_height);
    // Begin writing offscreen.
    fl_begin_offscreen( offscreen_image );
    // Set the font to the current values.
    fl_font( internal_font_index, internal_font_size );
    // Determine the number of lines in the internal text buffer.
    // NOTE: The "last" line doesn't end with a newline, so add one to the count.
    unsigned int number_of_lines = internal_text_buffer->count_lines(0, internal_text_buffer->length()) + 1;
    // Step through the lines, and render each into a bitmap object.
    for (unsigned int curr_line_num = 0; curr_line_num < number_of_lines; curr_line_num++)
    {
        // NOTE: Colors here should be shifted 3 bytes and ORed with 0x00FFFFFF... RGBA...
        // The last FF is especially important, because the alpha channel must be opaque to get the exact
        // color out that we put in... As these functions are really meant to be used with actual RGB[A] values,
        // and NOT our indexed color palettes.

        // Draw a background box.
        fl_draw_box(FL_FLAT_BOX, 0, 0, line_width, line_height,  (internal_background_index << 030) | 0x00FFFFFF);
        // Only draw box if in line/lyrics mode and it's a visible color.
        if (internal_karaoke_mode == KAR_MODE__LYRICS)
        {
            if (internal_box_index   < 16)  { fl_draw_box(FL_ROUNDED_BOX, 0, 0, line_width, line_height, (internal_box_index << 030) | 0x00FFFFFF);     };
            if (internal_frame_index < 16)  { fl_draw_box(FL_ROUNDED_FRAME, 0, 0, line_width, line_height, (internal_frame_index << 030) | 0x00FFFFFF); };
        };
        // Find the start of the current line.
        int line_start_pos = internal_text_buffer->skip_lines(0, curr_line_num);
        // Extract the current line.
        char* current_line_text = internal_text_buffer->line_text(line_start_pos);
        // Set the text color to requested effect index.
        fl_color( (internal_outline_index << 030) | 0x00FFFFFF );
        int left_start = (line_width - static_cast<int>(fl_width(current_line_text))) / 2;
        int top_start  = (line_height - fl_height()) / 2;
            top_start  = top_start + fl_height() - fl_descent();

        // Based on code from:
        // http://www.gamedev.net/community/forums/topic.asp?topic_id=503851
        // This straight-forward loop is simple enough that I don't
        // think it could really be copywritten...
        // HOWEVER, it could at least be optimized a bit.

        // Draw the circular outline of the text.
        for (int i = 0; i <= internal_round_size; i += 1)
        {
            for (int theta = 0; theta <= 360; theta++)
            {
                double cose = i * cos((double)((theta) * (M_PI / 180)));
                double sine = i * sin((double)((theta) * (M_PI / 180)));

                fl_draw(current_line_text, left_start-(int)sine, top_start-(int)cose);
            };
        };

        // Draw the square outline of the text.
        for (int bx_inc = -internal_square_size; bx_inc <= internal_square_size; bx_inc++)
        {
          for (int by_inc = -internal_square_size; by_inc <= internal_square_size; by_inc++)
          {
              fl_draw(current_line_text, left_start+bx_inc, top_start+by_inc);
          };
        };

        // Simple "antialias" testing code.
        if (internal_antialias_mode == 1)
        {
            fl_color( ((internal_foreground_index+1) << 030) | 0x00FFFFFF );
            fl_draw(current_line_text,  left_start-1,  top_start);
            fl_draw(current_line_text,  left_start+1,  top_start);
            fl_draw(current_line_text,  left_start,  top_start-1);
            fl_draw(current_line_text,  left_start,  top_start+1);
        };

        // Set the text color to requested foreground index.
        fl_color( (internal_foreground_index << 030) | 0x00FFFFFF );
        // Actually draw the foreground text.
        fl_draw(current_line_text,  left_start,  top_start);
        // Read the offscreen image into memory.
        fl_read_image(temp_buffer, 0, 0, line_width, line_height);
        // See if we need to create a new bitmap object, or if we can reuse the old.
        int set_initial = 0;
        while ( curr_line_num >= event_queue()->size() )  { generate_new_mediaevent(); set_initial = 1; };
        // Assign the current pointers.
        current_event_entry = event_queue()->at(curr_line_num);
        if (curr_line_num == 0)
        {
            // Assign the optional border and screen clear colors if in page mode.
            current_event_entry->border_index        = (internal_karaoke_mode == KAR_MODE__TITLES) ? internal_frame_index : 16;
            current_event_entry->memory_preset_index = (internal_karaoke_mode == KAR_MODE__TITLES) ? internal_box_index   : 16;
        };
        current_image       = current_event_entry->BMPObject;
        // Copy the line text to the event string buffer.
        // (Currently, this is only used to display the text on the timeline clip.)
        CDGMagic_TextEvent_Info *line_textinfo = static_cast<CDGMagic_TextEvent_Info*>(current_event_entry->user_obj);
        line_textinfo->karaoke_type = TEXT_ONLY;
        line_textinfo->text_string.assign(current_line_text);
        line_textinfo->line_num = curr_line_num;
        line_textinfo->word_num = 0;
        // Free the newly made current line (this is ugh!).
        free(current_line_text);
        // Set up the initial image values.
        current_image->alter_buffer_size(line_width, line_height); // Does nothing if same.
        current_image->fill_index(internal_fill_index);
        current_image->composite_index(internal_composite_index);
        current_image->should_composite(internal_should_composite);
        current_image->transition_row_mask(0xFFFFFFFF); // Basic, Left->Right->Top->Bottom transition.
        if (internal_karaoke_mode == KAR_MODE__TITLES)
        {
            if (curr_line_num > 0)  { current_image->should_composite(2); }; // Always COMPOSITE INTO, if past first line of page text clip.
            if (set_initial == 1)
            {
                current_image->y_offset( (curr_line_num % internal_lines_per_page) * line_height + 12 ); // Set the initial Y offset for new events.
            }
        }
        else
        {
            current_image->x_offset( event_queue()->at(0)->BMPObject->x_offset() ); // Set the initial X offset for new events.
            current_image->y_offset( event_queue()->at(0)->BMPObject->y_offset() ); // Set the initial Y offset for new events.
        };
        // Copy the image into the current BMP object.
        for (int px = 0; px < line_width * line_height; px++)  { current_image->linear_pixel(px, temp_buffer[px*3] ); };
    };
    // End offscreen drawing.
    fl_end_offscreen();
    // Delete the offscreen buffer.
    fl_delete_offscreen(offscreen_image);
    // Delete and remove any surplus objects in the buffer.
    while (event_queue()->size() > number_of_lines)
    {
        delete ( event_queue()->back()->BMPObject ); // Free the BMP.
        delete ( static_cast<CDGMagic_TextEvent_Info*>(event_queue()->back()->user_obj) ); // Free the string.
        delete ( event_queue()->back() );            // Free the MediaEvent.
        event_queue()->pop_back();                     // Remove the object.
    };
    // Delete the temp buffer.
    delete[] temp_buffer;
    // Set the palettes.
    set_all_palettes();
    // Get rid of any default label, since we no longer need it (we'll use the text instead from inside draw()).
    copy_label("");
}

void CDGMagic_TextClip::render_karaoke_to_bmp()
{
    printf("Now rendering karaoke type text...\n");

    CDGMagic_MediaEvent *current_event_entry;
    CDGMagic_BMPObject *line_image, *word_image;
    CDGMagic_TextEvent_Info *line_textinfo, *word_textinfo;
    const int line_width     = 288;
    const int line_height    = (internal_lines_per_page == 5) ? 36 : 24;
    const unsigned int lines_per_page = internal_lines_per_page;
    const unsigned int left_margin    =   6;
          unsigned int top_margin     =  12;

    // Maybe "Magic" was actually referring to all the numbers in the code base?... :-)
    // TODO: Move all this out to a reasonably simple, but comprehensive set of user options for:
    // Lines, Line Height, Top Margin, Line/Page Clear, Highlight Timing Options, etc.
    switch (internal_karaoke_mode)
    {
        case KAR_MODE__5BLNCT:
        case KAR_MODE__5BLNFD:
        case KAR_MODE__7MLNCT:
        case KAR_MODE__7MLNFD:
        case KAR_MODE__5BLINE:
        case KAR_MODE__5BPAGE: top_margin =  24; break; // 5 Line, bottom justified modes.
        case KAR_MODE__6MLINE:
        case KAR_MODE__6MPAGE: top_margin =  36; break; // 8 [6] Line, middle justified modes.
        case KAR_MODE__84BLIN: top_margin = 108; break; // 8 [4] Line, bottom justified mode.
                      default: top_margin =  12; break; // All other modes.
    };

    // Assign the default karaoke type.
    static_cast<CDGMagic_TextEvent_Info*>(event_queue()->front()->user_obj)->karaoke_type = KARAOKE_DRAW;

    // Create a temporary buffer to copy the offscreen buffer into.
    unsigned char *temp_buffer = new unsigned char[line_width*line_height*3];
    // Create a temporary offscreen buffer for text writing functions.
    Fl_Offscreen offscreen_image = fl_create_offscreen(line_width, line_height);
    // Begin writing offscreen.
    fl_begin_offscreen( offscreen_image );
    // Set the font to the current values.
    fl_font( internal_font_index, internal_font_size );
    // Determine the number of lines in the internal text buffer.
    // NOTE: The "last" line doesn't end with a newline, so add one to the count.
    unsigned int number_of_lines = internal_text_buffer->count_lines(0, internal_text_buffer->length()) + 1;
    signed   int event_queue_pos = 0;
    // Step through the lines, and render each into a bitmap object.
    for (unsigned int curr_line_num = 0; curr_line_num < number_of_lines; curr_line_num++)
    {
        // Find the start of the current line.
        int line_start_pos = internal_text_buffer->skip_lines(0, curr_line_num);
        // Extract the current line.
        char* current_line_text = internal_text_buffer->line_text(line_start_pos);

        // See if we need to erase the line, and make a bitmap to do it if so.
        // Only needed when in line-by-line display mode.
        if ( internal_page_mode == 0 )
        {
            // See if we need to create a new bitmap object, or if we can reuse the old.
            event_queue_pos = get_line_word_type_event(KARAOKE_ERASE, curr_line_num, -1);
            if ( event_queue_pos == -1 )  {  event_queue_pos = generate_new_mediaevent();  };
            // Assign the current pointers.
            current_event_entry = event_queue()->at(event_queue_pos);
            line_image          = current_event_entry->BMPObject;
            // Set some default parameters.
            current_event_entry->border_index = 16;
            current_event_entry->memory_preset_index = 16;
            current_event_entry->x_scroll = -1;
            current_event_entry->y_scroll = -1;
            // Copy the line text to the event string buffer. (Currently, this is only used to display the text on the timeline clip.)
            line_textinfo = static_cast<CDGMagic_TextEvent_Info*>(current_event_entry->user_obj);
            line_textinfo->text_string.assign(current_line_text);
            line_textinfo->line_num     = curr_line_num;
            line_textinfo->word_num     = -1;
            line_textinfo->karaoke_type = KARAOKE_ERASE;
            // Set up the initial image values.
            line_image->alter_buffer_size(line_width, line_height); // Does nothing if same.
            line_image->xor_only(0);
            line_image->fill_index(internal_fill_index);
            line_image->composite_index(internal_composite_index);
            line_image->transition_row_mask( get_line_transition_mask(curr_line_num % internal_lines_per_page) );
            // If we're in line palette mode, erase "to transparent", which is basically a very hacky "ignore".
            // Otherwise, use the older, original code.
            if (internal_karaoke_mode >= KAR_MODE__5BLNCT)
            {
                line_image->should_composite(2);
                for (int px = 0; px < line_width * line_height; px++)  { line_image->linear_pixel(px, 16); };
            }
            else
            {
                line_image->should_composite(1);
                for (int px = 0; px < line_width * line_height; px++)  { line_image->linear_pixel(px, internal_background_index); };
            };
            // Set the X/Y offsets.
            line_image->x_offset( left_margin );
            line_image->y_offset( (curr_line_num % lines_per_page) * line_height + top_margin );
        };
/*
        int extent_x = 0;
        int extent_y = 0;
        int extent_w = 0;
        int extent_h = 0;

        fl_text_extents( current_line_text, extent_x, extent_y, extent_w, extent_h );
        printf("Extents: x:%i, y:%i, w:%i, h:%i\n", extent_x, extent_y, extent_w, extent_h );
        printf("Special Height:%i, Height:%i, Descent:%i\n", fl_height(internal_font_index, internal_font_size), fl_height(), fl_descent() );
*/

        // NOTE Colors here should be shifted 3 bytes and ORed with 0x00FFFFFF... RGBA...
        // The last FF is especially important, because the alpha channel must be opaque to get the exact
        // color out that we put in... As these functions are really meant to be used with actual RGB[A] values,
        // and NOT our indexed color palettes.

        // Draw a solid background box, then set the text color to the requested effect index.
        if (internal_karaoke_mode >= KAR_MODE__5BLNCT)
        {
            // Draw a solid background box, WHICH IS ALWAYS INDEX 0 in new Cut/Fade modes. (The actual RGB color of palette index 0 is set later.)
            fl_draw_box(FL_FLAT_BOX, 0, 0, line_width, line_height, 0x00FFFFFF);
            // Calculate and set the effect index value.
            int actual_index = (curr_line_num % lines_per_page) * 3 + 1;
            fl_color( (actual_index << 030) | 0x00FFFFFF );
        }
        else
        {
            // Draw a solid background box, set the to the selected color index.
            fl_draw_box(FL_FLAT_BOX, 0, 0, line_width, line_height,  (internal_background_index << 030) | 0x00FFFFFF);
            // Set the effect color to the selected index.
            fl_color( (internal_outline_index << 030) | 0x00FFFFFF );
        };

        int left_start = (line_width - static_cast<int>(fl_width(current_line_text))) / 2;
        int actual_text_height = fl_height() - fl_descent();
        int top_start  = (actual_text_height-fl_descent()) + ( (line_height-actual_text_height) / 2 );

        // Outline is only supported for 5-line "line palette" -- NOT 7-line! -- and all standard modes.
        if (internal_karaoke_mode <= KAR_MODE__5BLNFD)
        {
        // Based on code from:
        // http://www.gamedev.net/community/forums/topic.asp?topic_id=503851
        // This straight-forward loop is simple enough that I don't
        // think it could really be copywritten...
        // HOWEVER, it could at least be optimized a bit.

        // Draw the circular outline of the text.
        for (int i = 0; i <= internal_round_size; i += 1)
        {
            for (int theta = 0; theta <= 360; theta++)
            {
                double cose = i * cos((double)((theta) * (M_PI / 180)));
                double sine = i * sin((double)((theta) * (M_PI / 180)));

                fl_draw(current_line_text, left_start-(int)sine, top_start-(int)cose);
            };
        };

        // Draw the square outline of the text.
        for (int bx_inc = -internal_square_size; bx_inc <= internal_square_size; bx_inc++)
        {
            for (int by_inc = -internal_square_size; by_inc <= internal_square_size; by_inc++)
            {
                fl_draw(current_line_text, left_start+bx_inc, top_start+by_inc);
            };
        };
        }; // End of outline drawing ignore for 7-line "line palette" modes.

        // Simple "antialias" testing code. ("Line palette" modes do not support antialias of any kind.)
        if ( (internal_antialias_mode == 1) && (internal_karaoke_mode <= KAR_MODE__84BLIN) )
        {
            fl_color( ((internal_foreground_index+1) << 030) | 0x00FFFFFF );
            fl_draw(current_line_text,  left_start-1,  top_start);
            fl_draw(current_line_text,  left_start+1,  top_start);
            fl_draw(current_line_text,  left_start,  top_start-1);
            fl_draw(current_line_text,  left_start,  top_start+1);
        };

        // Set the text color to requested foreground index.
        if (internal_karaoke_mode >= KAR_MODE__7MLNCT)
        {
            int actual_index = (curr_line_num % lines_per_page) * 2 + 2;
            fl_color( (actual_index << 030) | 0x00FFFFFF );
        }
        else if (internal_karaoke_mode >= KAR_MODE__5BLNCT)
        {
            int actual_index = (curr_line_num % lines_per_page) * 3 + 2;
            fl_color( (actual_index << 030) | 0x00FFFFFF );
        }
        else
        {
            fl_color( (internal_foreground_index << 030) | 0x00FFFFFF );
        };
        // Actually draw the foreground text.
        fl_draw(current_line_text,  left_start,  top_start);
        // Read the offscreen image into memory.
        fl_read_image(temp_buffer, 0, 0, line_width, line_height);

        // See if we need to create a new bitmap object, or if we can reuse the old.
        event_queue_pos = get_line_word_type_event(KARAOKE_DRAW, curr_line_num, 0);
        if ( event_queue_pos == -1 )  {  event_queue_pos = generate_new_mediaevent();  };
        // Assign the current pointers.
        current_event_entry = event_queue()->at(event_queue_pos);
        line_image          = current_event_entry->BMPObject;
        // Set the screen/border color in page-by-page mode, or disable in line-by-line mode.
        current_event_entry->border_index        = ((internal_page_mode==1)&&(curr_line_num>0)&&((curr_line_num%internal_lines_per_page)==0)) ? internal_box_index : 16;
        current_event_entry->memory_preset_index = ((internal_page_mode==1)&&(curr_line_num>0)&&((curr_line_num%internal_lines_per_page)==0)) ? internal_box_index : 16;
        // Copy the line text to the event string buffer.
        // (Currently, this is only used to display the text on the timeline clip.)
        line_textinfo = static_cast<CDGMagic_TextEvent_Info*>(current_event_entry->user_obj);
        line_textinfo->text_string.assign(current_line_text);
        line_textinfo->line_num     = curr_line_num;
        line_textinfo->word_num     = 0;
        line_textinfo->karaoke_type = KARAOKE_DRAW;
        // Free the newly made current line (this is ugh!).
        free(current_line_text);
        // Set up the initial image values.
        line_image->alter_buffer_size(line_width, line_height); // Does nothing if same.
        line_image->fill_index(internal_fill_index);
        line_image->xor_only(0);
        line_image->composite_index(internal_composite_index);
        line_image->should_composite(1);
        line_image->transition_row_mask( get_line_transition_mask(curr_line_num % internal_lines_per_page) );
        // Copy the image into the current BMP object.
        for (int px = 0; px < line_width * line_height; px++)  { line_image->linear_pixel(px, temp_buffer[px*3] ); };
        // Set the X/Y offsets.
        line_image->x_offset( left_margin );
        line_image->y_offset( (curr_line_num % lines_per_page) * line_height + top_margin );

        //##### Begin Highlight Rendering #####//
        float wipe_bandwidth = 0.0;        // Convert the bandwidth setting to actual bandwidth... (This definitely needs moved!)
        switch ( internal_xor_bandwidth )
        {
             case 0: wipe_bandwidth = 4.0/1.0; break;
             case 1: wipe_bandwidth = 3.0/1.0; break;
             case 2: wipe_bandwidth = 2.0/1.0; break;
             case 3: wipe_bandwidth = 3.0/2.0; break;
             case 4: wipe_bandwidth = 4.0/3.0; break;
            default: wipe_bandwidth = 3.0/1.0; break;
        };
        unsigned int word_start = 0;  // Character position of current word start.
        unsigned int word_end   = 0;  // Character position of current word end.
        unsigned int curr_word  = 1;  // Current word number (starts at 1).
        // For purposes of highlighting, first determine the larger border, then ignore if we're not highlighting it.
        int border_size = (internal_square_size > internal_round_size) ? internal_square_size : internal_round_size;
            border_size = (internal_highlight_mode & 0x02) ? border_size : 0;
            border_size = ((border_size == 0) && (internal_antialias_mode == 1)) ? 1 : border_size;
        // NOTE: fl_width seems to only return integer values (as a float)! Just convert to int first and be done with it.
        int word_x_pos      = left_start;
        int inter_word      = (fl_width(" ") + 1.0) / 2.0;
        int wipe_x_pos      = 0;
        int wipe_x_width    = 0;
        int wipe_min_x_pos  = 0;
        int wipe_max_x_pos  = 300;
        // Step through the words, rendering the wipes.
        while ( curr_word < 100 )
        {
            // Find the end of the current word.
            word_end = line_textinfo->text_string.find(' ', word_start);
            // Get a string reference to the single word only.
            std::string the_word( line_textinfo->text_string.substr(word_start, word_end-word_start) );
#ifdef _CDGMAGIC_KARAOKERENDER_INFO_
            printf("Karaoke Render: Line %i, Word %i <%s>\n", curr_line_num, curr_word, the_word.c_str() );
#endif
            // ----- Determine the word's size. -----
            // fl_width(*text) will determine the actual width of a word or character,
            // HOWEVER, you need to ADD the values returned by fl_text_extents(*text,dx,dy,w,h)
            // to find the bounding box which matches the actual start/end pixel locations.
            // That's because, depending on the font (especially, for example, italics),
            // there may be pixels drawn before/after those start/width locations.
            // Extents can even "overlap", but this code doesn't take that in to account yet.
            // (It's probably not strictly necessary for karaoke rendering, but ideally
            // each highlight segment should be rendered independently and only those pixels
            // which "belong" to that segment highlighted.)
            int ext_x = 0, ext_y = 0, ext_w = 0, ext_h = 0;
            fl_text_extents(the_word.c_str(), ext_x, ext_y, ext_w, ext_h);
            // Calculate the wipe starting position by adding in the x extant.
            wipe_x_pos = word_x_pos + ext_x;
            // Set the initial width to the extent width.
            wipe_x_width = ext_w;
            wipe_max_x_pos = (word_end == line_textinfo->text_string.npos) ? 300 : wipe_x_pos + wipe_x_width + inter_word;
            //wipe_max_x_pos = (word_end == line_textinfo->text_string.npos) ? 300 : word_x_pos + fl_width(the_word) + inter_word;
            // Accomodate the left hand border.
            wipe_x_pos   -= border_size;
            wipe_x_width += border_size;
            if (wipe_x_pos < wipe_min_x_pos)
            {
                wipe_x_width -= (wipe_min_x_pos - wipe_x_pos);
                wipe_x_pos = wipe_min_x_pos;
            };
            // Accomodate the right hand border.
            wipe_x_width += border_size;
            if ((wipe_x_pos+wipe_x_width) > wipe_max_x_pos)
            {
                wipe_x_width = (wipe_max_x_pos - wipe_x_pos);
            };
            wipe_min_x_pos = wipe_x_pos+wipe_x_width;
            if ( ext_w == 0 )  { wipe_x_width = 0; }; // Hacky hack so textless wipes are always 0 width.
        //##### Begin Highlight Event #####//
            // See if we need to create a new bitmap object, or if we can reuse the old.
            event_queue_pos = get_line_word_type_event(KARAOKE_WIPE, curr_line_num, curr_word);
            if ( event_queue_pos == -1 )  {  event_queue_pos = generate_new_mediaevent();  };
            // Assign the current pointers.
            current_event_entry = event_queue()->at(event_queue_pos);
            word_image          = current_event_entry->BMPObject;
            // Copy the line text to the event string buffer.
            // (Currently, this is only used to display the text on the timeline clip.)
            word_textinfo = static_cast<CDGMagic_TextEvent_Info*>(current_event_entry->user_obj);
            word_textinfo->text_string.assign(the_word);
            word_textinfo->line_num     = curr_line_num;
            word_textinfo->word_num     = curr_word++;
            word_textinfo->karaoke_type = KARAOKE_WIPE;
            // Set up the initial image values.
            word_image->alter_buffer_size(wipe_x_width, line_height); // Does nothing if same.
            word_image->xor_only(internal_frame_index);
            word_image->fill_index(0);
            word_image->composite_index(internal_composite_index);
            word_image->should_composite(2);
            word_image->xor_bandwidth(wipe_bandwidth);

        if (internal_karaoke_mode >= KAR_MODE__5BLNCT)
        {
            int actual_fg_index = 0;
            int actual_hl_index = 0;
            if (lines_per_page == 5) // 5 Lines per page.
            {
                actual_fg_index =  (curr_line_num % lines_per_page) * 3 + 2;
                actual_hl_index = ((curr_line_num % lines_per_page) * 3 + 3) ^ actual_fg_index;
            }
            else                     // 7 Lines per page.
            {
                actual_fg_index =  (curr_line_num % lines_per_page) * 2 + 2;
                actual_hl_index = ((curr_line_num % lines_per_page) * 2 + 3) ^ actual_fg_index;
            };
            word_image->xor_only(actual_hl_index);
            for (int py = 0; py < line_height; py++)
            {
                for (int px = 0; px < wipe_x_width; px++)
                {
                              int  cur_px  = line_image->pixel(px+wipe_x_pos, py);
                    unsigned char xor_idx  = (cur_px == actual_fg_index) ? 1 : 0;
                    word_image->pixel(px, py, xor_idx);
                };
            };
        }
        else
        {
            // Copy the image into the current BMP object.
            for (int py = 0; py < line_height; py++)
            {
                for (int px = 0; px < wipe_x_width; px++)
                {
                              int  cur_px  = line_image->pixel(px+wipe_x_pos, py);
                    unsigned char xor_idx  = ((internal_highlight_mode & 0x01) && (cur_px == internal_foreground_index)) ? 1 : 0;
                                  xor_idx |= ((internal_antialias_mode == 1) && (internal_highlight_mode & 0x01) && (cur_px == internal_foreground_index+1)) ? 1 : 0;
                                  xor_idx |= ((internal_highlight_mode & 0x02) && (cur_px == internal_outline_index)) ? 1 : 0;
                    word_image->pixel(px, py, xor_idx);
                };
            };
        };

            // Set the X/Y offsets.
            word_image->x_offset( left_margin + wipe_x_pos );
            word_image->y_offset( (curr_line_num % lines_per_page) * line_height + top_margin );
        //##### End Highlight Event #####//
            // Break out of the loop if we're at the end of the current line.
            if ( word_end == line_textinfo->text_string.npos )
            {
                clean_up_words(curr_line_num, curr_word);
                break;
            };
            // Assign the end positiong of this word to the next start search position.
            word_start = word_end+1;
            // Advance the word's graphical position.
            word_x_pos += fl_width( the_word.c_str() ) + fl_width(" ");
        };
    };
    // End offscreen drawing.
    fl_end_offscreen();
    // Delete the offscreen buffer.
    fl_delete_offscreen(offscreen_image);
    // Delete and remove any surplus objects in the buffer.
    clean_up_lines(number_of_lines);
    // Delete the temp buffer.
    delete[] temp_buffer;
    // Sort the event queue by start offset time.
    sort_event_queue();
    // Set the palettes.
    set_all_palettes();
    // Assign the user selected border and screen clear colors, always forcing to 0 in cut/fade modes.
    event_queue()->front()->border_index        = (internal_karaoke_mode >= KAR_MODE__5BLNCT) ? 0 : internal_box_index;
    event_queue()->front()->memory_preset_index = (internal_karaoke_mode >= KAR_MODE__5BLNCT) ? 0 : internal_box_index;
    // Get rid of any default label, since we no longer need it (we'll use the text instead from inside draw()).
    copy_label("");
}

int CDGMagic_TextClip::get_line_word_type_event(int requested_type, int requested_line, int requested_word)
{
    MediaEvent_Queue *clip_events = event_queue();
    for (unsigned int event_num = 0; event_num < clip_events->size(); event_num++ )
    {
        CDGMagic_MediaEvent *the_event = clip_events->at(event_num);
        CDGMagic_TextEvent_Info *inf = static_cast<CDGMagic_TextEvent_Info*>(the_event->user_obj);
        if ( (inf->line_num == requested_line) && (inf->karaoke_type == requested_type) && (inf->word_num == requested_word) )
        {
            return event_num;
        };
    };
    return -1;
}

unsigned long CDGMagic_TextClip::get_line_transition_mask(int requested_line_number)
{
    unsigned long return_mask = (internal_lines_per_page == 5) ? 0x07 : 0x03;
              int  row_height = (internal_lines_per_page == 5) ? 3 : 2;
              int  shift_rows = row_height * requested_line_number;
    // Copied from render_karaoke_to_bmp() for extra fun.
    switch (internal_karaoke_mode)
    {
        case KAR_MODE__5BLNCT:
        case KAR_MODE__5BLNFD:
        case KAR_MODE__7MLNCT:
        case KAR_MODE__7MLNFD:
        case KAR_MODE__5BLINE:
        case KAR_MODE__5BPAGE: shift_rows += 2; break; // 5 Line, bottom justified modes.
        case KAR_MODE__6MLINE:
        case KAR_MODE__6MPAGE: shift_rows += 3; break; // 8 [6] Line, middle justified modes.
        case KAR_MODE__84BLIN: shift_rows += 9; break; // 8 [4] Line, bottom justified mode.
                      default: shift_rows += 1; break; // All other modes.
    };
    // Shift the mask.
    return_mask <<= shift_rows;
    // Return the rows to be drawn for this line.
    return return_mask;
}

void CDGMagic_TextClip::clean_up_words(int requested_line, int requested_word)
{
    MediaEvent_Queue *clip_events = event_queue();
    unsigned int event_num = 0;
    while ( event_num < clip_events->size() )
    {
        CDGMagic_MediaEvent *the_event = clip_events->at(event_num);
        CDGMagic_TextEvent_Info *inf = static_cast<CDGMagic_TextEvent_Info*>(the_event->user_obj);
        if ( (inf->line_num == requested_line) && ((inf->word_num >= requested_word) || (inf->karaoke_type == TEXT_ONLY)) )
        {
            destroy_event(event_num);
        } else { event_num++; };
    };
    return;
}

void CDGMagic_TextClip::clean_up_lines(int requested_line)
{
    MediaEvent_Queue *clip_events = event_queue();
    unsigned int event_num = 0;
    while ( event_num < clip_events->size() )
    {
        CDGMagic_MediaEvent *the_event = clip_events->at(event_num);
        CDGMagic_TextEvent_Info *inf = static_cast<CDGMagic_TextEvent_Info*>(the_event->user_obj);
        if (  (inf->line_num >= requested_line)                                                     // Remove any existing lines after the last used line.

           || (    (internal_karaoke_mode < KAR_MODE__5BLNCT)                                       // Limit to non-palette switching modes.
                && (internal_page_mode == 0)                                                        // Limit to line-by-line mode.
                && (inf->karaoke_type != TEXT_ONLY)                                                 // DRAW, WIPE, or ERASE Events
                && (inf->text_string.length() == 0)                                                 // Line/word contains no text.
                && ((inf->line_num > 0) || (inf->karaoke_type == KARAOKE_ERASE))    )               // And not the first line draw event.

           || (    (internal_karaoke_mode < KAR_MODE__5BLNCT)                                       // Limit to non-palette switching modes.
                && (internal_page_mode == 1)                                                        // Page-by-page mode.
                && (inf->karaoke_type == KARAOKE_DRAW)                                              // DRAW Event only.
                && ((inf->line_num % internal_lines_per_page) != 0)                                 // Not the first line on a page.
                && (inf->text_string.length() == 0)    )                                            // Line/word contains no text.

           || ((inf->karaoke_type == KARAOKE_WIPE) && (inf->text_string.length() == 0))             // Remove ALL karaoke wipe events that don't contain text (i.e. blank).
           || ((internal_page_mode == 1) && (inf->karaoke_type == KARAOKE_ERASE))  )                // Remove ALL karaoke erase events when in page mode.
        {
            destroy_event(event_num);
        } else { event_num++; };
    };
    return;
}

void CDGMagic_TextClip::destroy_event(int requested_event)
{
    delete ( event_queue()->at(requested_event)->BMPObject ); // Free the BMP.
    delete ( static_cast<CDGMagic_TextEvent_Info*>(event_queue()->at(requested_event)->user_obj) ); // Free the string.
    delete ( event_queue()->at(requested_event) );            // Free the MediaEvent.
    event_queue()->erase(event_queue()->begin() + requested_event) ;   // Remove the object.
    return;
}

int CDGMagic_TextClip::generate_new_mediaevent()
{
    // Generate a new BMP object.
    CDGMagic_BMPObject *current_image = new CDGMagic_BMPObject();
    current_image->x_offset(6);  // Sensible default, for now, visible left justified.
    current_image->y_offset(36); // Hopefully sensible default, two blocks down from visible top.

    // Make a new text event info entry.
    CDGMagic_TextEvent_Info *current_textinfo = new CDGMagic_TextEvent_Info;
    current_textinfo->text_string.assign("");
    current_textinfo->line_num     = 0;
    current_textinfo->word_num     = 0;
    current_textinfo->karaoke_type = TEXT_ONLY;

    // Make a new event entry.
    CDGMagic_MediaEvent *current_event_entry = new CDGMagic_MediaEvent;
    // Set up the values.
    current_event_entry->start_offset = (event_queue()->size() > 0) ? event_queue()->back()->start_offset+100 : 0;
    current_event_entry->actual_start_offset = current_event_entry->start_offset;
    current_event_entry->duration  = 0;
    current_event_entry->actual_duration = current_event_entry->duration;
    current_event_entry->BMPObject = current_image;
    current_event_entry->PALObject = NULL;
    current_event_entry->user_obj = current_textinfo;
    current_event_entry->border_index = 16;
    current_event_entry->memory_preset_index = 16;
    current_event_entry->x_scroll = -1;
    current_event_entry->y_scroll = -1;

    // Add it to this clip's event queue.
    event_queue()->push_back(current_event_entry);
    return (event_queue()->size() - 1);
}

void CDGMagic_TextClip::set_all_palettes()
{
    // "Safe" conditional -- Uses original code for all older modes,
    // and duplicated code in a new function for the *very* experimental "Line Palette" mode.
    // This can be refactored when/if the palette switching modes are confirmed working.
    if (internal_karaoke_mode >= KAR_MODE__5BLNCT)
    {
        set_all_linepalettes();
        return;
    };

    // Determine the palette to use, without running beyond the end of the array.
    int palette_to_use = ((internal_palette_number >= 0) && (internal_palette_number < maximum_embedded_palettes)) ? internal_palette_number : 0;
    // Go through every clip and set the palettes for each event.
    for (unsigned int curr_clip = 0; curr_clip < event_queue()->size(); curr_clip++)
    {
        // Convenience pointer to the current Media Event.
        CDGMagic_MediaEvent *curr_event = event_queue()->at(curr_clip);
        // Convenience pointer to the current BMP Object.
        CDGMagic_BMPObject  *curr_img   = curr_event->BMPObject;
        // Set the PAL Object to NULL, so that the palette won't be updated for this clip.
        curr_event->PALObject = NULL;
        // Don't set the palette for a non-existent BMP Object.
        if (curr_img == NULL)  { continue; };
        // Set the update mask and, dissolve time, and delay in case they were set by the Magic Palette.
        // TODO: There should be a "reset" function, perhaps as part of PALObject (for now), or as part of a real MediaEvent object (NOT STRUCT).
        curr_img->draw_delay(0);
        curr_img->PALObject()->update_mask(0xFFFF);
        curr_img->PALObject()->dissolve(0);
        // Set the 16 color palette to the RGB values of the requested palette.
        for (int pal_idx = 0; pal_idx < 16; pal_idx++)
        {
            curr_img->PALObject()->color(pal_idx, embedded_palettes[palette_to_use][pal_idx]);
        };
    };
    // Assign the requested palette to the main clip, or not, as needed.
    event_queue()->front()->PALObject = (internal_palette_number >= 0) ? event_queue()->front()->BMPObject->PALObject() : NULL;
}

void CDGMagic_TextClip::set_all_linepalettes()
{
    // Determine the palette to use, without running beyond the end of the array.
    int palette_to_use = ((internal_palette_number >= 0) && (internal_palette_number < maximum_embedded_palettes)) ? internal_palette_number : 0;

    // Map the selected index values to the selected palette's RGB values, then rearrange the palette below.
    // (This is ugh!!... But it should work for now...)
    unsigned long temp_wipe_index = internal_foreground_index ^ internal_frame_index;
    unsigned long temp_back = (internal_background_index < 16) ? embedded_palettes[palette_to_use][internal_background_index] : FL_BLACK;
    unsigned long temp_outl = (internal_outline_index    < 16) ? embedded_palettes[palette_to_use][internal_outline_index   ] : temp_back;
    unsigned long temp_fore = (internal_foreground_index < 16) ? embedded_palettes[palette_to_use][internal_foreground_index] : temp_back;
    unsigned long temp_wipe = (temp_wipe_index           < 16) ? embedded_palettes[palette_to_use][temp_wipe_index          ] : temp_back;

    // Hard coded row number to palette update mapping.
    const unsigned long palette_row_masks[2][8] = { { 0x000E, 0x0070, 0x0380, 0x1C00, 0xE000, 0x0000, 0x0000, 0x0000 },    // Five  line masks.
                                                    { 0x000C, 0x0030, 0x00C0, 0x0300, 0x0C00, 0x3000, 0xC000, 0x0000 }  }; // Seven line masks.

    // Go through every clip and set the palettes for each event.
    for (unsigned int curr_clip = 0; curr_clip < event_queue()->size(); curr_clip++)
    {
        // Convenience pointer to the current Media Event.
        CDGMagic_MediaEvent *curr_event = event_queue()->at(curr_clip);
        // Convenience pointer to the current BMP Object.
        CDGMagic_BMPObject  *curr_img   = curr_event->BMPObject;
        // Set the PAL Object to NULL by default, so that the palette won't be updated for this event.
        curr_event->PALObject = NULL;
        // Don't set the palette for a non-existent image. (Currently, this should never happen...)
        if (curr_img == NULL)  { continue; };
        // Convenience pointer to the current line text info.
        CDGMagic_TextEvent_Info *textinfo = static_cast<CDGMagic_TextEvent_Info*>(curr_event->user_obj);
        // Calculate the current line (0 based).
        int this_row_num = (textinfo->line_num % internal_lines_per_page);
        // Set the updatable palette mask based on the row number of this line.
        // These correspond to the palette indices that are updated in the encoder...
        // Other indices are IGNORED, and are copied from the current values at that time index in the encoder.
        signed   int  mask_array_offset = (internal_lines_per_page == 7) ? 1 : 0;
        unsigned long palette_row_mask  = palette_row_masks[mask_array_offset][this_row_num];
        // Convenience pointer to the current palette.
        CDGMagic_PALObject  *curr_pal   = curr_img->PALObject();
        // Set the initial 0 index to the background RGB color.
        curr_pal->color(0, temp_back);
        // Set the remaining color palette to the RGB values of the requested palette.
        // BASED/DEPENDING ON THE LINE NUMBER...
        if (internal_lines_per_page == 5)  // Five line mode, 3 colors per line = x111222333444555
        {
             for (int curr_row = 0; curr_row < 5; curr_row++)
             {
                 int start_idx = curr_row * 3 + 1;
                 int vis_or_not = ((textinfo->karaoke_type == KARAOKE_DRAW) && (curr_row == this_row_num)) ? 0 : 1;
                 curr_pal->color(start_idx + 0, (vis_or_not == 1) ? temp_outl : temp_back);
                 curr_pal->color(start_idx + 1, (vis_or_not == 1) ? temp_fore : temp_back);
                 curr_pal->color(start_idx + 2, (vis_or_not == 1) ? temp_wipe : temp_back);
             };
        }
        else   // Seven line mode, 2 colors per line = xx11223344556677
        {
             for (int curr_row = 0; curr_row < 7; curr_row++)
             {
                 int start_idx = curr_row * 2 + 2;
                 int vis_or_not = ((textinfo->karaoke_type == KARAOKE_DRAW) && (curr_row == this_row_num)) ? 0 : 1;
                 curr_pal->color(start_idx + 0, (vis_or_not == 1) ? temp_fore : temp_back);
                 curr_pal->color(start_idx + 1, (vis_or_not == 1) ? temp_wipe : temp_back);
             };
        };
        // Since there is a BMP object and palette, now use it for the event palette.
        if (textinfo->karaoke_type == KARAOKE_WIPE)
        {
            curr_event->PALObject = NULL;
            curr_pal->dissolve(0);
            curr_img->draw_delay(0);
        }
        else
        {
            // Assign the BMP object palette to this event for update,
            curr_event->PALObject = curr_pal;
            // Set the update colors mask.
            curr_pal->update_mask( palette_row_mask );
            // Determine if a palette fade is needed, then set the dissolve and delay times.
            // Dissolve interval should generally be 5 or 6 (for field based fades at 60 or 50 fps) or 10 or 12 (for frame based fades at 30 or 25 Fps).
            // The "steps" then determine the actual fade time, 8 * 10 = 80 packs / 300 per second = 266ms ... This seems nice for testing purposes.
            // The draw should then be delayed roughly (fade packs + 8 or 12) to ensure no artifacting.
            // And, ideally, this should all (eventually) be user configurable.
            const int dissolve_steps    = 8;
            const int dissolve_interval = ((internal_karaoke_mode == KAR_MODE__5BLNFD) || (internal_karaoke_mode == KAR_MODE__7MLNFD)) ? 10 : 0;
            // Draw needs a delay to prevent visible over drawing, which must be longer when fading is used.
            const int draw_delay_time = (textinfo->karaoke_type == KARAOKE_DRAW) ? 12 + (dissolve_steps * dissolve_interval) : 0;
            // Apply the calculated values to the current BMP Object.
            curr_pal->dissolve(dissolve_interval, dissolve_steps);
            curr_img->draw_delay(draw_delay_time);
        };
    };
    // Assign the used palette to the clip... For the moment, this is non-optional.
    event_queue()->front()->PALObject = event_queue()->front()->BMPObject->PALObject();
    event_queue()->front()->PALObject->dissolve(0);
    event_queue()->front()->PALObject->update_mask( event_queue()->front()->PALObject->update_mask() | 0x0001 );
}

unsigned char CDGMagic_TextClip::karaoke_mode()  {  return internal_karaoke_mode;  }
void CDGMagic_TextClip::karaoke_mode(unsigned char requested_mode)
{
    // This is ugly... But it does work for now.

    // Set the karaoke mode to the requested mode.
    internal_karaoke_mode = requested_mode;
    // Map the karaoke mode to the required lines per page.
    switch (internal_karaoke_mode)
    {
         case KAR_MODE__5TLINE:
         case KAR_MODE__5BLINE:
         case KAR_MODE__5TPAGE:
         case KAR_MODE__5BPAGE:
         case KAR_MODE__5BLNCT:
         case KAR_MODE__5BLNFD: internal_lines_per_page = 5; break;
         case KAR_MODE__8MLINE:
         case KAR_MODE__8MPAGE: internal_lines_per_page = 8; break;
         case KAR_MODE__6MLINE:
         case KAR_MODE__6MPAGE: internal_lines_per_page = 6; break;
         case KAR_MODE__84TLIN:
         case KAR_MODE__84BLIN: internal_lines_per_page = 4; break;
         case KAR_MODE__7MLNCT:
         case KAR_MODE__7MLNFD: internal_lines_per_page = 7; break;
                       default: internal_lines_per_page = 1; break;
    };
    // Map the karaoke mode to the required page mode.
    switch (internal_karaoke_mode)
    {
         case KAR_MODE__5TPAGE:
         case KAR_MODE__5BPAGE:
         case KAR_MODE__8MPAGE:
         case KAR_MODE__6MPAGE: internal_page_mode = 1; break;
                       default: internal_page_mode = 0; break;
    };
}

unsigned char CDGMagic_TextClip::karaoke_page_mode()  {  return internal_page_mode;  }
unsigned char CDGMagic_TextClip::lines_per_page()  {  return internal_lines_per_page;  }

unsigned char CDGMagic_TextClip::highlight_mode()  {  return internal_highlight_mode;  }
void CDGMagic_TextClip::highlight_mode(unsigned char requested_mode)  {  internal_highlight_mode = requested_mode;  }

int CDGMagic_TextClip::font_size()  {  return internal_font_size;  }
void CDGMagic_TextClip::font_size(int requested_size)  {  internal_font_size = requested_size;  }

int CDGMagic_TextClip::font_index()  {  return internal_font_index;  }
void CDGMagic_TextClip::font_index(int requested_index)  {  internal_font_index = requested_index;  }

const char* CDGMagic_TextClip::font_face()  {  return Fl::get_font(internal_font_index);  }
void CDGMagic_TextClip::font_face(const char* requested_face)
{
    int total_fonts = Fl::set_fonts();
    for (int curr_font = 0; curr_font < total_fonts; curr_font++)
    {
        if ( strcmp(requested_face, Fl::get_font(curr_font)) == 0 )
        {
            internal_font_index = curr_font;
            return;
        };
    };
    internal_font_index = 0;
    // Let the user know if we couldn't find a matching font face.
    fl_alert("Requested font name could not be found on this system.\n"\
             "Using default instead, please check the rendered output for suitability.\n\n"\
             "Requested: %s\n"\
             "Using: %s",
             requested_face, Fl::get_font(0) );
}

unsigned char CDGMagic_TextClip::square_size()  {  return internal_square_size;  }
void          CDGMagic_TextClip::square_size(unsigned char requested_size)  {  internal_square_size = requested_size;  }

unsigned char CDGMagic_TextClip::round_size()  {  return internal_round_size;  }
void          CDGMagic_TextClip::round_size(unsigned char requested_size)  {  internal_round_size = requested_size;  }

unsigned char CDGMagic_TextClip::foreground_color()  {  return internal_foreground_index;  }
void CDGMagic_TextClip::foreground_color(unsigned char requested_index)  {  internal_foreground_index = requested_index;  }

unsigned char CDGMagic_TextClip::outline_color()  {  return internal_outline_index;  }
void          CDGMagic_TextClip::outline_color(unsigned char requested_index)  {  internal_outline_index = requested_index;  }

unsigned char CDGMagic_TextClip::box_color()  {  return internal_box_index;  }
void          CDGMagic_TextClip::box_color(unsigned char requested_index)  {  internal_box_index = requested_index;  }

unsigned char CDGMagic_TextClip::frame_color()  {  return internal_frame_index;  }
void          CDGMagic_TextClip::frame_color(unsigned char requested_index)  {  internal_frame_index = requested_index;  }

unsigned char CDGMagic_TextClip::background_color()  {  return internal_background_index;  }
void CDGMagic_TextClip::background_color(unsigned char requested_index)  {  internal_background_index = requested_index;  }

int CDGMagic_TextClip::should_composite()  {  return internal_should_composite;  }
void CDGMagic_TextClip::should_composite(int requested_setting)  {  internal_should_composite = requested_setting;  }

int CDGMagic_TextClip::xor_bandwidth() {  return internal_xor_bandwidth;  }
void CDGMagic_TextClip::xor_bandwidth(int requested_bandwidth)
{
    internal_xor_bandwidth = requested_bandwidth;\
    printf("xor_bandwidth set to mode %i\n", internal_xor_bandwidth);
}

int CDGMagic_TextClip::antialias_mode() {  return internal_antialias_mode;  }
void CDGMagic_TextClip::antialias_mode(int requested_mode)  {  internal_antialias_mode = requested_mode;  }

int CDGMagic_TextClip::default_palette_number() {  return internal_palette_number;  }
void CDGMagic_TextClip::default_palette_number(int requested_palette)  {  internal_palette_number = requested_palette; set_all_palettes();  }

unsigned char CDGMagic_TextClip::composite_color()  {  return internal_composite_index;  }
void CDGMagic_TextClip::composite_color(unsigned char requested_index)  {  internal_composite_index = requested_index;  }

unsigned char CDGMagic_TextClip::fill_color()  {  return internal_fill_index;  }
void CDGMagic_TextClip::fill_color(unsigned char requested_index)  {  internal_fill_index = requested_index;  }

Fl_Text_Buffer* CDGMagic_TextClip::text_buffer()  {  return internal_text_buffer;  }

void CDGMagic_TextClip::draw()
{
    int max_events_time = (event_queue()->back()->start_offset + event_queue()->back()->duration + 300) / 4;
    int set_duration_time = (duration()/4);
    int proper_duration_width = (max_events_time > set_duration_time) ? max_events_time : set_duration_time;
    int proper_width = (proper_duration_width >= 16) ? proper_duration_width : 16;
    if ( w() != proper_width )  {  size( proper_width, h() );  };
    // Set the box color depending on whether we're in focus or not.
    // [TODO: How could this be altered to allow multiple selections?]
    Fl_Color current_color = (Fl::focus() == this) ? selection_color() : color();
    // Draw the box with the current selection color.
    draw_box(box(), current_color);
    fl_font(FL_HELVETICA,  10);
    for (unsigned int curr_evnt = 0; curr_evnt < event_queue()->size(); curr_evnt++)
    {
        CDGMagic_MediaEvent* curr_event = event_queue()->at(curr_evnt);
        int    half_win_w = parent()->w() / 2;
        int     win_x_min = parent()->x() - half_win_w;
        int     win_x_max = parent()->x() + parent()->w() + half_win_w;
        int curr_x_offset = curr_event->start_offset / 4 + x();
        int   event_x_max = curr_x_offset + curr_event->duration / 4;

        // Only draw events that are on the visible screen, and not beyond the ends of the clip.
        if (  (((curr_x_offset >= win_x_min) && (curr_x_offset <= win_x_max)) || ((event_x_max >= win_x_min) && (event_x_max <= win_x_max)) )
           && ((curr_x_offset >= x()) && (curr_x_offset < (x() + w())))  )
        {
            CDGMagic_TextEvent_Info* current_textinfo = static_cast<CDGMagic_TextEvent_Info*>(curr_event->user_obj);
            if (current_textinfo->karaoke_type == TEXT_ONLY)
            {
                fl_color(FL_RED);
                fl_line(curr_x_offset, y(), curr_x_offset, y()+h());
                fl_draw(current_textinfo->text_string.c_str(), curr_x_offset+2, y()+fl_height()+10);
            }
            else if (current_textinfo->karaoke_type == KARAOKE_DRAW)
            {
                fl_color(FL_BLACK);
                fl_line(curr_x_offset, y(), curr_x_offset, y()+h()/2);
                fl_draw(current_textinfo->text_string.c_str(), curr_x_offset+2, y()+h()/2-fl_descent());
            }
            else if (current_textinfo->karaoke_type == KARAOKE_WIPE)
            {
                int current_duration = event_queue()->at(curr_evnt)->duration / 4;
                fl_rectf(curr_x_offset, y()+h()/2+4, current_duration, 5, FL_DARK_GREEN);
                fl_color(FL_DARK_GREEN);
                fl_line(curr_x_offset, y()+h()/2, curr_x_offset, y()+h());
                fl_draw(current_textinfo->text_string.c_str(), curr_x_offset+2, y()+h()-fl_descent()-2);
            }
            else if (current_textinfo->karaoke_type == KARAOKE_ERASE)
            {
                fl_color(FL_RED);
                fl_line(curr_x_offset, y(), curr_x_offset, y()+h()/2);
                //char line_num_txt[8]; sprintf(line_num_txt, "%i", current_textinfo->line_num % internal_lines_per_page);
                //fl_draw(line_num_txt, curr_x_offset-2-fl_width(line_num_txt), y()+fl_height()-fl_descent());
                fl_draw(current_textinfo->text_string.c_str(), curr_x_offset+2, y()+fl_height()-fl_descent());
            }
        };
    };
    fl_color(FL_BLACK);
    int label_width = 0, label_height = 0;
    measure_label(label_width, label_height);
    if (label_width < w()-10) { draw_label(); };
}

int CDGMagic_TextClip::handle(int incoming_event)
{
    static int orig_x = 0, new_x = 0, offset_x = 0, orig_duration = 0;
    static unsigned int last_button = 0, last_event = 0;
    //printf("CDGMagic_TextClip::handle( %i ), Key = %i, event: %i\n", incoming_event, Fl::event_key(), last_event);
    switch ( incoming_event )
    {
        case FL_MOVE:
        {
            for (unsigned int curr_evnt = 1; curr_evnt < event_queue()->size(); curr_evnt++)
            {
                int curr_x_offset = event_queue()->at(curr_evnt)->start_offset / 4 + x();
                int kar_mode_sel  = 0;
                if (internal_karaoke_mode >= KAR_MODE__5TLINE)
                {
                    CDGMagic_TextEvent_Info *tmp_info = static_cast<CDGMagic_TextEvent_Info*>(event_queue()->at(curr_evnt)->user_obj);
                    int curr_y_split  = y() + h() / 2;
                    if ( (Fl::event_y() < curr_y_split) && (tmp_info->karaoke_type != KARAOKE_WIPE) )  { kar_mode_sel = 1; };
                    if ( (Fl::event_y() > curr_y_split) && (tmp_info->karaoke_type == KARAOKE_WIPE) )  { kar_mode_sel = 1; };
                }
                else  { kar_mode_sel = 1; };
                int distance = Fl::event_x()-curr_x_offset;
                if ( (kar_mode_sel == 1) && (distance >= -5) && (distance <= 5) )
                {
                    last_event = curr_evnt;
                    window()->cursor(FL_CURSOR_HAND);
                    return 1;
                }
                else
                {
                    last_event = 0;
                    window()->cursor(FL_CURSOR_DEFAULT);
                };
            };
            break;
        };
        case FL_DRAG:
        {
            // Do nothing (pass off, but don't handle, event) if the left mouse button was not pressed.
            if ( last_button != FL_LEFT_MOUSE )  { break; };
            if (last_event <= 0)  { break; };
            if (last_event >= event_queue()->size())  { break; };
            // Calculate the new X and Y position from the initial click position vs. the current "event" position.
            new_x = offset_x + Fl::event_x();
            // Calculate what the new start time will be in subcode packs.
            int new_start = ( new_x - x() ) * 4;
            // The minimum start time is equal to the preceeding event's start time + four subcode packs (which is one frame).
            int min_start = event_queue()->at(last_event-1)->start_offset + 4;
            // The maximum start time is equal to one frame prior to the proceeding event's start time,
            // or one frame prior to the clip's total length if it's the last event.
            int max_start = (last_event <= (event_queue()->size()-2)) ? event_queue()->at(last_event+1)->start_offset - 4 : w() * 4 - 4;
            // Hacky hack to disable the min/max allowable times ONLY if the clip is karaoke.
            // TODO: Uh, just about anything but this :-).
            CDGMagic_TextEvent_Info* caller_textinfo = static_cast<CDGMagic_TextEvent_Info*>(event_queue()->at(last_event)->user_obj);
            if ( caller_textinfo->karaoke_type != TEXT_ONLY )  { min_start = 0; max_start = 2000000; };
            // If the control key is pressed and the requested start isn't earlier than allowed,
            // then we move this event and all proceeding events by the same amount.
            if ( (Fl::event_ctrl() != 0) && (new_start >= min_start) )
            {
                // Calculate the amount to nudge all the events.
                int add_offset = new_start - event_queue()->at(last_event)->start_offset;
                // Start at this event, and proceed to the end of the clip adding the nudge amount.
                for (unsigned int evnt = last_event; evnt < event_queue()->size(); evnt++)
                {
                    // Get the text info from the current event.
                    CDGMagic_TextEvent_Info* evnt_textinfo = static_cast<CDGMagic_TextEvent_Info*>(event_queue()->at(evnt)->user_obj);
                    // Only alter the times if the event is of the same type.
                    if (  ((Fl::event_shift() == 0) && (evnt_textinfo->karaoke_type == caller_textinfo->karaoke_type))
                    // If SHIFT is also pressed, then also restrict it to those on the same page.
                       || (   (Fl::event_shift() != 0)
                           && ((evnt_textinfo->line_num / internal_lines_per_page) == (caller_textinfo->line_num / internal_lines_per_page))
                           && (evnt_textinfo->karaoke_type == caller_textinfo->karaoke_type) )  )
                    {
                        event_queue()->at(evnt)->start_offset += add_offset;
                    }
                };
                // In this case, we actually extend the clip's length if the last event is less than one second prior to the current clip's duration.
                if (event_queue()->back()->start_offset > (w()*4-300))  { duration( event_queue()->back()->start_offset+300 ); };
            }
            else if (new_start < min_start)  { event_queue()->at(last_event)->start_offset = min_start; }
            else if (new_start > max_start)  { event_queue()->at(last_event)->start_offset = max_start; }
            else
            {
                // We're adjusting only this event's start time, so check if the shift key is pressed.
                // AND if the event is a karaoke type wipe... If so, then we alter the duration instead of the start time.
                if ( (Fl::event_shift() != 0) && (caller_textinfo->karaoke_type == KARAOKE_WIPE) )
                {
                    int dur_adjust = new_start - event_queue()->at(last_event)->start_offset;
                    int new_duration = orig_duration + dur_adjust;
                    new_duration = (new_duration >= 0) ? new_duration : 0;
                    event_queue()->at(last_event)->duration = new_duration;
                }
                // After all those conditional checks, it's just a normal move, so set the event's start time.
                else
                {
                    // Set the newly calculated start offset
                    int original_start = event_queue()->at(last_event)->start_offset;
                    event_queue()->at(last_event)->start_offset = new_start;
                    // Determine if this was a karaoke wipe event.
                    if ( caller_textinfo->karaoke_type == KARAOKE_WIPE )
                    {
                        // Find the PREVIOUS wipe event's start time and set the duration of the current wipe to next clip start - new start.
                        for (int evnt_inc = last_event-1; evnt_inc > 0; evnt_inc--)
                        {
                            caller_textinfo = static_cast<CDGMagic_TextEvent_Info*>(event_queue()->at(evnt_inc)->user_obj);
                            if (caller_textinfo->karaoke_type == KARAOKE_WIPE)
                            {
                                int    old_wipe_begin = original_start;
                                int    new_wipe_begin =      new_start;
                                int previous_wipe_end = event_queue()->at(evnt_inc)->start_offset + event_queue()->at(evnt_inc)->duration;
                                int old_wipe_delta = old_wipe_begin-previous_wipe_end;
                                int new_wipe_delta = new_wipe_begin-previous_wipe_end;
                                if ( (old_wipe_delta <= 4) || (new_wipe_delta <= 0) )
                                {
                                    int new_duration = new_start - event_queue()->at(evnt_inc)->start_offset;
                                    new_duration = (new_duration >= 0) ? new_duration : 0;
                                    event_queue()->at(evnt_inc)->duration = new_duration;
                                };
                                break; // Break out of the loop.
                            };
                        };
                        // Find the NEXT wipe event's start time and set the duration of the current wipe to next clip start - new start.
                        for (unsigned int evnt_inc = last_event+1; evnt_inc < event_queue()->size(); evnt_inc++)
                        {
                            caller_textinfo = static_cast<CDGMagic_TextEvent_Info*>(event_queue()->at(evnt_inc)->user_obj);
                            if (caller_textinfo->karaoke_type == KARAOKE_WIPE)
                            {
                                int    old_wipe_end = original_start + event_queue()->at(last_event)->duration;
                                int    new_wipe_end =      new_start + event_queue()->at(last_event)->duration;
                                int next_wipe_begin = event_queue()->at(evnt_inc)->start_offset;
                                int old_wipe_delta = next_wipe_begin-old_wipe_end;
                                int new_wipe_delta = next_wipe_begin-new_wipe_end;
                                if ( (old_wipe_delta <= 4) || (new_wipe_delta <= 4) )
                                {
                                    int new_duration = event_queue()->at(evnt_inc)->start_offset - new_start;
                                    new_duration = (new_duration >= 0) ? new_duration : 0;
                                    event_queue()->at(last_event)->duration = new_duration;
                                };
                                break; // Break out of the loop.
                            };
                        };
                    };
                };
            };
            redraw();
            // Return true because we handled the event.
            return 1;
        };
        case FL_PUSH:
        {
            // Bail out if we didn't get left click.
            if (Fl::event_button() != FL_LEFT_MOUSE)  { break; };
            last_button = Fl::event_button();
            if (last_event <= 0)  { break; };
            if (last_event >= event_queue()->size())  { break; };
            // Store the box origin points at the time of click.
            orig_x = event_queue()->at(last_event)->start_offset / 4 + x();
            orig_duration = event_queue()->at(last_event)->duration;
            // Calculate the offset for dragging.
            offset_x = orig_x - Fl::event_x() ;
            // Return non-zero because we used the event.
            return 1;
        };
        case FL_RELEASE:
        {
            if (Fl::event_button() != FL_LEFT_MOUSE) { break; };
            if (Fl::event_clicks() == 1)
            {
                do_edit(Fl::event_x_root(), Fl::event_y_root());
                return 1;
            };
            break;
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
            break;
        };
        case FL_LEAVE:
        {
            // Make sure the cursor gets set back to the default when the pointer leaves this clip.
            window()->cursor(FL_CURSOR_DEFAULT);
            return 1;
        };
    };

    return CDGMagic_MovableClipBox::handle(incoming_event);
}

void CDGMagic_TextClip::do_edit(int win_x, int win_y)
{
    // Bah, there's got to be a better way to resolve the circular header crap than void* / static_cast !!
    CDGMagic_TextClip_Window* temp_edit_win = static_cast<CDGMagic_TextClip_Window*>(internal_edit_window);
    // First, check if we need to make an edit window for *this* clip.
    if ( temp_edit_win == NULL )
    {
        temp_edit_win = new CDGMagic_TextClip_Window(this, win_x, win_y);
        internal_edit_window = temp_edit_win;
        temp_edit_win->show();
    }
    // Otherwise, we just bring the existing edit window for *this* clip to the front.
    else
    {
        temp_edit_win->position(win_x, win_y);
        temp_edit_win->show();
    };
}

int CDGMagic_TextClip::serialize(char_vector* incoming_save_vector)
{
    add_text(incoming_save_vector, "CDGMagic_TextClip::"  );
    add_char(incoming_save_vector, track_options()->track()   );
    add_int(incoming_save_vector, start_pack()          );
    add_int(incoming_save_vector, duration()            );
    add_text(incoming_save_vector, font_face() );
    add_int(incoming_save_vector, font_size() );
    // Remapping so new 001a karaoke modes remain compatible with 001 files.
    // This will be removed and rewritten when a new (perhaps TinyXML?) project format is implemented.
    char temp_kar_mode = (karaoke_mode() == 0) ? 100 : karaoke_mode() - 1;
    add_char(incoming_save_vector, temp_kar_mode );
    add_char(incoming_save_vector, highlight_mode() );
    add_char(incoming_save_vector, foreground_color() );
    add_char(incoming_save_vector, background_color() );
    add_char(incoming_save_vector, outline_color()    );
    add_char(incoming_save_vector, square_size()      );
    add_char(incoming_save_vector, round_size()       );
    add_char(incoming_save_vector, frame_color()      );
    add_char(incoming_save_vector, box_color()        );
    add_char(incoming_save_vector, fill_color()       );
    add_char(incoming_save_vector, composite_color()  );
    add_int(incoming_save_vector, should_composite()  );
    add_int(incoming_save_vector, xor_bandwidth() );
    add_int(incoming_save_vector, antialias_mode() );
    add_int(incoming_save_vector, default_palette_number() );
    add_int(incoming_save_vector, internal_text_buffer->length() );
    add_text(incoming_save_vector, internal_text_buffer->text()   );
    add_int(incoming_save_vector, static_cast<int>( event_queue()->size() ) );
    for (unsigned int evnt = 0; evnt < event_queue()->size(); evnt++ )
    {
        add_int(incoming_save_vector, event_queue()->at(evnt)->start_offset );
        add_int(incoming_save_vector, event_queue()->at(evnt)->duration );
        // Conditionals to prevent attempting to serialize null objects.
        if ( event_queue()->at(evnt)->BMPObject )
        {
            CDGMagic_BMPObject *tmp_bmp = event_queue()->at(evnt)->BMPObject;
            add_int(incoming_save_vector, tmp_bmp->width()             );
            add_int(incoming_save_vector, tmp_bmp->height()            );
            add_int(incoming_save_vector, tmp_bmp->x_offset()          );
            add_int(incoming_save_vector, tmp_bmp->y_offset()          );
            add_text(incoming_save_vector, (char*) tmp_bmp->transition_file()   );
            add_short(incoming_save_vector, tmp_bmp->transition_length() );
        };
        if ( event_queue()->at(evnt)->user_obj )
        {
            CDGMagic_TextEvent_Info *event_textinfo = static_cast<CDGMagic_TextEvent_Info*>(event_queue()->at(evnt)->user_obj);
            add_int(incoming_save_vector, event_textinfo->karaoke_type );
            add_int(incoming_save_vector, event_textinfo->line_num );
            add_int(incoming_save_vector, event_textinfo->word_num );
        };
    };
    return event_queue()->size();
}

int CDGMagic_TextClip::deserialize(char* incoming_save_data)
{
    printf( "Deserialize calling set_fonts...\n");
    int active_fonts = Fl::set_fonts(NULL);
    printf( "Active fonts on system: %i\n", active_fonts );

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

    printf("Setting font_face: %s, Offset: %i\n", &incoming_save_data[pos], pos);
    font_face( &incoming_save_data[pos] );
    pos += strlen( &incoming_save_data[pos] ) + 1;

    pos += get_int( &incoming_save_data[pos], tmp_int);
    printf("Setting font_size: %i, Offset: %i\n", tmp_int, pos);
    font_size( tmp_int );

    pos += get_char( &incoming_save_data[pos], tmp_char );
    printf("Setting karaoke_mode: %i, Offset: %i\n", tmp_char, pos);
    // Remapping so new 001a karaoke modes remain compatible with 001 files.
    // This will be removed and rewritten when a new (perhaps TinyXML?) project format is implemented.
    char temp_kar_mode = (tmp_char == 100) ? 0 : tmp_char + 1;
    karaoke_mode( temp_kar_mode );

    pos += get_char( &incoming_save_data[pos], tmp_char );
    printf("Setting highlight_mode: %i, Offset: %i\n", tmp_char, pos);
    highlight_mode( tmp_char );

    pos += get_char( &incoming_save_data[pos], tmp_char );
    printf("Setting foreground_color: %i, Offset: %i\n", tmp_char, pos);
    foreground_color( tmp_char );

    pos += get_char( &incoming_save_data[pos], tmp_char );
    printf("Setting background_color: %i, Offset: %i\n", tmp_char, pos);
    background_color( tmp_char );

    pos += get_char( &incoming_save_data[pos], tmp_char );
    printf("Setting outline_color: %i, Offset: %i\n", tmp_char, pos);
    outline_color( tmp_char );

    pos += get_char( &incoming_save_data[pos], tmp_char );
    printf("Setting square_size: %i, Offset: %i\n", tmp_char, pos);
    square_size( tmp_char );

    pos += get_char( &incoming_save_data[pos], tmp_char );
    printf("Setting round_size: %i, Offset: %i\n", tmp_char, pos);
    round_size( tmp_char );

    pos += get_char( &incoming_save_data[pos], tmp_char );
    printf("Setting frame_color: %i, Offset: %i\n", tmp_char, pos);
    frame_color( tmp_char );

    pos += get_char( &incoming_save_data[pos], tmp_char );
    printf("Setting box_color: %i, Offset: %i\n", tmp_char, pos);
    box_color( tmp_char );

    pos += get_char(&incoming_save_data[pos], tmp_char);// Fill Index
    printf("Setting fill_color %i\n", tmp_char);
    fill_color(tmp_char);

    pos += get_char(&incoming_save_data[pos], tmp_char);// Composite Index
    printf("Setting composite_color %i\n", tmp_char);
    composite_color(tmp_char);

    pos += get_int(&incoming_save_data[pos], tmp_int);  // Composite Type
    printf("Setting should_composite %i\n", tmp_int);
    should_composite(tmp_int);

    pos += get_int(&incoming_save_data[pos], tmp_int);  // Composite Type
    printf("Setting xor_bandwidth %i\n", tmp_int);
    xor_bandwidth(tmp_int);

    pos += get_int(&incoming_save_data[pos], tmp_int);  // Composite Type
    printf("Setting antialias_mode %i\n", tmp_int);
    antialias_mode(tmp_int);

    pos += get_int(&incoming_save_data[pos], tmp_int);  // Composite Type
    printf("Setting default_palette_number %i\n", tmp_int);
    default_palette_number(tmp_int);

    pos += get_int( &incoming_save_data[pos], tmp_int);
    printf("Text length: %i, Offset: %i\n", tmp_char, pos);

    internal_text_buffer->text( &incoming_save_data[pos] );
    pos += strlen( &incoming_save_data[pos] ) + 1;

    printf( "Deserialize calling render_text_to_bmp...\n");
    render_text_to_bmp();

    int total_events = 0;
    pos += get_int( &incoming_save_data[pos], total_events );
    for (int evnt_num = 0; evnt_num < total_events; evnt_num++)
    {
        int clip_time_offset = 0;
        int clip_time_duration = 0;
        int clip_x_offset = 0;
        int clip_y_offset = 0;

        int clip_kar_type = 0;
        int clip_line_num = 0;
        int clip_word_num = 0;

        pos += get_int( &incoming_save_data[pos], clip_time_offset);
        pos += get_int( &incoming_save_data[pos], clip_time_duration);
        pos += get_int(&incoming_save_data[pos], tmp_int);  // Width
        pos += get_int(&incoming_save_data[pos], tmp_int);  // Height
        pos += get_int(&incoming_save_data[pos], clip_x_offset);  // X Offset
        pos += get_int(&incoming_save_data[pos], clip_y_offset);  // Y Offset
        pos += strlen( &incoming_save_data[pos] ) + 1;  // Transition file.
        pos += get_short(&incoming_save_data[pos], tmp_short);  // Transition length

        pos += get_int(&incoming_save_data[pos], clip_kar_type);  // Karaoke type
        pos += get_int(&incoming_save_data[pos], clip_line_num);  // Line number
        pos += get_int(&incoming_save_data[pos], clip_word_num);  // Word number


        int event_to_set = get_line_word_type_event(clip_kar_type, clip_line_num, clip_word_num);
        if ( event_to_set >= 0 )
        {
#ifdef _CDGMAGIC_TEXTCLIP_DEBUG_
            printf("Setting clip offset: %i\n", clip_time_offset);
#endif
            event_queue()->at(event_to_set)->start_offset = clip_time_offset;
#ifdef _CDGMAGIC_TEXTCLIP_DEBUG_
            printf("Setting clip duration: %i\n", clip_time_duration);
#endif
            event_queue()->at(event_to_set)->duration = clip_time_duration;

            CDGMagic_BMPObject *tmp_bmp = event_queue()->at(event_to_set)->BMPObject;
#ifdef _CDGMAGIC_TEXTCLIP_DEBUG_
            printf("Setting x_offset %i\n", clip_x_offset); // X Offset is set by renderer.
#endif
            // If set here for karaoke, it can/will break wiping when the font is different. (Such as when the original font is unavailable.)
            if (clip_kar_type == TEXT_ONLY)  { tmp_bmp->x_offset(clip_x_offset); };
#ifdef _CDGMAGIC_TEXTCLIP_DEBUG_
            printf("Setting y_offset %i\n", clip_y_offset);
#endif
            tmp_bmp->y_offset(clip_y_offset);
        }
        else
        {
            printf("Could not find event in TextClip deserialize! (Idx:%i,T:%i,L:%i,W:%i)\n", event_to_set, clip_kar_type, clip_line_num, clip_word_num);
        };
    };

    return pos;
}
