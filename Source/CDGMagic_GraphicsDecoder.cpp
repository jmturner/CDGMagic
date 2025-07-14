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

#include <cstdio>
#include <cstring>        // For memset...

#include "CDGMagic_GraphicsDecoder.h"  // Class header.

CDGMagic_GraphicsDecoder::CDGMagic_GraphicsDecoder(const unsigned char color_order)
{
    rgba_order[0] = ((color_order>>6)&0x03)*010;  // Red shift.
    rgba_order[1] = ((color_order>>4)&0x03)*010;  // Green shift.
    rgba_order[2] = ((color_order>>2)&0x03)*010;  // Blue shift.
    rgba_order[3] = ((color_order>>0)&0x03)*010;  // Alpha shift.

    // TODO:  Support either RGB or RGBA via constructor options.
    //rgba_screen = new unsigned char [312*216*4];
    rgba_screen = new unsigned char [312*216*3];
    vram = new unsigned char[300*216];

    Reset();                 // Reset the machine to known state.
}

CDGMagic_GraphicsDecoder::~CDGMagic_GraphicsDecoder()
{
    delete[] vram;
    delete[] rgba_screen;
}

void CDGMagic_GraphicsDecoder::Reset()
{
    // Using memset for now... Some sources say std::fill might be a better choice in C++, though.
    //memset( rgba_screen, 0, 312*216*4*sizeof(char) );              // Clear the RGBA screen.
    memset( rgba_screen, 0, 312*216*3*sizeof(char) );              // Clear the RGBA screen.
    memset( vram, 0, 300*216*sizeof(char) );                       // Clear the VRAM.
    for (int idx=0; idx<16; idx++) { palette[idx] = 0xFF<<rgba_order[3]; }; // Clear the palette.

    dirty_screen = dirty_palette = dirty_border = 1; // Need update variables.
	h_offset = v_offset = border_color = 0;          // Offset and border variables.
    active_channels = 0x03;                          // Default channels (0 and 1);
}

void CDGMagic_GraphicsDecoder::QuickReset()
{
    dirty_screen = dirty_palette = dirty_border = 1; // Need update variables.
	h_offset = v_offset = 0;          // Offset variables.
}

signed int CDGMagic_GraphicsDecoder::decode_packs( const CD_SCPacket *pack_array, const int number )
{
    int commands_processed = 0;
    for ( int pack_num = 0; pack_num < number; pack_num++ )
    {
        const CD_SCPacket *this_pack = &pack_array[pack_num];
	    if ( (this_pack->command&0x3F) == TV_GRAPHICS )  // Check if we've got a graphics command.
	    {
            switch (this_pack->instruction&0x3F)        // Dispatch functions based on command.
            {
                case MEMORY_PRESET: proc_MEMORY_PRESET(this_pack); break;
                case BORDER_PRESET: proc_BORDER_PRESET(this_pack); break;
                case SET_TRANSPARENT: proc_SET_TRANSPARENT(this_pack); break;

                case LOAD_CLUT_LO:
                case LOAD_CLUT_HI: proc_LOAD_CLUT(this_pack); break;

                case COPY_FONT:
                case XOR_FONT: proc_WRITE_FONT(this_pack); break;

                case SCROLL_PRESET:
                case SCROLL_COPY: proc_DO_SCROLL(this_pack); break;
			}; // End of dispatching switch.
            commands_processed++;
		}; // End of command type check.
	}; // End of pack processing loop.

    return commands_processed;
}

