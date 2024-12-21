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

#include "CDGMagic_GraphicsEncoder.h"

/*
copy_fontblock_to_layer(int layer, CDGMagic_FontBlock* block_to_copy)
{



};

        for (int x_blk = 0; x_blk < 50; x_blk++)
        {
          int comp_block_offset =  x_blk * 6 + src_y_blk * comp_width * 12;
          tmp_block->x_location( x_blk );
          tmp_block->y_location( 0 );
          for (int y_px = 0; y_px < 12; y_px++)
          {
            for (int x_px = 0; x_px < 6; x_px++)
            {
                // Set the current pixel starting offset for the block location.
                int pixel_offset = comp_block_offset + x_px + y_px * comp_width;
                tmp_block->pixel_value(x_px, y_px, last_preset_index);
                for (int z_loc = 0; z_loc < 8; z_loc++)
                {
                    // Set the layer offset.
                    // NOTE: Track, which ranges from 0 to 7, offsets are thus compositing area * track + pixel_offset.
                    int layer_offset = comp_width * comp_height * z_loc + pixel_offset;
                    // If the pixel value of the current layer at the current location is less than 256, it's opaque.
                    // So set the value of the incoming block's pixel to that value instead.
                    if ( comp_buffer[layer_offset] < 256 )  {  tmp_block->pixel_value(x_px, y_px, comp_buffer[layer_offset]);  };
                };
            };
          };

copy_composite_to_vram
*/

int CDGMagic_GraphicsEncoder::copy_fontblock_to_vram(CDGMagic_FontBlock* given_block)
{
    // Store the original requested screen block coordinates.
    int x_location = given_block->x_location();
    int y_location = given_block->y_location();

    // Return an error if we were beyond the VRAM bounds.
    if ( (x_location < 0) || (x_location >= 50) ) { return 0; };
    if ( (y_location < 0) || (y_location >= 18) ) { return 0; };

    // Copy the block to the VRAM screen.
    // Calculate the initial offset to the top left pixel of the block.
    int vram_block_offset = x_location * 6 + y_location * VRAM_WIDTH * 12;
    // Run through the lines/Y.
    for (int y_pix = 0; y_pix < 12; y_pix++)
    {
        // Calculate the location to the left most pixel of the current line.
        int line_offset = vram_block_offset + y_pix * comp_width;
        // Run through the columns/X.
        for (int x_pix = 0; x_pix < 6; x_pix++)
        {
            // Set the current pixel offset.
            int pixel_offset = line_offset + x_pix;
            // Now set the pixel index.
            vram[pixel_offset] = given_block->pixel_value(x_pix, y_pix);
        };
    };
    // Return success.
    return 1;
}

int CDGMagic_GraphicsEncoder::get_composited_fontblock(CDGMagic_FontBlock* return_block)
{
    // Store the original requested compositing block coordinates.
    int x_location = return_block->x_location();
    int y_location = return_block->y_location();
    // Determine the furthest right/bottom block, so we don't go past the buffer.
    int max_x_block = comp_width /  6;
    int max_y_block = comp_height / 12;

    // The compositing screen (or canvas, etc.) can be larger than the actual VRAM screen,
    // which is fixed to 300x216 (50x18 blocks)... This is to accommodate arbitrary scrolling.
    // The block passed to this function should use COMPOSITING BLOCK COORDINATES...
    // This is later translated to SCREEN (VRAM) BLOCK COORDINATES (using current offsets) for display.

    // Now, see if we're inside the compositing screen...
    // If not, we just fill the block with our "virtual layer 0", which is the last used memory preset index.
    if  ( (x_location >= 0) && (x_location < max_x_block)
       && (y_location >= 0) && (y_location < max_y_block) )
    {
//printf("get_composited_fontblock x: %i, y: %i\n", x_location, y_location);
        // Calculate the initial offset to the top left pixel of the block.
        int comp_block_offset = x_location * 6 + y_location * comp_width * 12;
        // Calculate the span between layers.
        int layer_span = comp_width * comp_height;
        // Run through the lines/Y.
        for (int y_pix = 0; y_pix < 12; y_pix++)
        {
            // Calculate the location to the left most pixel of the current line.
            int line_offset = comp_block_offset + y_pix * comp_width;
            // Run through the columns/X.
            for (int x_pix = 0; x_pix < 6; x_pix++)
            {
                // Set the current pixel offset.
                int pixel_offset = line_offset + x_pix;
                // Now step through the layers, starting with the last used MEMORY PRESET (which is a "virtual layer 0").
                // Always setting the top most index if it's not transparent.
                return_block->pixel_value(x_pix, y_pix, last_preset_index);
                for (int z_loc = 0; z_loc < COMP_LAYERS; z_loc++)
                {
                    int layer_offset = layer_span * z_loc + pixel_offset;
                    // If the pixel value of the current layer at the current location is less than 256, it's opaque.
                    // So set the value of the incoming block's pixel to that value instead.
                    if ( comp_buffer[layer_offset] < 256 )  {  return_block->pixel_value(x_pix, y_pix, comp_buffer[layer_offset]);  };
                };
            };
        };
    }
    else
    {
        // We were outside the bounds of the compositor, so just fill the block with the preset index.
        return_block->color_fill( last_preset_index );
    };
    // Always returns one for now... Could add some status/errors checks if needed.
    return 1;
}


