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

#include "CDGMagic_TextClip_Window__Editor.h"

CDGMagic_TextClip_Window__Editor::CDGMagic_TextClip_Window__Editor(int X, int Y, int W, int H, const char *l) : Fl_Text_Editor(X, Y, W, H, l)
{
}

CDGMagic_TextClip_Window__Editor::~CDGMagic_TextClip_Window__Editor()
{
}

int CDGMagic_TextClip_Window__Editor::handle(int incoming_event)
{
    // This subclass just fires the registered callback after performing
    // default handling for common events that might change the cursor
    // position, to ensure the "cursor position" display is updated.

    // Perform the default action and store the return value.
    int return_value = Fl_Text_Editor::handle(incoming_event);

    // Fire the callback for (common) events that could change the cursor position.
    switch (incoming_event)
    {
        case    FL_DRAG:
        case FL_KEYDOWN:
        case    FL_PUSH: do_callback();
                         break;
        default: break; // Placate tools that insist on a default case :-).
    };

    // Return the value obtained from the default handler above.
    return return_value;
}
