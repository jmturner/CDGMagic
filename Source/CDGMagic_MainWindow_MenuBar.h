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

#ifndef CDGMAGIC_MAINWINDOW_MENUBAR_H
#define CDGMAGIC_MAINWINDOW_MENUBAR_H

#include <FL/Fl.H>
#include <FL/Fl_Menu_Bar.H>

class CDGMagic_MainWindow_MenuBar : public Fl_Menu_Bar
{
private:

public:
    CDGMagic_MainWindow_MenuBar(int menu_width, int menu_height);
};

#endif
