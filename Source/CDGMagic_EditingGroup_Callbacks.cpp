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

#include <FL/fl_ask.H>
#include <FL/Fl_Native_File_Chooser.H>

#include "CDGMagic_EditingGroup.h"
#include "CDGMagic_BMPClip.h"
#include "CDGMagic_TextClip.h"
#include "CDGMagic_ScrollClip.h"
#include "CDGMagic_VectorClip_Window.h"

void CDGMagic_EditingGroup::PBHead_PositionChange_callback(Fl_Widget *CallingWidget)
{
    if (Fl::event_button() == FL_RIGHT_MOUSE)  { RecalculatePreview_callback(NULL); };
    int new_pos = editing_lanes->playback_head->play_position();
    CD_SCPacket *stream = cdg_gen->Get_Stream();
#ifdef _EDITWIN_DEBUG_
    printf("PBHead_PositionChange_callback Position:%i\n", new_pos);
#endif
    // Play the packs if it won't overrun the buffer.
    if ( new_pos < cdg_gen->length() )
    {
        current_play_position = new_pos;
        preview_window->Decode_Packs( stream, new_pos );
    }
    else
    {
        printf("cdg command pack buffer would have overrun: Length=%i, Requested=%i\n", cdg_gen->length(), new_pos);
    };
    // Update the time display.
    time_display->set_time(new_pos);
    // Play only one buffer if scrubbing.
    if ( (Audio_Playback) && (Fl::pushed() == CallingWidget) && ((new_pos&0x07) == 0) )
    {
        Audio_Playback->Seek_To_Sample(new_pos * 147);
        Audio_Playback->Play_Single_Buffer();
    };
}

void CDGMagic_EditingGroup::Clip_PositionChange_callback(Fl_Widget* CallingWidget)
{
    int new_track = (CallingWidget->y() - editing_lanes->y() - 15 - ((editing_lanes->lane_height()+1)*2)) / (editing_lanes->lane_height()+1);
    if (new_track < 0) { new_track = 0; };
    if (new_track > 7) { new_track = 7; };
    dynamic_cast<CDGMagic_MediaClip*>(CallingWidget)->track_options(current_track_options[new_track]);
    int new_start = (CallingWidget->x()+editing_lanes->xposition()-editing_lanes->x()) * 4;
    dynamic_cast<CDGMagic_MediaClip*>(CallingWidget)->start_pack( new_start );
    printf("Clip: %p, track: %i, channel: %i, start_pack: %i\n",
            static_cast<void*>(CallingWidget),
            dynamic_cast<CDGMagic_MediaClip*>(CallingWidget)->track_options()->track(),
            dynamic_cast<CDGMagic_MediaClip*>(CallingWidget)->track_options()->channel(),
            dynamic_cast<CDGMagic_MediaClip*>(CallingWidget)->start_pack() );
}

void CDGMagic_EditingGroup::Queue_Next_Audio_Buffer(void *IncomingObject)
{
    CDGMagic_EditingGroup *editing_group = ((CDGMagic_EditingGroup*)IncomingObject);

    int new_pos = editing_group->Audio_Playback->Play_Next_Buffer();
    if (new_pos < 0)
    {
        printf("got %i from Play_Next_Buffer(), cancelling repeat_timeout.\n", new_pos);
        Fl::remove_timeout(Queue_Next_Audio_Buffer);
        editing_group->play_pause_button->value(0);
        return;
    };
    new_pos /= 147;  // There's 147 audio samples per graphics packet.
    editing_group->editing_lanes->playback_head->play_position(new_pos);
    editing_group->PBHead_PositionChange_callback( editing_group->editing_lanes->playback_head );
#ifdef _CDGMAGIC_USEPCLINUXAUDIO_
    Fl::repeat_timeout(editing_group->Audio_Playback->get_audio_interval(), Queue_Next_Audio_Buffer, IncomingObject);
#else
    //Fl::repeat_timeout(0.02, Queue_Next_Audio_Buffer, IncomingObject);
    Fl::repeat_timeout(0.01, Queue_Next_Audio_Buffer, IncomingObject);
#endif
}

void CDGMagic_EditingGroup::PlayPause_callback(Fl_Light_Button *CallingWidget)
{
    if (CallingWidget->value())
    {
        int current_sample = editing_lanes->playback_head->play_position() * 147;
        Audio_Playback->Seek_To_Sample(current_sample);
//        Audio_Playback->Start_Playback();
        CDGMagic_EditingGroup::Queue_Next_Audio_Buffer(this);
    }
    else
    {
        Fl::remove_timeout(Queue_Next_Audio_Buffer);
//        Audio_Playback->Stop_Playback();
    };
}

