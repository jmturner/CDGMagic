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

#ifndef CDGMAGIC_MAINWINDOW_MENUBAR_MENUITEMS_H
#define CDGMAGIC_MAINWINDOW_MENUBAR_MENUITEMS_H

#include "CDGMagic_Application.h"

Fl_Menu_Item MainWindow_MenuItems[] = {
    { "&File",                               0,     0, 0, FL_SUBMENU },
        { "&New Project",                    0,  NULL },
        { "&Open Project...",                0,  NULL, 0, FL_MENU_DIVIDER },
        { "&Save Project",                   0,  NULL, 0 },
        { "Save Project As...",              0,  NULL, 0, FL_MENU_DIVIDER },
        { "E&xit",                           0,  CDGMagic_Application::app_exit_callback, 0 },
        { 0 },
    { "&Tools",              0,    0, 0, FL_SUBMENU },
        { "&BMP Gradient to Transition...", 0,  CDGMagic_Application::bmp_to_transition_callback, 0 },
        { 0 },
/*
    { "&Import",              0,    0, 0, FL_SUBMENU },
        { "&Bitmap Image...", 0, NULL },
        { "&Palette...",      0, NULL },
        { "&Transition...",   0, NULL },
        { 0 },
*/
    { "&Window",      0,    0, 0, FL_SUBMENU },
        { "&Preview", 0, CDGMagic_Application::show_preview_window, 0 },
        { 0 },
    { "&Help",       0,    0, 0, FL_SUBMENU },
//        { "&Help",   0, NULL },
        { "&About",  0, CDGMagic_Application::app_about_callback, 0 },
        { 0 },
  { 0 }
};

#endif
