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

#ifndef CDGMAGIC_SCROLLCLIP_WINDOW_H
#define CDGMAGIC_SCROLLCLIP_WINDOW_H

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_RGB_Image.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Counter.H>

#include "CDGMagic_ScrollClip.h"

class CDGMagic_ScrollClip_Window : public Fl_Double_Window
{
private:
    Fl_Box*             PreviewBox;
    Fl_RGB_Image*       PreviewImage;
    Fl_Box*             PreviewLabel;

    unsigned char*      preview_buffer;

    CDGMagic_ScrollClip*   current_clip;
    CDGMagic_BMPObject* current_bmp_object;

    Fl_Button*          Trans_button;
    Fl_Counter*         fill_idx_counter;
    Fl_Counter*         comp_idx_counter;
    Fl_Choice*          Composite_choice;

    void        update_preview();
    void        update_current_values();

    void        DoOK_callback(Fl_Widget *CallingWidget);
    static void DoOK_callback_static(Fl_Widget *CallingWidget, void *IncomingObject);

    void        DoAdd_callback(Fl_Widget *CallingWidget);
    static void DoAdd_callback_static(Fl_Widget *CallingWidget, void *IncomingObject);

    void        DoTransition_callback(Fl_Widget *CallingWidget);
    static void DoTransition_callback_static(Fl_Widget *CallingWidget, void *IncomingObject);

    static void FillIdx_cb_static(Fl_Widget *CallingWidget, void *IncomingObject);
    static void CompIdx_cb_static(Fl_Widget *CallingWidget, void *IncomingObject);
    static void ShouldComp_cb_static(Fl_Widget *CallingWidget, void *IncomingObject);

protected:
    int handle(int event);

public:
    CDGMagic_ScrollClip_Window(CDGMagic_ScrollClip *clip_to_edit, int X, int Y);
    ~CDGMagic_ScrollClip_Window();

    int can_delete();
};

#endif

