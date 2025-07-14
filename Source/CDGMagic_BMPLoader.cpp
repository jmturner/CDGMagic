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

#include "CDGMagic_BMPLoader.h"

CDGMagic_BMPLoader::CDGMagic_BMPLoader(const char *file_path) : CDGMagic_BMPObject()
{
    // Make sure the file_path we were given isn't NULL.
    if ( file_path == NULL )  { throw (int)NO_PATH; };
    // Attempt to open the file_path for binary reading.
    FILE *InputFile = fopen( (const char*)file_path, "rb" );
    // Check to make sure we got a valid file handle.
    if ( InputFile == NULL )  { throw (int)OPEN_FAIL; };
    // Put the file cursor at the end of the file.
    fseek(InputFile, 0, SEEK_END);
    // Get the end of file cursor position (i.e. the file size).
    file_size = ftell(InputFile);
    // Set the file cursor back to the beginning of the file.
    rewind(InputFile);
    // Make sure the file isn't rediculously large (that is, < 10,000,000 bytes) or tiny.
    if ( file_size > 10000000 )  { if ( InputFile != NULL )  { fclose( InputFile ); }; throw (int)TO_LARGE; }
    if ( file_size < 100 )       { if ( InputFile != NULL )  { fclose( InputFile ); }; throw (int)TO_SMALL; }
    // Make a temp buffer for the header data.
    unsigned char *header = new unsigned char[16384];
    // Calculate the lesser amount of the buffer size or file size.
    int bytes_to_read = (file_size < 16384) ? file_size : 16384;
    // Read the header from the file.
    fread(header, bytes_to_read, 1, InputFile);
    // Check if we have a bitmap... ##########TODO: Proper cleanup!!, and probably a struct instead.##########
    if ( (header[0x00] != 'B') || (header[0x01] != 'M') )
    {
        if ( InputFile != NULL )  { fclose( InputFile ); };
        delete[] header;
        throw (int)NOT_BMP;
    };
    // Get the header file size.
    int internal_filesize = 0;
    internal_filesize |= (header[0x02] << 000);
    internal_filesize |= (header[0x03] << 010);
    internal_filesize |= (header[0x04] << 020);
    internal_filesize |= (header[0x05] << 030);
    // Check if the header file size matches the actual file size.
    if ( internal_filesize != file_size )
    {
        if ( InputFile != NULL )  { fclose( InputFile ); };
        delete[] header;
        throw (int)BAD_SIZE;
    };
    // Find out where the actual data starts.
    int data_address = 0;
    data_address |= (header[0x0A] << 000);
    data_address |= (header[0x0B] << 010);
    data_address |= (header[0x0C] << 020);
    data_address |= (header[0x0D] << 030);
    // Check if the header file size matches the actual file size.
    if ( data_address >= file_size )
    {
        if ( InputFile != NULL )  { fclose( InputFile ); };
        delete[] header;
        throw (int)BAD_OFFSET;
    };
    // Get the header size. (40 bytes for Windows v3.0)
    int header_size = 0;
    header_size |= (header[0x0E] << 000);
    header_size |= (header[0x0F] << 010);
    header_size |= (header[0x10] << 020);
    header_size |= (header[0x11] << 030);
    // Check if we have a standard Windows v3.0 header (the only one we support).
    if ( header_size != 40 )
    {
        if ( InputFile != NULL )  { fclose( InputFile ); };
        delete[] header;
        throw (int)UNSUPPORTED_HEADER;
    };
    // Get the image width.
    bmp_width = 0;
    bmp_width |= (header[0x12] << 000);
    bmp_width |= (header[0x13] << 010);
    bmp_width |= (header[0x14] << 020);
    bmp_width |= (header[0x15] << 030);
    // Get the image height.
    bmp_height = 0;
    bmp_height |= (header[0x16] << 000);
    bmp_height |= (header[0x17] << 010);
    bmp_height |= (header[0x18] << 020);
    bmp_height |= (header[0x19] << 030);
    // Check if we have sane sizes.
    if (bmp_width > 32768)
    {
        if ( InputFile != NULL )  { fclose( InputFile ); };
        delete[] header;
        throw (int)TOO_WIDE;
    };
    if (bmp_height > 32768)
    {
        if ( InputFile != NULL )  { fclose( InputFile ); };
        delete[] header;
        throw (int)TOO_TALL;
    };
    // Get the number of color planes.
    int color_planes = 0;
    color_planes |= (header[0x1A] << 000);
    color_planes |= (header[0x1B] << 010);
    // Check if we support the color planes.
    if (color_planes != 1)
    {
        if ( InputFile != NULL )  { fclose( InputFile ); };
        delete[] header;
        throw (int)INVALID_COLORPLANES;
    };
    // Get the number of bits per pixel.
    int bits_per_pixel = 0;
    bits_per_pixel |= (header[0x1C] << 000);
    bits_per_pixel |= (header[0x1D] << 010);
    // Check if we support the color depth. (Must be 8 or 4bit indexed for now.)
    if ((bits_per_pixel != 8) && (bits_per_pixel != 4))
    {
        if ( InputFile != NULL )  { fclose( InputFile ); };
        delete[] header;
        throw (int)UNSUPPORTED_COLORDEPTH;
    };
    // Get the compression method.
    int compression_method = 0;
    compression_method |= (header[0x1E] << 000);
    compression_method |= (header[0x1F] << 010);
    // Check if we support the color depth. (Must be 8 or 4bit indexed for now.)
    if (compression_method != 0)
    {
        if ( InputFile != NULL )  { fclose( InputFile ); };
        delete[] header;
        throw (int)UNSUPPORTED_COMPRESSION;
    };
    // Get the number of palette entries.
    int palette_entries = 0;
    palette_entries |= (header[0x2E] << 000);
    palette_entries |= (header[0x2F] << 010);
    palette_entries |= (header[0x30] << 020);
    palette_entries |= (header[0x31] << 030);
    // Check if we support the color entries. (Must be 256 or less.)
    if ((palette_entries < 0) || (palette_entries > 256))
    {
        if ( InputFile != NULL )  { fclose( InputFile ); };
        delete[] header;
        throw (int)UNSUPPORTED_PALETTE;
    };
	// If the palette has 0 indices, that means it's actually 256 colors.
    if (palette_entries == 0)  { palette_entries = 256; };
    // Load the color entries.
    for ( int entry = 0; entry < palette_entries; entry++ )
    {
        int hdr_offset = 0x36 + entry * 4;
        int curr_clr = 0xFF;  // "Alpha" (not really used.
        curr_clr |= (header[hdr_offset+0] << 010); // Blue
        curr_clr |= (header[hdr_offset+1] << 020); // Green
        curr_clr |= (header[hdr_offset+2] << 030); // Red
        internal_palette->color(entry, curr_clr);
    };
    // Delete the temp header array.
    delete[] header;
    // Seek to the beginning of the bitmap data.
    fseek(InputFile, data_address, SEEK_SET);
    // Round the width to 32bit (4 byte) padding.
	// TODO: Maybe extend this to also support 1-bit, if not 2-bit images as well.
    int aligned_width = (bits_per_pixel == 4) ? (bmp_width + 7) / 8 * 4 : (bmp_width + 3) / 4 * 4;
    // Calculate the actual bytes that need to be read.
	bytes_to_read = aligned_width*bmp_height;
    // Allocate an internal buffer to read from the file.
    unsigned char* bmp_buffer = new unsigned char[bytes_to_read];
    // Resize the inherited bitmap object's buffer.
    alter_buffer_size(bmp_width, bmp_height);
    // Read the bitmap data.
    int bytes_actually_read = fread(bmp_buffer, 1, bytes_to_read, InputFile);
    if (bytes_actually_read != bytes_to_read)
    {
        if ( InputFile != NULL )  { fclose( InputFile ); };
        delete[] bmp_buffer;
        throw (int)INSUFFICIENT_DATA;
    };
    // Flip the data around into the main data buffer.
    for (int cur_y = 0; cur_y < bmp_height; cur_y++)
    {
        for (int cur_x = 0; cur_x < aligned_width; cur_x++)
        {
            if (bits_per_pixel == 8)
            {
                if (cur_x < bmp_width)
                {
                    pixel(cur_x, cur_y, bmp_buffer[cur_x+(bmp_height-cur_y-1)*aligned_width]);
                };
            }
            else
            {
                if ((cur_x*2) < bmp_width)
                {
                    pixel(cur_x*2+0, cur_y, bmp_buffer[cur_x+(bmp_height-cur_y-1)*aligned_width] >> 4);
                    pixel(cur_x*2+1, cur_y, bmp_buffer[cur_x+(bmp_height-cur_y-1)*aligned_width] & 0x0F);
                };
            };
        };
    };
    // Delete the temp buffer array.
    delete[] bmp_buffer;
    // Close the file, since we've buffered the data.
    if ( InputFile != NULL )
    {
        fclose( InputFile );
        InputFile = NULL;
    };
    // Allocate and copy the filename.
    internal_filepath = new char[ strlen(file_path)+1 ];
    strcpy(internal_filepath, file_path);
}

