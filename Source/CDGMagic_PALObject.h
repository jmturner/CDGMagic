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

#ifndef CDGMAGIC_PALOBJECT_H
#define CDGMAGIC_PALOBJECT_H

class CDGMagic_PALObject
{
private:
    unsigned long *internal_palette;
    unsigned long internal_update_mask;
    signed   int  internal_dissolve_interval;
    signed   int  internal_dissolve_steps;

public:
    CDGMagic_PALObject();
    ~CDGMagic_PALObject();

    signed int    number_of_colors() const;
    unsigned long color(unsigned char requested_index) const;
    void          color(unsigned char requested_index, unsigned long requested_value);
    unsigned long update_mask() const;
    void          update_mask(unsigned long requested_mask);
    signed int    dissolve_interval() const;
    signed int    dissolve_steps() const;
    void          dissolve(signed int requested_interval, signed int requested_steps = 15);
};

#endif