// ########## Process scrolls on the buffer vram. ########## //
// ########## This code comes from the decoder, maybe see if it can be unduplicated. ########## //

void CDGMagic_GraphicsEncoder::proc_VRAM_HSCROLL(int direction, int copy_flag, int color)
{
    unsigned char buf[6];
    if ( direction == 0x02 )
    {
        // Step through the lines one at a time... Each is 300 bytes long.
        for (int y_src = 0; y_src < (300*216); y_src += 300)
        {
            // Copy the first six pixels to the buffer.
            for (int x_src = 0; x_src < 6; x_src++)  { buf[x_src] = vram[x_src+y_src]; };
            int dst = y_src; // Destination starts at the first line
            for (int x_src = 6; x_src < 300; x_src++)  { vram[dst++] = vram[x_src+y_src]; };
            if ( copy_flag )  { for (int x_src = 0; x_src < 6; x_src++)  { vram[dst++] = buf[x_src]; }; }
            else              { for (int x_src = 0; x_src < 6; x_src++)  { vram[dst++] = color; }; };
        };
    }
    else if ( direction == 0x01 )
    {
        // Step through the lines on at a time.
        for (int y_src = 0; y_src < (300*216); y_src += 300)
        {
            // Copy the first six lines to the buffer.
            for (int x_src = 0; x_src < 6; x_src++)  { buf[x_src] = vram[x_src+y_src+294]; };
            int dst = y_src+299;
            for (int x_src = 0; x_src < 294; x_src++)  { vram[dst] = vram[dst-6]; dst--; };
            dst = y_src;
            if ( copy_flag )  { for (int x_src = 0; x_src < 6; x_src++)  { vram[dst++] = buf[x_src]; }; }
            else              { for (int x_src = 0; x_src < 6; x_src++)  { vram[dst++] = color; }; };
        };
    };
}

void CDGMagic_GraphicsEncoder::proc_VRAM_VSCROLL(int direction, int copy_flag, int color)
{
    unsigned char buf[300*12];
    if ( direction == 0x02 )
    {
        int dst_idx = 0;  // Buffer destination starts at 0.
        // Copy the top 300x12 pixels into the buffer.
        for (int src_idx = 0; src_idx < (300*12); src_idx++)  { buf[dst_idx++] = vram[src_idx]; };

        dst_idx = 0; // Destination starts at the first line.
        for (int src_idx = (300*12); src_idx < (300*216); src_idx++)  { vram[dst_idx++] = vram[src_idx]; };

        dst_idx = (300*204); // Destination begins at line 204.
        if ( copy_flag )
        {
            for (int src_idx = 0; src_idx < (300*12); src_idx++)  { vram[dst_idx++] = buf[src_idx]; };
        }
        else
        {
            for (int src_idx = 0; src_idx < (300*12); src_idx++)  { vram[dst_idx++] = color; };
        };
    }
    else if ( direction == 0x01 )
    {
        int dst_idx = 0;  // Buffer destination starts at 0.
        // Copy the bottom 300x12 pixels into the buffer.
        for (int src_idx = (300*204); src_idx < (300*216); src_idx++)  { buf[dst_idx++] = vram[src_idx]; };

        for (int src_idx = (300*204)-1; src_idx > 0; src_idx--)  { vram[src_idx+(300*12)] = vram[src_idx]; };

        if ( copy_flag )
        {
            for (int src_idx = 0; src_idx < (300*12); src_idx++)  { vram[src_idx] = buf[src_idx]; };
        }
        else
        {
            for (int src_idx = 0; src_idx < (300*12); src_idx++)  { vram[src_idx] = color; };
        };
    };
}
