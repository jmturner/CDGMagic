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

#include "CDGMagic_TextClip_Window__CtrlButton.h"

CDGMagic_TextClip_Window__CtrlButton::CDGMagic_TextClip_Window__CtrlButton(int X, int Y, int W, int H, const char *l) : Fl_Button(X, Y, W, H, l)
{
}

CDGMagic_TextClip_Window__CtrlButton::~CDGMagic_TextClip_Window__CtrlButton()
{
}

int CDGMagic_TextClip_Window__CtrlButton::handle(int incoming_event)
{
    // This subclass just fires the registered callback when the CTRL Key is pressed.
    // Passing all events on as normal.
    // This is to allow a single CTRL key press to set the duration of word highlighting.
    // It formerly required CTRL+Space, which can be somwhat awkward in faster songs.

    // Fire the callback if the CTRL key was pressed.
    if ( (incoming_event == FL_KEYDOWN) && ((Fl::event_key() == FL_Control_R) || (Fl::event_key() == FL_Control_L)) )
    {
        printf("Got CTRL on CDGMagic_TextClip_Window__CtrlButton widget.\n");
        do_callback();
        set_changed();
        simulate_key_action();
        return 1;
    };

    // Return the value obtained from the default handler.
    return Fl_Button::handle(incoming_event);
}
