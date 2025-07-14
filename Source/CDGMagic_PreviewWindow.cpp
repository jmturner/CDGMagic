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

#include "CDGMagic_PreviewWindow.h"

CDGMagic_PreviewWindow::CDGMagic_PreviewWindow() : Fl_Double_Window(0, 0, 0)
{
    const char *previewwindow_title  = "CD+G Preview";
    const int   previewwindow_width  = 312;
    const int   previewwindow_height = 216;

    label( previewwindow_title );
    size( previewwindow_width, previewwindow_height );

    // Window should be only a child of the main application window (non-modal).
    set_non_modal();

    begin();

        // Create a new GraphicsDecoder object.
        cdg_vm = new CDGMagic_GraphicsDecoder(CDGMagic_GraphicsDecoder::RGBA_ORDER);
        // Create a new 24bit image and set the pointer to the CDG screen data pointer.
        PreviewImage = new Fl_RGB_Image( cdg_vm->GetRGBAScreen(), 312, 216, 3, 0 );
        // Create a new empty box and associate the image with it.
        PreviewBox = new Fl_Box(0, 0, 312, 216);
        PreviewBox->box(FL_NO_BOX);
        PreviewBox->image(PreviewImage);

    end();

    last_play_position = 0;
}

CDGMagic_PreviewWindow::~CDGMagic_PreviewWindow()
{
    delete PreviewImage;
    delete cdg_vm;
}

signed int CDGMagic_PreviewWindow::Decode_Packs(const CD_SCPacket *pack_array, const int play_position)
{
    if (last_play_position < 0)  { last_play_position = 0; };  // This shouldn't be possible, but check just in case.
    int to_dec = play_position - last_play_position;           // Calculate how many pack we need to play.
    // Check if this time is prior to where we are, and reset if so.
    if (to_dec < 0)  { Reset(); to_dec = play_position; };
#ifdef _PREVWIN_DEBUG_
    printf("old: %i, dec: %i, new: %i\n", last_play_position, to_dec, play_position);
#endif
    const signed int return_value = cdg_vm->decode_packs( &pack_array[last_play_position], to_dec );
    RefreshDisplay();
    last_play_position = play_position;
    return return_value;
}

void CDGMagic_PreviewWindow::Reset()
{
    last_play_position = 0;
    cdg_vm->Reset();
    RefreshDisplay();
}

void CDGMagic_PreviewWindow::RefreshDisplay()
{
    cdg_vm->GetRGBAScreen();
    PreviewImage->uncache();
    PreviewBox->redraw();
}

int CDGMagic_PreviewWindow::handle(int incoming_event)
{
    static int orig_x = 0, orig_y = 0;

    switch ( incoming_event )
    {
        case FL_PUSH:
            if (Fl::event_button() != FL_LEFT_MOUSE) { break; };
            orig_x = Fl::event_x_root() - x();
            orig_y = Fl::event_y_root() - y();
            return 1;

        case FL_DRAG:
            if (Fl::event_button() != FL_LEFT_MOUSE) { break; };
            position(Fl::event_x_root()-orig_x, Fl::event_y_root()-orig_y);
            return 1;
    };

    return Fl_Double_Window::handle(incoming_event);
}
