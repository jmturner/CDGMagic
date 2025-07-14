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

#ifndef CDGMAGIC_EDITINGGROUP_H
#define CDGMAGIC_EDITINGGROUP_H

#include <deque>
#include <algorithm>

#include <FL/Fl.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Choice.H>

#include "CDGMagic_TimeOutput.h"
#include "CDGMagic_PreviewWindow.h"
#include "CDGMagic_TrackOptions.h"
#include "CDGMagic_EditingLanes.h"
#include "CDGMagic_AudioPlayback.h"
#include "CDGMagic_PALGlobalClip.h"
#include "CDGMagic_BMPClip.h"
#include "CDGMagic_GraphicsEncoder.h"

class CDGMagic_EditingGroup : public Fl_Group
{
protected:
    int handle(int incoming_event);

    Fl_Light_Button *play_pause_button;
    Fl_Button *recalculate_button, *add_text_button, *add_bmp_button, *add_scroll_button;
    Fl_Button *add_vector_button, *set_wave_button, *save_cdg_button;
    CDGMagic_TimeOutput *time_display;
    int current_play_position;
    unsigned char *wave_rgb_bmp;
    Fl_Box *wave_box;
    Fl_RGB_Image *WAVE_Fl_RGB;
    CDGMagic_PALGlobalClip* global_pal_box;
    CDGMagic_TrackOptions* current_track_options[8];

    std::deque<CDGMagic_MediaClip*> *TimeLine_Deque;

    CDGMagic_AudioPlayback *Audio_Playback;
    CDGMagic_GraphicsEncoder *cdg_gen;

    static void PBHead_PositionChange_callback_static(Fl_Widget *CallingWidget, void *IncomingObject);
    void PBHead_PositionChange_callback(Fl_Widget *CallingWidget);

    static void Clip_PositionChange_callback_static(Fl_Widget *CallingWidget, void *IncomingObject);
    void Clip_PositionChange_callback(Fl_Widget *CallingWidget);

    static void Queue_Next_Audio_Buffer(void *IncomingObject);
    static void PlayPause_callback_static(Fl_Widget *CallingWidget, void *IncomingObject);
    void PlayPause_callback(Fl_Light_Button *CallingWidget);

    static void RecalculatePreview_callback_static(Fl_Widget *CallingWidget, void *IncomingObject);
    void RecalculatePreview_callback(Fl_Widget *CallingWidget);

    static void AddBMP_callback_static(Fl_Widget *CallingWidget, void *IncomingObject);
    void AddBMP_callback(Fl_Widget *CallingWidget);

    static void AddText_callback_static(Fl_Widget *CallingWidget, void *IncomingObject);
    void AddText_callback(Fl_Widget *CallingWidget);

    static void AddScroll_callback_static(Fl_Widget *CallingWidget, void *IncomingObject);
    void AddScroll_callback(Fl_Widget *CallingWidget);

    static void AddVector_callback_static(Fl_Widget *CallingWidget, void *IncomingObject);
    void AddVector_callback(Fl_Widget *CallingWidget);

    static void SetWAVE_callback_static(Fl_Widget *CallingWidget, void *IncomingObject);
    void SetWAVE_callback(Fl_Widget *CallingWidget);
    void Create_Wave_Object(const char* requested_file);

    void DoProjectNew_callback(Fl_Widget *CallingWidget);

    void DoCDGSave_callback(Fl_Widget *CallingWidget);
    void DoProjectSave_callback(Fl_Widget *CallingWidget);
    void DoProjectOpen_callback(Fl_Widget *CallingWidget);
    static void DoResync_callback_static(Fl_Widget *CallingWidget, void *IncomingObject);
    static void samplerate_select_cb_static(Fl_Widget *CallingWidget, void *IncomingObject);

public:
    CDGMagic_EditingLanes *editing_lanes;
    CDGMagic_PreviewWindow *preview_window;

    CDGMagic_EditingGroup(int X, int Y, int W, int H, const char *L=0);
    ~CDGMagic_EditingGroup();

    int delete_clip(CDGMagic_MediaClip *the_clip);
    void seek_and_play(int requested_seek_time);

    static void DoCDGSave_callback_static(Fl_Widget *CallingWidget, void *IncomingObject);
    static void DoProjectNew_callback_static(Fl_Widget *CallingWidget, void *IncomingObject);
    static void DoProjectSave_callback_static(Fl_Widget *CallingWidget, void *IncomingObject);
    static void DoProjectOpen_callback_static(Fl_Widget *CallingWidget, void *IncomingObject);
};

#endif