unsigned char* CDGMagic_GraphicsDecoder::GetRGBAScreen()
{

#ifdef _CDG_DECODER_VIEWALL_
    // Debugging code -- disables border and offset to see full screen.
    for (int y_loc = 0; y_loc < 216; y_loc++)
    {
        for (int x_loc = 0; x_loc < 300; x_loc++)
        {
            int scr_loc = x_loc + (y_loc*300);
            //int dest_loc = ((x_loc+6)+(y_loc*312))*4;
            int dest_loc = ((x_loc+6)+(y_loc*312))*3;
            rgba_screen[dest_loc+0] = palette[vram[scr_loc]&0x0F] >> 030; // Red
            rgba_screen[dest_loc+1] = palette[vram[scr_loc]&0x0F] >> 020; // Green
            rgba_screen[dest_loc+2] = palette[vram[scr_loc]&0x0F] >> 010; // Blue
            //rgba_screen[dest_loc+3] = palette[vram[scr_loc]&0x0F] & 0xFF; // Alpha
        };
    };
#else
    for (int y_loc = 12; y_loc < 204; y_loc++)
    {
        for (int x_loc = 6; x_loc < 294; x_loc++)
        {
            int scr_loc = (x_loc+h_offset) + ((y_loc+v_offset)*300);
            //int dest_loc = ((x_loc+6)+(y_loc*312))*4;
            int dest_loc = ((x_loc+6)+(y_loc*312))*3;
            rgba_screen[dest_loc+0] = palette[vram[scr_loc]&0x0F] >> 030; // Red
            rgba_screen[dest_loc+1] = palette[vram[scr_loc]&0x0F] >> 020; // Green
            rgba_screen[dest_loc+2] = palette[vram[scr_loc]&0x0F] >> 010; // Blue
            //rgba_screen[dest_loc+3] = palette[vram[scr_loc]&0x0F] & 0xFF; // Alpha
        };
    };

    if ( dirty_border || dirty_palette )
    {
        proc_fill_RGB(  0,   0, 312,  12, border_color); // Top border.
        proc_fill_RGB(  0, 204, 312,  12, border_color); // Bottom border.
        proc_fill_RGB(  0,  12,  12, 192, border_color); // Left border.
        proc_fill_RGB(300,  12,  12, 192, border_color); // Right border.
    };
#endif
    dirty_screen = dirty_palette = dirty_border = 0;
    return rgba_screen;
}

unsigned char* CDGMagic_GraphicsDecoder::GetVRAM()
{
    return vram;
}

unsigned long* CDGMagic_GraphicsDecoder::GetPalette()
{
    //palette[border_color] = 0x00100010; // Quick hack for overlay transparency.
    return palette;
}

unsigned char CDGMagic_GraphicsDecoder::GetBorderIndex() const
{
    return border_color;
}

unsigned int CDGMagic_GraphicsDecoder::GetOffset(const unsigned int h_or_v) const
{
    // Return the requested directional offset (or 0 if unknown).
    if ( h_or_v == H_OFFSET )  { return h_offset; }
    else if ( h_or_v == V_OFFSET )  { return v_offset; };
    return 0;
}

void CDGMagic_GraphicsDecoder::proc_MEMORY_PRESET(const CD_SCPacket *cdg_pack)
{
    for (int the_pix = 0; the_pix < 300*216; the_pix++)  { vram[the_pix] = (cdg_pack->data[0] & 0x0F); };
    dirty_screen = 1;
}

void CDGMagic_GraphicsDecoder::proc_LOAD_CLUT(const CD_SCPacket *cdg_pack)
{
    // If instruction is 0x1E then 8*0=0, if 0x1F then 8*1=8 for offset.
    int pal_offset = (cdg_pack->instruction&0x01) * 8;

    for (int pal_inc = 0; pal_inc < 8; pal_inc++)
    {
        // Set the temp value to the current alpha level.
        unsigned long temp_rgb = 0x80<<rgba_order[3];//palette[pal_inc+pal_offset]&(0xFF<<rgba_order[3]);
        unsigned long temp_entry = 0x00000000;
        // Set red.
        temp_entry = (cdg_pack->data[pal_inc*2]&0x3C) >> 2;
        temp_rgb |= (temp_entry * 17) << rgba_order[0];
        // Set green.
        temp_entry = ((cdg_pack->data[pal_inc*2]&0x03)<<2) | ((cdg_pack->data[pal_inc*2+1]&0x30)>>4);
        temp_rgb |= (temp_entry * 17) << rgba_order[1];
        // Set blue.
        temp_entry = (cdg_pack->data[pal_inc*2+1]&0x0F);
        temp_rgb |= (temp_entry * 17) << rgba_order[2];
        // If the current color is different from previous, then update it.
        if ( palette[pal_inc+pal_offset] != temp_rgb )
		{
            palette[pal_inc+pal_offset] = temp_rgb;
			dirty_palette = 1;
		};
    };
}

void CDGMagic_GraphicsDecoder::proc_SET_TRANSPARENT(const CD_SCPacket *cdg_pack) const
{
/*
    // Step through the 16 colors to set transparency.
    for (int pal_inc = 0; pal_inc < 16; pal_inc++)
    {
        // Get the 6bit alpha value for the current CLUT entry.
        // Invert it because 0x00 == fully opaque, 0x3F == transparent.
        unsigned char alpha_value = ((cdg_pack->data[pal_inc])&0x3F)*4;
        // Clamp it high if it should have been fully opaque.
        if ( alpha_value == 0xFC )  { alpha_value = 0xFF; };
        // Get the current palette value, and remove the alpha.
        unsigned long temp_rgb = palette[pal_inc]&(~(0xFF << rgba_order[3]));
        // OR it to set the new alpha value.
        temp_rgb |= (alpha_value << rgba_order[3]);
        // Check if it the same, and update if not.
        if ( palette[pal_inc] != temp_rgb )
		{
            palette[pal_inc] = temp_rgb;
			dirty_palette = 1;
		};
    };
*/
}

