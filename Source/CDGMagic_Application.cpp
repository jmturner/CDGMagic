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

#include <sys/stat.h>
#include <cerrno>

#include "CDGMagic_Application.h"

#include "CDGMagic_BMPLoader.h"

CDGMagic_Application::CDGMagic_Application(int argc, char **argv)
{
    // Create the main application window.
    CDGMagic_MainWindow *MainWindow = new CDGMagic_MainWindow();
    // Set the main window callback to the application exit cleanup.
    MainWindow->callback( app_exit_callback );
    // Make the main window visible.
    MainWindow->show(argc, argv);

    // Create the preview window.
    CDGMagic_PreviewWindow *preview_window = new CDGMagic_PreviewWindow();
    internal_static_preview_window = preview_window;
    // Set the pointer in the main window subclass.
    MainWindow->set_prev(preview_window);
    // Position the preview window to bottom left of the main window.
    preview_window->position(MainWindow->x()+MainWindow->w()-preview_window->w(), MainWindow->y()+MainWindow->h()+50);
    // Make the preview window visible.
    preview_window->show();
    // Update the display (just shows a default black screen).
    preview_window->RefreshDisplay();
}

void CDGMagic_Application::app_exit_callback(Fl_Widget *CallingWidget, void *IncomingObject)
{
    printf("Exiting application...\n");
    // Stupid always ask dialog... [TODO: Make this smart -- only ask for unsaved data.]
    int user_choice = fl_choice("Are you sure you want exit?", "No, don't exit.", "Yes (discarding data!)", NULL);
    // Check the return value. [TODO: Again, make this smart!]
    if (user_choice == 1)
    {
        exit(0);
    };
    printf("User declined to exit.\n");
}

void CDGMagic_Application::app_about_callback(Fl_Widget *CallingWidget, void *IncomingObject)
{
    // TODO: Make a real/better ABOUT dialog...
    // Probably should show the GNU GPL, as well as the licenses for FLTK and PortAudio...
    fl_message("%s\n"\
               "is released under the terms of the\n"\
               "GNU General Public License v2 or,\n"\
               "at your option, any later version.\n"\
               "\n"\
               "Based in part on the work of the FLTK project:\n"\
               "http://www.fltk.org/\n"\
               "\n"\
               "And the PortAudio Portable Real-Time Audio Library:\n"\
               "http://www.portaudio.com/\n"\
               "\n"\
               "Please see \"copying.txt\" for additional license information.",
               CDGMAGIC_APP_VERSION_STRING);
}

void CDGMagic_Application::bmp_to_transition_callback(Fl_Widget *CallingWidget, void *IncomingObject)
{
    Fl_Native_File_Chooser file_chooser;
    file_chooser.title("Open bitmap image file...");
#ifdef WIN32
    file_chooser.filter("Bitmap Image (*.bmp)\t*.bmp");
#else    // The parenthesis somehow break the non-Windows native file choosers?!...
    file_chooser.filter("Bitmap Image\t*.bmp\n");
#endif
    file_chooser.show();
    // Returns NULL if the hit cancel, so don't even attempt to make BitMap object in that case.
    if (file_chooser.count() != 1) { return; };

    CDGMagic_BMPLoader *current_image;
    try
    {
        current_image = new CDGMagic_BMPLoader( file_chooser.filename() );
    }
    // Just put up an alert with the error and bail if there was a problem.
    catch (int except)
    {
        fl_alert("BMP Exception: %i\n%s", except, CDGMagic_BMPLoader::error_to_text(except) );
        return;
    };

    std::vector<font_intensity> gradient_vector;

    // Step through the Y fonts.
    for (int y_blk = 0; y_blk < current_image->height() / 12; y_blk++)
    {
        // Step through the X fonts.
        for (int x_blk = 0; x_blk < current_image->width() / 6; x_blk++)
        {
            font_intensity tmp_fnt;
            tmp_fnt.x_blk = x_blk;
            tmp_fnt.y_blk = y_blk;
            int temp_val = 0;
            for (char y_px = 0; y_px < 6; y_px++)
            {
                for (char x_px = 0; x_px < 6; x_px++)
                {
                    int temp_px = current_image->get_rgb_pixel( x_blk*6+x_px, y_blk*12+y_px );
                    temp_val += ((temp_px >> 030) & 0xFF);
                    temp_val += ((temp_px >> 020) & 0xFF);
                    temp_val += ((temp_px >> 010) & 0xFF);
                };
            };
            tmp_fnt.intensity = temp_val;
            if ( (x_blk >= 1) && (x_blk <= 48) && (y_blk >= 1) && (y_blk <= 16) )
            {
                gradient_vector.push_back( tmp_fnt );
            };
        };
    };

    delete current_image;
    std::stable_sort(gradient_vector.begin(), gradient_vector.end(), gradient_sorter);

    // Ask to the user to select a file.
    Fl_Native_File_Chooser transition_chooser;
    file_chooser.type( Fl_Native_File_Chooser::BROWSE_SAVE_FILE );
    file_chooser.title("Save transition file...");
#ifdef WIN32
    file_chooser.filter("CD+Graphics Magic Transition (*.cmt)\t*.cmt");
#else    // The parenthesis somehow break the non-Windows native file choosers?!...
    file_chooser.filter("CD+Graphics Magic Transition\t*.cmt\n");
#endif
    file_chooser.show();
    if (file_chooser.count() != 1)  { return; };

    // Make a string object for the filename, then check if we need to append the file extension.
    std::string save_path_string( file_chooser.filename() );
    unsigned int ext_pos = save_path_string.rfind(".cmt");
    if (ext_pos != (save_path_string.size() - 4))
    {
        save_path_string.append(".cmt");
    };

    // Make sure file doesn't exist (by checking attributes)!.
    struct stat statFileInfo;  // File info for attributes.
    int stat_err = stat( save_path_string.c_str(), &statFileInfo );
    if ((stat_err != -1) || (errno != ENOENT))
    {
        std::string save_prompt_string("The requested file already exists:\n");
        save_prompt_string.append(save_path_string);
        save_prompt_string.append("\n\nAre you SURE you want to OVERWRITE it?");
        int user_choice = fl_choice(save_prompt_string.c_str(), "Yes, OVERWRITE file with new data.", "No, don't overwrite.", NULL);
        // Check the return value, and return if they don't want to overwrite.
        if (user_choice == 1)  { return; };
    };

    FILE *Transition_File = fopen( save_path_string.c_str(), "wb");
    if ( Transition_File == NULL )
    {
         fl_alert("Could not open output file!");
         return;
    };

    unsigned char *transition_bytes = new unsigned char[1536];
    for (unsigned int blk = 0; blk < 768; blk++)
    {
        transition_bytes[blk*2+0] = (blk < gradient_vector.size()) ?  gradient_vector.at(blk).x_blk : 0;
        transition_bytes[blk*2+1] = (blk < gradient_vector.size()) ?  gradient_vector.at(blk).y_blk : 0;
    };

    fwrite(transition_bytes, 1536, 1, Transition_File);
    fclose(Transition_File);
    delete[] transition_bytes;
    printf("Transition file has been created!\n");
}

bool CDGMagic_Application::gradient_sorter(font_intensity entry_a, font_intensity entry_b)
{
    return (entry_a.intensity > entry_b.intensity);
}


CDGMagic_PreviewWindow* CDGMagic_Application::internal_static_preview_window = NULL;
void CDGMagic_Application::show_preview_window(Fl_Widget *CallingWidget, void *IncomingObject)
{
    if (internal_static_preview_window) { internal_static_preview_window->show(); };
}