void CDGMagic_EditingGroup::seek_and_play(int requested_seek_time)
{
    if ( Audio_Playback != NULL )
    {
        play_pause_button->value(0);
        play_pause_button->do_callback();
        Audio_Playback->Seek_To_Sample(requested_seek_time * 147);
        editing_lanes->playback_head->play_position(requested_seek_time);
        play_pause_button->value(1);
        play_pause_button->do_callback();
    };
}

void CDGMagic_EditingGroup::RecalculatePreview_callback(Fl_Widget *CallingWidget)
{
    // Compute the new graphics.
    cdg_gen->compute_graphics(TimeLine_Deque);
    // Reset the display decoder.
    preview_window->Reset();
    // Get the current playback position.
    int new_pos = editing_lanes->playback_head->play_position();
    CD_SCPacket *stream = cdg_gen->Get_Stream();
    // Play the packs if it won't overrun the buffer.
    if ( new_pos < cdg_gen->length() )
    {
        preview_window->Decode_Packs( stream, new_pos );
    };
    // Update the editing lanes display.
    editing_lanes->redraw();
}

int CDGMagic_EditingGroup::delete_clip(CDGMagic_MediaClip* the_clip)
{
    printf( "CDGMagic_EditingGroup::delete_clip - ");

    for (size_t iter = 0; iter < TimeLine_Deque->size(); ++iter)
    {
        if (TimeLine_Deque->at(iter) == the_clip)
        {
            printf("Deleting clip at position %i .\n", iter);
            TimeLine_Deque->erase(TimeLine_Deque->begin() + iter);
            return 1;
        }
    }

    printf("Could not find clip!\n");
    return 0;
}

void CDGMagic_EditingGroup::AddBMP_callback(Fl_Widget *CallingWidget)
{
    // [TODO: Make a "smarter" adder that can check for minimum length (fewest packs possible) and set the w() to that.]

    // Create a new BitMap Loader object pointer.
    CDGMagic_BMPClip *current_clip = new CDGMagic_BMPClip( editing_lanes->x()+1, editing_lanes->y()+15+(editing_lanes->lane_height()+1)*2, 800, editing_lanes->lane_height() );
    current_clip->set_playtime_var( &current_play_position );
    current_clip->add_bmp_file();
    // Make sure we have at least one good bitmap before adding to the edit group.
    if ( current_clip->event_queue()->size() < 1 )
    {
        printf("CDGMagic_EditingGroup::AddBMP_callback - Failed to open or allocate bitmap object!\n");
        //delete current_clip;  // This doesn't seem to cause any issues, but it should probably be delete_widget??
        Fl::delete_widget( current_clip );
        return;
    };
    // Assign the callback that's performed on the mouse-up action.
    current_clip->callback(Clip_PositionChange_callback_static, this);
    // Add the clip to the deque of timeline clips.
    printf("CDGMagic_EditingGroup::AddBMP_callback - s: %i, events: %i\n", current_clip->start_pack(), current_clip->event_queue()->size() );
    TimeLine_Deque->push_back(current_clip);
    // Add the clip "box" to the Media Lane object.
    editing_lanes->add(current_clip);
    editing_lanes->redraw();
}

void CDGMagic_EditingGroup::AddText_callback(Fl_Widget *CallingWidget)
{
    // [TODO: Make a "smarter" adder that can check for minimum length (fewest packs possible) and set the w() to that.]

    // Create a new text clip object pointer.
    CDGMagic_TextClip *current_image = new CDGMagic_TextClip( editing_lanes->x()+1, editing_lanes->y()+15+(editing_lanes->lane_height()+1)*2, 800, editing_lanes->lane_height() );
    current_image->set_playtime_var( &current_play_position );

    // Assign the callback that's performed on the mouse-up action.
    current_image->callback(Clip_PositionChange_callback_static, this);
    // Add the clip the deque of timeline clips.
    printf("CDGMagic_EditingGroup::AddText_callback - s: %i, events: %i\n", current_image->start_pack(), current_image->event_queue()->size() );
    TimeLine_Deque->push_back(current_image);
    // Add the clip "box" to the Media Lane object.
    editing_lanes->add(current_image);
    editing_lanes->redraw();
}

