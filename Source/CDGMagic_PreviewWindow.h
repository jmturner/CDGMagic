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

#ifndef CDGMAGIC_PREVIEWWINDOW_H
#define CDGMAGIC_PREVIEWWINDOW_H

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_RGB_Image.H>

#include "CDGMagic_GraphicsDecoder.h"

class CDGMagic_PreviewWindow : public Fl_Double_Window
{
private:
    Fl_Box *PreviewBox;
    Fl_RGB_Image *PreviewImage;
    int last_play_position;


protected:
    int handle(int event);

public:
    CDGMagic_PreviewWindow();
    ~CDGMagic_PreviewWindow();

    signed int Decode_Packs(const CD_SCPacket *pack_array, const int play_position);
    void Reset();
    void RefreshDisplay();

    CDGMagic_GraphicsDecoder *cdg_vm;
};

#endif

