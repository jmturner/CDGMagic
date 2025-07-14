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

#include "CDGMagic_TextClip_Window.h"

CDGMagic_TextClip_Window::CDGMagic_TextClip_Window(CDGMagic_TextClip *clip_to_edit, int X, int Y) : Fl_Double_Window(0, 0, 0)
{
    const char *textwindow_title      = "CD+G Titles/Lyrics Editing";
    const int   textwindow_width      = 680;
    const int   textwindow_height     = 500;

    label( textwindow_title );
    size( textwindow_width, textwindow_height );
    position( X-w()/2, Y );

    current_clip = clip_to_edit;
    current_page = 0;
    current_bmp_object = NULL;

    // Create an image buffer.
    preview_buffer = new unsigned char[300*216*3];

    int active_fonts = Fl::set_fonts(NULL);
    printf( "Active fonts on system: %i\n", active_fonts );

    Fl_Button *OK_button = new Fl_Button(10, h()-40, 100, 30, "OK");
    OK_button->tooltip("Close this window.\n(Note: Doesn't automatically render, etc.)");
    OK_button->callback(DoOK_callback_static, this);

    Fl_Button *FixTime_button = new Fl_Button(120, h()-40, 80, 30, "Fix Timing");
    FixTime_button->tooltip("Attempt to automatically set best draw/erase times based on the wipe timing.\n"\
                            "CTRL+Click: Also set consecutive wipe durations based on their start times.\n"\
                            "SHIFT+Click: Alternate style line-by-line draw/erase timing.\n"\
                            "(Note: Karaoke Mode ONLY / May or may not work well.)");
    FixTime_button->callback(DoFixTime_callback_static, this);

    SetStart_button = new CDGMagic_TextClip_Window__CtrlButton(210, h()-40, 80, 30, "Set Start");
    SetStart_button->tooltip("Advance to the next event, then set the start time to the current play position.\n"\
                             "CTRL (when focused): Set the end time of the current wipe in karaoke mode.\n"\
                             "Shortcut: F9");
    SetStart_button->callback(DoSetStart_callback_static, this);
    SetStart_button->shortcut(FL_F + 9);

    // TODO: Figure out if there's *any* way to destroy this window in the clip's destructor
    // without FLTK putting up errors in a messagebox... Although this *seems* to be an FLTK bug.
    text_editor = new CDGMagic_TextClip_Window__Editor( 0, 4, 300, h()-50 );
    text_editor->buffer( current_clip->text_buffer() );
    text_editor->textsize( current_clip->font_size() );
    text_editor->textfont( current_clip->font_index() );

    PreviewImage = new Fl_RGB_Image( preview_buffer, 300, 216, 3, 0 );
    // Create a new empty box and associate the image with it.
    PreviewBox = new Fl_Box(310, 4, 300+Fl::box_dw(FL_DOWN_BOX), 216+Fl::box_dh(FL_DOWN_BOX));
    PreviewBox->box(FL_DOWN_BOX);
    PreviewBox->image(PreviewImage);

    PreviewLabel = new Fl_Box(PreviewBox->x(), PreviewBox->y()+PreviewBox->h(), PreviewBox->w(), 20);
    PreviewLabel->label("Preview Image");
    PreviewLabel->box(FL_FLAT_BOX);

    EditorLabel = new Fl_Box(PreviewBox->x()+PreviewBox->w()+5, PreviewBox->y()+5, textwindow_width-(PreviewBox->x()+PreviewBox->w()+5), 80);
    EditorLabel->box(FL_FLAT_BOX);
    EditorLabel->align(FL_ALIGN_INSIDE|FL_ALIGN_LEFT|FL_ALIGN_TOP);

    text_editor->callback(ShowCursorPos_cb_static, this); // Make sure the label exists before setting the callback to modify it.
    text_editor->do_callback(); // Update the cursor position label, if needed... Only used in karaoke mode, for now.

    Fl_Group *controls_group = new Fl_Group(305, 250, w()-320, h()-260);
    controls_group->box(FL_FLAT_BOX);//FL_UP_BOX);
    controls_group->resizable(NULL);
    controls_group->begin();

        Fl_Button *preview_button = new Fl_Button(controls_group->x()+5, controls_group->y(), 100, 20, "Render Text");
        preview_button->tooltip( "Render the current text using the current settings.\n"\
                                 "(You must do this after making changes to the options below.)\n"\
                                 "Shortcut: CTRL+Enter");
        preview_button->callback( prevbut_cb_static, this );
        preview_button->shortcut(FL_CTRL | FL_Enter);

        Page_Counter = new Fl_Counter(controls_group->x()+150, controls_group->y(), 120, 20, "BMP:");
        Page_Counter->align(FL_ALIGN_LEFT);
        Page_Counter->bounds(1.0, 10000.0);
        Page_Counter->step(1.0);
        Page_Counter->lstep(10.0);
        Page_Counter->value(1.0);
        Page_Counter->tooltip( "Current line/word preview." );
        Page_Counter->callback(DoPageChange_callback_static, this);

        Fl_Button *play_button = new Fl_Button(controls_group->x()+285, controls_group->y(), 80, 20, "Play");
        play_button->tooltip( "Set word display to 1,\n"\
                              "Start audio playback from clip beginning,\n"\
                              "Focus Set Start button to begin timing.\n"\
                              "Shortcut: F12");
        play_button->shortcut(FL_F + 12);
        play_button->callback( playbut_cb_static, this );

        Fl_Choice *Mode_choice = new Fl_Choice(controls_group->x()+255, controls_group->y()+60, 100, 20, "Type:");
        Mode_choice->add("Titles\\/Page", 0, NULL);
        Mode_choice->add("Lyrics\\/Line", 0, NULL);
        Mode_choice->add("Karaoke (5\\/Top\\/Line)", 0, NULL);
        Mode_choice->add("Karaoke (5\\/Bottom\\/Line)", 0, NULL);
        Mode_choice->add("Karaoke (5\\/Top\\/Page)", 0, NULL);
        Mode_choice->add("Karaoke (5\\/Bottom\\/Page)", 0, NULL);
        Mode_choice->add("Karaoke (8\\/Line)", 0, NULL);
        Mode_choice->add("Karaoke (8\\/Page)", 0, NULL);
        Mode_choice->add("Karaoke (8\\/6-Middle\\/Line)", 0, NULL);
        Mode_choice->add("Karaoke (8\\/6-Middle\\/Page)", 0, NULL);
        Mode_choice->add("Karaoke (8\\/4-Top\\/Line)", 0, NULL);
        Mode_choice->add("Karaoke (8\\/4-Bottom\\/Line)", 0, NULL);
        Mode_choice->add("Karaoke (5\\/Bottom\\/Line-Cut)", 0, NULL);
        Mode_choice->add("Karaoke (5\\/Bottom\\/Line-Fade)", 0, NULL);
        Mode_choice->add("Karaoke (7\\/Middle\\/Line-Cut)", 0, NULL);
        Mode_choice->add("Karaoke (7\\/Middle\\/Line-Fade)", 0, NULL);
        Mode_choice->value( current_clip->karaoke_mode() );
        Mode_choice->tooltip("Titles: Multi-line, single page text. (No highlight.)\n"\
                             "Lyrics: Single line, multi-page text. (No highlight.)\n"\
                             "Karaoke: Multi-line, multi-page text and highlighting.\n"\
                             "(5 = 36 pixel high lines, 8 = 24 pixel high lines.)\n"\
                             "(Line = When line finished, erase and draw next.)\n"\
                             "(Page = When page finished, clear screen and draw next.)\n"\
                             "(Cut/Fade = Line-by-line, no visible drawing. [Very Experimental!]");
        Mode_choice->callback( modeselect_cb_static, this );

        Fl_Choice *Highlight_choice = new Fl_Choice(controls_group->x()+255, controls_group->y()+85, 100, 20, "Wipe:");
        Highlight_choice->add("None", 0, NULL);
        Highlight_choice->add("Text", 0, NULL);
        Highlight_choice->add("Outline", 0, NULL);
        Highlight_choice->add("Both", 0, NULL);
        Highlight_choice->value( current_clip->highlight_mode() );
        Highlight_choice->tooltip("Karaoke highlight/wipe area.\n"\
                                  "None: Don't XOR anything.\n"\
                                  "Text: Only the foreground text index will be XORed.\n"\
                                  "Outline: Only the outline index will be XORed.\n"\
                                  "Both: Both foreground text and outline will be XORed.");
        Highlight_choice->callback( highlightselect_cb_static, this );

        Fl_Choice *Palette_choice = new Fl_Choice(controls_group->x()+255, controls_group->y()+110, 100, 20, "Palette:");
        Palette_choice->add("None", 0, NULL);
        Palette_choice->add("Default", 0, NULL);
        Palette_choice->add("Preset 1", 0, NULL);
        Palette_choice->add("Preset 2", 0, NULL);
        Palette_choice->add("Preset 3", 0, NULL);
        Palette_choice->add("Preset 4", 0, NULL);
        Palette_choice->add("Preset 5", 0, NULL);
        Palette_choice->add("Preset 6", 0, NULL);
        Palette_choice->add("Preset 7", 0, NULL);
        Palette_choice->add("Preset 8", 0, NULL);
        Palette_choice->value( current_clip->default_palette_number()+1 );
        Palette_choice->tooltip("Predefined palette to set at the beginning of the clip.\n"\
                                "None: Don't set palette. (A palette must be set via another clip or the screen will remain black.)\n"\
                                "Preset <X>: Predefined palettes that may (or may not) work well for karaoke.");
        Palette_choice->callback( paletteselect_cb_static, this );

        Fl_Check_Button *antialias_check = new Fl_Check_Button(controls_group->x()+335, controls_group->y()+30, 20, 20, "AA:");
        antialias_check->align( FL_ALIGN_LEFT );
        antialias_check->value( current_clip->antialias_mode() );
        antialias_check->tooltip( "Enable/disable simple antialiasing.\n"\
                                  "(Uses color index foreground + 1.)" );
        antialias_check->callback( antialiasmode_cb_static, this );

        Fl_Choice *font_select = new Fl_Choice(controls_group->x()+90, controls_group->y()+30, 200, 20, "Font Face:");
        for (int current_font = 0; current_font < active_fonts; current_font++)
        {
            font_select->add( Fl::get_font(current_font) );
        };
        font_select->tooltip("Font face style/name.\n"\
                             "B: Bold\n"\
                             "I: Italic\n"\
                             "P: Bold and Italic" );
        font_select->callback( fontchange_cb_static, this );
        font_select->value( current_clip->font_index() );

        Fl_Counter *font_size = new Fl_Counter(controls_group->x()+90, controls_group->y()+60, 100, 20, "Font Size:");
        font_size->align(FL_ALIGN_LEFT);
        font_size->step(1.0, 10.0);
        font_size->bounds(1.0, 200.0);
        font_size->value( current_clip->font_size() );
        font_size->tooltip( "Size/height of font in pixels." );
        font_size->callback( fontsize_cb_static, this );

        Fl_Counter *square_size_counter = new Fl_Counter(controls_group->x()+90, controls_group->y()+85, 100, 20, "Sq Size:");
        square_size_counter->align(FL_ALIGN_LEFT);
        square_size_counter->step(1.0, 10.0);
        square_size_counter->bounds(0.0, 255.0);
        square_size_counter->value( current_clip->square_size() );
        square_size_counter->tooltip( "Size (width) of square text outline." );
        square_size_counter->callback( SqrSz_cb_static, this );

        Fl_Counter *round_size_counter = new Fl_Counter(controls_group->x()+90, controls_group->y()+110, 100, 20, "Rnd Size:");
        round_size_counter->align(FL_ALIGN_LEFT);
        round_size_counter->step(1.0, 10.0);
        round_size_counter->bounds(0.0, 255.0);
        round_size_counter->value( current_clip->round_size() );
        round_size_counter->tooltip( "Size (width) of rounded text outline." );
        round_size_counter->callback( RndSz_cb_static, this );

        fg_idx_counter = new Fl_Counter(controls_group->x()+90, controls_group->y()+140, 100, 20, "Fg Idx:");
        fg_idx_counter->align(FL_ALIGN_LEFT);
        fg_idx_counter->step(1.0, 10.0);
        fg_idx_counter->bounds(0.0, 255.0);
        fg_idx_counter->value( current_clip->foreground_color() );
        fg_idx_counter->tooltip( "Foreground (main text) color index." );
        fg_idx_counter->callback( FgIdx_cb_static, this );

        ol_idx_counter = new Fl_Counter(controls_group->x()+255, controls_group->y()+140, 100, 20, "Ol Idx:");
        ol_idx_counter->align(FL_ALIGN_LEFT);
        ol_idx_counter->step(1.0, 10.0);
        ol_idx_counter->bounds(0.0, 255.0);
        ol_idx_counter->value( current_clip->outline_color() );
        ol_idx_counter->tooltip( "Text Outline color index." );
        ol_idx_counter->callback( OlIdx_cb_static, this );

        bg_idx_counter = new Fl_Counter(controls_group->x()+90, controls_group->y()+165, 100, 20, "Bg Idx:");
        bg_idx_counter->align(FL_ALIGN_LEFT);
        bg_idx_counter->step(1.0, 10.0);
        bg_idx_counter->bounds(0.0, 255.0);
        bg_idx_counter->value( current_clip->background_color()  );
        bg_idx_counter->tooltip( "Background (behind text, outline, box, and frame) color index.\n"\
                                 "(In karaoke mode, it is recommended to set this to the Box Idx.)" );
        bg_idx_counter->callback( BgIdx_cb_static, this );

        box_idx_counter = new Fl_Counter(controls_group->x()+255, controls_group->y()+165, 100, 20, "Box Idx:");
        box_idx_counter->align(FL_ALIGN_LEFT);
        box_idx_counter->step(1.0, 10.0);
        box_idx_counter->bounds(0.0, 255.0);
        box_idx_counter->value( current_clip->box_color() );
        box_idx_counter->tooltip( "Titles Mode (0-15): Memory Preset color index.\n"\
                                  "Titles Mode (16-255): Disable Memory Preset.\n"\
                                  "Lyrics Mode: Box color index.\n"\
                                  "Karaoke Mode (0-15): Memory & Border Preset Index\n"\
                                  "Karaoke Mode (16-255): Disable Memory & Border Preset" );
        box_idx_counter->callback( BoxIdx_cb_static, this );

        fill_idx_counter = new Fl_Counter(controls_group->x()+90, controls_group->y()+190, 100, 20, "Fill Idx:");
        fill_idx_counter->align(FL_ALIGN_LEFT);
        fill_idx_counter->step(1.0, 10.0);
        fill_idx_counter->bounds(0.0, 255.0);
        fill_idx_counter->value( current_clip->fill_color() );
        fill_idx_counter->tooltip( "Fill (area outside text rendering canvas) color index." );
        fill_idx_counter->callback( FillIdx_cb_static, this );

        frm_idx_counter = new Fl_Counter(controls_group->x()+255, controls_group->y()+190, 100, 20, "Frm Idx:");
        frm_idx_counter->align(FL_ALIGN_LEFT);
        frm_idx_counter->step(1.0, 10.0);
        frm_idx_counter->bounds(0.0, 255.0);
        frm_idx_counter->value( current_clip->frame_color() );
        frm_idx_counter->tooltip( "Titles Mode (0-15): Border Preset color index.\n"\
                                  "Titles Mode (16-255): Disable Border Preset.\n"\
                                  "Lyrics Mode: Frame color index.\n"\
                                  "Karaoke Mode: Wipe/Highlighting XOR Index.\n"\
                                  "(Example: Indices 0 to 3 XOR 4 = Indices 4 to 7)" );
        frm_idx_counter->callback( FrmIdx_cb_static, this );

        Fl_Counter *comp_idx_counter = new Fl_Counter(controls_group->x()+90, controls_group->y()+215, 100, 20, "Comp Idx:");
        comp_idx_counter->align(FL_ALIGN_LEFT);
        comp_idx_counter->step(1.0, 10.0);
        comp_idx_counter->bounds(0.0, 255.0);
        comp_idx_counter->value( current_clip->composite_color() );
        comp_idx_counter->tooltip( "Composite (transparent) color index." );
        comp_idx_counter->callback( CompIdx_cb_static, this );

        Fl_Check_Button *should_comp_check = new Fl_Check_Button(controls_group->x()+200, controls_group->y()+215, 20, 20, "Comp?");
        should_comp_check->align(FL_ALIGN_RIGHT);
        should_comp_check->value( current_clip->should_composite() );
        should_comp_check->tooltip( "Enable/disable compositing of composite color index.\n"\
                                    "(When deselected the entire clip is opaque.)" );
        should_comp_check->callback( ShouldComp_cb_static, this );

        Fl_Choice *Bandwidth_choice = new Fl_Choice(controls_group->x()+305, controls_group->y()+215, 50, 20, "Bw:");
        Bandwidth_choice->add("1\\/4", 0, NULL);
        Bandwidth_choice->add("1\\/3", 0, NULL);
        Bandwidth_choice->add("1\\/2", 0, NULL);
        Bandwidth_choice->add("2\\/3", 0, NULL);
        Bandwidth_choice->add("3\\/4", 0, NULL);
        Bandwidth_choice->value( current_clip->xor_bandwidth() );
        Bandwidth_choice->tooltip("Karaoke highlight/wipe bandwidth.\n"\
                                  "Maximum amount of bandwidth used for karaoke highlighting.\n"\
                                  "Slower songs can use more bandwidth for smoother highlight.");
        Bandwidth_choice->callback( XorBw_cb_static, this );

    controls_group->end();
    update_current_values();
    update_counter_colors();
}

