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

#include "CDGMagic_MainWindow.h"
#include "CDGMagic_MainWindow_MenuBar.h"

CDGMagic_MainWindow::CDGMagic_MainWindow() : Fl_Double_Window(0, 0, 0)
{
    const char *mainwindow_title      = CDGMAGIC_APP_VERSION_STRING;
    const int   mainwindow_width      = 1920-400;
    const int   mainwindow_height     = 573;
    const int   mainwindow_menuheight =  25;

#ifdef WIN32
    // For some reason, this appears to have scaling artifacts,
    // even though it should, in theory, be the correct size...
    // Maybe FLTK only sets the "large" 32x32 icon... Hrm.
    // Also, this needs a completely different syntax for *nix, and possibly Mac...
    // TODO: Uh, fix it... But low priority, as it's only the window icon...
    icon( (char*)LoadIcon(fl_display, MAKEINTRESOURCE(500)) );
#endif

    // Set the MainWindow title bar to the title above.  [TODO: Set this to the currently opened project file.]
    label( mainwindow_title );
    // Set up the default size of MainWindow from above.  [TODO: Set this from a preferences file to the last position.]
    size( mainwindow_width, mainwindow_height );
    size_range( 300, mainwindow_height, 0, mainwindow_height );
    position( 200, 100 );

    // Create the menu bar for the main application window.
    CDGMagic_MainWindow_MenuBar *MainWindow_MenuBar = new CDGMagic_MainWindow_MenuBar(mainwindow_width, mainwindow_menuheight);

    // Create the editing group control.
    editing_group = new CDGMagic_EditingGroup(0, MainWindow_MenuBar->y()+MainWindow_MenuBar->h()+5, w(), h()-MainWindow_MenuBar->y()-MainWindow_MenuBar->h()-5);

    // Ugh, this is way past ugly/WTF-y...
    // TODO: Either move the menus completely to a header file, or build them entirely in a class function...

    int item_index = 0;

    item_index = MainWindow_MenuBar->find_index("&File/&New Project");
    MainWindow_MenuBar->insert(item_index, "&File/&New Project", 0, CDGMagic_EditingGroup::DoProjectNew_callback_static , editing_group );

    item_index = MainWindow_MenuBar->find_index("&File/&Save Project");
    MainWindow_MenuBar->insert(item_index, "&File/&Save Project", FL_CTRL+'s', CDGMagic_EditingGroup::DoProjectSave_callback_static , editing_group );

    item_index = MainWindow_MenuBar->find_index("&File/Save Project As...");
    MainWindow_MenuBar->insert(item_index, "&File/Save Project As...", 0, CDGMagic_EditingGroup::DoProjectSave_callback_static , editing_group, FL_MENU_DIVIDER );

    item_index = MainWindow_MenuBar->find_index("&File/&Open Project...");
    MainWindow_MenuBar->insert(item_index, "&File/&Open Project...", FL_CTRL+'o', CDGMagic_EditingGroup::DoProjectOpen_callback_static , editing_group, FL_MENU_DIVIDER );

    // Make the editing group the resizable part of the MainWindow (prevents the menu from being screwy).
    resizable(editing_group);
}

void CDGMagic_MainWindow::set_prev(CDGMagic_PreviewWindow *prev_win)  { editing_group->preview_window = prev_win; }
