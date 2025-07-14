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

/*
 *  Resize algorithm, currently used solely for timeline picons.
 *
 *  NOTE: This *was* formerly using a simple bilinear algorithm
 *  under a Code Project Open License.
 *
 *  That license, however,  is NOT GPL compatible.
 *  (Nor is it even OSI approved!)
 *
 *  So, for now, I've put back the original, lower quality
 *  nearest neighbor "algorithm" (if one can even call it that).
 *
 *  Again, this is only used for picons, so it's no big loss.
 *  The "bilinear" code was just barely superior.
 *
 *  TODO: Replace this with a decent Lanczos or Bicubic method
 *        using a GPL compatible license.
 *        Alternately, change it so that it only shrinks images
 *        (those small enough to display as-is are left alone)
 *        using a simple averaging algorithm, which might even
 *        be better and faster for picons...
 *
 */

#include <cstdio>

#include "CDGMagic_BMPObject.h"

void CDGMagic_BMPObject::bilinear_resize(unsigned char *out_img, int out_width, int out_height, int requested_depth)
{
    int rgb_depth = (requested_depth == 4) ? 4 : 3;  // If the requested_depth is 4 bytes per pixel, set it, otherwise 3 bytes per pixel.
    int in_width = internal_bmp_width;
    int in_height = internal_bmp_height;
    int max_src = in_width * in_height;
    int max_dest = out_width * out_height * rgb_depth;
    const unsigned char *in_temp = internal_bmp_data;
    double x_scale_inc = static_cast<double>(in_width)  / static_cast<double>(out_width);
    double y_scale_inc = static_cast<double>(in_height) / static_cast<double>(out_height);

    //printf("CDGMagic_BMPObject doing bilinear resize.\n");
    printf("CDGMagic_BMPObject doing nearest neighbor resize.\n");

    for (int y_px = 0; y_px < out_height; y_px++)
    {
        int src_y = static_cast<int>( static_cast<double>(y_px) * y_scale_inc );
        int src_y_offset = src_y * in_width;
        int dest_y_offset = y_px * out_width;

        for (int x_px = 0; x_px < out_width; x_px++)
        {
            int src_x = static_cast<int>( static_cast<double>(x_px) * x_scale_inc );
            int src_location = src_y_offset + src_x;
            int dest_location = (dest_y_offset + x_px) * rgb_depth;

            if      (src_location < 0)         { src_location = 0; }
            else if (src_location > max_src)   { src_location = max_src; };

            if      (dest_location < 0)        { dest_location = 0; }
            else if (dest_location > max_dest) { dest_location = max_dest; };

            unsigned long rgba_color = internal_palette->color( in_temp[src_location] );

            out_img[dest_location+0] = (rgba_color >> 030) & 0xFF;
            out_img[dest_location+1] = (rgba_color >> 020) & 0xFF;
            out_img[dest_location+2] = (rgba_color >> 010) & 0xFF;

            if (rgb_depth == 4)  { out_img[dest_location+3] = 0xFF; };
        };
    };
}