#ifdef _CDGMAGIC_ENABLE_SCROLLCLIP_
void CDGMagic_EditingGroup::AddScroll_callback(Fl_Widget *CallingWidget)
{
    CDGMagic_ScrollClip *current_clip = new CDGMagic_ScrollClip( editing_lanes->x()+1, editing_lanes->y()+15+(editing_lanes->lane_height()+1)*2, 800, editing_lanes->lane_height() );
    current_clip->set_time_var(&current_play_position);
    current_clip->add_bmp_file();
    // Make sure we have at least one good bitmap before adding to the edit group.
    if ( current_clip->event_queue()->size() < 1 )
    {
        printf("CDGMagic_EditingGroup::AddScroll_callback - Failed to open or allocate bitmap object!\n");
        delete current_clip;
        return;
    };
    // Assign the callback that's performed on the mouse-up action.
    current_clip->callback(Clip_PositionChange_callback_static, this);
    // Add the clip to the deque of timeline clips.
    printf("CDGMagic_EditingGroup::AddScroll_callback - s: %i, events: %i\n", current_clip->start_pack(), current_clip->event_queue()->size() );
    TimeLine_Deque->push_back(current_clip);
    // Add the clip "box" to the Media Lane object.
    editing_lanes->add(current_clip);
    editing_lanes->redraw();
}
#endif

#ifdef _CDGMAGIC_ENABLE_VECTORCLIP_
void CDGMagic_EditingGroup::AddVector_callback(Fl_Widget *CallingWidget)
{
    CDGMagic_VectorClip_Window *test_window = new CDGMagic_VectorClip_Window(200, 200);
    test_window->set_time_var(&current_play_position);
    test_window->show();
}
#endif

void CDGMagic_EditingGroup::SetWAVE_callback(Fl_Widget *CallingWidget)
{
    // Ask to the user to select a file.
    Fl_Native_File_Chooser file_chooser;
    file_chooser.title("Open WAVE audio file...");
#ifdef WIN32
    file_chooser.filter("WAVE Audio (*.wav)\t*.wav");
#else    // The parenthesis somehow break the non-Windows native file choosers?!...
    file_chooser.filter("WAVE Audio\t*.wav\n");
#endif
    file_chooser.show();
    if (file_chooser.count() == 1)
    {
        Create_Wave_Object( file_chooser.filename() );
        Fl::check();
        editing_lanes->playback_head->play_position(304);
        editing_lanes->scroll_to(1, editing_lanes->yposition());
        Fl::check();
    };
}

