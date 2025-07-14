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

#ifndef CDGMAGIC_BMPOBJECT_H
#define CDGMAGIC_BMPOBJECT_H

#include "CDGMagic_PALObject.h"

class CDGMagic_BMPObject
{
protected:
    unsigned int        internal_bmp_width;
    unsigned int        internal_bmp_height;
    signed int          internal_x_offset;
    signed int          internal_y_offset;
    unsigned short      internal_transition_length;
    unsigned char*      internal_transition_blocks;
    char*               internal_transition_file;
    CDGMagic_PALObject* internal_palette;
    unsigned char       internal_xor_only;
    int                 internal_should_composite;
    unsigned char       internal_composite_index;
    unsigned char       internal_fill_index;
    unsigned char*      internal_bmp_data;
    float               internal_xor_bandwidth;
    unsigned int        internal_draw_delay;

public:
    CDGMagic_BMPObject(int requested_width = 0, int requested_height = 0);
    ~CDGMagic_BMPObject();
    signed int           width() const;
    signed int           height() const;
    unsigned char        fill_index() const;
    void                 fill_index(unsigned char requested_index);
    unsigned char        xor_only() const;
    void                 xor_only(unsigned char requested_setting);
    int                  should_composite() const;
    void                 should_composite(int requested_setting);
    unsigned char        composite_index() const;
    void                 composite_index(unsigned char requested_index);
    float                xor_bandwidth() const;
    void                 xor_bandwidth(float requested_bandwidth);
    CDGMagic_PALObject*  PALObject();
    unsigned long        get_rgb_pixel(unsigned int requested_x, unsigned int requested_y);
    unsigned char        linear_pixel(unsigned int requested_pixel) const;
    void                 linear_pixel(unsigned int requested_pixel, unsigned char requested_index);
    unsigned char        pixel(unsigned int requested_x, unsigned int requested_y) const;
    void                 pixel(unsigned int requested_x, unsigned int requested_y, unsigned char requested_index);
    void                 alter_buffer_size(unsigned int requested_width, unsigned int requested_height);
    void                 bilinear_resize(unsigned char *out_img, int out_width, int out_height, int requested_depth = 3);
    int                  x_offset() const;
    void                 x_offset(int requested_x_offset);
    int                  y_offset() const;
    void                 y_offset(int requested_y_offset);
    unsigned short       transition_length() const;
    unsigned char        transition_block(unsigned short requested_block_num, unsigned char x_or_y) const;
    unsigned char        transition_file(const char* selected_file);
    const char*          transition_file() const;
    void                 transition_row_mask(unsigned long requested_row_mask);
    int                  draw_delay() const;
    void                 draw_delay(int requested_delay);

    virtual const char*  file_path(); // Implemented in file reading subclasses... But a BMPObject could also just be rendered (no file).
};

#endif
