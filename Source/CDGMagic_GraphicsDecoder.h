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

// Library to decode CD+Graphics subcode packs to an RGB surface.

#ifndef CDGMACHINE_H
#define CDGMACHINE_H

// Some structures to make life easier...
// Mostly recommended in the very old Jim Bumgadner "CD+G Revealed" document.
// Straightforward enough, so no need to really change them.

// 24 byte Compact Disc Subcode Packet.
#ifndef CD_SCPACKET_STRUCT
#define CD_SCPACKET_STRUCT
typedef struct CD_SCPacket
{                     // command is HI three bits for MODE, LO three bits for ITEM.
  char	command;      // MODE is always 1 for graphics, item can be 0, 1, or 2 depending on type.
  char	instruction;
  char	parityQ[2];
  char	data[16];
  char	parityP[4];
} CD_SCPacket;
#endif

class CDGMagic_GraphicsDecoder
{
private:
    enum cdg_commands              // Some useful enums for CDG stuff.
	{
        // These are line graphics mode instructions... Need to implement this.
		LINE_GRAPHICS   = 0x08,    // 50x4 (48x2) font line graphics mode.
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
        SET_TRANSPARENT = 0x1C,    // Set transparency of CLUT entries.
        // These are extended graphics instructions... Would love to know how to implement this.
        EXT_GRAPHICS    = 0x0A,    // Extended graphics mode.
        MEMORY_CONTROL  = 0x03,    // Control options for additional memory?
        ADD_FONT        = 0x06,    // Like COPY_FONT, but for high pixel nybble?
        XOR_ADD_FONT    = 0x0E     // Like XOR_FONT, but for high pixel nybble?
        /*
        There are also 48 additional instructions for extended graphics mode:
            Load CLUT Color             (0x10 through 0x2F) (= 32 instructions)
            Load CLUT Additional Color  (0x30 through 0x3F) (= 16 instrustions)

        I assume Load CLUT Color is similar to LOAD_CLUT_LO and LOAD_CLUT_HI of the
        normal graphics mode, and loads the low or high 4 bit RGB values for the 256 color indices.
        I also assume the Load CLUT Additional Colors are similar, and loads the higher or
        lower two bit RGB values (+EG is 18bit color, +G is 12bit) for the 256 color indices.
        Implementing it for *display* should be fairly trivial...
        But generating content appears quite a bit trickier.
        It seems you need to histogram your pictures picking the 256 most common colors in
        descending order. Then load the 16 most used colors via the normal CD+G mode
        for compatibility with all players, and the additional 240 colors for extended players.

        I think memory control is for something like selecting between
        a full 256 color picture or two 16 color images loaded in the same VRAM.
        Alternately, it could control when/how the palette is swapped, since the time
        it take for 48 instructions to load the color table would be very noticable.
        */
    };

    //unsigned long rgba_screen[312*216];    // The RGBA rendered screen array.
    unsigned char *rgba_screen;            // The RGBA rendered screen array.
    unsigned char *vram;                   // The CDG machine 16 color video RAM.
    unsigned long palette[16];             // The 16 color RGBA palette for rendering.
    unsigned char rgba_order[4];           // Decoded bitshift order to use for RGBA rendering.

    //int has_tv_graphics, has_line_graphics;
    int dirty_palette, dirty_screen, dirty_border;
	int h_offset, v_offset, border_color;
    int active_channels;

    void proc_MEMORY_PRESET(const CD_SCPacket *cdg_pack);
    void proc_BORDER_PRESET(const CD_SCPacket *cdg_pack);
    void proc_LOAD_CLUT(const CD_SCPacket *cdg_pack);
    void proc_SET_TRANSPARENT(const CD_SCPacket *cdg_pack) const;
    void proc_WRITE_FONT(const CD_SCPacket *cdg_pack);
    void proc_DO_SCROLL(const CD_SCPacket *cdg_pack);
    void proc_fill_RGB(int x_start, int y_start, int x_end, int y_end, unsigned int color);
    void proc_VRAM_HSCROLL(int direction, int copy_flag, int color);
    void proc_VRAM_VSCROLL(int direction, int copy_flag, int color);

public:
    // Some common enums for RGBA ordering.
    enum rgba_options
    {
        RGBA_ORDER = 0xE4,    // Most libraries/OSes use this.
        BGRA_ORDER = 0x6C,    // Windows uses this order.
        ARGB_ORDER = 0x93     // Some video editing software.
    };
    // Two common enums for RGBA ordering.
    enum offset_direction
    {
        H_OFFSET = 0,    // Request horizontal offset.
        V_OFFSET = 1     // Request vertical offset.
    };
    // These should never need to be changed... But it's good just in case.
    enum screen_size
    {
        RGBA_WIDTH     = 312,          // Width (in pixels) of rendered screen.
        RGBA_HEIGHT    = 216,          // Height (in pixels) of rendered screen.
        INDEXED_WIDTH  = RGBA_WIDTH,   // Width (in pixels) of indexed screen.
        INDEXED_HEIGHT = RGBA_HEIGHT,  // Height (in pixels) of indexed screen.
        VRAM_WIDTH     = 300,          // Width (in pixels) of raw video memory.
        VRAM_HEIGHT    = 216           // Height (in pixels) of raw video memory.
    };
    // rgba_order is bitmask order field.
    CDGMagic_GraphicsDecoder(const unsigned char color_order);  // Class constructor.
    ~CDGMagic_GraphicsDecoder();   // Class destructor.
    // Functions definitions.
    signed   int   decode_packs(const CD_SCPacket *pack_array, const int number); // Decode packets.
	               void  Reset();              // Clear palette, offsets, VRAM, rendered screen, etc.
	               void  QuickReset();         // Just reset the offsets to 0 and force update.
	//const unsigned long* GetRGBAScreen();      // Return a pointer to the RGBA screen array.
	unsigned char* GetRGBAScreen();      // Return a pointer to the RGBA screen array.
	unsigned char* GetIndexedScreen();   // Return a pointer to indexed color screen array.
	unsigned char* GetVRAM();            // Return a pointer to raw VRAM.
	unsigned long* GetPalette();         // Retrieve the current palette.
	unsigned char  GetBorderIndex() const;// Retrieve the border color index.
	unsigned int   GetOffset(const unsigned int h_or_v) const; // Retrieve the "viewport" offset.
	unsigned int   GetChannelsMask();    // Retrieve active channels mask.
	unsigned int   GetChannel(const unsigned int channel); // Is a specific channel active?.
	         void  SetChannelsMask(const int mask);        // Set active channels mask.
	         void  SetChannel(const int channel, const int active); // Set a specific channel on/off.
	signed   int   IsScreenDirty();      // Check if call to GetScreen() is needed.
	signed   int   IsPaletteDirty();     // Check if palette is dirty.
	signed   int   IsVRAMDirty();        // Check if VRAM is dirty.
};

#endif