CDGMagic_TextClip_Window::~CDGMagic_TextClip_Window()
{
    current_clip->null_edit();
    // Delete the preview buffer, allocated in the constructor.
    delete[] preview_buffer; preview_buffer = NULL;
}

void CDGMagic_TextClip_Window::DoOK_callback(Fl_Widget *CallingWidget)
{
    printf("CDGMagic_TextClip_Window::DoOK_callback\n");
    current_clip->font_size( text_editor->textsize() );
    Fl::delete_widget(this);
}

int CDGMagic_TextClip_Window::can_delete()
{
    show();
    return fl_choice("This clip has a window open for editing.\nDelete anyway?", "No (don't delete)", "Yes (delete this clip)", NULL);
}

void CDGMagic_TextClip_Window::DoSetStart_callback(Fl_Widget *CallingWidget)
{
    // Subtract the clip start time from the current play time to the event offset.
    int new_event_offset = current_clip->play_time() - current_clip->start_pack();

    if ( current_clip->karaoke_mode() <= KAR_MODE__LYRICS )
    {
        // Advance the page count by one.
        Page_Counter->value( Page_Counter->value() + 1.0 );
        // Update the preview image and controls.
        DoPageChange_callback( Page_Counter );
        // Set the event time for current event.
        current_clip->event_queue()->at(current_page)->start_offset = new_event_offset;
        // Print out stupid debug stuff.
        printf("Event: %i, Time: %i, Start: %i, Offset: %i\n", current_page, current_clip->play_time(), current_clip->start_pack(), new_event_offset);
    }
    else
    {
        static int should_set_dur = 1;
        // If the control key was pressed, we only set the duration of the current wipe.
        if (  (Fl::event_key() == FL_Control_R) || (Fl::event_key() == FL_Control_L)  )
        {
            current_clip->event_queue()->at(current_page)->duration = new_event_offset - current_clip->event_queue()->at(current_page)->start_offset;
            should_set_dur = 0; // Disable setting of this event's duration by the next start event.
            printf("Clip %i, Set duration: %i\n", current_page, current_clip->event_queue()->at(current_page)->duration);
            return;
        };
        // Find the next wipe event in the queue.
           do { current_page++; }
        while (   ( current_page < current_clip->event_queue()->size() )
               && ( static_cast<CDGMagic_TextEvent_Info*>(current_clip->event_queue()->at(current_page)->user_obj)->karaoke_type != KARAOKE_WIPE ) );
        // Reflect the current page number.
        Page_Counter->value( current_page+1 );
        // Update the preview image and controls.
        DoPageChange_callback( Page_Counter );

        // Now we can set the start of this event, and duration of previous.
        if ( current_page > 0 ) // Don't set any offset for the very first event.
        {
            // Set the time for current event.
            current_clip->event_queue()->at(current_page)->start_offset = new_event_offset;
            // Print out stupid debug stuff.
            printf("Set Start:: KAR-Event: %i, Pt: %i, St: %i, Os: %i, Dr: %i\n",
                    current_page, current_clip->play_time(), current_clip->start_pack(),
                    current_clip->event_queue()->at(current_page)->start_offset,
                    current_clip->event_queue()->at(current_page)->duration );

            // Find and set the duration of the previous event.
            if (should_set_dur)
            {
                unsigned int tmp_page = (current_page > 1) ? current_page-1 : 0;
                while (   ( tmp_page > 1 )
                       && ( static_cast<CDGMagic_TextEvent_Info*>(current_clip->event_queue()->at(tmp_page)->user_obj)->karaoke_type != KARAOKE_WIPE ) )
                {
                    tmp_page--;
                };
                if ( static_cast<CDGMagic_TextEvent_Info*>(current_clip->event_queue()->at(tmp_page)->user_obj)->karaoke_type == KARAOKE_WIPE )
                {
                    int previous_start = current_clip->event_queue()->at(tmp_page)->start_offset;
                    int previous_duration  = new_event_offset - previous_start;
                    current_clip->event_queue()->at(tmp_page)->duration = previous_duration;
                    printf("Set   Dur:: KAR-Event: %i, Pt: %i, St: %i, Os: %i, Dr: %i\n",
                            tmp_page, current_clip->play_time(), current_clip->start_pack(),
                            current_clip->event_queue()->at(tmp_page)->start_offset,
                            current_clip->event_queue()->at(tmp_page)->duration );
                };
            };
            should_set_dur = 1; // Hacky hack... Got a new start event, so start autosetting durations again.
        };
        printf("##########\n");
    };
}

