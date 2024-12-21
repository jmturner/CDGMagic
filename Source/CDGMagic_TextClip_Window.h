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

#ifndef CDGMAGIC_TEXTCLIP_WINDOW_H
#define CDGMAGIC_TEXTCLIP_WINDOW_H

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Counter.H>
#include <FL/Fl_Check_Button.H>

#include "CDGMagic_TextClip_Window__Editor.h"
#include "CDGMagic_TextClip_Window__CtrlButton.h"

#include "CDGMagic_TextClip.h"

class CDGMagic_TextClip_Window : public Fl_Double_Window
{
private:
    CDGMagic_TextClip_Window__Editor *text_editor;
    Fl_Box *EditorLabel;
    Fl_Box *PreviewBox;
    Fl_Box *PreviewLabel;
    Fl_RGB_Image *PreviewImage;
    Fl_Counter *Page_Counter;
    CDGMagic_TextClip_Window__CtrlButton *SetStart_button;

    Fl_Counter *fg_idx_counter, *bg_idx_counter, *ol_idx_counter;
    Fl_Counter *box_idx_counter, *frm_idx_counter, *fill_idx_counter;

    unsigned char *preview_buffer;

    unsigned int current_page;
    int x_offset, y_offset;

    CDGMagic_TextClip *current_clip;
    CDGMagic_BMPObject* current_bmp_object;

    static void ShowCursorPos_cb_static(Fl_Widget *CallingWidget, void *IncomingObject);
    static void modeselect_cb_static(Fl_Widget *CallingWidget, void *IncomingObject);
    static void highlightselect_cb_static(Fl_Widget *CallingWidget, void *IncomingObject);
    static void prevbut_cb_static(Fl_Widget *CallingWidget, void *IncomingObject);
    static void playbut_cb_static(Fl_Widget *CallingWidget, void *IncomingObject);
    static void fontchange_cb_static(Fl_Widget *CallingWidget, void *IncomingObject);
    static void fontsize_cb_static(Fl_Widget *CallingWidget, void *IncomingObject);
    static void SqrSz_cb_static(Fl_Widget *CallingWidget, void *IncomingObject);
    static void RndSz_cb_static(Fl_Widget *CallingWidget, void *IncomingObject);
    static void OlIdx_cb_static(Fl_Widget *CallingWidget, void *IncomingObject);
    static void BoxIdx_cb_static(Fl_Widget *CallingWidget, void *IncomingObject);
    static void FrmIdx_cb_static(Fl_Widget *CallingWidget, void *IncomingObject);
    static void FgIdx_cb_static(Fl_Widget *CallingWidget, void *IncomingObject);
    static void BgIdx_cb_static(Fl_Widget *CallingWidget, void *IncomingObject);
    static void FillIdx_cb_static(Fl_Widget *CallingWidget, void *IncomingObject);
    static void CompIdx_cb_static(Fl_Widget *CallingWidget, void *IncomingObject);
    static void ShouldComp_cb_static(Fl_Widget *CallingWidget, void *IncomingObject);
    static void XorBw_cb_static(Fl_Widget *CallingWidget, void *IncomingObject);
    static void antialiasmode_cb_static(Fl_Widget *CallingWidget, void *IncomingObject);
    static void paletteselect_cb_static(Fl_Widget *CallingWidget, void *IncomingObject);

    void DoOK_callback(Fl_Widget *CallingWidget);
    static void DoOK_callback_static(Fl_Widget *CallingWidget, void *IncomingObject);

    void DoFixTime_callback(Fl_Widget *CallingWidget);
    static void DoFixTime_callback_static(Fl_Widget *CallingWidget, void *IncomingObject);

    void DoSetStart_callback(Fl_Widget *CallingWidget);
    static void DoSetStart_callback_static(Fl_Widget *CallingWidget, void *IncomingObject);

    void DoPageChange_callback(Fl_Counter *CallingWidget);
    static void DoPageChange_callback_static(Fl_Widget *CallingWidget, void *IncomingObject);

    void update_preview();
    void update_current_values();
    void update_counter_colors();

    static bool kar_wipe_sorter(CDGMagic_MediaEvent *event_a, CDGMagic_MediaEvent *event_b);
    int         set_draw_time(int requested_type, int requested_line, int requested_start);
    int         get_draw_time(int requested_type, int requested_line);
    unsigned long get_embedded_palette_color(unsigned char requested_palette, unsigned char requested_index);

protected:
    int handle(int event);

public:
    CDGMagic_TextClip_Window(CDGMagic_TextClip *clip_to_edit, int X, int Y);
    ~CDGMagic_TextClip_Window();

    int can_delete();
};

#endif
