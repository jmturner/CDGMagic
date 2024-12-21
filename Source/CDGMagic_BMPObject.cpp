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
#include <cstring>

#include <string>

#include "CDGMagic_BMPObject.h"

CDGMagic_BMPObject::CDGMagic_BMPObject(int requested_width, int requested_height)
{
    // Set the defaults to 0.
    internal_bmp_width  = 0;
    internal_bmp_height = 0;
    internal_fill_index = 0;
    internal_composite_index = 0;
    internal_should_composite = 0;
    internal_xor_only   = 0;
    internal_x_offset   = 0;
    internal_y_offset   = 0;
    internal_bmp_data   = 0;
    internal_xor_bandwidth = 3.0;
    internal_draw_delay = 0;

    // Create a generic default top->bottom->left->right transition.
    internal_transition_length = 768;
    internal_transition_blocks = new unsigned char[ 768*2 ];
    internal_transition_file   = NULL;
    for (int cur_blk = 0; cur_blk < 768; cur_blk++)
    {
        internal_transition_blocks[cur_blk*2+0] = (cur_blk / 16) + 1; // X location (1 to 48).
        internal_transition_blocks[cur_blk*2+1] = (cur_blk % 16) + 1; // Y location (1 to 16).
    };

    // Create a new (256 index * 32bit RGB) palette object.
    internal_palette = new CDGMagic_PALObject();
    // Do memory/size allocation.
    alter_buffer_size(requested_width, requested_height);
}

CDGMagic_BMPObject::~CDGMagic_BMPObject()
{
    delete[] internal_transition_blocks;
    delete[] internal_transition_file;

    delete[] internal_bmp_data;
    delete internal_palette;
}

void CDGMagic_BMPObject::alter_buffer_size(unsigned int requested_width, unsigned int requested_height)
{
    // Do nothing if we're already the requested size.
    // This COULD be changed to just update the width/height
    // and only reallocate if we're not the same LINEAR size...
    if ( (internal_bmp_width == requested_width) && (internal_bmp_height == requested_height) )  { return; };
    // Calculate the required memory.
    int size_of_data = requested_width * requested_height;
    // If our size is above zero and below 8MB, then make it so.
    if ( (size_of_data > 0) && (size_of_data <= 0x800000) )
    {
#ifdef _BMPOBJECT_DEBUG_
        // Print some debug info.
        printf("CDGMagic_BMPObject allocate: %i x %i pixels.\n", requested_width, requested_height);
#endif
        // First, kill any previous allocation.
        delete[] internal_bmp_data;
        // Then allocate the memory.
        internal_bmp_data = new unsigned char[size_of_data];
        // Set the sizes.
        internal_bmp_width  = requested_width;
        internal_bmp_height = requested_height;
    }
    else
    {
        printf("CDGMagic_BMPObject did not allocate buffer! w: %i, h: %i, sz: %i\n", requested_width, requested_height, size_of_data);
    };
    if (requested_width  == 0)  { internal_bmp_width = 0;  };
    if (requested_height == 0)  { internal_bmp_height = 0; };
}

unsigned long CDGMagic_BMPObject::get_rgb_pixel(unsigned int requested_x, unsigned int requested_y)
{
    // This is a convenience function to save us from typing/remembering a lot of silly:
    // rgb_color = object->PALObject()->color( object->pixel(x, y) );
    return internal_palette->color( pixel(requested_x, requested_y) );
}

unsigned char CDGMagic_BMPObject::linear_pixel(unsigned int requested_pixel) const
{
    if (requested_pixel < (internal_bmp_width * internal_bmp_height))
    {
        return internal_bmp_data[requested_pixel];
    };
    return internal_fill_index;
}

void CDGMagic_BMPObject::linear_pixel(unsigned int requested_pixel, unsigned char requested_index)
{
    if (requested_pixel < (internal_bmp_width * internal_bmp_height))
    {
        internal_bmp_data[requested_pixel] = requested_index;
    };
}

unsigned char CDGMagic_BMPObject::pixel(unsigned int requested_x, unsigned int requested_y) const
{
    if ( (requested_x < internal_bmp_width) && (requested_y < internal_bmp_height) )
    {
        return internal_bmp_data[requested_x+requested_y*internal_bmp_width];
    };
    return internal_fill_index;
}

void CDGMagic_BMPObject::pixel(unsigned int requested_x, unsigned int requested_y, unsigned char requested_index)
{
    if ( (requested_x < internal_bmp_width) && (requested_y < internal_bmp_height) )
    {
        internal_bmp_data[requested_x+requested_y*internal_bmp_width] = requested_index;
    };
}

void CDGMagic_BMPObject::transition_row_mask(unsigned long requested_row_mask)
{
    // Step through row 1 to 16, setting column 1 to 48 as requested.
    internal_transition_length = 0;
    for (int current_column = 1; current_column < 49; current_column++)
    {
        for (int current_row = 1; current_row < 17; current_row++)
        {
            if ( (requested_row_mask >> current_row) & 0x01 )
            {
                internal_transition_blocks[internal_transition_length*2+0] = current_column;
                internal_transition_blocks[internal_transition_length*2+1] = current_row;
                internal_transition_length++;
            };
        };
    };
    // Remove any file path setting.
    delete[] internal_transition_file;
    internal_transition_file = NULL;
}

