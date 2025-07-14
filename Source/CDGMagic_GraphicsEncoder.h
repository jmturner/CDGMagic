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

#ifndef CDGMAGIC_GRAPHICSGENERATOR_H
#define CDGMAGIC_GRAPHICSGENERATOR_H

#include <cstring>
#include <algorithm>
#include <deque>

#include "CDGMagic_FontBlock.h"
#include "CDGMagic_MediaClip.h"
#include "CDGMagic_FontBlock.h"

// Some structures to make life easier...
// Mostly recommended in the very old Jim Bumgardner "CD+G Revealed" document.
// Straightforward enough, so no need to really change them.

// 24 byte Compact Disc Subcode Packet.
#ifndef CD_SCPACKET_STRUCT
#define CD_SCPACKET_STRUCT
typedef struct CD_SCPacket
{                          // COMMAND is HI three bits for MODE, LO three bits for ITEM.
    char     command;      // MODE is always 1 for graphics, item can be 0, 1, or 2 depending on type.
    char instruction;
    char  parityQ[2];
    char    data[16];
    char  parityP[4];
} CD_SCPacket;
#endif

typedef std::deque<CDGMagic_FontBlock*> FontBlock_Deque;

#ifndef TL_FONTQUEUE_STRUCT
#define TL_FONTQUEUE_STRUCT
typedef struct Timeline_FontQueue
{
    int     event_num;
    int     start_pack;
    FontBlock_Deque font_queue;
} Timeline_FontQueue;
#endif

#ifndef TL_EVENTQUEUE_STRUCT
#define TL_EVENTQUEUE_STRUCT
typedef struct Timeline_EventQueue
{
    int     event_num;
    int     start_pack;
    CDGMagic_MediaEvent* event_obj;
} Timeline_EventQueue;
#endif

class CDGMagic_GraphicsEncoder
{
private:
    // Some useful enums for CD+G stuff.  (This is also duplicated somewhat in the decoder...)
    // TODO: Maybe move these enums to a separate (shared) header?
    enum cdg_commands
	{
        // These are line graphics mode instructions... Need to implement this.
		LINE_GRAPHICS   = 0x08,    // 50x4 (48x2 visible) font line graphics mode.
        WRITE_FONT      = 0x04,    // I assume this is similar to Mode 0x09's COPY_FONT.
        WRITE_SCROLL    = 0x0C,    // Scroll... But not sure how this should work.
        // These are TV Graphics mode instructions.
	    TV_GRAPHICS     = 0x09,    // 50x18 (48x16) 16 color TV graphics mode.
        MEMORY_PRESET   = 0x01,    // Set all VRAM to palette index.
        BORDER_PRESET   = 0x02,    // Set border to palette index.
        LOAD_CLUT_LO    = 0x1E,    // Load Color Look Up Table index 0 through 7.
        LOAD_CLUT_HI    = 0x1F,    // Load Color Look Up Table index 8 through 15.
        COPY_FONT       = 0x06,    // Copy 12x6 pixel font to screen.
        XOR_FONT        = 0x26,    // XOR 12x6 pixel font with existing onscreen values.
        SCROLL_PRESET   = 0x14,    // Update scroll offset, copying if 0x20 or 0x10.
        SCROLL_COPY     = 0x18,    // Update scroll offset, setting color if 0x20 or 0x10.
        SET_TRANSPARENT = 0x1C     // Set 6-bit transparency of CLUT entries 0 through 15.
        // Need to add and implement Extended Graphics commands here.
    };
    // These should never need to be changed... But it's good to name them.
    enum screen_size
    {
        VRAM_WIDTH     = 300,          // Width (in pixels) of raw video memory.
        VRAM_HEIGHT    = 216,          // Height (in pixels) of raw video memory.
        COMP_LAYERS    =   8,          // Number of "layers" for the compositing engine.
        CLUT_SIZE_G    =  16,          // Number of look up table entries for TV Graphics.
        CLUT_SIZE_EG   = 256           // Number of look up table entries for Extended Graphics.
    };
    enum offset_direction
    {
        H_OFFSET = 0,    // Request horizontal offset.
        V_OFFSET = 1     // Request vertical offset.
    };

    unsigned short *vram;           // The CDG machine 256 color video RAM.
    unsigned short *comp_buffer;    // The buffer used for the compositing "engine".
    unsigned long *g_palette;       // The  16 color RGBA palette for TV Graphics.
    unsigned long *eg_palette;      // The 256 color RGBA palette for Extended Graphics.
    CD_SCPacket   *cdg_stream;      // The generated subcode packet stream.
    int last_preset_index;
    int last_border_index;
    unsigned char last_x_offset, last_y_offset;
    int x_block_offset, y_block_offset;
    int current_length;
    unsigned int comp_width;
    unsigned int comp_height;

    // Utility sorter functions.
	static bool     timeline_sorter(CDGMagic_MediaClip *entry_a, CDGMagic_MediaClip *entry_b);
	static bool     timeline_event_sorter(Timeline_EventQueue entry_a, Timeline_EventQueue entry_b);
	static bool     xor_fontblock_sorter(CDGMagic_FontBlock* entry_a, CDGMagic_FontBlock* entry_b);
	// Allocate/clean up the memory used for generating the graphics.
    void  begin_computation();
    void  end_computation();
    // Globally (that is, applies to all clips/channels) set VRAM functions.
    int             set_pal(        int current_position, CDGMagic_MediaEvent *the_event);
    int             set_memory(     int current_position, CDGMagic_MediaEvent *the_event);
    int             set_border(     int current_position, CDGMagic_MediaEvent *the_event);
    FontBlock_Deque set_scroll(int &current_position, CDGMagic_MediaEvent *the_event);
    // Unique (that is, applies only to the track/clip) VRAM functions.
    FontBlock_Deque bmp_to_fonts(CDGMagic_MediaClip *incoming_clip);
    int             xor_to_fonts(CDGMagic_MediaClip *incoming_clip, FontBlock_Deque* font_deque_return);
    int             copy_compare_fontblock(CDGMagic_FontBlock *block_to_compare);

    int             write_fontblock(int current_position, CDGMagic_FontBlock *block_to_write);
    void            write_fontblock_single(int instruction, unsigned char channel, int x_block, int y_block, unsigned char color_one, unsigned char color_two, CD_SCPacket &pack_to_write);

    int             copy_fontblock_to_vram(CDGMagic_FontBlock* given_block);
    int             get_composited_fontblock(CDGMagic_FontBlock* return_block);
    void            proc_VRAM_HSCROLL(int direction, int copy_flag, int color);
    void            proc_VRAM_VSCROLL(int direction, int copy_flag, int color);
    unsigned char   calculate_inter_rgb_value(unsigned char initial_value, unsigned char ending_value, double inter_progress);

public:
    CDGMagic_GraphicsEncoder(const unsigned int new_length, const unsigned int new_width = VRAM_WIDTH, const unsigned int new_height = VRAM_HEIGHT);  // Class constructor.
    ~CDGMagic_GraphicsEncoder();   // Class destructor.

    // Maybe this could either be "pumped" by the caller,
    // or have another function to call from a different thread
    // that could be used to update a progress bar...
    // As there's no feedback currently for what "could" be a long-ish operation.
    int  compute_graphics(std::deque<CDGMagic_MediaClip*> *TimeLine_Deque);
    int  length() const;
    CD_SCPacket *Get_Stream();
};

#endif