void CDGMagic_TextClip_Window::DoFixTime_callback(Fl_Widget *CallingWidget)
{
    // Bail if we're not karaoke, as this won't work otherwise.
    if (current_clip->karaoke_mode() <= KAR_MODE__LYRICS)  { return; };
    printf("Inside fix times callback\n");
    // Convenience variable to save some typing later.
    MediaEvent_Queue *clip_events = current_clip->event_queue();
    // Do a custom sort to ensure all events are arranged by line, word order.
    std::sort(clip_events->begin(), clip_events->end(), kar_wipe_sorter);

    const int lines_per_page = current_clip->lines_per_page();
    const int line_pal_mode  = (current_clip->karaoke_mode() >= KAR_MODE__5BLNCT) ? 1 : 0;

    const int fade_interval  = 30; // Time between fading lines.
    // Set the start time for all the lines on the first page to just after the clip beginning.
    int initial_draw_time = 0;
    for (int curr_line = 0; curr_line < lines_per_page; curr_line++ )
    {
        // Set the draw time for the line.
        int time_add = set_draw_time(KARAOKE_DRAW, curr_line, initial_draw_time);
        // If in palette cut/fade mode, the erase is actually the show, so set to 3000ms after the draw.
        // This isn't synchronized in any way, for now, so that's just a reasonable average value...
        if (line_pal_mode)  { set_draw_time(KARAOKE_ERASE, curr_line, initial_draw_time + 900); };
        // Increment the draw time, depending on if the line existed.
        initial_draw_time += (line_pal_mode == 1) ? (time_add * fade_interval) : (time_add * 50);
    };
    // Make sure all wipe events have consecutive start times.
    // Also update their durations if the CTRL key was pressed while clicking the button.
    int should_set_duration = (Fl::event_state() & FL_CTRL) ? 1 : 0;
    int line_clear_type = ( (Fl::event_state() & FL_SHIFT) || line_pal_mode ) ? 1 : 0;
    int page_clear_mode = current_clip->karaoke_page_mode();
    CDGMagic_MediaEvent *last_wipe_event = NULL;
    CDGMagic_TextEvent_Info *last_wipe_info = NULL;
    int lastline_start_time = 0;
    for (unsigned int event_num = 0; event_num < clip_events->size(); event_num++ )
    {
        CDGMagic_MediaEvent     *the_event  = clip_events->at(event_num);
        CDGMagic_TextEvent_Info *event_info = static_cast<CDGMagic_TextEvent_Info*>(the_event->user_obj);
        if (event_info->karaoke_type == KARAOKE_WIPE)
        {
            if (last_wipe_event != NULL)
            {
                // Ensure that all wipes have consecutive start times...
                // NOTE: This really screws up "added" text... Determine a reasonable way around that.
                if (the_event->start_offset <= last_wipe_event->start_offset)
                {
                    the_event->start_offset = last_wipe_event->start_offset + 32;
                };
                // Should we actually update the durations, or leave them alone?
                if (should_set_duration)
                {
                    last_wipe_event->duration = the_event->start_offset - last_wipe_event->start_offset;
                };
                // We have a new line and we're not in page mode, so we can get rid of the old one.
                if (last_wipe_info->line_num < event_info->line_num)
                {
                    // Calculate current and last page.
                    int last_page = last_wipe_info->line_num / lines_per_page;
                    int this_page = event_info->line_num     / lines_per_page;
                    // Check which page mode is selected.
                    if (page_clear_mode == 0)
                    {
                        // All the duplicated code in this conditional can be refactored once it's determined which paths work correctly.
                        // The first path is the original - working - line-by-line code.
                        // The second is a newer line-by-line mode that only draws the next page when the last line of the current page is being highlighted...
                        // A sort of hybrid line/page mode.
                        // The third is similar newer style code modified for testing of the very experimental "line palette" fading mode.
                        if (line_clear_type == 0) // Original line-by-line mode.
                        {
                            // ##### ORIGINAL VERSION 001a #####
                            // Set the initial erase time to the start of this wipe.
                            int erase_time      = the_event->start_offset;
                            // Only do additional timing adjustments if the event duration is set, AND we're at the end of the page.
                            if ( (last_wipe_event->duration > 0) && (last_page < this_page) )
                            {
                                // Calculate the end of the last wipe.
                                int last_wipe_end   = last_wipe_event->start_offset + last_wipe_event->duration;
                                // Calculate the time between the end of last wipe and the beginning of this one.
                                int inter_wipe_time = the_event->start_offset - last_wipe_end;
                                // Split the difference for the erase time.
                                erase_time      = last_wipe_end + (inter_wipe_time / 2);
                                // Clamp the this line erase time to at most 2000ms after the last word wipe.
                                erase_time = ( erase_time > (last_wipe_end+600) ) ? (last_wipe_end+600) : erase_time;
                            }
                            else
                            {
                                erase_time += 100; // Hacky hack, delay erase time by 1/3 seconds.
                            };
                            // Somehat bizarre loop accounts for erase/draw events without a corresponding wipe event.
                            // The first iteration erases the line with the last wipe event,
                            // then any subsequent iterations erase/draw those lines which contain no wipe events.
                            // continuing until one line prior to the current line.
                            // This (in fact, this whole function) will definitely need changed
                            // when a more comprehensive karaoke editor is implemented.
                            for (int this_line = last_wipe_info->line_num; this_line < event_info->line_num; this_line++)
                            {
                                erase_time += (set_draw_time(KARAOKE_ERASE, this_line,                  erase_time) * 50);
                                erase_time += (set_draw_time(KARAOKE_DRAW , this_line + lines_per_page, erase_time) * 50);
                            };
                        }
                        else if (line_pal_mode == 0) // New line-by-line mode, accessed by SHIFT+Click on the Fix Timing button, BUT NOT LINE PALETTE MODE.
                        {
                            // ##### NEW VERSION 001b #####
                            // Determine if this line-to-line event transition occurs on the same page, or across pages.
                            if (last_page == this_page)
                            {
                                // Inside a single page, a line will remain onscreen until the earlier of either:
                                // 1/3 seconds AFTER the BEGINNING of the first highlight on the next line.
                                // 2000ms AFTER the END of the last highlight on the current line.
                                int erase_time_a = the_event->start_offset + 100;
                                int erase_time_b = last_wipe_event->start_offset + last_wipe_event->duration + 600;
                                int erase_time = (erase_time_a < erase_time_b) ? erase_time_a : erase_time_b;
                                // First, account for all lines that need to be erased like the original code above.
                                // This will usually be just one, but it includes blank lines, so can occasionally be many.
                                for (int this_line = last_wipe_info->line_num; this_line < event_info->line_num; this_line++)
                                {
                                    erase_time += (set_draw_time(KARAOKE_ERASE, this_line, erase_time) * 50);
                                };
                                // Store the start time of this line for use at the page transition.
                                lastline_start_time = the_event->start_offset;
                            }
                            // The current and last events cross a page boundary, and need to be handled differently.
                            else if (last_page < this_page)
                            {
                                // Calculate the time between the start of the last line and the start of the line.
                                int inter_line_time = 0;
                                if (lastline_start_time > 0)  { inter_line_time = the_event->start_offset - lastline_start_time; };
                                // Calculate the end of the last wipe.
                                int last_wipe_end   = last_wipe_event->start_offset;
                                    last_wipe_end  += (last_wipe_event->duration > 0) ? last_wipe_event->duration : 100;
                                // Calculate the time between the end of last wipe and the beginning of this one.
                                int inter_wipe_time = the_event->start_offset - last_wipe_end;

                                // Across a page boundary the following timing applies:

                                // A line will remain onscreen until the earliest of either:
                                // 1/3 seconds AFTER the BEGINNING of the first highlight on the next page.
                                // 2000ms AFTER the END of the last highlight on the previous page.
                                // 1/3 of the duration between pages (the time between end of the last page wipe and the start the current page wipe).
                                int erase_time_a = the_event->start_offset + 100;
                                int erase_time_b = last_wipe_end + 600;
                                int erase_time_c = last_wipe_end + (inter_wipe_time / 3);
                                int erase_time   = (erase_time_a < erase_time_b) ? erase_time_a : erase_time_b;
                                    erase_time   = (erase_time   < erase_time_c) ? erase_time   : erase_time_c;
                                // A new page will be drawn at the later of:
                                // 1/3 seconds AFTER the BEGINNING of the first highlight on the last line of previous page.
                                // 3000ms BEFORE the FIRST highlight on this page.
                                // 1/3 of the duration between pages (the time between end of the last page wipe and the start the current page wipe).
                                int draw_time_a = lastline_start_time + 100;
                                int draw_time_b = the_event->start_offset - 900;
                                int draw_time_c = last_wipe_end + (inter_wipe_time / 3);
                                int draw_time   = (draw_time_a > draw_time_b) ? draw_time_a : draw_time_b;
                                    draw_time   = (draw_time   > draw_time_c) ? draw_time   : draw_time_c;

                                // Now, after all that calculating, just go and throw half of it away :-)...
                                // Seriously, though, if the space between pages is less than 4000ms,
                                // then always draw the new page lines BEFORE clearing the current page lines,
                                // otherwise clear the current page lines prior to drawing the next page.
                                // (Eventually, the latter case could possibly default to a screen clear or fade, etc.)
                                if (inter_wipe_time < 1200)
                                {
                                    // Reset the draw time to 1/3 + 1/6 seconds after the start of the last line highlighting.
                                    draw_time = draw_time_a + 50;
                                    // Set the draw times for all next page lines prior to the current line.
                                    int start_line = last_page * lines_per_page;
                                    for (int this_line = start_line; this_line < last_wipe_info->line_num; this_line++)
                                    {
                                        draw_time += (set_draw_time(KARAOKE_DRAW,  this_line + lines_per_page, draw_time) * 50);
                                    };
                                    // Now, reset the draw time to 1/3 seconds after the start of this line highlighting...
                                    // ONLY if not already beyond that time.
                                    if (draw_time < draw_time_c)  { draw_time = draw_time_c; };
                                    // Set the erase times for the previous line(s).
                                    for (int this_line = last_wipe_info->line_num; this_line < event_info->line_num; this_line++)
                                    {
                                        draw_time += (set_draw_time(KARAOKE_ERASE, this_line, draw_time) * 50);
                                    };
                                    // Set the draw times for the rest of the lines on this page.
                                    for (int this_line = last_wipe_info->line_num; this_line < event_info->line_num; this_line++)
                                    {
                                        draw_time += (set_draw_time(KARAOKE_DRAW,  this_line + lines_per_page, draw_time) * 50);
                                    };
                                }
                                else
                                {
                                    // Set the erase times for the previous line(s).
                                    for (int this_line = last_wipe_info->line_num; this_line < event_info->line_num; this_line++)
                                    {
                                        erase_time += (set_draw_time(KARAOKE_ERASE, this_line, erase_time) * 50);
                                    };
                                    // Reset the draw time if already beyond it.
                                    if (draw_time < erase_time)  { draw_time = erase_time; };
                                    // Set the draw times for the entire next page.
                                    int start_line = last_page * lines_per_page;
                                    int   end_line = start_line + lines_per_page;
                                    for (int this_line = start_line; this_line < end_line; this_line++)
                                    {
                                        draw_time += (set_draw_time(KARAOKE_DRAW,  this_line + lines_per_page, draw_time) * 50);
                                    };
                                };
                            };
                        }
                        else // New line-by-line mode, accessed by SHIFT+Click on the Fix Timing button, WHILE USING LINE PALETTE MODE.
                        {
                            // ##### NEW VERSION 001b -- MODIFIED FOR TESTING LINE PALETTE MODE #####
                            // Determine if this line-to-line event transition occurs on the same page, or across pages.
                            if (last_page == this_page)
                            {
                                // In line palette mode, lines will always disappear 250ms after the last highlight on the line.
                                int erase_time = last_wipe_event->start_offset + last_wipe_event->duration + 75;
                                // Account for all lines that need to be erased.
                                // This will usually be just one, but it includes blank lines, so can occasionally be many.
                                for (int this_line = last_wipe_info->line_num; this_line < event_info->line_num; this_line++)
                                {
                                    erase_time += (set_draw_time(KARAOKE_DRAW,  this_line + lines_per_page, erase_time) * fade_interval);
                                };
                                // Store the start time of this line for use at the page transition.
                                lastline_start_time = the_event->start_offset;
                            }
                            // The current and last events cross a page boundary, and need to be handled differently.
                            else if (last_page < this_page)
                            {
                                // In line palette mode, lines will always disappear 250ms after the last highlight on the line.
                                int erase_time = last_wipe_event->start_offset + last_wipe_event->duration + 75;
                                // Calculate the end of the last wipe.
                                int last_wipe_end   = last_wipe_event->start_offset;
                                    last_wipe_end  += (last_wipe_event->duration > 0) ? last_wipe_event->duration : 100;
                                // Calculate the time between the end of last wipe and the beginning of this one.
                                int inter_wipe_time = the_event->start_offset - last_wipe_end;

                                // Across a page boundary the following timing applies:

                                // A line will remain onscreen until 250ms after the last wipe on that line.

                                // A new page will be drawn at the later of:
                                // 250ms AFTER the BEGINNING of the first highlight on the last line of previous page.
                                // 3000ms BEFORE the FIRST highlight on this page.
                                int draw_time_a = lastline_start_time + 65;
                                int draw_time_b = the_event->start_offset - 900;
                                int draw_time   = (draw_time_a > draw_time_b) ? draw_time_a : draw_time_b;

                                // Now, if the space between pages is less than 6000ms,
                                // then always draw the new page lines BEFORE clearing the current page lines,
                                // otherwise clear the current page lines prior to drawing the next page.
                                // (Eventually, the latter case could possibly default to a screen clear or fade, etc.)
                                if (inter_wipe_time < 1800)
                                {
                                    // Set the draw times for all next page lines prior to the current line.
                                    int start_line = last_page * lines_per_page;
                                    for (int this_line = start_line; this_line < last_wipe_info->line_num; this_line++)
                                    {
                                        // Calculate the minimum draw time for this line... Not really accurate, but a decent guess for now.
                                        int min_time  = get_draw_time(KARAOKE_DRAW, this_line + lines_per_page);
                                            min_time += (lines_per_page == 5) ? 300 : 200;
                                        // Advance the draw time if it's deemed to be too early.
                                           draw_time  = (min_time > draw_time) ? min_time : draw_time;
                                        // Set the draw time for this line.
                                           draw_time += (set_draw_time(KARAOKE_ERASE, this_line + lines_per_page, draw_time) * fade_interval);
                                    };
                                    // Now, reset the draw time to the later of either the current draw time,
                                    // or the erase time for the last line on the page (calculated above).
                                    if (draw_time < erase_time)  { draw_time = erase_time; };
                                    // Set the erase times for the previous line(s).
                                    int reveal_time = draw_time;
                                    for (int this_line = last_wipe_info->line_num; this_line < event_info->line_num; this_line++)
                                    {
                                        draw_time += (set_draw_time(KARAOKE_DRAW, this_line + lines_per_page, draw_time) * fade_interval);
                                    };
                                    // Increase the draw time to account for invisible drawing of the new ending lines.
                                    draw_time += 300;
                                    // Set the draw times for the rest of the lines on this page.
                                    for (int this_line = last_wipe_info->line_num; this_line < event_info->line_num; this_line++)
                                    {
                                        reveal_time += (lines_per_page == 5) ? 300 : 200;
                                        set_draw_time(KARAOKE_ERASE, this_line + lines_per_page, reveal_time);
                                    };
                                }
                                else
                                {
                                    int min_time = erase_time;
                                    // Set the erase times for the previous line(s).
                                    for (int this_line = last_wipe_info->line_num; this_line < event_info->line_num; this_line++)
                                    {
                                        erase_time += (set_draw_time(KARAOKE_DRAW,  this_line + lines_per_page, erase_time) * fade_interval);
                                    };
                                    // Set the initial reveal time.
                                    int reveal_time = draw_time_b;
                                    // Set the draw times for the entire next page.
                                    int start_line = last_page * lines_per_page;
                                    int   end_line = start_line + lines_per_page;
                                    for (int this_line = start_line; this_line < end_line; this_line++)
                                    {
                                        // Calculate the minimum draw time for this line... Not really accurate, but a decent guess for now.
                                           min_time += (lines_per_page == 5) ? 300 : 200;
                                        // Advance the draw time if it's deemed to be too early.
                                        reveal_time  = (min_time > reveal_time) ? min_time : reveal_time;
                                        // Set the draw time for this line.
                                        reveal_time += (set_draw_time(KARAOKE_ERASE, this_line + lines_per_page, reveal_time) * fade_interval);
                                    };
                                };
                            };
                        };
                    }
                    // Page by page mode is enabled, we're past the first page, and we're at a page boundary.
                    else if ( (event_info->line_num >= lines_per_page) && (last_page < this_page) )
                    {
                        int start_line = (event_info->line_num / lines_per_page) * lines_per_page;
                        // Set the start of this page to 1/6th seconds after the end of the last wipe.
                        int page_draw_time = last_wipe_event->start_offset + last_wipe_event->duration + 50;
                        // Don't draw a new page more than three seconds before the first wipe.
                        if ( (the_event->start_offset - page_draw_time) > 900 )
                        {
                            page_draw_time = the_event->start_offset - 900;
                        }
                        // Calculate the available packs between the start of the first wipe on this page, and the desired page draw time.
                        int available_time = the_event->start_offset - page_draw_time;
                        // Calculate the absolute minimum packs from wiping the last event to drawing this page.
                        int   minimum_time = the_event->start_offset - last_wipe_event->start_offset - 24; // Sensible minimum, but not guaranteed.
                        // Alert the user if there's not likely to be enough room, even after correction.
                        if (minimum_time < 32)
                        {
                            fl_alert("There was not enough bandwidth to clear screen on line %i.\n"\
                                     "The rendered graphics will probably contain artifacts.",
                                     last_wipe_info->line_num);
                        };
                        // If there's not enough room between the end of the last wipe and the start of this page,
                        // but there IS enough room between the start of the last wipe and the start of this page,
                        // then shorten the duration of the last wipe as needed to make room.
                        if (available_time < 32)
                        {
                            int new_duration = the_event->start_offset - last_wipe_event->start_offset - 150;
                            if (new_duration > 0)
                            {
                                last_wipe_event->duration = new_duration;
                                page_draw_time = last_wipe_event->start_offset + new_duration + 50;
                            }
                            else
                            {
                                last_wipe_event->duration = 0;
                                page_draw_time = last_wipe_event->start_offset + 32;
                            };
                        };
                        // Set the draw times for events on this page.
                        for (int this_line = start_line; this_line < (start_line + lines_per_page); this_line++)
                        {
                            page_draw_time += (set_draw_time(KARAOKE_DRAW , this_line, page_draw_time) * 50);
                        };
                    };
                };
            };
            last_wipe_event = the_event;
            last_wipe_info = event_info;
        }; // End of (== KARAOKE_WIPE) check.
    }; // End of for loop through event queue.
    // Account for all draw/erase events without wipes after the LAST wipe.

    // Convenience pointer to the *last event in the clip* and text info.
    CDGMagic_MediaEvent     *the_event  = clip_events->back();
    CDGMagic_TextEvent_Info *event_info = static_cast<CDGMagic_TextEvent_Info*>(the_event->user_obj);
    // Determine the last line we touched above.
    int start_line = last_wipe_info->line_num;
    // Find the last actual line of the clip.
    int end_line   = event_info->line_num;
    // Conditional for hacked in line palette modes.
    if (line_pal_mode == 0)
    {
        // Calculate the erase time of the last line (which was intentionally skipped above).
        int erase_time = last_wipe_event->start_offset + last_wipe_event->duration + 600;
        // Set the erase time for the last highlighted line.
        set_draw_time(KARAOKE_ERASE, start_line, erase_time);
        // Advance the erase time by 1/6 seconds.
        erase_time += 50;
        // Advance the start line by one line in line-by-line mode, or one page in page-by-page mode.
        start_line += (page_clear_mode == 0) ? lines_per_page : 1;
        // Step through and set the draw/erase times for all the clips beyond the last wipe.
        for (int this_line = start_line; this_line <= end_line; this_line++)
        {
            erase_time += (set_draw_time(KARAOKE_DRAW , this_line, erase_time) * 50);
            erase_time += (set_draw_time(KARAOKE_ERASE, this_line, erase_time) * 50);
        };
    }
    else
    {
        // Calculate the erase time of the last line (which was intentionally skipped above).
        int erase_time = last_wipe_event->start_offset + last_wipe_event->duration + 300;
        // Step through and set the draw/erase times for all the clips beyond the last wipe.
        // This probably won't do anything very useful, but does keep extra newlines at the
        // end of the text from residiing well past the end of the timeline.
        for (int this_line = start_line; this_line <= end_line; this_line++)
        {
            erase_time += (set_draw_time(KARAOKE_DRAW , this_line + lines_per_page, erase_time) * fade_interval);
        };
        // Advance one second.
        erase_time += 300;
        // Place the blank/last page fades... (These should really be deleted or never created by the parent clip.)
        for (int this_line = start_line; this_line <= end_line; this_line++)
        {
            erase_time += (set_draw_time(KARAOKE_ERASE, this_line + 1, erase_time) * fade_interval);
        };
    };
    // Resort the clip based on start offset times.
    current_clip->sort_event_queue();
    // Update the display of the parent clip.
    current_clip->redraw();
}

