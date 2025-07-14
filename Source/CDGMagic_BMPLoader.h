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

#ifndef CDGMAGIC_BMPLOADER_H
#define CDGMAGIC_BMPLOADER_H

#include "CDGMagic_BMPObject.h"

class CDGMagic_BMPLoader : public CDGMagic_BMPObject
{
private:
    signed int file_size;
    signed int bmp_width;
    signed int bmp_height;
    char *internal_filepath;

public:
    enum fail_exceptions
	{
        NO_PATH,
        OPEN_FAIL,
        TO_LARGE,
        TO_SMALL,
        NOT_BMP,
        BAD_SIZE,
        BAD_OFFSET,
        UNSUPPORTED_HEADER,
        TOO_WIDE,
        TOO_TALL,
        INVALID_COLORPLANES,
        UNSUPPORTED_COLORDEPTH,
        UNSUPPORTED_COMPRESSION,
        UNSUPPORTED_PALETTE,
        INSUFFICIENT_DATA
    };

    CDGMagic_BMPLoader(const char *file_path);
    ~CDGMagic_BMPLoader();
    const char* file_path();
    static const char* error_to_text(int error_number);
};

#endif
