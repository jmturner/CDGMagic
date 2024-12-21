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

#include "CDGMagic_FontBlock.h"

CDGMagic_FontBlock::CDGMagic_FontBlock(int req_x, int req_y, int req_startpack)
{
    x_block = req_x;  // Define the x position, in CDG block coordinates.
    y_block = req_y;  // Define the y position, in CDG block coordinates.
    internal_start_pack = req_startpack;  // The earliest temporal position to write this font block.

    internal_vram_only = 0;
    internal_xor_only  = 0;
    z_index = 0; // Z/compositing index defaults to 0 (first layer, or ignored).
    internal_channel = 0; // Channel is always zero by default.
    internal_transparent_index = 256; // 256 can't exist in index value, so all opaque by default.
    internal_overlay_index = 256;     // 256 can't exist in index value, so all opaque by default.
    number_of_colors = 1; // Obviously, can't be less than one color.
    numcolors_is_dirty = 1;   // Need to recalculate colors.
    prom_colors_is_dirty = 1; // Default number of colors is zero.

    // This might seem a bit silly, but it nearly doubles the size of the object,
    // and can save up to about 1MB per minute of images, if prominence is never called.
    prominence_of_colors = NULL; // Start off with NULL array, only allocate if needed.
    internal_bmp_data = new unsigned char[6*12];  // Allocate the storage for block data (4 or 8bit indexed color).
}

CDGMagic_FontBlock::~CDGMagic_FontBlock()
{
    delete[] internal_bmp_data;  // Clean up the data portion.
    delete[] prominence_of_colors; // Clean up the palette prominence.
}

int CDGMagic_FontBlock::start_pack() const { return internal_start_pack; }
void CDGMagic_FontBlock::start_pack(int req_startpack)  { internal_start_pack = req_startpack; }

int CDGMagic_FontBlock::x_location() const { return x_block; }
void CDGMagic_FontBlock::x_location(int req_x)  { x_block = req_x; }

int CDGMagic_FontBlock::y_location() const { return y_block; }
void CDGMagic_FontBlock::y_location(int req_y)  { y_block = req_y; }

unsigned char CDGMagic_FontBlock::z_location() const { return z_index; }
void CDGMagic_FontBlock::z_location(unsigned char req_z)  { z_index = req_z; }

unsigned char CDGMagic_FontBlock::channel() const { return internal_channel; }
void CDGMagic_FontBlock::channel(unsigned char req_channel)  { internal_channel = req_channel; }

unsigned char CDGMagic_FontBlock::vram_only() const { return internal_vram_only; }
void CDGMagic_FontBlock::vram_only(unsigned char req_option)  { internal_vram_only = req_option; }

unsigned char CDGMagic_FontBlock::xor_only() const { return internal_xor_only; }
void CDGMagic_FontBlock::xor_only(unsigned char req_option)  { internal_xor_only = req_option; }

signed short CDGMagic_FontBlock::replacement_transparent_color() const { return internal_transparent_index; }
void CDGMagic_FontBlock::replacement_transparent_color(signed short requested_transparent)
{
    numcolors_is_dirty = 1;
    prom_colors_is_dirty = 1;
    internal_transparent_index = requested_transparent;
}

signed short CDGMagic_FontBlock::overlay_transparent_color() const { return internal_overlay_index; }
void CDGMagic_FontBlock::overlay_transparent_color(signed short requested_overlay)
{
    numcolors_is_dirty = 1;
    prom_colors_is_dirty = 1;
    internal_overlay_index = requested_overlay;
}

unsigned char CDGMagic_FontBlock::is_fully_transparent()
{
    for (int px = 0; px < 6*12; px++)  {  if (internal_bmp_data[px] != internal_transparent_index) { return 0; }  };
    return 1;
}

void CDGMagic_FontBlock::pixel_value(unsigned char req_x, unsigned char req_y, unsigned char clr_val)
{
    if ((req_x < 6) && (req_y < 12))
    {
        // The colors *could* be dirty, so recompute.
        numcolors_is_dirty = 1;
        prom_colors_is_dirty = 1;
        // Set the requested pixel value.
        internal_bmp_data[req_x+req_y*6] = clr_val;
    }
    else
    {
        printf("CDGMagic_FontBlock: Requested set pixel was invalid! [x: %i, y: %i]\n", req_x, req_y);
    };
}