bool CDGMagic_TextClip_Window::kar_wipe_sorter(CDGMagic_MediaEvent *event_a, CDGMagic_MediaEvent *event_b)
{
    CDGMagic_TextEvent_Info *info_a = static_cast<CDGMagic_TextEvent_Info*>(event_a->user_obj);
    CDGMagic_TextEvent_Info *info_b = static_cast<CDGMagic_TextEvent_Info*>(event_b->user_obj);
    if (info_a->line_num == info_b->line_num)  { return (info_a->word_num < info_b->word_num); };
    return (info_a->line_num < info_b->line_num);
}

int CDGMagic_TextClip_Window::set_draw_time(int requested_type, int requested_line, int requested_start)
{
    // Convenience pointer to the current clip's event queue.
    MediaEvent_Queue *clip_events = current_clip->event_queue();
    // Step through the queue looking for the requested event.
    for (unsigned int event_num = 0; event_num < clip_events->size(); event_num++ )
    {
        // Convenience pointers for the current event and text info.
        CDGMagic_MediaEvent     *the_event  = clip_events->at(event_num);
        CDGMagic_TextEvent_Info *event_info = static_cast<CDGMagic_TextEvent_Info*>(the_event->user_obj);
        if ( (event_info->line_num == requested_line) && (event_info->karaoke_type == requested_type) )
        {
            // Set the requested event to the requested start time.
            the_event->start_offset = requested_start;
            // Let the caller know that the event was found.
            return 1;
        };
    };
    // Return 0 by default, that is if the event was not found.
    return 0;
}

