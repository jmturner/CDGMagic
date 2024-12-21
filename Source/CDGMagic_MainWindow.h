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

#ifndef CDGMAGIC_MAINWINDOW_H
#define CDGMAGIC_MAINWINDOW_H

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Box.H>

#include "CDGMagic_EditingGroup.h"
#include "CDGMagic_PreviewWindow.h"

class CDGMagic_MainWindow : public Fl_Double_Window
{
private:
    CDGMagic_EditingGroup *editing_group;

public:
    CDGMagic_MainWindow(void);

    void set_prev(CDGMagic_PreviewWindow *prev_win);

};

#endif