void CDGMagic_GraphicsDecoder::proc_WRITE_FONT(const CD_SCPacket *cdg_pack)
{
    // First, get the channel...
    int subcode_channel = ((cdg_pack->data[0] & 0x30) >> 2) | ((cdg_pack->data[1] & 0x30) >> 4);
    int xor_var = cdg_pack->instruction & 0x20;
    // Then see if we should display it.
    if ( (active_channels>>subcode_channel)&0x01 )
    {
        int x_location = cdg_pack->data[3] & 0x3F; // Get horizontal font location.
        int y_location = cdg_pack->data[2] & 0x1F; // Get vertical font location.

        // Verify we're not going to overrun the boundaries (i.e. bad data from a scratched disc).
        if ( (x_location<=49) && (y_location<=17) )
        {
            int start_pixel = (x_location*6) + (y_location*3600);
            for (int y_inc = 0; y_inc < 12; y_inc++)
            {
                int pix_pos = start_pixel + (y_inc * 300);
                for (int x_dec = 5; x_dec >= 0; x_dec--)
                {
                    int bit_set = (cdg_pack->data[y_inc+4] >> x_dec) & 0x01;
                    if ( xor_var ) { vram[pix_pos++] ^= (cdg_pack->data[bit_set] & 0x0F); }
                    else           { vram[pix_pos++]  = (cdg_pack->data[bit_set] & 0x0F); };
                }; // End of X loop.
            }; // End of Y loop.
        }; // End of location check.
    }; // End of channel check.
}

void CDGMagic_GraphicsDecoder::proc_BORDER_PRESET(const CD_SCPacket *cdg_pack)
{
    if ( (cdg_pack->data[0]&0x0F) != border_color )  // Check that new color is different.
	{
        border_color = (cdg_pack->data[0] & 0x0F);   // Set the new border color.
        dirty_border = 1;                            // Border (or screen) needs update.
    };
}

void CDGMagic_GraphicsDecoder::proc_DO_SCROLL(const CD_SCPacket *cdg_pack)
{
    // Set the X (horizontal) offset... Make sure it's in the range 0-5.
    if ( (cdg_pack->data[1]&0x07) < 6 )  { h_offset = (cdg_pack->data[1] & 0x07); };
    // Set the Y (vertical) offset... Make sure it's in the range 0-11.
    if ( (cdg_pack->data[2]&0x0F) < 12 ) { v_offset = (cdg_pack->data[2] & 0x0F); };

    int direction = 0;
    int color = cdg_pack->data[0]&0x0F;
    int copy_flag = (cdg_pack->instruction&0x08) >> 3;   // Determine type (memory preset or copy).

    // Process horizontal commands.
    if ( (direction = ((cdg_pack->data[1]&0x30) >> 4)) )  { proc_VRAM_HSCROLL( direction, copy_flag, color ); };
    // Process vertical commands.
    if ( (direction = ((cdg_pack->data[2]&0x30) >> 4)) )  { proc_VRAM_VSCROLL( direction, copy_flag, color ); };

    dirty_screen = 1;
}

void CDGMagic_GraphicsDecoder::proc_fill_RGB(int x_start, int y_start, int x_len, int y_len, unsigned int color)
{
    for (int y_loc = (y_start*312); y_loc < ((y_start+y_len)*312); y_loc+=312)
    {
        for (int x_loc = x_start; x_loc < (x_start+x_len); x_loc++)
        {
            //int dest_loc = (x_loc+y_loc)*4;
            int dest_loc = (x_loc+y_loc)*3;
            rgba_screen[dest_loc+0] = palette[color&0x0F] >> 030; // Red
            rgba_screen[dest_loc+1] = palette[color&0x0F] >> 020; // Green
            rgba_screen[dest_loc+2] = palette[color&0x0F] >> 010; // Blue
            //rgba_screen[dest_loc+3] = palette[color&0x0F] & 0xFF; // Alpha
        };
    };
}

void CDGMagic_GraphicsDecoder::proc_VRAM_HSCROLL(int direction, int copy_flag, int color)
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

void CDGMagic_GraphicsDecoder::proc_VRAM_VSCROLL(int direction, int copy_flag, int color)
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
