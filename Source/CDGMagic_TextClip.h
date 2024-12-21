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

#ifndef CDGMAGIC_TEXTCLIP_H
#define CDGMAGIC_TEXTCLIP_H

#include <string>

#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Text_Buffer.H>

#include "CDGMagic_MediaClip.h"
#include "CDGMagic_MovableClipBox.h"
#include "CDGMagic_BMPObject.h"

struct CDGMagic_TextEvent_Info
{
    std::string text_string;
    short line_num;
    short word_num;
    short karaoke_type;
};

enum CDGMagic_TextEvent_KaraokeTypes
{
    TEXT_ONLY,
    KARAOKE_DRAW,
    KARAOKE_WIPE,
    KARAOKE_ERASE
};

enum CDGMagic_KaraokeModes
{
    KAR_MODE__TITLES = 0x00,  //   Title: Single page, multiline/overlay text mode.
    KAR_MODE__LYRICS = 0x01,  //  Lyrics: Multipage, single line text mode.
    KAR_MODE__5TLINE = 0x02,  // Karaoke: 5 Lines (36 pixels), Top    justified, Line-by-line display.
    KAR_MODE__5BLINE = 0x03,  // Karaoke: 5 Lines (36 pixels), Bottom justified, Line-by-line display.
    KAR_MODE__5TPAGE = 0x04,  // Karaoke: 5 Lines (36 pixels), Top    justified, Page-by-page display.
    KAR_MODE__5BPAGE = 0x05,  // Karaoke: 5 Lines (36 pixels), Bottom justified, Page-by-page display.
    KAR_MODE__8MLINE = 0x06,  // Karaoke: 8 Lines (24 pixels), Middle justified, Line-by-line display.
    KAR_MODE__8MPAGE = 0x07,  // Karaoke: 8 Lines (24 pixels), Middle justified, Page-by-page display.
    KAR_MODE__6MLINE = 0x08,  // Karaoke: 6 Lines (24 pixels), Middle justified, Line-by-line display.
    KAR_MODE__6MPAGE = 0x09,  // Karaoke: 6 Lines (24 pixels), Middle justified, Page-by-page display.
    KAR_MODE__84TLIN = 0x0A,  // Karaoke: 4 Lines (24 pixels), Top    justified, Line-by-line display. (For simultaneous duets, or pictures/info below lyrics.)
    KAR_MODE__84BLIN = 0x0B,  // Karaoke: 4 Lines (24 pixels), Bottom justified, Line-by-line display. (For simultaneous duets, or pictures/info above lyrics.)
    KAR_MODE__5BLNCT = 0x0C,  // Karaoke: 5 Lines (36 pixels, 3 colors), Top    justified, "Line Cut"  display.
    KAR_MODE__5BLNFD = 0x0D,  // Karaoke: 5 Lines (36 pixels, 3 colors), Top    justified, "Line Fade" display.
    KAR_MODE__7MLNCT = 0x0E,  // Karaoke: 7 Lines (24 pixels, 2 colors), Middle justified, "Line Cut"  display.
    KAR_MODE__7MLNFD = 0x0F   // Karaoke: 7 Lines (24 pixels, 2 colors), Middle justified, "Line Fade" display.
};

const int maximum_embedded_palettes = 0x09;

const unsigned long embedded_palettes[maximum_embedded_palettes][16] =
{
    { 0x00000000, 0xFFFFFF00, 0xFF000000, 0x00FF0000,
      0x0000FF00, 0xFFFF0000, 0xFF00FF00, 0x00FFFF00,
      0x44444400, 0x88888800, 0x88000000, 0x00880000,
      0x00008800, 0x88880000, 0x88008800, 0x00888800 },

    { 0x00777700, 0x00000000, 0xFFFFFF00, 0x33333300,
      0x00777700, 0xAA550000, 0x00FF0000, 0x11551100,
      0x00777700, 0x55555500, 0xFF770000, 0x55220000,
      0x00777700, 0x33338800, 0xFF000000, 0x55000000 },

    { 0x44446600, 0x00000000, 0xFFFFFF00, 0x44444400,
      0x44446600, 0x77777700, 0x00FF0000, 0x22882200,
      0x00000000, 0x22222200, 0x44444400, 0x66666600,
      0x88888800, 0xAAAAAA00, 0xCCCCCC00, 0xEEEEEE00 },

    { 0x44446600, 0x00000000, 0xEEEEEE00, 0x44444400,
      0x44446600, 0x77777700, 0xFFFF0000, 0x55551100,
      0x00000000, 0x22222200, 0x44444400, 0x66666600,
      0x88888800, 0xAAAAAA00, 0xCCCCCC00, 0xEEEEEE00 },

    { 0x44446600, 0x00000000, 0xFFFFFF00, 0x44444400,
      0x44446600, 0x88FF0000, 0x00000000, 0x55555500,
      0x00000000, 0x22222200, 0x44444400, 0x66666600,
      0x88888800, 0xAAAAAA00, 0xCCCCCC00, 0xEEEEEE00 },

    { 0x2222CC00, 0x00000000, 0xFFFFFF00, 0x44444400,
      0x2222CC00, 0xBB550000, 0xEEDD0000, 0x88774400,
      0x00000000, 0x22222200, 0x44444400, 0x66666600,
      0x88888800, 0xAAAAAA00, 0xCCCCCC00, 0xEEEEEE00 },

    { 0x00007700, 0x00000000, 0xFFFFFF00, 0x44444400,
      0x00007700, 0xDD000000, 0x00DD0000, 0x11331100,
      0x00000000, 0x22222200, 0x44444400, 0x66666600,
      0x88888800, 0xAAAAAA00, 0xCCCCCC00, 0xEEEEEE00 },

    { 0x00000000, 0x44444400, 0xFFFFFF00, 0x44444400,
      0x00000000, 0xAA550000, 0x00FF0000, 0x11551100,
      0x00000000, 0x22227700, 0xFFFF0000, 0x55551100,
      0x00000000, 0x22777700, 0xFF00FF00, 0x55115500 },

    { 0x00000000, 0x11118800, 0xFFFFFF00, 0x33333300,
      0xFFEE0000, 0x33220000, 0x00AAFF00, 0x00113300,
      0x00880000, 0x00220000, 0xFF444400, 0x33111100,
      0xFF22BB00, 0x33001100, 0xFFAA0000, 0x33110000 }
};

