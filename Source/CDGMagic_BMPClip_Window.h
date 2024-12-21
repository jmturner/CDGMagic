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

#ifndef CDGMAGIC_BMPCLIP_WINDOW_H
#define CDGMAGIC_BMPCLIP_WINDOW_H

#include <FL/Fl.H>
#include <FL/filename.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_RGB_Image.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Counter.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Hold_Browser.H>

#include "CDGMagic_BMPClip.h"

class CDGMagic_BMPClip_Window : public Fl_Double_Window
{
private:
    Fl_Box*             PreviewBox;
    Fl_RGB_Image*       PreviewImage;
    Fl_Box*             PreviewLabel;

    unsigned char*      preview_buffer;

    signed int          animate_progress;
    unsigned int        current_page;
    CDGMagic_BMPClip*   current_clip;
    CDGMagic_BMPObject* current_bmp_object;

    Fl_Hold_Browser*    bmp_list_control;
    Fl_Button*          Trans_button;
    Fl_Counter*         fill_idx_counter;
    Fl_Counter*         comp_idx_counter;
    Fl_Choice*          Composite_choice;
    Fl_Choice*          Screen_choice;
    Fl_Choice*          Border_choice;
    Fl_Check_Button*    update_pal_check;

    void        update_preview();
    void        update_current_values();
    void        update_browser_text(int requested_event_number);

    void        DoOK_callback(Fl_Button *CallingWidget);
    static void DoOK_callback_static(Fl_Widget *CallingWidget, void *IncomingObject);

    void        DoApplyAll_callback(Fl_Button *CallingWidget);
    static void DoApplyAll_callback_static(Fl_Widget *CallingWidget, void *IncomingObject);

    void        DoAdd_callback(Fl_Button *CallingWidget);
    static void DoAdd_callback_static(Fl_Widget *CallingWidget, void *IncomingObject);

    void        DoRemove_callback(Fl_Button *CallingWidget);
    static void DoRemove_callback_static(Fl_Widget *CallingWidget, void *IncomingObject);

    void        DoSetStart_callback(Fl_Button *CallingWidget);
    static void DoSetStart_callback_static(Fl_Widget *CallingWidget, void *IncomingObject);

    void        DoTransition_callback(Fl_Button *CallingWidget);
    static void DoTransition_callback_static(Fl_Widget *CallingWidget, void *IncomingObject);

    void        DoPageChange_callback();
    void        DoBMPBrowser_callback(Fl_Hold_Browser *CallingWidget);
    static void DoBMPBrowser_callback_static(Fl_Widget *CallingWidget, void *IncomingObject);

    static void FillIdx_cb_static(Fl_Widget *CallingWidget, void *IncomingObject);
    static void CompIdx_cb_static(Fl_Widget *CallingWidget, void *IncomingObject);
    static void ShouldComp_cb_static(Fl_Widget *CallingWidget, void *IncomingObject);
    static void ScreenIdx_cb_static(Fl_Widget *CallingWidget, void *IncomingObject);
    static void BorderIdx_cb_static(Fl_Widget *CallingWidget, void *IncomingObject);
    static void SetPal_cb_static(Fl_Widget *CallingWidget, void *IncomingObject);
    static void animate_cb_static(Fl_Widget *CallingWidget, void *IncomingObject);
    static void animate_progress_static(void *IncomingObject);

protected:
    int handle(int event);

public:
    CDGMagic_BMPClip_Window(CDGMagic_BMPClip *clip_to_edit, int X, int Y);
    ~CDGMagic_BMPClip_Window();

    int can_delete();
};

#endif