int CDGMagic_TextClip_Window::get_draw_time(int requested_type, int requested_line)
{
    // Convenience pointer to the current clip's event queue.
    MediaEvent_Queue *clip_events = current_clip->event_queue();
    // Step through the queue looking for the requested event.
    for (unsigned int event_num = 0; event_num < clip_events->size(); event_num++ )
    {
        // Convenience pointers for the current event and text info.
        CDGMagic_MediaEvent     *the_event  = clip_events->at(event_num);
        CDGMagic_TextEvent_Info *event_info = static_cast<CDGMagic_TextEvent_Info*>(the_event->user_obj);
        if ( (event_info->line_num == requested_line) && (event_info->karaoke_type == requested_type) )
        {
            // Return the requested event's start time.
            return (the_event->start_offset);
        };
    };
    // Return -1 by default, that is if the event was not found.
    return -1;
}

//REMOVE
//void CDGMagic_TextClip_Window::DoPageChange_callback(Fl_Counter *CallingWidget)
//{
//    // Make sure we have at least one clip loaded.
//    if (current_clip->event_queue()->size() < 1)  { printf("Could not change page, event_queue()->size() was %i\n", current_clip->event_queue()->size() ); return; };
//    // Get the requested page number.
//    current_page = static_cast<int>( CallingWidget->value() ) - 1;
//    // Set it to the last available image if we would run past the buffer.
//    if ( current_page >= current_clip->event_queue()->size() )  { current_page = current_clip->event_queue()->size()-1; };
//    // Set the page.
//    CallingWidget->value( current_page+1 );
//    // Reload the current values.
//    update_current_values();
//}
void CDGMagic_TextClip_Window::DoPageChange_callback(Fl_Counter *CallingWidget)
{
    // Make sure we have at least one clip loaded.
    int queue_size = static_cast<int>(current_clip->event_queue()->size());
    if (queue_size < 1) {
        printf("Could not change page, event_queue()->size() was %d\n", queue_size);
        return;
    }

    // Get the requested page number.
    current_page = static_cast<int>(CallingWidget->value()) - 1;

    // Set to last available image if out of bounds.
    if (current_page >= queue_size) {
        current_page = queue_size - 1;
    }

    // Update widget value (1-based).
    CallingWidget->value(current_page + 1);

    // Reload the current values.
    update_current_values();
}


