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

#ifndef CDGMAGIC_APPLICATION_H
#define CDGMAGIC_APPLICATION_H

#include <FL/Fl.H>
#include <FL/Fl_Widget.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Text_Buffer.H>

#include "CDGMagic_MainWindow.h"
#include "CDGMagic_PreviewWindow.h"

struct font_intensity
{
    int x_blk;
    int y_blk;
    int intensity;
};

class CDGMagic_Application
{
private:
    static CDGMagic_PreviewWindow* internal_static_preview_window;
	static bool gradient_sorter(font_intensity entry_a, font_intensity entry_b);

public:
    CDGMagic_Application(int argc, char **argv);

    static void app_exit_callback(Fl_Widget *CallingWidget, void *IncomingObject);
    static void app_about_callback(Fl_Widget *CallingWidget, void *IncomingObject);
    static void show_preview_window(Fl_Widget *CallingWidget, void *IncomingObject);
    static void bmp_to_transition_callback(Fl_Widget *CallingWidget, void *IncomingObject);
};

#endif