class CDGMagic_TextClip_Window; // Silly forward declaration due to circular header inclusion.

class CDGMagic_TextClip : public CDGMagic_MediaClip, public CDGMagic_MovableClipBox
{
protected:
    int handle(int incoming_event);
    void do_edit(int win_x, int win_y);
    int generate_new_mediaevent();
    int get_line_word_type_event(int requested_type, int requested_line, int requested_word);
    unsigned long get_line_transition_mask(int requested_line_number);
    void clean_up_words(int requested_line, int requested_word);
    void clean_up_lines(int requested_line);
    void destroy_event(int requested_event);
    void set_all_palettes();
    void set_all_linepalettes();

    unsigned char    internal_karaoke_mode;
    unsigned char    internal_lines_per_page;
    unsigned char    internal_page_mode;
    unsigned char    internal_highlight_mode;
    unsigned char    internal_square_size;
    unsigned char    internal_round_size;
    unsigned char    internal_foreground_index;
    unsigned char    internal_background_index;
    unsigned char    internal_outline_index;
    unsigned char    internal_box_index;
    unsigned char    internal_frame_index;
    unsigned char    internal_fill_index;
    unsigned char    internal_composite_index;
    int              internal_xor_bandwidth;
    int              internal_should_composite;
    int              internal_palette_number;
    int              internal_antialias_mode;
    int              internal_font_size;
    int              internal_font_index;
    Fl_Text_Buffer*  internal_text_buffer;
    CDGMagic_TextClip_Window* internal_edit_window;

public:
    CDGMagic_TextClip(int X, int Y, int W, int H);
    ~CDGMagic_TextClip();

              int serialize(char_vector* incoming_save_vector);
              int deserialize(char* incoming_save_data);

             void draw();
             void render_text_to_bmp();
             void render_karaoke_to_bmp();
             void null_edit();

    unsigned char karaoke_mode();
    void          karaoke_mode(unsigned char requested_mode);

    unsigned char karaoke_page_mode();
    unsigned char lines_per_page();

    unsigned char highlight_mode();
    void          highlight_mode(unsigned char requested_mode);

              int font_size();
             void font_size(int requested_size);

              int font_index();
             void font_index(int requested_index);

    unsigned char square_size();
    void          square_size(unsigned char requested_size);

    unsigned char round_size();
    void          round_size(unsigned char requested_size);

    unsigned char foreground_color();
             void foreground_color(unsigned char requested_index);

    unsigned char background_color();
             void background_color(unsigned char requested_index);

    unsigned char outline_color();
    void          outline_color(unsigned char requested_index);

    unsigned char box_color();
    void          box_color(unsigned char requested_index);

    unsigned char frame_color();
    void          frame_color(unsigned char requested_index);

    unsigned char fill_color();
    void          fill_color(unsigned char requested_index);

    unsigned char composite_color();
    void          composite_color(unsigned char requested_index);

    int           should_composite();
    void          should_composite(int requested_setting);

    int           xor_bandwidth();
    void          xor_bandwidth(int requested_bandwidth);

    int           antialias_mode();
    void          antialias_mode(int requested_mode);

    int           default_palette_number();
    void          default_palette_number(int requested_palette);

      const char* font_face();
             void font_face(const char* requested_face);

  Fl_Text_Buffer* text_buffer();
};

#endif
