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

int CDGMagic_GraphicsEncoder::write_fontblock(int current_position, CDGMagic_FontBlock *block_to_write)
{
//##### Ugh, this function "works", but is terribly messy, and uses a class global array (!!!). #####//
//##### MUST REFACTOR THIS... VERY, VERY BAD!! #####//

    // Prevent overrun of stream buffer.
    if (current_position+4 >= current_length)  { return current_position; };
//printf("write_fontblock at x:%i, y:%i, vram: %i\n", block_to_write->x_location(), block_to_write->y_location(), block_to_write->vram_only() );
    // Don't write if identical to current onscreen.
    if (  (block_to_write->vram_only() == 0)
       && (copy_compare_fontblock(block_to_write) == 0) )  { return current_position; };

    // Don't write packets past the confines of the screen.
    if (block_to_write->x_location() <   0)  { return current_position; };
    if (block_to_write->x_location() >= 50)  { return current_position; };
    if (block_to_write->y_location() <   0)  { return current_position; };
    if (block_to_write->y_location() >= 18)  { return current_position; };

    // Some debugging code...
    // If we have some kind of transparency set, the write could be screwed because
    // num_colors() and prominent_color() won't work the way we expect.
    // Transparency needs to be dealt with before we get to this point.
    // And yes, it would be better if this weren't so spaghetti/tightly coupled.
    if (block_to_write->replacement_transparent_color() < 256)
    {
        printf("UM, in __FILE__,__LINE__ ## replacement_transparent_color() should be >=256!!!\n");
    };
    if (block_to_write->overlay_transparent_color() < 256)
    {
        printf("UM, in __FILE__,__LINE__ ## overlay_transparent_color() should be >=256!!!\n");
    };

    // TEMP: Used to compute the actual number of packets comsumed for debugging.
    // int temp_orig_position = current_position;

    // Hacky hack for XOR karaoke highlighting.
    if (block_to_write->xor_only() > 0)
    {
        write_fontblock_single(XOR_FONT, block_to_write->channel(),
                               block_to_write->x_location(), block_to_write->y_location(),
                               0, block_to_write->xor_only(),
                               cdg_stream[current_position]);
        for (int y_pos = 0; y_pos < 12; y_pos++ )
        {
            int the_line = 0;
            for (int x_pos = 0; x_pos < 6; x_pos++ )
            {
                int pix_val = (block_to_write->pixel_value(x_pos, y_pos) > 0);
                the_line |= (pix_val << (5-x_pos));
            };
            cdg_stream[current_position].data[4+y_pos] = the_line;
        };
        // Increment the position counter by one.
        current_position++;
        // And return.
        return current_position;
    };

    // Blocks with one color only use one pack, with both colors set to that index.
    if (block_to_write->num_colors() == 1)
    {
        write_fontblock_single(COPY_FONT, block_to_write->channel(),
                               block_to_write->x_location(), block_to_write->y_location(),
                               block_to_write->prominent_color(), block_to_write->prominent_color(),
                               cdg_stream[current_position]);
        // Only one color, so set all data to 1s.
        int the_line = 0x3F;
        for (int y_pos = 0; y_pos < 12; y_pos++ )  { cdg_stream[current_position].data[4+y_pos] = the_line; };
        // Increment the position counter by one.
        current_position++;
    }
    // Blocks with two colors only use one packet, with 0 set to most used index, 1 set to the other index.
    else if (block_to_write->num_colors() == 2)
    {
        int colors_to_write[2];
        colors_to_write[0] = block_to_write->prominent_color(0);
        colors_to_write[1] = block_to_write->prominent_color(1);

        write_fontblock_single(COPY_FONT, block_to_write->channel(),
                               block_to_write->x_location(), block_to_write->y_location(),
                               colors_to_write[0], colors_to_write[1],
                               cdg_stream[current_position]);

        for (int y_pos = 0; y_pos < 12; y_pos++ )
        {
            int the_line = 0;
            for (int x_pos = 0; x_pos < 6; x_pos++ )
            {
                int pix_val = (block_to_write->pixel_value(x_pos, y_pos) == colors_to_write[1]);
                the_line |= (pix_val << (5-x_pos));
            };
            cdg_stream[current_position].data[4+y_pos] = the_line;
        };
        // Increment the position counter by one.
        current_position++;
    }
    // Blocks with three colors always use two packs...
    // First pack 0 is most used index, 1 is second or third color.
    // Second pack is XOR, which color 0 is index 0 for "don't change", and 1 is the result of color2 XOR color3.
    else if (block_to_write->num_colors() == 3)
    {
        int colors_to_write[3];
        colors_to_write[0] = block_to_write->prominent_color(1);
        colors_to_write[1] = block_to_write->prominent_color(0);
        colors_to_write[2] = block_to_write->prominent_color(2);

        write_fontblock_single(COPY_FONT, block_to_write->channel(),
                               block_to_write->x_location(), block_to_write->y_location(),
                               colors_to_write[0], colors_to_write[1],
                               cdg_stream[current_position]);

        for (int y_pos = 0; y_pos < 12; y_pos++ )
        {
            int the_line = 0;
            for (int x_pos = 0; x_pos < 6; x_pos++ )
            {
                int pix_val = (block_to_write->pixel_value(x_pos, y_pos) == colors_to_write[1]);
                pix_val |= (block_to_write->pixel_value(x_pos, y_pos) == colors_to_write[2]);
                the_line |= (pix_val << (5-x_pos));
            };
            cdg_stream[current_position].data[4+y_pos] = the_line;
        };
        // Increment the position counter by one.
        current_position++;
        write_fontblock_single(XOR_FONT, block_to_write->channel(),
                               block_to_write->x_location(), block_to_write->y_location(),
                               0x00, colors_to_write[1] ^ colors_to_write[2],
                               cdg_stream[current_position]);
        for (int y_pos = 0; y_pos < 12; y_pos++ )
        {
            int the_line = 0;
            for (int x_pos = 0; x_pos < 6; x_pos++ )
            {
                int pix_val = (block_to_write->pixel_value(x_pos, y_pos) == colors_to_write[2]);
                the_line |= (pix_val << (5-x_pos));
            };
            cdg_stream[current_position].data[4+y_pos] = the_line;
        };
        // Increment the position counter by one.
        current_position++;
    };
    // Return the current position, for colors = 1,2,3.
    if (block_to_write->num_colors() < 4)  { return current_position; };

    // NOW we know we've got to get creative, as we have more than three colors.
    // We need to do some calculations to determine the best (fastest) way to write to the screen.

    unsigned char colors_OR  = 0x00; // All colors ORed together.
    unsigned char colors_XOR = 0x00; // All colors XORed together.
    unsigned char colors_AND = 0xFF; // All colors ANDed together.

    unsigned char AND_bits  = 0x00; // Number of bits on in every used color.
    unsigned char OR_bits   = 0x00; // Number of bits on in any used color.
    unsigned char used_bits = 0x00; // Number of bits that need to be set individually.

    // Step through all the available colors, AND/OR/XORing the values together in their respective variables.
    for (int curr_idx = 0; curr_idx < block_to_write->num_colors(); curr_idx++)
    {
        colors_OR  |= block_to_write->prominent_color(curr_idx);
        colors_XOR ^= block_to_write->prominent_color(curr_idx);
        colors_AND &= block_to_write->prominent_color(curr_idx);
    };

    // Step through the place values, counting the occurences.
    for (int curr_bit = 0; curr_bit < 4; curr_bit++)
    {
        AND_bits += (colors_AND >> curr_bit) & 0x01;
        OR_bits  += (colors_OR  >> curr_bit) & 0x01;
    };

    used_bits = OR_bits - AND_bits;

    // If there's four colors, BUT all four bits are set and the indices are not XOR related,
    // then we write in three packs: color0,1/index0,color2/index0,color3 .
    // A common example would be index 1,2,4,8 = 0001,0010,0100,1000
    // This would be drawn as:
    // Pack 0: COPY 1,2
    // Pack 1:  XOR 0,6  [2 ^ 4 = 6]
    // Pack 2:  XOR 0,10 [2 ^ 8 = 10]
    if ((block_to_write->num_colors() == 4) && (used_bits > 2) && (colors_XOR != 0x00))
    {
        unsigned char colors_to_write[4];
        colors_to_write[0] = block_to_write->prominent_color(1);
        colors_to_write[1] = block_to_write->prominent_color(0);
        colors_to_write[2] = block_to_write->prominent_color(2);
        colors_to_write[3] = block_to_write->prominent_color(3);

        write_fontblock_single(COPY_FONT, block_to_write->channel(),
                               block_to_write->x_location(), block_to_write->y_location(),
                               colors_to_write[0], colors_to_write[1],
                               cdg_stream[current_position]);

        for (int y_pos = 0; y_pos < 12; y_pos++ )
        {
            int the_line = 0;
            for (int x_pos = 0; x_pos < 6; x_pos++ )
            {
                int pix_val = (block_to_write->pixel_value(x_pos, y_pos) == colors_to_write[1]);
                pix_val |= (block_to_write->pixel_value(x_pos, y_pos) == colors_to_write[2]);
                pix_val |= (block_to_write->pixel_value(x_pos, y_pos) == colors_to_write[3]);
                the_line |= (pix_val << (5-x_pos));
            };
            cdg_stream[current_position].data[4+y_pos] = the_line;
        };
        // Increment the position counter by one.
        current_position++;
        write_fontblock_single(XOR_FONT, block_to_write->channel(),
                               block_to_write->x_location(), block_to_write->y_location(),
                               0x00, colors_to_write[1] ^ colors_to_write[2],
                               cdg_stream[current_position]);
        for (int y_pos = 0; y_pos < 12; y_pos++ )
        {
            int the_line = 0;
            for (int x_pos = 0; x_pos < 6; x_pos++ )
            {
                int pix_val = (block_to_write->pixel_value(x_pos, y_pos) == colors_to_write[2]);
                the_line |= (pix_val << (5-x_pos));
            };
            cdg_stream[current_position].data[4+y_pos] = the_line;
        };
        // Increment the position counter by one.
        current_position++;
        write_fontblock_single(XOR_FONT, block_to_write->channel(),
                               block_to_write->x_location(), block_to_write->y_location(),
                               0x00, colors_to_write[1] ^ colors_to_write[3],
                               cdg_stream[current_position]);
        for (int y_pos = 0; y_pos < 12; y_pos++ )
        {
            int the_line = 0;
            for (int x_pos = 0; x_pos < 6; x_pos++ )
            {
                int pix_val = (block_to_write->pixel_value(x_pos, y_pos) == colors_to_write[3]);
                the_line |= (pix_val << (5-x_pos));
            };
            cdg_stream[current_position].data[4+y_pos] = the_line;
        };
        // Increment the position counter by one.
        current_position++;
    }
    // Write bitwise if there's five or more colors.
    // OR, if the colors weren't XORable, since we know by passing the checks above
    // that we must have at least four colors to get here.
    else if ( (block_to_write->num_colors() > 4) || (colors_XOR != 0x00) )
    {
        // Set the initial drawing type to COPY.
        unsigned char copy_type = COPY_FONT;

        // The color index place value.
        for (int pal_bit = 3; pal_bit >= 0; pal_bit--)
        {
            // If this bit place is NEVER set, then skip this loop interation.
            if ( ((colors_OR >> pal_bit) & 0x01) == 0x00 )  { continue; };
            // If this bit place is ALWAYS set, then skip this loop iteration.
            if ( ((colors_AND >> pal_bit) & 0x01) == 0x01 )  { continue; };

            // First color always starts as 0.
            unsigned char this_color_0 = 0x00;
            // Second color is set to the bit place value, which is 8,4,2,1.
            unsigned char this_color_1 = 0x01 << pal_bit;

            // If we're on the first iteration of the loop, and some bits are set in every color,
            // then OR those bits for the first COPY type index.
            // (Since we skip iterations which are always 1 with a continue statement above, this sets them along with another place value.)
            if ( (copy_type == COPY_FONT) && (colors_AND > 0x00) )  { this_color_0 |= colors_AND; this_color_1 |= colors_AND; };
            // Write out the header for the current block.
            write_fontblock_single(copy_type, block_to_write->channel(),
                                   block_to_write->x_location(), block_to_write->y_location(),
                                   this_color_0, this_color_1,
                                   cdg_stream[current_position]);
            // Set the drawing type to XOR after the first loop iteration.
            copy_type = XOR_FONT;
            // Loop through the pixels, setting the 6 bit line values.
            for (int y_pos = 0; y_pos < 12; y_pos++ )
            {
                int the_line = 0;
                for (int x_pos = 0; x_pos < 6; x_pos++ )
                {
                    int pix_val = (block_to_write->pixel_value(x_pos, y_pos) >> pal_bit) & 0x01;
                    the_line |= (pix_val << (5-x_pos));
                };
                cdg_stream[current_position].data[4+y_pos] = the_line;
            };
            // Advance the current position.
            current_position++;
        }; // End color index.
    }
    // Now we know we have four XOR related color indices.
    else
    {
        // Four XORable colors is easy, write any two, then the other two will XOR with each other.
        unsigned char colors_to_write[4];
        colors_to_write[0] = block_to_write->prominent_color(0);
        colors_to_write[1] = block_to_write->prominent_color(1);
        // This will always work with 4 colors.
        colors_to_write[2] = block_to_write->prominent_color(2) ^ block_to_write->prominent_color(0);
        // Temp debugging code to prove it.
        colors_to_write[3] = block_to_write->prominent_color(3) ^ block_to_write->prominent_color(1);
        if (colors_to_write[2] != colors_to_write[3])  { printf("FAILURE :: ##### In XOR write of 4 colors, XOR value was NOT THE SAME!!#####\n"); };

        write_fontblock_single(COPY_FONT, block_to_write->channel(),
                               block_to_write->x_location(), block_to_write->y_location(),
                               colors_to_write[1], colors_to_write[0],
                               cdg_stream[current_position]);
        // Loop through the pixels, setting the 6 bit line values.
        for (int y_pos = 0; y_pos < 12; y_pos++ )
        {
            int the_line = 0;
            for (int x_pos = 0; x_pos < 6; x_pos++ )
            {
                int pix_val  = (block_to_write->pixel_value(x_pos, y_pos) == colors_to_write[0]);
                    pix_val |= (block_to_write->pixel_value(x_pos, y_pos) == block_to_write->prominent_color(2));
                the_line |= (pix_val << (5-x_pos));
            };
            cdg_stream[current_position].data[4+y_pos] = the_line;
        };
        // Advance the current position.
        current_position++;

        write_fontblock_single(XOR_FONT, block_to_write->channel(),
                               block_to_write->x_location(), block_to_write->y_location(),
                               0x00, colors_to_write[2],
                               cdg_stream[current_position]);
        // Loop through the pixels, setting the 6 bit line values.
        for (int y_pos = 0; y_pos < 12; y_pos++ )
        {
            int the_line = 0;
            for (int x_pos = 0; x_pos < 6; x_pos++ )
            {
                int pix_val  = (block_to_write->pixel_value(x_pos, y_pos) == block_to_write->prominent_color(2));
                    pix_val |= (block_to_write->pixel_value(x_pos, y_pos) == block_to_write->prominent_color(3));
                the_line |= (pix_val << (5-x_pos));
            };
            cdg_stream[current_position].data[4+y_pos] = the_line;
        };
        // Advance the current position.
        current_position++;
    };
    // Return the new position.
    return current_position;
}

void CDGMagic_GraphicsEncoder::write_fontblock_single(int instruction, unsigned char channel,
                                                      int x_block, int y_block,
                                                      unsigned char color_one, unsigned char color_two,
                                                      CD_SCPacket &pack_to_write)
{
    pack_to_write.command = TV_GRAPHICS;
    pack_to_write.instruction = instruction;
    // Set the current "on" and "off" color.
    pack_to_write.data[0] = color_one | ((channel << 2) & 0x30);
    pack_to_write.data[1] = color_two | ((channel << 4) & 0x30);
    // Set the current x/y block location.
    pack_to_write.data[2] = y_block;
    pack_to_write.data[3] = x_block;
}