unsigned char CDGMagic_FontBlock::pixel_value(unsigned char req_x, unsigned char req_y)
{
    if ((req_x < 6) && (req_y < 12))
    {
        return internal_bmp_data[req_x+req_y*6];
    };
    printf("CDGMagic_FontBlock: Requested get pixel was invalid! [x: %i, y: %i]\n", req_x, req_y);
    return 0;
}

void CDGMagic_FontBlock::color_fill(unsigned char clr_val)
{
    numcolors_is_dirty = 1;
    prom_colors_is_dirty = 1;
    for (int pxl = 0; pxl < 6*12; pxl++)  { internal_bmp_data[pxl] = clr_val; };
}

unsigned char CDGMagic_FontBlock::num_colors()
{
    if ( numcolors_is_dirty )
    {
        // Initialize number of colors.
        number_of_colors = 0;
        // Create an array for color indices.
        unsigned char clrs[256];
        // Initialize all elements to 0.
        for (int px = 0; px < 256; px++)  { clrs[px] = 0; };
        // Iterate through the 6*12 block, incrementing the clrs index element for that color.
        for (int px = 0; px < 6*12; px++)  {  clrs[ internal_bmp_data[px] ]++;  };
        // Increment for each index that was present at least once.
        for (int px = 0; px < 256; px++)  { if ( clrs[px] > 0 )  { number_of_colors++; }; };
        // Set the number of colors to be clean.
        numcolors_is_dirty = 0;
    };
    // Return the number of colors.
    return number_of_colors;
}

unsigned char CDGMagic_FontBlock::prominent_color(unsigned char prominence)
{
    if ( prom_colors_is_dirty )
    {
        // On first pass through, allocate the array.
        if ( prominence_of_colors == NULL )
        {
            prominence_of_colors = new unsigned char[6*12]; // Different color for every pixel is max.
        };
        // Ensure we have a current color count.
        num_colors();
        // Create the vector list for sorting.
        std::vector<Palette_Entry> clrs_vector;
        // Create a temp palette entry structure.
        Palette_Entry current_pal_entry;
        // Create an array for color indices.
        unsigned char clrs[256];
        // Initialize all elements to 0.
        for (int idx = 0; idx < 256; idx++)  { clrs[idx] = 0; };
        // Iterate through the 6*12 block, incrementing the clrs index element for that color.
        for (int px = 0; px < 6*12; px++)  {  clrs[ internal_bmp_data[px] ]++;  };
        // If the color index was present at least once, then add it to the vector.
        for (int idx = 0; idx < 256; idx++)
        {
            if ( (clrs[idx] > 0) && (idx != internal_transparent_index) )
            {
                current_pal_entry.index      = idx;
                current_pal_entry.occurrence = clrs[idx];
                clrs_vector.push_back(current_pal_entry);
            };
        };
        // Sort the vector according to the prominence.
        std::sort(clrs_vector.begin(), clrs_vector.end(), color_sorter);
        // Make sure we don't overrun the array -- this shouldn't, theoretically, happen.
        if (clrs_vector.size() > 6*12)
        {
            printf("prominent_color would have overrun array: %i (Size: %i)\n", clrs_vector.size(), 6*12);
            return 0;
        };
        // Stuff the vector into the class array for prominence.
        for (unsigned int prm_idx = 0; prm_idx < clrs_vector.size(); prm_idx++)
        {
            prominence_of_colors[prm_idx] = clrs_vector.at(prm_idx).index;
        }
        // Prominent color array is no longer dirty.
        prom_colors_is_dirty = 0;
    };

    // Return the color index of the requested prominence.
    if (prominence < number_of_colors)  { return prominence_of_colors[prominence]; };
    // Or return 0 if the request was invalid (maybe should throw here?).
    printf("prominent_color was requested invalid prominence: %i\n", prominence);
    return 0;
}

bool CDGMagic_FontBlock::color_sorter(Palette_Entry entry_a, Palette_Entry entry_b)
{
    // Sort in descending order... Most used color is 0, etc...
    return (entry_a.occurrence > entry_b.occurrence);
}
