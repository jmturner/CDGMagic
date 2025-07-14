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

#ifndef CDGMAGIC_PALGLOBALCLIP_WINDOW_H
#define CDGMAGIC_PALGLOBALCLIP_WINDOW_H

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>

#include "CDGMagic_PALGlobalClip.h"

class CDGMagic_PALGlobalClip_Window : public Fl_Double_Window
{
private:
    CDGMagic_PALGlobalClip*   current_clip;
    void update_current_values();

protected:
    int handle(int event);

public:
    CDGMagic_PALGlobalClip_Window(CDGMagic_PALGlobalClip *clip_to_edit, int X, int Y);
    ~CDGMagic_PALGlobalClip_Window();

    void        DoOK_callback(Fl_Widget *CallingWidget);
    void        update_preview();
    int can_delete();
};

#endif
