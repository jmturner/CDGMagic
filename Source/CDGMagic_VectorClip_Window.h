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

// TODO: Actually implement the clip for this...
//       And fix the window record/playback code, as it only kinda works.

#ifndef CDGMAGIC_VECTORCLIP_WINDOW_H
#define CDGMAGIC_VECTORCLIP_WINDOW_H

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_RGB_Image.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Counter.H>

#include "CDGMagic_BMPClip.h"

struct line_cords
{
    int time;
    float x1;
    float y1;
    float x2;
    float y2;
};

class CDGMagic_VectorClip_Window : public Fl_Double_Window
{
private:
    Fl_Box *PreviewBox;
    int min_x, max_x;
    int min_y, max_y;
    int *current_time;
    int record_mode;

    std::deque<line_cords> record_buffer_deque;

    void DoOK_callback(Fl_Widget *CallingWidget);
    static void DoOK_callback_static(Fl_Widget *CallingWidget, void *IncomingObject);

    void DoRecord_callback(Fl_Widget *CallingWidget);
    static void DoRecord_callback_static(Fl_Widget *CallingWidget, void *IncomingObject);

    void DoPlay_callback(Fl_Widget *CallingWidget);
    static void DoPlay_callback_static(Fl_Widget *CallingWidget, void *IncomingObject);
    static void Do_Vector_Update(void *IncomingObject);

    void DoClear_callback(Fl_Widget *CallingWidget);
    static void DoClear_callback_static(Fl_Widget *CallingWidget, void *IncomingObject);

    void add_and_draw_line(int x1, int y1, int x2, int y2);
    int get_time();

protected:
    int handle(int event);

public:
    CDGMagic_VectorClip_Window(int X, int Y);
    ~CDGMagic_VectorClip_Window();

    void set_time_var(int *time);
};

#endif

