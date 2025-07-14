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

#include "CDGMagic_EditingGroup.h"

CDGMagic_EditingGroup::CDGMagic_EditingGroup(int X, int Y, int W, int H, const char *L) : Fl_Group(X,Y,W,H,L)
{
    const int button_x_loc     =  10;
    const int button_y_loc     =  60;
    const int button_y_spacing =  40;
    const int button_width     = 120;
    const int button_height    = 30;
    // We want a basic, opaque but borderless box.
    box(FL_FLAT_BOX);
    // Create the custom editing lanes object.
    editing_lanes = new CDGMagic_EditingLanes( x()+200, y(), w()-200, h() );
    editing_lanes->end();
    // Make the editing lanes the resizable object.
    resizable(editing_lanes);

    editing_lanes->playback_head->callback( PBHead_PositionChange_callback_static, this);

    cdg_gen = new CDGMagic_GraphicsEncoder(300);
    TimeLine_Deque = new std::deque<CDGMagic_MediaClip*>;
    current_play_position = 0;

    Fl_Group *track_options_group = new Fl_Group(editing_lanes->x()-55, editing_lanes->y() + 15 + (editing_lanes->lane_height()+1)*2, 55, (editing_lanes->lane_height()+1)*8);
    track_options_group->box(FL_FLAT_BOX);
    track_options_group->resizable(NULL);
    track_options_group->color(46);
    track_options_group->end();
    for (int current_track = 0; current_track < 8; current_track++)
    {
        //char lbl_txt[16];  sprintf(lbl_txt, "%i", current_track+1);
        int x_loc = editing_lanes->x()-53;
        int y_loc = editing_lanes->y() + 15 + 5 + (editing_lanes->lane_height()+1)*(current_track+2);
        int w_siz = 51;
        int h_siz = editing_lanes->lane_height()-10;
        current_track_options[current_track] = new CDGMagic_TrackOptions(x_loc, y_loc, w_siz, h_siz);
        //current_track_options->copy_label(lbl_txt);
        current_track_options[current_track]->track(current_track);
        current_track_options[current_track]->resizable(NULL);
        track_options_group->add(current_track_options[current_track]);
    };

    time_display = new CDGMagic_TimeOutput((editing_lanes->x()-160)/2, editing_lanes->y()+7, 160, 40, NULL );

    //##### Create the editing controls on the left side. #####//
    play_pause_button = new Fl_Light_Button(button_x_loc, button_y_loc+button_y_spacing*1, button_width, button_height, "Play / Pause");
    play_pause_button->box(FL_PLASTIC_UP_BOX);
    play_pause_button->shortcut(" ");
    play_pause_button->deactivate();
    play_pause_button->callback(PlayPause_callback_static, this);

    recalculate_button = new Fl_Button(button_x_loc, button_y_loc+button_y_spacing*2, button_width, button_height, "Recalculate");
    recalculate_button->box(FL_PLASTIC_UP_BOX);
    recalculate_button->tooltip("Recalculate the preview to reflect the current timeline layout.");
    recalculate_button->callback(RecalculatePreview_callback_static, this);

    add_text_button = new Fl_Button(button_x_loc, button_y_loc+button_y_spacing*4, button_width, button_height, "Add Text...");
    add_text_button->box(FL_PLASTIC_UP_BOX);
    add_text_button->tooltip("Add a single line syncronized text clip to the timeline.");
    add_text_button->callback(AddText_callback_static, this);

    add_bmp_button = new Fl_Button(button_x_loc, button_y_loc+button_y_spacing*5, button_width, button_height, "Add BMP...");
    add_bmp_button->box(FL_PLASTIC_UP_BOX);
    add_bmp_button->tooltip("Add a bitmap file sequence clip to the timeline.");
    add_bmp_button->callback(AddBMP_callback_static, this);
/*
    add_scroll_button = new Fl_Button(button_x_loc, button_y_loc+button_y_spacing*6, button_width, button_height, "Scroll BMP...");
    add_scroll_button->box(FL_PLASTIC_UP_BOX);
    add_scroll_button->tooltip("Add a scrollable (single) bitmap file clip to the timeline.");
    add_scroll_button->callback(AddScroll_callback_static, this);
*/
/*
    add_vector_button = new Fl_Button(button_x_loc, button_y_loc+button_y_spacing*6, button_width, button_height, "Vector Art...");
    add_vector_button->box(FL_PLASTIC_UP_BOX);
    add_vector_button->callback(AddVector_callback_static, this);
    add_vector_button->deactivate();
*/
    set_wave_button = new Fl_Button(button_x_loc, button_y_loc+button_y_spacing*8, button_width, button_height, "Set WAVE...");
    set_wave_button->box(FL_PLASTIC_UP_BOX);
    set_wave_button->tooltip("Open/set the WAVE (audio) file to be used for this project.\n"\
                             "NOTE: MUST BE LPCM/44.1KHz/16bit/Stereo!\n"\
                             "It is recommended to TURN DOWN YOUR VOLUME when trying this program for the first time.");
    set_wave_button->callback(SetWAVE_callback_static, this);

    save_cdg_button = new Fl_Button(button_x_loc, button_y_loc+button_y_spacing*9, button_width, button_height, "Export CDG...");
    save_cdg_button->box(FL_PLASTIC_UP_BOX);
    save_cdg_button->tooltip("Export the current preview as a new subcode packets (.cdg) file.");
    save_cdg_button->callback(DoCDGSave_callback_static, this);

    Fl_Choice *samplerate_choice = new Fl_Choice(button_x_loc+20, button_y_loc+button_y_spacing*11, button_width-20, 25, "SR:");
    samplerate_choice->add("22,050 Hz", 0, NULL);
    samplerate_choice->add("32,000 Hz", 0, NULL);
    samplerate_choice->add("44,100 Hz", 0, NULL);
    samplerate_choice->add("48,000 Hz", 0, NULL);
    samplerate_choice->value(2);
    samplerate_choice->tooltip("Playback sample rate.\n"\
                               "Used to play back faster/slower than real-time.\n");
    samplerate_choice->callback( samplerate_select_cb_static, this );
/*
    Fl_Button* resync_timeline_button = new Fl_Button(button_x_loc, button_y_loc+button_y_spacing*11, button_width, button_height, "Resync Timeline");
    resync_timeline_button->box(FL_PLASTIC_UP_BOX);
    resync_timeline_button->callback(DoResync_callback_static, this);
*/
    //##### End of left side controls. #####//

    Audio_Playback = NULL;
    wave_rgb_bmp = NULL;
    wave_box = NULL;
    WAVE_Fl_RGB = NULL;
    global_pal_box = NULL;
}