unsigned long CDGMagic_TextClip_Window::get_embedded_palette_color(unsigned char requested_palette, unsigned char requested_index)
{
    if ( (requested_palette < maximum_embedded_palettes) && (requested_index < 16) )
    {
        // A valid palette and index was requested, so return the associated RGBA value.
        return ( embedded_palettes[requested_palette][requested_index] );
    };
    // Default color is black, fully opaque.
    return FL_BLACK;
}

void CDGMagic_TextClip_Window::update_counter_colors()
{
    // Convenience variable for index into embedded_palettes array.
    int curr_pal_num = current_clip->default_palette_number();
    // Set the various controls RGB colors.
      fg_idx_counter->color( get_embedded_palette_color(curr_pal_num, current_clip->foreground_color()) ); fg_idx_counter->redraw();
      ol_idx_counter->color( get_embedded_palette_color(curr_pal_num, current_clip->outline_color()   ) ); ol_idx_counter->redraw();
      bg_idx_counter->color( get_embedded_palette_color(curr_pal_num, current_clip->background_color()) ); bg_idx_counter->redraw();
    fill_idx_counter->color( get_embedded_palette_color(curr_pal_num, current_clip->fill_color()      ) ); fill_idx_counter->redraw();
     box_idx_counter->color( get_embedded_palette_color(curr_pal_num, current_clip->box_color()       ) ); box_idx_counter->redraw();

    int frm_control_index = current_clip->frame_color();
    // XOR the frame color to find the preview index when in karaoke mode.
    if ( current_clip->karaoke_mode() >= KAR_MODE__5TLINE)
    {
        if ( current_clip->highlight_mode() <= 1 )  {  frm_control_index ^= current_clip->foreground_color();  }
        else                                        {  frm_control_index ^= current_clip->outline_color();     }
        // If highlighting BOTH foreground/outline, then XOR and show both colors.
        if ( current_clip->highlight_mode() == 3 )
        {
            frm_idx_counter->labelcolor( get_embedded_palette_color(curr_pal_num, current_clip->frame_color() ^ current_clip->foreground_color()) );
        }
        else
        {
            frm_idx_counter->labelcolor(FL_BLACK);
        };
    }
    else // Not karaoke mode, so set label color to black.
    {
        frm_idx_counter->labelcolor(FL_BLACK);
    };
    frm_idx_counter->color( get_embedded_palette_color(curr_pal_num, frm_control_index) );
    frm_idx_counter->redraw();
}

void CDGMagic_TextClip_Window::update_current_values()
{
    // Set the temp values to the requested page's pointers.
    current_bmp_object = current_clip->event_queue()->at(current_page)->BMPObject;
    // Set the cursor in the text edit control to the beginning of the line we're on.
    CDGMagic_TextEvent_Info *event_info = static_cast<CDGMagic_TextEvent_Info*>(current_clip->event_queue()->at(current_page)->user_obj);
    int top_line = (event_info->line_num > 0) ? event_info->line_num-1 : 0;
    text_editor->scroll( top_line, 0 );
    // Update the preview display.
    update_preview();
}