CDGMagic_BMPLoader::~CDGMagic_BMPLoader()
{
    // We allocated the filepath, so delete it.
    delete[] internal_filepath;
    // NOTE: The bitmap buffer was allocated by and is deleted in the BMPObject class.
}

const char* CDGMagic_BMPLoader::file_path()  {  return internal_filepath;  }

const char* CDGMagic_BMPLoader::error_to_text(int error_number)
{
    switch (error_number)
    {
        case                 NO_PATH: return("No path was given!");
        case               OPEN_FAIL: return("Could not open file!");
        case                TO_LARGE: return("File is larger than arbitrary 10MB limit!");
        case                TO_SMALL: return("File is smaller than arbitrary 100B limit!");
        case                 NOT_BMP: return("This file does not start with bitmap magic BM!");
        case                BAD_SIZE: return("This file's header size and actual size are inconsistent!");
        case              BAD_OFFSET: return("This file's data offset is beyond the end of file!");
        case      UNSUPPORTED_HEADER: return("This file uses an unsupported header version!");
        case                TOO_WIDE: return("This file is wider than the arbitrary limit of 16K pixels!");
        case                TOO_TALL: return("This file is taller than the arbitrary limit of 16K pixels!");
        case     INVALID_COLORPLANES: return("This file uses an unsupported number of color planes!");
        case  UNSUPPORTED_COLORDEPTH: return("This file is not 4 or 8 bits per pixel indexed color!");
        case UNSUPPORTED_COMPRESSION: return("This file is compressed, only uncompressed BMPs are supported!");
        case     UNSUPPORTED_PALETTE: return("This file uses an unsupported number of palette entries!");
        case       INSUFFICIENT_DATA: return("Insufficient data, this file may have been truncated!");
    };
    return("Unknown error number!");
}