CDGMagic_EditingGroup::~CDGMagic_EditingGroup()
{
    if (wave_box)        { Fl::delete_widget( wave_box ); wave_box = NULL;       };
    if (WAVE_Fl_RGB)     { delete   WAVE_Fl_RGB;          WAVE_Fl_RGB = NULL;    };
    if (wave_rgb_bmp)    { delete[] wave_rgb_bmp;         wave_rgb_bmp = NULL;   };
    if (Audio_Playback)  { delete   Audio_Playback;       Audio_Playback = NULL; };
    delete cdg_gen;
    delete TimeLine_Deque;
}

int CDGMagic_EditingGroup::handle(int incoming_event)
{
#ifdef _DEBUG_EDITINGGROUP_EVENTS_
    printf("EdtGROUP_e:%i,X:%i,Y:%i,EvX:%i,EvY:%i,Push:%i,Below:%i\n", incoming_event, x(), y(), Fl::event_x(), Fl::event_y(), (int)Fl::pushed(), (int)Fl::belowmouse() );
#endif
    // This could maybe be moved to the EditingLanes, so the wheel ONLY moves the timeline when the cursor is over it.
    // At present, it will move it whenever the main application window has focus.
    if (incoming_event == FL_MOUSEWHEEL)
    {
        int page_width    = editing_lanes->w() / 4;
        int max_x_pos     = (wave_box == NULL) ? 0 : wave_box->w() - editing_lanes->w();
        int min_x_pos     = 0;
        int current_x_pos = editing_lanes->xposition();
        int new_x_pos     = current_x_pos + (Fl::event_dy() * page_width);
        new_x_pos = (new_x_pos > max_x_pos) ? max_x_pos : new_x_pos; // Constrain to maximum (which could be negative).
        new_x_pos = (new_x_pos < min_x_pos) ? min_x_pos : new_x_pos; // Constrain to actual minimum (typically 0);
        editing_lanes->scroll_to(new_x_pos, editing_lanes->yposition());  // Scroll to the new position.
        return 1;
    };
    return Fl_Group::handle(incoming_event);
}
