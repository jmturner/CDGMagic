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

/*
     ##########  CDG Magic EXPERIMENTAL VERSION  ##########

     Creates Compact Disc + Graphics sub codes on Win/Mac/Linux ...
     Or, at least it hopefully will eventually...
*/

#include "CDGMagic_Application.h"

int main(int argc, char **argv)
{
    CDGMagic_Application(argc, argv);
    return Fl::run();
}
