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

#include "CDGMagic_PALObject.h"

CDGMagic_PALObject::CDGMagic_PALObject()
{
    // Default color update mask, all colors needed.
    internal_update_mask = 0xFFFF;
    // No fade/dissolve by default.
    internal_dissolve_interval = 0;
    internal_dissolve_steps    = 0;
    // Allocate palette memory.
    internal_palette = new unsigned long[256];
    // Set it all to 0;
    for (int idx = 0; idx < 256; idx++)  {  internal_palette[idx] = 0x000000FF;  };
}

CDGMagic_PALObject::~CDGMagic_PALObject()
{
    // Delete the allocated palette entries.
    delete[] internal_palette;
}

signed int CDGMagic_PALObject::number_of_colors() const {  return 256;  }

unsigned long CDGMagic_PALObject::color(unsigned char requested_index) const
{
    return internal_palette[requested_index];
}

void CDGMagic_PALObject::color(unsigned char requested_index, unsigned long requested_value)
{
    internal_palette[requested_index] = requested_value;
}

unsigned long CDGMagic_PALObject::update_mask() const
{
    return internal_update_mask;
}

void CDGMagic_PALObject::update_mask(unsigned long requested_mask)
{
    internal_update_mask = requested_mask;
}

signed int CDGMagic_PALObject::dissolve_interval() const
{
    return internal_dissolve_interval;
}

signed int CDGMagic_PALObject::dissolve_steps() const
{
    return internal_dissolve_steps;
}

void CDGMagic_PALObject::dissolve(signed int requested_interval, signed int requested_steps)
{
    internal_dissolve_interval = requested_interval;
    internal_dissolve_steps    = requested_steps;
}