void CDGMagic_EditingGroup::Create_Wave_Object(const char* requested_file)
{
    if (requested_file == NULL)  { return; };

    delete Audio_Playback;
    Audio_Playback = NULL;

    Fl_Native_File_Chooser file_chooser;
    const char *file_to_open = requested_file;

    while (Audio_Playback == NULL)
    {
        try
        {
            Audio_Playback = new CDGMagic_AudioPlayback( file_to_open );
        }
        catch (int except)
        {
            play_pause_button->deactivate();
            // Allocate a temp buffer to store the user message.
            char *choice_message = new char[4096];
            // Format the message.
            snprintf(choice_message, 4096, "%s\n\nWAVE Exception: %i\n%s\n\nWould you like to open a different file?", file_to_open, except, CDGMagic_AudioPlayback::error_to_text(except) );
            // Ask the user if we should try again.
            int user_choice = fl_choice(choice_message, "No, give up.", "Yes, try to open a different file.", NULL);
            // Clean up the message buffer.
            delete[] choice_message;
            // If the user opted out, then just return, otherwise we'll go around again.
            if (user_choice == 0) { return; };
            // Try to open another file.
            file_chooser.type( Fl_Native_File_Chooser::BROWSE_FILE );
            file_chooser.title("Open WAVE audio file... (TURN DOWN VOLUME!)");
#ifdef WIN32
            file_chooser.filter("WAVE Audio (*.wav)\t*.wav");
#else    // The parenthesis somehow break the non-Windows native file choosers?!...
            file_chooser.filter("WAVE Audio\t*.wav\n");
#endif
            file_chooser.show();
            // Returns NULL if the hit cancel, so we'll just assume they didn't want to try again after all.
            if (file_chooser.count() != 1) { return; };
            file_to_open = file_chooser.filename();
        };
    };

    delete[] wave_rgb_bmp;
    delete cdg_gen;

    printf("Creating CDG encoder object, length (in packs)=%i\n", Audio_Playback->ViewFrames()*4);
    cdg_gen = new CDGMagic_GraphicsEncoder(Audio_Playback->ViewFrames()*4);

    float *wave_float_bmp = Audio_Playback->Get_WAVE_BMP();
    printf("WAVE_View_Length: %i\n", Audio_Playback->ViewFrames());

    wave_rgb_bmp = new unsigned char[Audio_Playback->ViewFrames() * 50 * 3];

    for (int px_x = 0; px_x < Audio_Playback->ViewFrames(); px_x++)
    {
      for (int px_y = 0; px_y < 50; px_y++)
      {
        int pix_pos = (Audio_Playback->ViewFrames()* px_y * 3) + (px_x * 3);
        if ( px_y < 25 )
        {
          if ( (24-px_y) <= wave_float_bmp[px_x*2] * 25.0 )
          {
              wave_rgb_bmp[pix_pos+0] = 0x00;
              wave_rgb_bmp[pix_pos+1] = 0xE0;
              wave_rgb_bmp[pix_pos+2] = 0x80;
          }
          else
          {
              wave_rgb_bmp[pix_pos+0] = 0x40;
              wave_rgb_bmp[pix_pos+1] = 0x40;
              wave_rgb_bmp[pix_pos+2] = 0x40;
          };
        }
        else
        {
          if ( (px_y-25) <= wave_float_bmp[px_x*2+1] * 25.0 )
          {
              wave_rgb_bmp[pix_pos+0] = 0x00;
              wave_rgb_bmp[pix_pos+1] = 0xE0;
              wave_rgb_bmp[pix_pos+2] = 0x80;
          }
          else
          {
              wave_rgb_bmp[pix_pos+0] = 0x40;
              wave_rgb_bmp[pix_pos+1] = 0x40;
              wave_rgb_bmp[pix_pos+2] = 0x40;
          };
        };
      };
    };

    if (wave_box)  { Fl::delete_widget( wave_box ); wave_box = NULL; };
    if (WAVE_Fl_RGB)  { delete WAVE_Fl_RGB; WAVE_Fl_RGB = NULL; };

    // TODO: This should probably resize the PAL box if it exists, instead of always deleting it.
    // Which is OK for now, but when global commands editing is implemented, this will blow away any changes the user has made(!).
    if (global_pal_box)  { Fl::delete_widget( global_pal_box ); global_pal_box = NULL; };

    WAVE_Fl_RGB = new Fl_RGB_Image( wave_rgb_bmp, Audio_Playback->TotalFrames(), 50, 3, 0);
    wave_box = new Fl_Box(editing_lanes->x()+1, editing_lanes->y()+15, Audio_Playback->TotalFrames(), 50);
    wave_box->box(FL_FLAT_BOX);
    wave_box->color(FL_BLACK, FL_BLACK);
    wave_box->image(WAVE_Fl_RGB);
    editing_lanes->add( wave_box );

    global_pal_box = new CDGMagic_PALGlobalClip(editing_lanes->x()+1, editing_lanes->y()+66, Audio_Playback->TotalFrames(), 50);
    TimeLine_Deque->push_back( global_pal_box );
    editing_lanes->add( global_pal_box );

    Audio_Playback->Start_Playback();
    play_pause_button->activate();
}

