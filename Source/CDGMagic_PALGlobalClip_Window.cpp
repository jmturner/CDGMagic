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

#include "CDGMagic_PALGlobalClip_Window.h"

CDGMagic_PALGlobalClip_Window::CDGMagic_PALGlobalClip_Window(CDGMagic_PALGlobalClip *clip_to_edit, int X, int Y) : Fl_Double_Window(0, 0, 0)
{
    const char *textwindow_title      = "Global Graphics Commands";
    const int   textwindow_width      = 700;
    const int   textwindow_height     = 500;

    label( textwindow_title );
    size( textwindow_width, textwindow_height );
    position( X-w()/2, Y );

    // Main window should be resizeable.
    resizable(this);

    current_clip = clip_to_edit;
}

CDGMagic_PALGlobalClip_Window::~CDGMagic_PALGlobalClip_Window()
{
    current_clip->null_edit();
}

void CDGMagic_PALGlobalClip_Window::DoOK_callback(Fl_Widget *CallingWidget)
{
    printf("CDGMagic_TextClip_Window::DoOK_callback\n");
    Fl::delete_widget(this);
}

int CDGMagic_PALGlobalClip_Window::can_delete()
{
    show();
    return fl_choice("This clip has a window open for editing.\nDelete anyway?", "No (don't delete)", "Yes (delete this clip)", NULL);
}

void CDGMagic_PALGlobalClip_Window::update_current_values()
{
    // Placeholder.
}

void CDGMagic_PALGlobalClip_Window::update_preview()
{
    // Placeholder.
}

int CDGMagic_PALGlobalClip_Window::handle(int incoming_event)
{
    // Placeholder.
    return Fl_Double_Window::handle(incoming_event);
}
