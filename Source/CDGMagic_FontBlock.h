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

#ifndef CDGMAGIC_FONTBLOCK_H
#define CDGMAGIC_FONTBLOCK_H

#include <cstdio>

#include <algorithm>
#include <vector>

class CDGMagic_FontBlock
{

struct Palette_Entry
{
    unsigned char index;
    unsigned char occurrence;
};


private:
    int internal_start_pack;
    int x_block, y_block;
    unsigned char internal_vram_only;
    unsigned char internal_xor_only;
    unsigned char z_index;
    unsigned char internal_channel;
    signed short internal_transparent_index;
    signed short internal_overlay_index;
    unsigned char *internal_bmp_data;
    unsigned char numcolors_is_dirty;
    unsigned char number_of_colors;
    unsigned char prom_colors_is_dirty;
    unsigned char *prominence_of_colors;

	static bool color_sorter(Palette_Entry entry_a, Palette_Entry entry_b);

public:
    CDGMagic_FontBlock(int req_x = 0, int req_y = 0, int req_startpack = 0);
    ~CDGMagic_FontBlock();

    unsigned char num_colors();
    unsigned char prominent_color(unsigned char prominence = 0);
    unsigned char pixel_value(unsigned char req_x, unsigned char req_y);
    void pixel_value(unsigned char req_x, unsigned char req_y, unsigned char clr_val);
    signed short replacement_transparent_color() const;
    void replacement_transparent_color(signed short requested_transparent);
    signed short overlay_transparent_color() const;
    void overlay_transparent_color(signed short requested_overlay);
    unsigned char is_fully_transparent();
    void color_fill(unsigned char clr_val);
    int start_pack() const;
    void start_pack(int req_startpack);
    int x_location() const;
    void x_location(int req_x);
    int y_location() const;
    void y_location(int req_y);
    unsigned char z_location() const;
    void z_location(unsigned char req_z);
    unsigned char channel() const;
    void channel(unsigned char req_option);
    unsigned char vram_only() const;
    void vram_only(unsigned char req_option);
    unsigned char xor_only() const;
    void xor_only(unsigned char req_option);
};

#endif