void CDGMagic_EditingGroup::DoCDGSave_callback(Fl_Widget *CallingWidget)
{
    CD_SCPacket *stream = cdg_gen->Get_Stream();

    // Ask to the user to select a file.
    Fl_Native_File_Chooser file_chooser;
    file_chooser.type( Fl_Native_File_Chooser::BROWSE_SAVE_FILE );
    file_chooser.title("Save CD+G Subcode file...");
#ifdef WIN32
    file_chooser.filter("CD+G Subcode File (*.cdg)\t*.cdg");
#else    // The parenthesis somehow break the non-Windows native file choosers?!...
    file_chooser.filter("CD+G Subcode File\t*.cdg\n");
#endif
    file_chooser.show();
    if (file_chooser.count() != 1)  { return; };

    // Make a string object for the filename, then check if we need to append the file extension.
    std::string save_path_string( file_chooser.filename() );
    unsigned int ext_pos = save_path_string.rfind(".cdg");
    if (ext_pos != (save_path_string.size() - 4))
    {
        save_path_string.append(".cdg");
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

    FILE *CDG_File = fopen( save_path_string.c_str(), "wb");
    if ( CDG_File == NULL )
    {
         fl_alert("Could not open file!");
         return;
    };
    fwrite(stream, cdg_gen->length(), sizeof(CD_SCPacket), CDG_File);
    fclose(CDG_File);
    printf("Output CDG file has been created!\n");
}

void CDGMagic_EditingGroup::DoProjectNew_callback(Fl_Widget *CallingWidget)
{
    int do_new = 0; // By default, don't clear the clips.
    // If clips exist on the time line, then ask the user if they're sure.
    if ( TimeLine_Deque->size() > 0 )
    {
        do_new = fl_choice("Are you sure you want to discard all changes and start over?", "No (continue working)", "Yes (discard)", NULL);
    }
    else  { do_new = 1; }  // There's no clips on the timeline, so it's safe to delete all (which is just, possibly, the audio clip).
    // Do the actual deletion, only if it should be done.
    if (do_new == 1)
    {
        printf("Deleting all timeline clips...\n");
        editing_lanes->playback_head->position(editing_lanes->x()-12, editing_lanes->playback_head->y());
        editing_lanes->redraw(); Fl::check();
        editing_lanes->scroll_to(0, editing_lanes->yposition());
        editing_lanes->redraw(); Fl::check();
        for (unsigned int clip = 0; clip < TimeLine_Deque->size(); clip++)
        {
            Fl_Widget *tmp_widget = dynamic_cast<Fl_Widget*>( TimeLine_Deque->at(clip) );
            Fl::delete_widget( tmp_widget );
        };
        TimeLine_Deque->clear();
	    global_pal_box = NULL;  // PAL clip is delete_widget'ed as part of the loop above.
        if (wave_box)        { Fl::delete_widget( wave_box ); wave_box = NULL;       };
        if (WAVE_Fl_RGB)     { delete   WAVE_Fl_RGB;          WAVE_Fl_RGB = NULL;    };
        if (wave_rgb_bmp)    { delete[] wave_rgb_bmp;         wave_rgb_bmp = NULL;   };
        if (Audio_Playback)  { delete   Audio_Playback;       Audio_Playback = NULL; };
        play_pause_button->deactivate();
        editing_lanes->redraw();
        Fl::check();
    };
}

void CDGMagic_EditingGroup::DoProjectSave_callback(Fl_Widget *CallingWidget)
{
    // Ask to the user to select a file.
    Fl_Native_File_Chooser file_chooser;
    file_chooser.type( Fl_Native_File_Chooser::BROWSE_SAVE_FILE );
    file_chooser.title("Save CD+Graphics Magic Project file...");
#ifdef WIN32
    file_chooser.filter("CD+Graphics Magic Project (*.cmp)\t*.cmp");
#else    // The parenthesis somehow break the non-Windows native file choosers?!...
    file_chooser.filter("CD+Graphics Magic Project\t*.cmp\n");
#endif
    file_chooser.show();
    if (file_chooser.count() != 1)  { return; };

    // Make a string object for the filename, then check if we need to append the file extension.
    std::string save_path_string( file_chooser.filename() );
    unsigned int ext_pos = save_path_string.rfind(".cmp");
    if (ext_pos != (save_path_string.size() - 4))
    {
        save_path_string.append(".cmp");
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

    char_vector* The_Save_Vector = new char_vector;
    CDGMagic_MediaClip* tmp_clip = new CDGMagic_MediaClip(0, 0);

    tmp_clip->add_text(The_Save_Vector, "CDGMagic_ProjectFile::" );

    if (Audio_Playback)
    {
        tmp_clip->add_text(The_Save_Vector, "CDGMagic_AudioPlayback::" );
        tmp_clip->add_text(The_Save_Vector, (char*) Audio_Playback->file_path() );
        tmp_clip->add_int(The_Save_Vector, editing_lanes->playback_head->play_position() );
    };

    tmp_clip->add_text(The_Save_Vector, "CDGMagic_TrackOptions::" );
    for (int trk = 0; trk < 8; trk++)
    {
        tmp_clip->add_char(The_Save_Vector, current_track_options[trk]->channel() );
    };

    tmp_clip->add_int(The_Save_Vector, static_cast<int>( TimeLine_Deque->size() ) );

    for (unsigned int tl_clip = 0; tl_clip < TimeLine_Deque->size(); tl_clip++)
    {
        TimeLine_Deque->at(tl_clip)->serialize( The_Save_Vector );
    };

    if ( The_Save_Vector->size() < 1 ) { fl_alert("Hmm... The save vector's size was insufficient!\nThis should not be possible."); return; };

    char* char_stream = &( The_Save_Vector->at(0) );

    FILE *PRJ_File = fopen( save_path_string.c_str(), "wb");
    if ( PRJ_File == NULL )
    {
        fl_alert("Could not open requested project save file!");
        delete The_Save_Vector;
        delete tmp_clip;
        return;
    };

    fwrite(char_stream, The_Save_Vector->size(), sizeof(char), PRJ_File);
    fclose(PRJ_File);
    printf("Project file might have been saved.\n");

    delete The_Save_Vector;
    delete tmp_clip;
}

void CDGMagic_EditingGroup::DoProjectOpen_callback(Fl_Widget *CallingWidget)
{
    // Delete any existing clips from the timeline.
    DoProjectNew_callback(NULL);
    // Hacky hack... Check for existence of timeline clips, indicating the user elected not to open a project above, and bail if found.
    if ( TimeLine_Deque->size() > 0 )  { return; };
    // Ask to the user to select a file.
    Fl_Native_File_Chooser file_chooser;
    file_chooser.title("Open CD+Graphics Magic Project file...");
#ifdef WIN32
    file_chooser.filter("CD+Graphics Magic Project (*.cmp)\t*.cmp");
#else    // The parenthesis somehow break the non-Windows native file choosers?!...
    file_chooser.filter("CD+Graphics Magic Project\t*.cmp\n");
#endif
    file_chooser.show();
    if (file_chooser.count() != 1)  { return; };

    FILE *PRJ_File = fopen(file_chooser.filename(), "rb");
    if ( PRJ_File == NULL )
    {
         fl_alert("Could not open file!");
         return;
    };
    // Put the file cursor at the end of the file.
    fseek(PRJ_File, 0, SEEK_END);
    // Get the end of file cursor position (i.e. the file size).
    int project_size = ftell(PRJ_File);
    int prj_allocate_size = project_size + 1024;
    // Set the file cursor back to the beginning of the file.
    rewind(PRJ_File);
    // Allocate a char array for the file.
    char* current_project = new char[prj_allocate_size];
    for (int byte = 0; byte < prj_allocate_size; byte++) { current_project[byte] = 0x00; };
    // Read the file in the array.
    fread(current_project, sizeof(char), project_size, PRJ_File);
    // Close the file.
    fclose(PRJ_File);

    // Attempt to recreate the objects.
    int prj_offset = 0;

    if ( strncmp( &current_project[prj_offset], "CDGMagic_ProjectFile::", strlen("CDGMagic_ProjectFile::")) == 0 )
    {
        prj_offset += strlen("CDGMagic_ProjectFile::") + 1;
        printf("prj_offset: %i\n", prj_offset);
        if ( strncmp( &current_project[prj_offset], "CDGMagic_AudioPlayback::", strlen("CDGMagic_AudioPlayback::")) != 0 ) { printf("Weird audio\n"); return; };
        prj_offset += strlen("CDGMagic_AudioPlayback::") + 1;
        printf("prj_offset: %i\n", prj_offset);
        // Something like the following \ to / conversion might be needed for non-Windows OSes?...
        //while ( curr_file.find("\\") != curr_file.npos )  { curr_file.replace(curr_file.find("\\"), 1, "/"); };
        printf("Trying to open: %s\n", &current_project[prj_offset] );
//        Fl::check();
        // Try to create an audio object with using the given pathname.
        Create_Wave_Object( &current_project[prj_offset] );
        // If no audio object was created, then free memory and stop any further processing.
        if (Audio_Playback == NULL)  { delete[] current_project; return; };
        // Advance the offset counter.
        prj_offset += strlen( &current_project[prj_offset] ) + 1;
        printf("prj_offset: %i\n", prj_offset);

/*
        int set_play_pos = 0;
        set_play_pos |= (current_project[prj_offset++] << 030);
        set_play_pos |= (current_project[prj_offset++] << 020);
        set_play_pos |= (current_project[prj_offset++] << 010);
        set_play_pos |= (current_project[prj_offset++] << 000);
*/
        prj_offset += 4;
        printf("prj_offset: %i\n", prj_offset);

        if ( strncmp( &current_project[prj_offset], "CDGMagic_TrackOptions::", strlen("CDGMagic_TrackOptions::")) != 0 ) { printf("No track options!\n"); return; };
        prj_offset += strlen("CDGMagic_TrackOptions::") + 1;
        for (int trk = 0; trk < 8; trk++)
        {
            current_track_options[trk]->channel( current_project[prj_offset++] );
        };

        // Now step through and instanciate clips.
        int num_events = 0;
        num_events |= (current_project[prj_offset++] << 030);
        num_events |= (current_project[prj_offset++] << 020);
        num_events |= (current_project[prj_offset++] << 010);
        num_events |= (current_project[prj_offset++] << 000);
        printf("prj_offset: %i\n", prj_offset);

        int failed_clips = 0;
        printf("Trying to open %i events...\n", num_events);
        for (int curr_evnt = 0; curr_evnt < num_events; curr_evnt++)
        {
            if ( strncmp( &current_project[prj_offset], "CDGMagic_BMPClip::", strlen("CDGMagic_BMPClip::")) == 0 )
            {
                printf("##### CDGMagic_BMPClip #####\n");
                prj_offset += strlen("CDGMagic_BMPClip::") + 1;
                printf("prj_offset: %i\n", prj_offset);
                // Create a new BitMap Loader object pointer.
                CDGMagic_BMPClip *current_clip = new CDGMagic_BMPClip( editing_lanes->x()+1, editing_lanes->y()+15, 50, editing_lanes->lane_height() );
                // Set the shared play time variable.
                current_clip->set_playtime_var( &current_play_position );
                // Assign the callback that's performed on the mouse-up action.
                current_clip->callback(Clip_PositionChange_callback_static, this);
                // Add the clip "box" to the Media Lane object.
                editing_lanes->add(current_clip);
                // Attempt to deserialize.
                prj_offset += current_clip->deserialize( &current_project[prj_offset] );
                // Make sure we have at least one good bitmap.
                if ( current_clip->event_queue()->size() < 1 )
                {
                    printf("CDGMagic_EditingGroup::DoProjectOpen_callback - Failed to open or allocate bitmap object!\n");
                    Fl::delete_widget(current_clip);
                    failed_clips++;
                    continue;
                };
                // Fire the callback to update various stuff.
                //current_clip->do_callback();
                // Add the clip to the deque of timeline clips.
                //REMOVE
                printf("CDGMagic_EditingGroup::AddBMP_callback - s: %i, events: %i\n", current_clip->start_pack(), current_clip->event_queue()->size() );
                //printf("CDGMagic_EditingGroup::AddBMP_callback - s: %d, events: %zu\n", current_clip->start_pack(), current_clip->event_queue()->size());
                TimeLine_Deque->push_back(current_clip);
                continue;
            };

            if ( strncmp( &current_project[prj_offset], "CDGMagic_TextClip::", strlen("CDGMagic_TextClip::")) == 0 )
            {
                printf("##### CDGMagic_TextClip #####\n");
                prj_offset += strlen("CDGMagic_TextClip::") + 1;
                printf("prj_offset: %i\n", prj_offset);
                // Create a new text clip object pointer.
                CDGMagic_TextClip *current_clip = new CDGMagic_TextClip( editing_lanes->x()+1, editing_lanes->y()+15, 50, editing_lanes->lane_height() );
                // Set the shared play time variable.
                current_clip->set_playtime_var( &current_play_position );
                // Assign the callback that's performed on the mouse-up action.
                current_clip->callback(Clip_PositionChange_callback_static, this);
                // Add the clip "box" to the Media Lane object.
                editing_lanes->add(current_clip);
                // Attempt to deserialize.
                prj_offset += current_clip->deserialize( &current_project[prj_offset] );
                // Fire the callback to update various stuff.
                //current_clip->do_callback();
                // Add the clip to the deque of timeline clips.
                printf("CDGMagic_EditingGroup::AddText_callback - s: %i, events: %i\n", current_clip->start_pack(), current_clip->event_queue()->size() );
                TimeLine_Deque->push_back(current_clip);
                continue;
            };

            printf("##### Unknown clip type! #####\n");
        };
        // Set the play head position.
        editing_lanes->playback_head->play_position(304);
        editing_lanes->scroll_to( 1, editing_lanes->yposition() );
        Fl::check();

        for (unsigned int wdgt = 0; wdgt < TimeLine_Deque->size(); wdgt++)
        {
            Fl_Widget *tmp_widget = dynamic_cast<Fl_Widget*>( TimeLine_Deque->at(wdgt) );
            tmp_widget->do_callback();
        };
        Fl::check();
        // Let the user know if clips failed to load.
        if (failed_clips)  {  fl_alert("Errors prevented %i clip(s) from being loaded!", failed_clips);  };
    }
    else
    {
        fl_alert("The selected file does not appear to be a CD+Graphics Magic Project!");
    };

    // Give back the allocated array.
    delete[] current_project;
    // Render the current project preview.
    RecalculatePreview_callback(NULL);
}

void CDGMagic_EditingGroup::DoResync_callback_static(Fl_Widget *CallingWidget, void *IncomingObject)
{
    CDGMagic_EditingGroup* tmp_this = static_cast<CDGMagic_EditingGroup*>( IncomingObject );

    for (unsigned int wdgt = 0; wdgt < tmp_this->TimeLine_Deque->size(); wdgt++)
    {
        Fl_Widget *tmp_widget = dynamic_cast<Fl_Widget*>( tmp_this->TimeLine_Deque->at(wdgt) );
        tmp_widget->position(tmp_widget->x(), tmp_widget->y());
        tmp_widget->do_callback();
    };
}

void CDGMagic_EditingGroup::samplerate_select_cb_static(Fl_Widget *CallingWidget, void *IncomingObject)
{
    CDGMagic_EditingGroup *tmp_obj = static_cast<CDGMagic_EditingGroup*>( IncomingObject );
    Fl_Choice *tmp_choice = (Fl_Choice*)CallingWidget;
    if ( tmp_obj->Audio_Playback != NULL )
    {
        tmp_obj->Audio_Playback->Set_SampleRate( static_cast<int>(tmp_choice->value()) );
    };
}

// ###################################################################
// # Duplicate functions due to FLTK requiring "static" callbacks... #
// ###################################################################

void CDGMagic_EditingGroup::PBHead_PositionChange_callback_static(Fl_Widget *CallingWidget, void *IncomingObject)
{
    ((CDGMagic_EditingGroup*)IncomingObject)->PBHead_PositionChange_callback(CallingWidget);
}

void CDGMagic_EditingGroup::Clip_PositionChange_callback_static(Fl_Widget *CallingWidget, void *IncomingObject)
{
    ((CDGMagic_EditingGroup*)IncomingObject)->Clip_PositionChange_callback(CallingWidget);
}

void CDGMagic_EditingGroup::PlayPause_callback_static(Fl_Widget *CallingWidget, void *IncomingObject)
{
    ((CDGMagic_EditingGroup*)IncomingObject)->PlayPause_callback( (Fl_Light_Button*) CallingWidget);
}

void CDGMagic_EditingGroup::RecalculatePreview_callback_static(Fl_Widget *CallingWidget, void *IncomingObject)
{
    ((CDGMagic_EditingGroup*)IncomingObject)->RecalculatePreview_callback(CallingWidget);
}

void CDGMagic_EditingGroup::AddBMP_callback_static(Fl_Widget *CallingWidget, void *IncomingObject)
{
    ((CDGMagic_EditingGroup*)IncomingObject)->AddBMP_callback(CallingWidget);
}

void CDGMagic_EditingGroup::AddText_callback_static(Fl_Widget *CallingWidget, void *IncomingObject)
{
    ((CDGMagic_EditingGroup*)IncomingObject)->AddText_callback(CallingWidget);
}
#ifdef _CDGMAGIC_ENABLE_SCROLLCLIP_
void CDGMagic_EditingGroup::AddScroll_callback_static(Fl_Widget *CallingWidget, void *IncomingObject)
{
    ((CDGMagic_EditingGroup*)IncomingObject)->AddScroll_callback(CallingWidget);
}
#endif
#ifdef _CDGMAGIC_ENABLE_VECTORCLIP_
void CDGMagic_EditingGroup::AddVector_callback_static(Fl_Widget *CallingWidget, void *IncomingObject)
{
    ((CDGMagic_EditingGroup*)IncomingObject)->AddVector_callback(CallingWidget);
}
#endif
void CDGMagic_EditingGroup::SetWAVE_callback_static(Fl_Widget *CallingWidget, void *IncomingObject)
{
    ((CDGMagic_EditingGroup*)IncomingObject)->SetWAVE_callback(CallingWidget);
}

void CDGMagic_EditingGroup::DoProjectNew_callback_static(Fl_Widget *CallingWidget, void *IncomingObject)
{
    ((CDGMagic_EditingGroup*)IncomingObject)->DoProjectNew_callback(CallingWidget);
}

void CDGMagic_EditingGroup::DoProjectSave_callback_static(Fl_Widget *CallingWidget, void *IncomingObject)
{
    ((CDGMagic_EditingGroup*)IncomingObject)->DoProjectSave_callback(CallingWidget);
}

void CDGMagic_EditingGroup::DoProjectOpen_callback_static(Fl_Widget *CallingWidget, void *IncomingObject)
{
#ifdef _CDGMAGIC_NOLANESHANDLEREDRAW_
    ((CDGMagic_EditingGroup*)IncomingObject)->editing_lanes->set_handler_redraw(1);  // Don't ask...
#endif
    ((CDGMagic_EditingGroup*)IncomingObject)->DoProjectOpen_callback(CallingWidget);
#ifdef _CDGMAGIC_NOLANESHANDLEREDRAW_
    ((CDGMagic_EditingGroup*)IncomingObject)->editing_lanes->set_handler_redraw(0);  // Don't ask...
#endif
}

void CDGMagic_EditingGroup::DoCDGSave_callback_static(Fl_Widget *CallingWidget, void *IncomingObject)
{
    ((CDGMagic_EditingGroup*)IncomingObject)->DoCDGSave_callback(CallingWidget);
}