void CDGMagic_TextClip_Window::update_preview()
{
    int x_offset = current_bmp_object->x_offset();
    int y_offset = current_bmp_object->y_offset();

    char temp_label[32];
    snprintf(temp_label, 32, "X: %i, Y: %i", x_offset, y_offset);
    PreviewLabel->copy_label(temp_label);

    unsigned int tmp_color = 0x00;
    unsigned int src_x = 0, src_y = 0, dst_pxl = 0;

    if (current_clip->karaoke_mode() == KAR_MODE__LYRICS)
    {
        for (int y_inc = 0; y_inc < 216; y_inc++)
        {
            src_y = (y_inc - y_offset);
            for (int x_inc = 0; x_inc < 300; x_inc++)
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
    else if (current_clip->karaoke_mode() == KAR_MODE__TITLES)
    {
        for (int the_px = 0; the_px < 300*216; the_px++)
        {
           preview_buffer[the_px*3] = current_clip->fill_color();
        };
        unsigned char this_pxl = 0;
        unsigned char comp_idx = current_clip->composite_color();
        for (unsigned int curr_evnt = 0; curr_evnt < current_clip->event_queue()->size(); curr_evnt++)
        {
            CDGMagic_MediaEvent *temp_event = current_clip->event_queue()->at(curr_evnt);
            CDGMagic_BMPObject  *temp_bmp   = temp_event->BMPObject;
            int x_ofs   = temp_bmp->x_offset();
            int y_ofs   = temp_bmp->y_offset();
            int x_start = x_ofs;
            int y_start = y_ofs;
            int x_end   = x_start + temp_bmp->width();
            int y_end   = y_start + temp_bmp->height();
            if (x_start < 0)  { x_start = 0; } else if (x_start > 300)  { x_start = 300; };
            if (x_end   < 0)  { x_end   = 0; } else if (x_end   > 300)  { x_end   = 300; };
            if (y_start < 0)  { y_start = 0; } else if (y_start > 216)  { y_start = 216; };
            if (y_end   < 0)  { y_end   = 0; } else if (y_end   > 216)  { y_end   = 216; };
            for (int y_inc = y_start; y_inc < y_end; y_inc++)
            {
                src_y = (y_inc - y_ofs);
                for (int x_inc = x_start; x_inc < x_end; x_inc++)
                {
                    src_x = (x_inc - x_ofs);
                    dst_pxl = (x_inc + y_inc * 300) * 3;
                    // Only draw the pixel if it's not transparent (in titles mode).
                    if ( (this_pxl = temp_bmp->pixel(src_x, src_y)) != comp_idx )
                    {
                        preview_buffer[dst_pxl] = this_pxl;
                    };
                };
            };
        };
        // Hacky convert back to RGB.
        for (int the_px = 0; the_px < 300*216; the_px++)
        {
            dst_pxl = the_px * 3;
            tmp_color = current_bmp_object->PALObject()->color( preview_buffer[dst_pxl] );
            preview_buffer[dst_pxl+0] = (tmp_color >> 030) & 0xFF;
            preview_buffer[dst_pxl+1] = (tmp_color >> 020) & 0xFF;
            preview_buffer[dst_pxl+2] = (tmp_color >> 010) & 0xFF;
        };
    }
    else
    {
        // OK, this is REALLY bad...
        // The RGB buffer is cleared to index on pixel boundaries,
        // Then the indices for the current page lyrics and wipes are loaded and XORed.
        // Then the buffer is converted back to RGB... (!!!!)

        // Assign convenience variables for the event and textinfo.
        CDGMagic_MediaEvent     *origin_event    = current_clip->event_queue()->at(current_page);
        CDGMagic_TextEvent_Info *origin_textinfo = static_cast<CDGMagic_TextEvent_Info*>(origin_event->user_obj);
        // Clear the screen buffer... (Maybe this should paint the border color, too, if it's set?)
        // Also, should maybe add a way to set the transitions for this clip?
        unsigned char actual_background_index = (current_clip->karaoke_mode() >= KAR_MODE__5BLNCT) ? 0 : current_clip->background_color();
        for (int the_px = 0; the_px < 300*216; the_px++)
        {
           preview_buffer[the_px*3] = actual_background_index;
        };
        // Step through the array and draw all the lines, then all words, on this page.
        const int lines_per_page = current_clip->lines_per_page();
        //int start_line = origin_textinfo->line_num / lines_per_page * lines_per_page; // Page-ish mode.
        int start_line = origin_textinfo->line_num;
        int   end_line = start_line + lines_per_page;
        // Draw all the lines first.
        for (unsigned int curr_evnt = 0; curr_evnt < current_clip->event_queue()->size(); curr_evnt++)
        {
            CDGMagic_MediaEvent     *temp_event    = current_clip->event_queue()->at(curr_evnt);
            CDGMagic_TextEvent_Info *temp_textinfo = static_cast<CDGMagic_TextEvent_Info*>(temp_event->user_obj);
            // Draw lines from start_line end_line-1 if they're draw events.
            if (  (temp_textinfo->line_num >= start_line)
               && (temp_textinfo->line_num < end_line)
               && (temp_textinfo->karaoke_type == KARAOKE_DRAW)  )
            {
                CDGMagic_BMPObject *temp_bmp = temp_event->BMPObject;
                int x_start = temp_bmp->x_offset();
                int x_end   = x_start + temp_bmp->width();
                int y_start = temp_bmp->y_offset();
                int y_end   = y_start + temp_bmp->height();
                for (int y_inc = y_start; y_inc < y_end; y_inc++)
                {
                    src_y = (y_inc - y_start);
                    for (int x_inc = x_start; x_inc < x_end; x_inc++)
                    {
                        src_x = (x_inc - x_start);
                        dst_pxl = (x_inc + y_inc * 300) * 3;
                        // Draw the pixel normally for draw events.
                        preview_buffer[dst_pxl] = temp_bmp->pixel(src_x, src_y);
                    };
                };
            };
        };
        // Then highlight up to the current word.
        for (unsigned int curr_evnt = 0; curr_evnt < current_clip->event_queue()->size(); curr_evnt++)
        {
            CDGMagic_MediaEvent     *temp_event    = current_clip->event_queue()->at(curr_evnt);
            CDGMagic_TextEvent_Info *temp_textinfo = static_cast<CDGMagic_TextEvent_Info*>(temp_event->user_obj);
            // Wipe only up to the current event, and only for currently viewed lines.
            if (  (temp_textinfo->line_num >= start_line)
               && (temp_textinfo->line_num < end_line)
               && (temp_textinfo->karaoke_type == KARAOKE_WIPE)
               && (curr_evnt <= current_page)  )
            {
                CDGMagic_BMPObject *temp_bmp = temp_event->BMPObject;
                unsigned char       temp_xor = temp_bmp->xor_only();
                int x_start = temp_bmp->x_offset();
                int x_end   = x_start + temp_bmp->width();
                int y_start = temp_bmp->y_offset();
                int y_end   = y_start + temp_bmp->height();
                for (int y_inc = y_start; y_inc < y_end; y_inc++)
                {
                    src_y = (y_inc - y_start);
                    for (int x_inc = x_start; x_inc < x_end; x_inc++)
                    {
                        src_x = (x_inc - x_start);
                        dst_pxl = (x_inc + y_inc * 300) * 3;
                        // XOR this pixel with the current pixel index.
                        preview_buffer[dst_pxl] ^= (temp_bmp->pixel(src_x, src_y) * temp_xor);
                    };
                };
            };
        };
        // Hacky convert back to RGB.
        for (int the_px = 0; the_px < 300*216; the_px++)
        {
            dst_pxl = the_px * 3;
            tmp_color = current_bmp_object->PALObject()->color( preview_buffer[dst_pxl] );
            preview_buffer[dst_pxl+0] = (tmp_color >> 030) & 0xFF;
            preview_buffer[dst_pxl+1] = (tmp_color >> 020) & 0xFF;
            preview_buffer[dst_pxl+2] = (tmp_color >> 010) & 0xFF;
        };
    };
    PreviewImage->uncache();
    PreviewBox->redraw();
    PreviewBox->redraw_label();
}

int CDGMagic_TextClip_Window::handle(int incoming_event)
{
    static int orig_x = 0, orig_y = 0;
    static int x_off_a = 0, y_off_a = 0;

    if ((Fl::belowmouse() == PreviewBox) && (current_bmp_object != NULL) && (current_clip->karaoke_mode() <= KAR_MODE__LYRICS))
    {
        switch ( incoming_event )
        {
            case FL_PUSH:
                if (Fl::event_button() != FL_LEFT_MOUSE) { break; };
                orig_x = Fl::event_x();
                orig_y = Fl::event_y();
                x_off_a = current_bmp_object->x_offset();
                y_off_a = current_bmp_object->y_offset();
                return 1;

            case FL_DRAG:
                if (Fl::event_button() != FL_LEFT_MOUSE) { break; };
                current_bmp_object->x_offset( x_off_a + Fl::event_x() - orig_x );
                current_bmp_object->y_offset( y_off_a + Fl::event_y() - orig_y );
                update_preview();
                return 1;

            case FL_RELEASE:
                if (Fl::event_button() != FL_LEFT_MOUSE) { break; };
                // Don't set all x/y events for paged titles mode (karaoke mode is ignored above), or in lyrics mode if CTRL was held.
                // (The latter is an undocumented "feature"... As any changes get obliterated by the next Render Text.)
                if ( (current_clip->karaoke_mode() == KAR_MODE__TITLES) || (Fl::event_state()&FL_CTRL) )  { break; };
                int temp_x = current_bmp_object->x_offset();
                int temp_y = current_bmp_object->y_offset();
                // Set all the text event x/y offsets in this clip to the new value.
                for (unsigned int bmp_idx = 0; bmp_idx < current_clip->event_queue()->size(); bmp_idx++)
                {
                    current_clip->event_queue()->at(bmp_idx)->BMPObject->x_offset( temp_x );
                    current_clip->event_queue()->at(bmp_idx)->BMPObject->y_offset( temp_y );
                };
                return 1;
        };
    };

    return Fl_Double_Window::handle(incoming_event);
}

//############################################################################//

void CDGMagic_TextClip_Window::ShowCursorPos_cb_static(Fl_Widget *CallingWidget, void *IncomingObject)
{
    CDGMagic_TextClip_Window *tmp_obj = static_cast<CDGMagic_TextClip_Window*>( IncomingObject );

    char temp_label[256];
    int line = tmp_obj->text_editor->buffer()->count_lines(0, tmp_obj->text_editor->insert_position());
    const int lines_per_page = (tmp_obj->current_clip->lines_per_page() > 0) ? tmp_obj->current_clip->lines_per_page() : 1;
    int page = line / lines_per_page;
    int row  = line % lines_per_page;
    snprintf(temp_label, 256, "Cur Pos\n\nLn: %i\nPg: %i\nRw: %i", line+1, page+1, row+1);
    tmp_obj->EditorLabel->copy_label(temp_label);
}

void CDGMagic_TextClip_Window::modeselect_cb_static(Fl_Widget *CallingWidget, void *IncomingObject)
{
    CDGMagic_TextClip_Window *tmp_obj = static_cast<CDGMagic_TextClip_Window*>( IncomingObject );
    Fl_Choice *tmp_choice = dynamic_cast<Fl_Choice*>( CallingWidget );
    tmp_obj->current_clip->karaoke_mode( tmp_choice->value() );
    tmp_obj->update_counter_colors();
}

void CDGMagic_TextClip_Window::highlightselect_cb_static(Fl_Widget *CallingWidget, void *IncomingObject)
{
    CDGMagic_TextClip_Window *tmp_obj = static_cast<CDGMagic_TextClip_Window*>( IncomingObject );
    Fl_Choice *tmp_choice = dynamic_cast<Fl_Choice*>( CallingWidget );
    // Highlight is outline/foreground bitfield, so:
    // 0 = No highlight.
    // 1 = Just foreground text (including antialias).
    // 2 = Just outline (square or round).
    // 3 = Highlight all, both foreground and outline.
    tmp_obj->current_clip->highlight_mode( tmp_choice->value() );
    tmp_obj->update_counter_colors();
}

void CDGMagic_TextClip_Window::prevbut_cb_static(Fl_Widget *CallingWidget, void *IncomingObject)
{
    CDGMagic_TextClip_Window* this_win = static_cast<CDGMagic_TextClip_Window*>(IncomingObject);
    this_win->current_bmp_object = NULL;
    this_win->current_clip->render_text_to_bmp();
    this_win->DoPageChange_callback( this_win->Page_Counter );
}

void CDGMagic_TextClip_Window::playbut_cb_static(Fl_Widget *CallingWidget, void *IncomingObject)
{
    CDGMagic_TextClip_Window* this_win = static_cast<CDGMagic_TextClip_Window*>(IncomingObject);
    // Advance the page count by one.
    this_win->Page_Counter->value( 1.0 );
    // Update the preview image and controls.
    this_win->DoPageChange_callback( this_win->Page_Counter );
    // Seek to beginning of clip and begin playback.
    this_win->current_clip->seek_and_play( this_win->current_clip->start_pack() );
    // Send keyboard input to the set start button/control.
    Fl::focus( this_win->SetStart_button );
}

void CDGMagic_TextClip_Window::fontsize_cb_static(Fl_Widget *CallingWidget, void *IncomingObject)
{
    CDGMagic_TextClip_Window *tmp_obj = static_cast<CDGMagic_TextClip_Window*>( IncomingObject );
    Fl_Counter *tmp_count = dynamic_cast<Fl_Counter*>( CallingWidget );

    tmp_obj->text_editor->textsize( static_cast<int>(tmp_count->value()) );
    // Select/deselect hack seems to the only way to force a full redraw?...
    // There MUST be a better way to do this.
    tmp_obj->current_clip->text_buffer()->highlight(0, tmp_obj->current_clip->text_buffer()->length() );
    tmp_obj->current_clip->text_buffer()->unhighlight();
    tmp_obj->text_editor->redraw();

    tmp_obj->current_clip->font_size( static_cast<int>(tmp_count->value()) );
}

void CDGMagic_TextClip_Window::fontchange_cb_static(Fl_Widget *CallingWidget, void *IncomingObject)
{
    CDGMagic_TextClip_Window *tmp_obj = static_cast<CDGMagic_TextClip_Window*>( IncomingObject );
    Fl_Choice *tmp_choice = dynamic_cast<Fl_Choice*>( CallingWidget );

    tmp_obj->text_editor->textfont( tmp_choice->value() );
    // Select/deselect hack seems to the only way to force a full redraw?...
    // There MUST be a better way to do this.
    tmp_obj->current_clip->text_buffer()->highlight(0, tmp_obj->current_clip->text_buffer()->length() );
    tmp_obj->current_clip->text_buffer()->unhighlight();
    tmp_obj->text_editor->redraw();

    tmp_obj->current_clip->font_index( tmp_choice->value() );
}

void CDGMagic_TextClip_Window::SqrSz_cb_static(Fl_Widget *CallingWidget, void *IncomingObject)
{
    CDGMagic_TextClip_Window *tmp_obj = static_cast<CDGMagic_TextClip_Window*>( IncomingObject );
    Fl_Counter *tmp_count = dynamic_cast<Fl_Counter*>( CallingWidget );
    tmp_obj->current_clip->square_size( static_cast<int>(tmp_count->value()) );
}

void CDGMagic_TextClip_Window::RndSz_cb_static(Fl_Widget *CallingWidget, void *IncomingObject)
{
    CDGMagic_TextClip_Window *tmp_obj = static_cast<CDGMagic_TextClip_Window*>( IncomingObject );
    Fl_Counter *tmp_count = dynamic_cast<Fl_Counter*>( CallingWidget );
    tmp_obj->current_clip->round_size( static_cast<int>(tmp_count->value()) );
}

void CDGMagic_TextClip_Window::FgIdx_cb_static(Fl_Widget *CallingWidget, void *IncomingObject)
{
    CDGMagic_TextClip_Window *tmp_obj = static_cast<CDGMagic_TextClip_Window*>( IncomingObject );
    Fl_Counter *tmp_count = dynamic_cast<Fl_Counter*>( CallingWidget );
    tmp_obj->current_clip->foreground_color( static_cast<int>(tmp_count->value()) );
    tmp_obj->update_counter_colors();
}

void CDGMagic_TextClip_Window::OlIdx_cb_static(Fl_Widget *CallingWidget, void *IncomingObject)
{
    CDGMagic_TextClip_Window *tmp_obj = static_cast<CDGMagic_TextClip_Window*>( IncomingObject );
    Fl_Counter *tmp_count = dynamic_cast<Fl_Counter*>( CallingWidget );
    tmp_obj->current_clip->outline_color( static_cast<int>(tmp_count->value()) );
    tmp_obj->update_counter_colors();
}

void CDGMagic_TextClip_Window::BoxIdx_cb_static(Fl_Widget *CallingWidget, void *IncomingObject)
{
    CDGMagic_TextClip_Window *tmp_obj = static_cast<CDGMagic_TextClip_Window*>( IncomingObject );
    Fl_Counter *tmp_count = dynamic_cast<Fl_Counter*>( CallingWidget );
    tmp_obj->current_clip->box_color( static_cast<int>(tmp_count->value()) );
    tmp_obj->update_counter_colors();
}

void CDGMagic_TextClip_Window::FrmIdx_cb_static(Fl_Widget *CallingWidget, void *IncomingObject)
{
    CDGMagic_TextClip_Window *tmp_obj = static_cast<CDGMagic_TextClip_Window*>( IncomingObject );
    Fl_Counter *tmp_count = dynamic_cast<Fl_Counter*>( CallingWidget );
    tmp_obj->current_clip->frame_color( static_cast<int>(tmp_count->value()) );
    tmp_obj->update_counter_colors();
}

void CDGMagic_TextClip_Window::BgIdx_cb_static(Fl_Widget *CallingWidget, void *IncomingObject)
{
    CDGMagic_TextClip_Window *tmp_obj = static_cast<CDGMagic_TextClip_Window*>( IncomingObject );
    Fl_Counter *tmp_count = dynamic_cast<Fl_Counter*>( CallingWidget );
    tmp_obj->current_clip->background_color( static_cast<int>(tmp_count->value()) );
    tmp_obj->update_counter_colors();
}

void CDGMagic_TextClip_Window::FillIdx_cb_static(Fl_Widget *CallingWidget, void *IncomingObject)
{
    CDGMagic_TextClip_Window *tmp_obj = static_cast<CDGMagic_TextClip_Window*>( IncomingObject );
    Fl_Counter *tmp_count = dynamic_cast<Fl_Counter*>( CallingWidget );
    tmp_obj->current_clip->fill_color( static_cast<int>(tmp_count->value()) );
    tmp_obj->update_counter_colors();
}

void CDGMagic_TextClip_Window::CompIdx_cb_static(Fl_Widget *CallingWidget, void *IncomingObject)
{
    CDGMagic_TextClip_Window *tmp_obj = static_cast<CDGMagic_TextClip_Window*>( IncomingObject );
    Fl_Counter *tmp_count = dynamic_cast<Fl_Counter*>( CallingWidget );
    tmp_obj->current_clip->composite_color( static_cast<int>(tmp_count->value()) );
}

void CDGMagic_TextClip_Window::ShouldComp_cb_static(Fl_Widget *CallingWidget, void *IncomingObject)
{
    CDGMagic_TextClip_Window *tmp_obj = static_cast<CDGMagic_TextClip_Window*>( IncomingObject );
    Fl_Check_Button *tmp_count = dynamic_cast<Fl_Check_Button*>( CallingWidget );
    tmp_obj->current_clip->should_composite( tmp_count->value() );
}

void CDGMagic_TextClip_Window::XorBw_cb_static(Fl_Widget *CallingWidget, void *IncomingObject)
{
    CDGMagic_TextClip_Window *tmp_obj = static_cast<CDGMagic_TextClip_Window*>( IncomingObject );
    Fl_Choice *tmp_choice = dynamic_cast<Fl_Choice*>( CallingWidget );
    tmp_obj->current_clip->xor_bandwidth( tmp_choice->value() );
}

void CDGMagic_TextClip_Window::antialiasmode_cb_static(Fl_Widget *CallingWidget, void *IncomingObject)
{
    CDGMagic_TextClip_Window *tmp_obj = static_cast<CDGMagic_TextClip_Window*>( IncomingObject );
    Fl_Check_Button *tmp_check = dynamic_cast<Fl_Check_Button*>( CallingWidget );
    tmp_obj->current_clip->antialias_mode( tmp_check->value() );
}

void CDGMagic_TextClip_Window::paletteselect_cb_static(Fl_Widget *CallingWidget, void *IncomingObject)
{
    CDGMagic_TextClip_Window *tmp_obj = static_cast<CDGMagic_TextClip_Window*>( IncomingObject );
    Fl_Choice *tmp_choice = dynamic_cast<Fl_Choice*>( CallingWidget );
    tmp_obj->current_clip->default_palette_number( tmp_choice->value()-1 );
    tmp_obj->update_current_values();
    tmp_obj->update_counter_colors();
}

//############################################################################//

void CDGMagic_TextClip_Window::DoOK_callback_static(Fl_Widget *CallingWidget, void *IncomingObject)
{
    ((CDGMagic_TextClip_Window*)IncomingObject)->DoOK_callback(CallingWidget);
}

void CDGMagic_TextClip_Window::DoFixTime_callback_static(Fl_Widget *CallingWidget, void *IncomingObject)
{
    ((CDGMagic_TextClip_Window*)IncomingObject)->DoFixTime_callback(CallingWidget);
}

void CDGMagic_TextClip_Window::DoSetStart_callback_static(Fl_Widget *CallingWidget, void *IncomingObject)
{
    ((CDGMagic_TextClip_Window*)IncomingObject)->DoSetStart_callback(CallingWidget);
}

void CDGMagic_TextClip_Window::DoPageChange_callback_static(Fl_Widget *CallingWidget, void *IncomingObject)
{
    ((CDGMagic_TextClip_Window*)IncomingObject)->DoPageChange_callback( dynamic_cast<Fl_Counter*>(CallingWidget) );
}