unsigned char CDGMagic_BMPObject::transition_file(const char* selected_file)
{
    // File handle.
    FILE* InputFile = NULL;
    // Set the transition back to the default if given NULL.
    if ( selected_file == NULL )
    {
        for (int cur_blk = 0; cur_blk < 768; cur_blk++)
        {
            internal_transition_blocks[cur_blk*2+0] = (cur_blk / 16) + 1; // X location (1 to 48).
            internal_transition_blocks[cur_blk*2+1] = (cur_blk % 16) + 1; // Y location (1 to 16).
        };
        internal_transition_length = 768;
        delete[] internal_transition_file;
        internal_transition_file = NULL;
        return 0;
    };
    // Attempt to open the file_path for binary reading.
    InputFile = fopen( (const char*)selected_file, "rb" );
    // Check to make sure we got a valid file handle.
    if ( InputFile == NULL )  { return 1; };
    // Put the file cursor at the end of the file.
    fseek(InputFile, 0, SEEK_END);
    // Get the end of file cursor position (i.e. the file size).
    if ( ftell(InputFile) != 1536 )
    {
        if (InputFile != NULL)  { fclose(InputFile); };
        return 1;
    };
    // Set the file cursor back to the beginning of the file.
    rewind(InputFile);
    // Read the current transition.
    fread(internal_transition_blocks, 1, 1536, InputFile);
    // Close the file.
    fclose(InputFile);
    // Constrain the block locations.
    for (int cur_blk = 0; cur_blk < 768; cur_blk++)
    {
        if ( internal_transition_blocks[cur_blk*2+0] > 49 ) { internal_transition_blocks[cur_blk*2+0] = 49; };
        if ( internal_transition_blocks[cur_blk*2+1] > 17 ) { internal_transition_blocks[cur_blk*2+1] = 17; };
    };
    // [Re]set the current transition length.
    internal_transition_length = 768;
    // [Re]allocate and copy the file path.
    delete[] internal_transition_file;
    internal_transition_file = new char[ strlen(selected_file)+1 ];
    strcpy(internal_transition_file, selected_file);
    // The transition file was set successfully.
    return 0;
}

unsigned char CDGMagic_BMPObject::transition_block(unsigned short requested_block_num, unsigned char x_or_y) const
{
     // Silliness so we don't overrun array.
    int block_offset = ((requested_block_num > 0) && (requested_block_num < internal_transition_length)) ? requested_block_num * 2 : 0;
    unsigned char xy_offset = (x_or_y != 0) ? 1 : 0;
    // Return the requested block location.
    return internal_transition_blocks[ block_offset + xy_offset ];
}

const char* CDGMagic_BMPObject::transition_file() const {  return internal_transition_file;  }
unsigned short CDGMagic_BMPObject::transition_length() const {  return internal_transition_length;  }

CDGMagic_PALObject* CDGMagic_BMPObject::PALObject()  {  return internal_palette;  }

signed int CDGMagic_BMPObject::width() const {  return internal_bmp_width;  }
signed int CDGMagic_BMPObject::height() const {  return internal_bmp_height;  }

unsigned char CDGMagic_BMPObject::xor_only() const {  return internal_xor_only;  }
void CDGMagic_BMPObject::xor_only(unsigned char requested_setting)  {  internal_xor_only = requested_setting;  }

int CDGMagic_BMPObject::should_composite() const {  return internal_should_composite;  }
void CDGMagic_BMPObject::should_composite(int requested_setting)  {  internal_should_composite = requested_setting;  }

unsigned char CDGMagic_BMPObject::composite_index() const {  return internal_composite_index;  }
void CDGMagic_BMPObject::composite_index(unsigned char requested_index)  {  internal_composite_index = requested_index;  }

float CDGMagic_BMPObject::xor_bandwidth() const {  return internal_xor_bandwidth;  }
void CDGMagic_BMPObject::xor_bandwidth(float requested_bandwidth)  {  internal_xor_bandwidth = (requested_bandwidth >= 1.0) ? requested_bandwidth : 1.0;  }

unsigned char CDGMagic_BMPObject::fill_index() const {  return internal_fill_index;  }
void CDGMagic_BMPObject::fill_index(unsigned char requested_index)  {  internal_fill_index = requested_index;  }

int CDGMagic_BMPObject::x_offset() const { return internal_x_offset; }
void CDGMagic_BMPObject::x_offset(int requested_x_offset)  { internal_x_offset = requested_x_offset; }

int CDGMagic_BMPObject::y_offset() const { return internal_y_offset; }
void CDGMagic_BMPObject::y_offset(int requested_y_offset)  { internal_y_offset = requested_y_offset; }

int CDGMagic_BMPObject::draw_delay() const { return internal_draw_delay; }
void CDGMagic_BMPObject::draw_delay(int requested_delay)  { internal_draw_delay = requested_delay; }

const char* CDGMagic_BMPObject::file_path()  { return 0; }
