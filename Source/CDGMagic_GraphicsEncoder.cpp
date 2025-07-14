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

#include "CDGMagic_GraphicsEncoder.h"

CDGMagic_GraphicsEncoder::CDGMagic_GraphicsEncoder(const unsigned int new_length, const unsigned int new_width, const unsigned int new_height)
{
    // Set the variables for the optional compositing width/height.
    comp_width  = (new_width  < VRAM_WIDTH)  ? VRAM_WIDTH  : new_width;
    comp_height = (new_height < VRAM_HEIGHT) ? VRAM_HEIGHT : new_height;
    // Set the internal length variable to the new requested length.
    current_length = new_length;
    // Allocate memory for the raw subcode stream.
    cdg_stream     = new CD_SCPacket[current_length];
    // Defaultish values, just so all members are initialized to "something".
    vram = NULL;
    comp_buffer       = NULL;
    g_palette         = NULL;
    eg_palette        = NULL;
    last_preset_index =    0;
    last_border_index =    0;
    last_x_offset     =    0;
    last_y_offset     =    0;
    x_block_offset    =    0;
    y_block_offset    =    0;

    // TODO: Split the persistent memory sets from the temp allocation/set/delete.
    // Then, only set the persistent vars to a known state here.
    begin_computation();  // Reset all the vars.
    end_computation();    // Clean up the allocated memory.
}

CDGMagic_GraphicsEncoder::~CDGMagic_GraphicsEncoder()
{
    delete[] cdg_stream;
}

void CDGMagic_GraphicsEncoder::begin_computation()
{
    // Allocate memory for 300 x 216 x 8bit video RAM buffer.
    vram        = new unsigned short[VRAM_WIDTH * VRAM_HEIGHT];
    // Allocate memory for the 8 track compositing "engine" (mighty big word for what it does :-) ).
    // This is set the user requested size, to accommodate composited scrolling ("panning").
    // TODO: Buffer written font blocks somehow instead,
    // instead of allocating large blocks of memory that will likely serve little purpose.
    comp_buffer = new unsigned short[comp_width * comp_height * COMP_LAYERS];
    // Allocate memory for the 16 entry, 4 bytes per pixel TV Graphics look up table (palette).
    g_palette   = new unsigned long[CLUT_SIZE_G];
    // Allocate memory for the 256 entry, 4 bytes per pixel Extended Graphics look up table (palette).
    eg_palette  = new unsigned long[CLUT_SIZE_EG];

    // Set the memory/variables to sane, known values (basically all 0s).
    // Using memset for now... std::fill might be a better choice in C++, though???

    // Clear the VRAM to index 0.
    for (unsigned int px = 0; px < VRAM_WIDTH * VRAM_HEIGHT; px++)  { vram[px] = 0x00; };
    // Clear the compositing engine to 256 (all transparent).
    for (unsigned int px = 0; px < comp_width * comp_height * COMP_LAYERS; px++)  { comp_buffer[px] = 256; };
    // Clear the raw subcode commands stream.
    memset( cdg_stream, 0, current_length*sizeof(CD_SCPacket) );
    // Clear the 16 color TV Graphics look up table. (RGBA order, Alpha is opaque by default).
    for (int idx = 0; idx < CLUT_SIZE_G; idx++) { g_palette[idx] = 0x000000FF; };
    // Clear the 256 color Extended Graphics look up table. (RGBA order, Alpha is opaque by default).
    for (int idx = 0; idx < CLUT_SIZE_EG; idx++) { eg_palette[idx] = 0x000000FF; };
    // Set the memory preset index to default (usually 0).
    last_preset_index = 0;
    // Set the border index to default (usually 0).
    last_border_index = 0;
    // Set the scroll offsets to 0.
    last_x_offset = 0;
    last_y_offset = 0;
    // Set the block offsets to 0.
    x_block_offset = 0;
    y_block_offset = 0;
}

void CDGMagic_GraphicsEncoder::end_computation()
{
    // Deallocate the memory at the end of the computation.
    delete[] eg_palette;
    delete[] g_palette;
    delete[] comp_buffer;
    delete[] vram;
}

int  CDGMagic_GraphicsEncoder::compute_graphics(std::deque<CDGMagic_MediaClip*> *TimeLine_Deque)
{
    // Clear out the old stream data and allocate the buffers.
    begin_computation();

    // First, sort the timeline deque items by time position.
    std::sort(TimeLine_Deque->begin(), TimeLine_Deque->end(), timeline_sorter);

    // Then, go through and make sure all the event queues have been sorted.
    for (unsigned int cur_evnt = 0; cur_evnt < TimeLine_Deque->size(); cur_evnt++)
    {
        TimeLine_Deque->at(cur_evnt)->sort_event_queue();
    };

    // Print out a bit of useless info.
    printf("Now generating graphics data for %i events...\n", TimeLine_Deque->size() );

    // Ugh, this really needs to fixed... Somehow...
    std::deque< Timeline_FontQueue > playout_queue;  // Deque of fonts to write.
    std::deque< Timeline_EventQueue > global_queue;  // Deque of global events to write.
    FontBlock_Deque                    xor_playout;  // Deque of xor-only fonts, for karaoke highlighting,

             int current_pack    = 0;
    unsigned int current_event   = 0;
    unsigned int current_playout = 0;

    // Step through every pack, and put an event there if needed.
    //for ( int current_pack = 0; current_pack < current_length-300; current_pack++ )
    while (current_pack < current_length-300)
    {
        // If the current_event is less than total number of events, then we have more to do.
        // If the event start pack is equal to or prior to where we are, then queue up the event.
        if ((current_event < TimeLine_Deque->size()) && (TimeLine_Deque->at(current_event)->start_pack() <= current_pack))
        {
            printf("event: %i / %i, requested pack: %i, actual pack: %i\n", current_event, TimeLine_Deque->size(), TimeLine_Deque->at(current_event)->start_pack(), current_pack );

            // Go through and convert all the current event's bitmap data into a deque of font blocks.
            // Then add them to the playout queue.
            Timeline_FontQueue temp_tl_fq;
            temp_tl_fq.event_num   = current_event;
            temp_tl_fq.start_pack  = TimeLine_Deque->at(current_event)->start_pack();
            temp_tl_fq.font_queue  = bmp_to_fonts( TimeLine_Deque->at(current_event) );
            playout_queue.push_back( temp_tl_fq );

            // XOR only blocks are a special case, which are handled seperately for now.
            // Then add them to the XOR-only playout queue.
            int num_of_xor_fonts = xor_to_fonts( TimeLine_Deque->at(current_event), &xor_playout );
            if (num_of_xor_fonts > 0)
            {
                // Sort it if there were new font blocks added.
                std::stable_sort(xor_playout.begin(), xor_playout.end(), xor_fontblock_sorter);
            };

            // Very hacky, but for now just copy the event queue out.
            for (unsigned int evnt = 0; evnt < TimeLine_Deque->at(current_event)->event_queue()->size(); evnt++)
            {
                CDGMagic_MediaEvent* current_mediaevent = ( TimeLine_Deque->at(current_event)->event_queue()->at(evnt) );

                Timeline_EventQueue temp_tl_ev;
                temp_tl_ev.event_num   = current_event;
                temp_tl_ev.start_pack  = TimeLine_Deque->at(current_event)->start_pack() + current_mediaevent->start_offset;
                temp_tl_ev.event_obj   = current_mediaevent;
                global_queue.push_back( temp_tl_ev );
            };
            // And then sort it.
            std::sort(global_queue.begin(), global_queue.end(), timeline_event_sorter);

            // Advance the current_event counter for the next event.
            current_event++;
        };

        while( (global_queue.size() > 0) && (global_queue.front().start_pack <= current_pack) )
        {
            // Assign a convenience pointer to the media event.
            CDGMagic_MediaEvent* current_mediaevent = global_queue.front().event_obj;
            // Check for any global (affects/applies to all tracks/clips) graphics events.
            // Set the scroll offset and/or perform a scroll, if the offsets are valid (>= 0).
            //if ( (current_mediaevent->x_scroll >= 0) || (current_mediaevent->y_scroll >= 0) )
            // Hacky hack to temporarily add support for the "zero offsets" clip.
            if ( (current_mediaevent->x_scroll == 0) && (current_mediaevent->y_scroll == 0) )
            {
                Timeline_FontQueue temp_tl_fq;
                temp_tl_fq.event_num   = global_queue.front().event_num;
                temp_tl_fq.start_pack  = current_pack;
                // Set scroll should automatically select between copy/preset variants,
                // add any offscreen fonts that need to be drawn back in to the font queue,
                // set the scroll command, and advance the current_pack offset as necessary.
                // (Of course, it does pretty much none of those things at present...)
                temp_tl_fq.font_queue  = set_scroll( current_pack, current_mediaevent );
                playout_queue.push_back( temp_tl_fq );
            };

            // If a PALObject exists, and it should fade/dissolve, then calculate and create the new inter-palettes.
            if ( (current_mediaevent->PALObject != NULL) && (current_mediaevent->PALObject->dissolve_interval() > 0) )
            {
                // Very hacky way to handle fading/dissolving from the current palette to a new palette.
                // This is about four kinds of evil and is (mostly) just to test some fading/dissolving techniques.

                // Convenience variables for start time, length, and steps.
                const int dissolve_start    = global_queue.front().start_pack;
                const int dissolve_interval = current_mediaevent->PALObject->dissolve_interval();
                const int dissolve_steps    = current_mediaevent->PALObject->dissolve_steps();
                // Convenience variable for the destination/ending palette values.
                CDGMagic_PALObject *end_pal = current_mediaevent->PALObject;
                // Go through the palette calculating RGB values as necessary until equal to the destination.
                // NOTE: This is probably not the best/smoothest/most chroma-accurate way to fade... It works alright for testing, though.
                for (int progress = 1; progress <= dissolve_steps; progress++)
                {
                    double dissolve_progress = static_cast<double>(progress) / static_cast<double>(dissolve_steps);
                    // Create a new PAL Object, and set the colors and mask.
                    CDGMagic_PALObject  *new_pal = new CDGMagic_PALObject();
                    new_pal->update_mask( end_pal->update_mask() );
                    // Add/subtract the actual RGB values.
                    for (int idx = 0; idx < 16; idx++)
                    {
                        // Ignore unused color indices.
                        if ( ((end_pal->update_mask() >> idx) & 0x01) == 0 )  { continue; };
                        // Store the ending packed RGBA value.
                        unsigned long end_color = end_pal->color(idx);
                        // Calculate the interim color values.
                        unsigned char inter_red   = calculate_inter_rgb_value( (g_palette[idx] >> 030) & 0xFF, (end_color >> 030) & 0xFF, dissolve_progress);
                        unsigned char inter_green = calculate_inter_rgb_value( (g_palette[idx] >> 020) & 0xFF, (end_color >> 020) & 0xFF, dissolve_progress);
                        unsigned char inter_blue  = calculate_inter_rgb_value( (g_palette[idx] >> 010) & 0xFF, (end_color >> 010) & 0xFF, dissolve_progress);
                        // Pack the interim color value to set.
                        end_color  = 0x000000FF;
                        end_color |= ((inter_red   & 0xFF) << 030);
                        end_color |= ((inter_green & 0xFF) << 020);
                        end_color |= ((inter_blue  & 0xFF) << 010);
                        // Set the new RGB value for this index.
                        new_pal->color(idx, end_color);
                    };
                    // Create a new media event.
                    CDGMagic_MediaEvent *new_event = new CDGMagic_MediaEvent();
                    new_event->PALObject = new_pal;
                    // Copy the old initial values to the new event IF THIS IS THE FIRST, so they don't get thrown away.
                    new_event->border_index        = (progress == 1) ? current_mediaevent->border_index        : 16;
                    new_event->memory_preset_index = (progress == 1) ? current_mediaevent->memory_preset_index : 16;
                    new_event->x_scroll = -1;
                    new_event->y_scroll = -1;
                    // Make a temporary event queue structure.
                    Timeline_EventQueue temp_tl_ev;
                    // Hacky hack, set the event number to -1 (which will never be used by the unsigned event number)...
                    // This indicates the event must be deleted here after it's used... Ugh...
                    temp_tl_ev.event_num   = -1;
                    // Calculate where the next palette change will take place.
                    temp_tl_ev.start_pack  = dissolve_start + (dissolve_interval * (progress-1));
                    // Assign the newly create media event to the temp queue.
                    temp_tl_ev.event_obj   = new_event;
                    // Add it to the global events queue.
                    global_queue.push_back( temp_tl_ev );
                };
                // Remove the initial event.
                global_queue.pop_front();
                // Since a bunch of events were just added, resort the global event queue.
                std::sort(global_queue.begin(), global_queue.end(), timeline_event_sorter);
                // Reassign the convenience pointer to the first event in the queue.
                current_mediaevent = global_queue.front().event_obj;
            };

            // Set the palette, if a PALObject exists.
            if ( current_mediaevent->PALObject != NULL )  {  current_pack = set_pal( current_pack, current_mediaevent );  };
            // Set the border color, if the border index is valid (less than 16).
            if ( current_mediaevent->border_index  < 16 )  {  current_pack = set_border(  current_pack, current_mediaevent );  };
            // Clear the screen, if the memory preset index is valid (less than 16).
            if ( current_mediaevent->memory_preset_index < 16 )  {  current_pack = set_memory( current_pack, current_mediaevent );  };

            // Clean up the hacked in events above, if needed.
            if ( global_queue.front().event_num == -1 )
            {
                delete ( current_mediaevent->PALObject );
                delete ( current_mediaevent );
            };

            // Remove the event from the deque.
            global_queue.pop_front();
        };

        if (xor_playout.size() > 0)
        {
            while(  (xor_playout.size() > 0)
                 && (xor_playout.front()->start_pack() <= current_pack) )
            {
                current_pack = write_fontblock( current_pack, xor_playout.front() );
                delete ( xor_playout.front() );
                xor_playout.pop_front();
            };
        };

        // Store the starting pack time for after the conditional.
        int orig_pack = current_pack;
        // Check if there might be something to write out.
        if (playout_queue.size() > 0)
        {
            for (unsigned int que_cnt = 0; que_cnt < playout_queue.size(); que_cnt++)
            {
                // If the playout counter is over the number of queued playouts, then go back to the beginning.
                if (current_playout >= playout_queue.size())  {  current_playout = 0;  };

                //printf("current_pack=%i, que_cnt=%i, current_playout=%i, playout_queue.size()=%i, font_queue.size()=%i\n",
                //        current_pack,    que_cnt,    current_playout,    playout_queue.size(),    playout_queue.at(current_playout).font_queue.size() );

                // Check if there's actually something to write.
                if ( (playout_queue[current_playout].font_queue.size() > 0) && (playout_queue[current_playout].font_queue.front()->start_pack() <= current_pack) )
                {
                    // Go through the packs in this event's queue until we either:
                    // Find a pack to write, run out of font blocks, or reach an event later than the current time.
                    while ( (current_pack == orig_pack) &&
                            (playout_queue[current_playout].font_queue.size() > 0) &&
                            (playout_queue[current_playout].font_queue.front()->start_pack() <= current_pack ) )
                    {
                        // Write out the font block to the screen.
                        current_pack = write_fontblock( current_pack, playout_queue[current_playout].font_queue.front() );
                        // Delete the font block (which was a pointer), so we don't leak...
                        // NOTE: This doesn't follow RAII principle... It should be redesigned so it does.
                        delete ( playout_queue[current_playout].font_queue.front() );
                        // Remove the block from the queue.
                        playout_queue[current_playout].font_queue.pop_front();
                    };
                    // Stupid break from next for loop...
                    que_cnt = playout_queue.size();
                };
                // Check if that was the last block in the current clip's queue to write.
                if (playout_queue[current_playout].font_queue.size() <= 0)
                {
                    printf("erasing playout queue member %i\n", current_playout);
                    // Set the new end time.
                    TimeLine_Deque->at(playout_queue.at(current_playout).event_num)->duration(current_pack - playout_queue.at(current_playout).start_pack);
                    // Remove the clip from the queue if it was.
                    playout_queue.erase( playout_queue.begin()+current_playout );
                }
                else
                {
                    // Otherwise, advance the playout counter to get a block from the next clip.
                    current_playout++;
                };
            };
        };
        // Make sure the counter gets advanced, even if nothing was drawn.
        if ( current_pack == orig_pack )  { current_pack++; };
    };

    // Clear the global events queue...
    while(global_queue.size() > 0)
    {
        if ( global_queue.front().event_num == -1 )
        {
            delete ( global_queue.front().event_obj->PALObject );
            delete ( global_queue.front().event_obj );
        };
        global_queue.pop_front();
    };
    // Clean up the XOR (currently only karaoke) queue.
    while(xor_playout.size() > 0)  { delete ( xor_playout.front() ); xor_playout.pop_front(); };
    // Clean up the font blocks playout queue.
    while(playout_queue.size() > 0)
    {
        while( playout_queue.front().font_queue.size() > 0 )
        {
            delete ( playout_queue.front().font_queue.front() );
            playout_queue.front().font_queue.pop_front();
        };
        printf("erasing playout queue member BEYOND TIMELINE END!\n");
        // Set the new end time.
        TimeLine_Deque->at(playout_queue.front().event_num)->duration(current_pack - playout_queue.front().start_pack);
        // Remove the clip from the queue.
        playout_queue.pop_front();
    };
    // Do some memory cleanup.
    end_computation();

    // Successfully generated the graphics stream... (This can't properly fail, yet.)
    return 1;
}

unsigned char CDGMagic_GraphicsEncoder::calculate_inter_rgb_value(unsigned char initial_value, unsigned char ending_value, double inter_progress)
{
    // Calculate the difference between the starting and ending value.
    double value_difference = ending_value - initial_value;
    // Calculate the progress from the initial to ending value.
    double inter_difference = value_difference * inter_progress;
    // Add the current inter difference to the initial value to obtain the actual current progress,
    // and give the result to the caller.
    return ((initial_value + inter_difference) / 17 * 17);
}

int CDGMagic_GraphicsEncoder::set_pal(int current_position, CDGMagic_MediaEvent *the_event)
{
    for (int lo_or_hi = 0; lo_or_hi < 2; lo_or_hi++)
    {
        // Should we load the lo (0-7) or hi (8-15) color set?
        int pal_offset = lo_or_hi * 8;
        // Move on to the next iteration if there's no colors to set for this portion of the palette.
        if ( ((the_event->PALObject->update_mask() >> pal_offset) & 0xFF) == 0 )  { continue; };
        // Command is TV Graphics. (Extended Graphics uses different CLUT commands.)
        cdg_stream[current_position].command = TV_GRAPHICS;
        // Set the instruction.
        cdg_stream[current_position].instruction = (lo_or_hi) ? LOAD_CLUT_HI : LOAD_CLUT_LO;
        // Go through and actually set the colors.
        for (int pal_inc = 0; pal_inc < 8; pal_inc++)
        {
            // Calculate the actual index value.
            unsigned int actual_idx = pal_inc + pal_offset;
            // Set the current palette RGB value to either the existing or new value as dictated by the update mask.
            g_palette[actual_idx] = ((the_event->PALObject->update_mask() >> actual_idx) & 0x01) ? the_event->PALObject->color(actual_idx) : g_palette[actual_idx];
            // Set the current temporary RGB value to the current palette RGB value for this index.
            unsigned long temp_rgb = g_palette[actual_idx];
            // Pack the 4bit RGB values into subcode format.
            cdg_stream[current_position].data[pal_inc*2+0]  = (((temp_rgb >> 030) & 0xFF) / 17) << 2;         // Red.
            cdg_stream[current_position].data[pal_inc*2+0] |= (((temp_rgb >> 020) & 0xFF) / 17) >> 2;         // Green hi.
            cdg_stream[current_position].data[pal_inc*2+1]  = ((((temp_rgb >> 020) & 0xFF) / 17) & 0x03) << 4;// Green lo.
            cdg_stream[current_position].data[pal_inc*2+1] |= (((temp_rgb >> 010) & 0xFF) / 17);              // Blue
        };
        // Incrememnt the stream position.
        current_position++;
    };
    // Return the position.
    return current_position;
}

int CDGMagic_GraphicsEncoder::set_memory(int current_position, CDGMagic_MediaEvent *the_event)
{
    // Write out the first 8 preset packets normally.
    for (int repeat_value = 0; repeat_value < 8; repeat_value++)
    {
        // Command is TV Graphics.
        cdg_stream[current_position].command = TV_GRAPHICS;
        // Instruction is load memory preset.
        cdg_stream[current_position].instruction = MEMORY_PRESET;
        // Color index is the requested index.
        cdg_stream[current_position].data[0] = the_event->memory_preset_index;
        // Repeat value is the current repeat value.
        cdg_stream[current_position].data[1] = repeat_value;
        // Incrememnt the packet position.
        current_position++;
    };
    // Then write out the last 8 with the program name in the "extra" 14 data bytes.
    // Some rare players might think the data is bad if it's not all 0x00,
    // but the first 8 "good" packets should hopefully appease them.
    // TODO: Make the additional data here optional, and with user configurable text.
    // (There's actually 16*14=224 potential characters (upper case only) of "message" space available.)
    for (int repeat_value = 8; repeat_value < 16; repeat_value++)
    {
        // Command is TV Graphics.
        cdg_stream[current_position].command = TV_GRAPHICS;
        // Instruction is load memory preset.
        cdg_stream[current_position].instruction = MEMORY_PRESET;
        // Color index is the requested index.
        cdg_stream[current_position].data[0x00] = the_event->memory_preset_index;
        // Repeat value is the current repeat value.
        cdg_stream[current_position].data[0x01] = repeat_value;
        cdg_stream[current_position].data[0x02] = ('C' - 0x20) & 0x3F;
        cdg_stream[current_position].data[0x03] = ('D' - 0x20) & 0x3F;
        cdg_stream[current_position].data[0x04] = ('+' - 0x20) & 0x3F;
        cdg_stream[current_position].data[0x05] = ('G' - 0x20) & 0x3F;
        cdg_stream[current_position].data[0x06] = ('M' - 0x20) & 0x3F;
        cdg_stream[current_position].data[0x07] = ('A' - 0x20) & 0x3F;
        cdg_stream[current_position].data[0x08] = ('G' - 0x20) & 0x3F;
        cdg_stream[current_position].data[0x09] = ('I' - 0x20) & 0x3F;
        cdg_stream[current_position].data[0x0A] = ('C' - 0x20) & 0x3F;
        cdg_stream[current_position].data[0x0B] = (' ' - 0x20) & 0x3F;
        cdg_stream[current_position].data[0x0C] = ('0' - 0x20) & 0x3F;
        cdg_stream[current_position].data[0x0D] = ('0' - 0x20) & 0x3F;
        cdg_stream[current_position].data[0x0E] = ('1' - 0x20) & 0x3F;
        cdg_stream[current_position].data[0x0F] = ('B' - 0x20) & 0x3F;
        // Incrememnt the packet position.
        current_position++;
    };
    // Clear the VRAM to the requested index.
    for (unsigned int px = 0; px < VRAM_WIDTH * VRAM_HEIGHT; px++)  { vram[px] = the_event->memory_preset_index; };
    // Clear the compositing engine to 256 (all transparent).
    for (unsigned int px = 0; px < comp_width * comp_height * COMP_LAYERS; px++)  { comp_buffer[px] = 256; };
    // Set the last used memory preset index to this index.
    last_preset_index = the_event->memory_preset_index;
    // Return the position.
    return current_position;
}

int CDGMagic_GraphicsEncoder::set_border(int current_position, CDGMagic_MediaEvent *the_event)
{
    // Command is TV Graphics.
    cdg_stream[current_position].command = TV_GRAPHICS;
    // Instruction is load border preset.
    cdg_stream[current_position].instruction = BORDER_PRESET;
    // Color index is the requested index.
    cdg_stream[current_position].data[0] = the_event->border_index;
    // Incrememnt the packet position.
    current_position++;
    // Set the last used border index to this index.
    last_border_index = the_event->border_index;
    // Return the position.
    return current_position;
}

FontBlock_Deque CDGMagic_GraphicsEncoder::set_scroll(int &current_position, CDGMagic_MediaEvent *the_event)
{
    // Make the return value.
    FontBlock_Deque font_deque_return;
    // Command is TV Graphics.
    cdg_stream[current_position].command = TV_GRAPHICS;
    // Instruction is scroll.
    cdg_stream[current_position].instruction = SCROLL_COPY;
    cdg_stream[current_position].data[1] = (the_event->x_scroll >= 0) ? the_event->x_scroll : last_x_offset ;
    cdg_stream[current_position].data[2] = (the_event->y_scroll >= 0) ? the_event->y_scroll : last_y_offset ;
    last_x_offset = cdg_stream[current_position].data[1] & 0x0F;
    last_y_offset = cdg_stream[current_position].data[2] & 0x0F;
/*
    if ( cdg_stream[current_position].data[1] & 0x10 )  { x_block_offset++; };
    if ( cdg_stream[current_position].data[1] & 0x20 )  { x_block_offset--; };
    if ( cdg_stream[current_position].data[2] & 0x10 )  { y_block_offset++; };
    // We should scroll up.
    if ( cdg_stream[current_position].data[2] & 0x20 )
    {
        // Actually scroll the screen data around.
        proc_VRAM_VSCROLL( 0x20, 1, 0);
        for (int x_blk = 0; x_blk < 50; x_blk++)
        {
            // Create a new block to draw.
            CDGMagic_FontBlock *current_block = new CDGMagic_FontBlock(x_blk, y_block_offset+8, current_position);
            get_composited_fontblock( current_block ); // Loads the specified block.
            current_block->y_location( 0 ); // Draw block in the offscreen area.
            current_block->vram_only( 1 );  // Just copy directly to VRAM, not compositor.
            font_deque_return.push_back(current_block); // Adds it to the deque.

            //printf("Block x: %i, y: %i\n", current_block->x_location(), current_block->y_location() );
        };
        // Advance the stream position.
        current_position++;
        // Increment the vertical compositor-to-screen offset.
        y_block_offset++;
        // Return the font deque.
        return font_deque_return;
    };
*/
    // Incrememnt the packet position.
    current_position++;
    // Return the font deque.
    return font_deque_return;
}

CD_SCPacket* CDGMagic_GraphicsEncoder::Get_Stream(void)
{
    return cdg_stream;
}

int CDGMagic_GraphicsEncoder::length() const
{
    return current_length;
}

// Convert an indexed bitmap object into a deque of FontBlocks.
// The width/height/x,y offsets are stored in the bitmap object.
std::deque<CDGMagic_FontBlock*> CDGMagic_GraphicsEncoder::bmp_to_fonts(CDGMagic_MediaClip *incoming_clip)
{
    // Make the return value. [NOTE: This is probably the slowest/worst way to accomplish this, but also the "prettiest" looking... Hrm.]
    std::deque<CDGMagic_FontBlock*> font_deque_return;

    // Make things a little neater by using a direct pointer to the clip's event queue.
    MediaEvent_Queue *incoming_deque = incoming_clip->event_queue();

    // Step through each bitmap object event.
    for (unsigned int cur_int_evnt = 0; cur_int_evnt < incoming_deque->size(); cur_int_evnt++ )
    {
        CDGMagic_BMPObject* bmp_object = incoming_deque->at(cur_int_evnt)->BMPObject;
        if ((bmp_object == NULL) || (bmp_object->xor_only() != 0))  { continue; };
        // Step through the block elements. (This is to implement "transitions".)
        for (int trans_block = 0; trans_block < bmp_object->transition_length(); trans_block++ )
        {
            // Set the earliest start time.
            int start_pack = incoming_clip->start_pack() + incoming_deque->at(cur_int_evnt)->start_offset + bmp_object->draw_delay();
            // Set the initial x,y offset values (block is 6 wide, 12 high).
            int x_offset = bmp_object->transition_block(trans_block, 0) + x_block_offset;
            int y_offset = bmp_object->transition_block(trans_block, 1) + y_block_offset;
            // Set up a temporary block
            CDGMagic_FontBlock *curr_fontblock = new CDGMagic_FontBlock(x_offset, y_offset, start_pack);
            // And set the z/track/compositing/whatever index, but only if the object exists (can return NULL).
            if ( incoming_clip->track_options() )
            {
                curr_fontblock->z_location( incoming_clip->track_options()->track() );
                curr_fontblock->channel( incoming_clip->track_options()->channel() );
            };
            // Then add the offset offsets (which can be negative).
            x_offset = x_offset *  6 - bmp_object->x_offset();
            y_offset = y_offset * 12 - bmp_object->y_offset();
            // Now pluck out the pixel values.
            for (int y_pxl = 0; y_pxl < 12; y_pxl++ )
            {
                for (int x_pxl = 0; x_pxl < 6; x_pxl++ )
                {
                    curr_fontblock->pixel_value(x_pxl, y_pxl, bmp_object->pixel(x_pxl+x_offset, y_pxl+y_offset) );
                }; // End X.
            }; // End Y.
            // See if we should composite either over or into the layer (ignore if not at all).
            if (bmp_object->should_composite() == 1)   { curr_fontblock->replacement_transparent_color( bmp_object->composite_index() ); }
            else if (bmp_object->should_composite() == 2)  { curr_fontblock->overlay_transparent_color( bmp_object->composite_index() ); };
            // Add the current block to the block deque.
            font_deque_return.push_back(curr_fontblock);
        }; // End Transition Block.
    };
    // Return the new font queue.
    return font_deque_return;
}

// Convert an XORable only bitmap object into a deque of FontBlocks.
// The width/height/x,y offsets are stored in the bitmap object.
int CDGMagic_GraphicsEncoder::xor_to_fonts(CDGMagic_MediaClip *incoming_clip, FontBlock_Deque* font_deque_return)
{
    // How many fonts did we create?
    int number_of_fontblocks = 0;
    // Make things a little neater by using a direct pointer to the clip's event queue.
    MediaEvent_Queue *incoming_deque = incoming_clip->event_queue();
    // Step through each bitmap object event.
    for (unsigned int cur_int_evnt = 0; cur_int_evnt < incoming_deque->size(); cur_int_evnt++ )
    {
        CDGMagic_BMPObject* bmp_object = incoming_deque->at(cur_int_evnt)->BMPObject;
        // Make sure we have a BMP object, and that it's an XOR variant.
        if ((bmp_object == NULL) || (bmp_object->xor_only() == 0))  { continue; };
        // Ignore this object if the width or height are 0, as there's nothing to draw.
        if ((bmp_object->width() == 0) || (bmp_object->height() == 0))  { continue; };
        // Make a fake transition for testing...
        // NOTE: A lot of this should probably be moved to BMPObject,
        // maybe even going so far as having BMPObject generate font blocks itself,
        // and having the encoder be responsible only for compositing/timing/converting
        // to subcode format...
        // Alternately, this could moved to a separate bmp-to-fontblock class.
        int trans_x_left   = bmp_object->x_offset() /  6;
        int trans_x_right  = (bmp_object->x_offset()+bmp_object->width()-1) / 6;
        int trans_y_top    = bmp_object->y_offset() / 12;
        int trans_y_bottom = (bmp_object->y_offset()+bmp_object->height()-1) / 12;
        int trans_height   = trans_y_bottom - trans_y_top + 1;
        // Determine how long we have.
        // First, see how many subcode packs we can use.
        // For testing, 1/3 total bandwidth (100 commands per second) is considered available for wiping.
        // This should probably be user configurable (at the least), or --even better-- somehow calculated automatically.
        // That would probably need an object that can be given a pack time, and allow it to determine how far the wipe
        // has progressed, then generate the font blocks necessary, which would be given high priority for drawing.
        // That way, if a lot is happening on screen and the update isn't being called often, the wipe would be large/blocky,
        // but would become smooth automatically when more bandwidth was available to call it more often... Or something.
        int wipe_duration   = incoming_deque->at(cur_int_evnt)->duration;
        int allocated_packs = wipe_duration / bmp_object->xor_bandwidth();
        // Also, if for some reason the calculated duration is would give us no packs,
        // then clamp to 1 to avoid divide by zero and some other oddities.
        if ( allocated_packs <= 0 )  { allocated_packs = 1; };
        /*
        // Calculate the number of pixels we should wipe per column, which is "evened out" so it can be:
        // 1 (6 packs per block), 2 (3 packs per block), 3 (2 packs per block), or 6 (1 packs per block).
        // First, calculate the maximum number of font updates that may be required.
        double dbl_pxs_cols = static_cast<double>(bmp_object->width()) * static_cast<double>(trans_height);
        // Then, determine how many font updates we must "throw away" for the given duration and bandwidth.
        // Do a simple ceiling division, truncating to integer... (Might be better to use <cmath> ceil()?)
        int px_per_col = static_cast<int>(  (dbl_pxs_cols + static_cast<double>(allocated_packs) - 1.0) / static_cast<double>(allocated_packs)  );
        // Clamp the result to evenly divisible font width values of 6, 3, 2, or 1.
        if (px_per_col > 6)  { px_per_col = 6; } else if (px_per_col > 3)  { px_per_col = 3; } else if (px_per_col <= 0)  { px_per_col = 1; };
        // Determine how many columns we'll have to wipe now that we've calculated the effective column size.
        int actual_wipe_cols = trans_width * (6 / px_per_col);
        */

int actual_packs = 0;
int px_per_col   = 0;
int start_x_col  = 0;
int actual_wipe_cols = 0;

do
{
    // Clamp the pixels per column to 6, or incrememnt as necessary.
    if (px_per_col >= 3)  { px_per_col = 6; }  else  { px_per_col++; };
    // Calculate the left and right most pixels of the wipe.
    int  left_most_pixel = bmp_object->x_offset();
    int right_most_pixel = bmp_object->x_offset()+bmp_object->width()-1;
    // Calculate the left and right most columns, of the current column size, for those locations.
    int  left_most_column =  left_most_pixel / px_per_col;
    int right_most_column = right_most_pixel / px_per_col;
    // Calculate how many actual columns there are.
    actual_wipe_cols = right_most_column - left_most_column + 1;
    // Calculate the column offset of the first column to highlight.
    start_x_col = left_most_column - (trans_x_left * (6 / px_per_col));
    // Calculate how many packs would actually be consumed if all the column segments needed highlighting.
    actual_packs = actual_wipe_cols * trans_height;
    // If we have too many packs, then try again by increasing the column size.
} while ( (px_per_col < 6) && (actual_packs > allocated_packs) );

        // Now, determine the amount of "pack time" we should add between each column to get an even wipe.
        // "Actual columns" are used, instead of total font updates, because an entire column is always updated as one unit.
        double col_time_adv =  static_cast<double>(wipe_duration) / static_cast<double>(actual_wipe_cols);
        // Clamp the value to zero if it's negative, which sets all the packs to same start value,
        // and basically means "display as fast as possible" to the subcode packet "scheduler".
        if (col_time_adv <= 0.0) { col_time_adv = 0.0; };
        int curr_actual_col = 0;

#ifdef _CDGMAGIC_KARAOKEHIGHLIGHT_DEBUG_
        int trans_width    = trans_x_right - trans_x_left + 1;
        printf("Dr:%i, Wp:%i, ClPk:%f, ClPx:%i, Cl:%i, Wd:%i, Tw:%i, Th:%i\n",
               incoming_deque->at(cur_int_evnt)->duration,   //   Dr: Total duration (in packs) of the wipe.
               allocated_packs,                              //   Wp: Packs actually available for use by the wipe. (total duration * allowed bandwidth)
               col_time_adv,                                 // ClPk: Amount of time (in packs) to advance counter between columns.
               px_per_col,                                   // ClPx: Number of pixels in each wipe column. (1,2,3,6 depending on width and bandwidth)
               actual_wipe_cols,                             //   Cl: Number of actual columns of the column size.
               bmp_object->width(),                          //   Wd: The total width, in pixels, of the wipe.
               trans_width,                                  //   Tw: Transition block width.
               trans_height);                                //   Th: Transition block height.
#endif
        for (int x_tile = trans_x_left; x_tile <= trans_x_right; x_tile++ )
        {
            for (int x_wipe_col = start_x_col; x_wipe_col < 6/px_per_col; x_wipe_col++)
            {
                for (int y_tile = trans_y_top; y_tile <= trans_y_bottom; y_tile++ )
                {
                    // Set the earliest start time.
                    int start_pack = incoming_clip->start_pack() + incoming_deque->at(cur_int_evnt)->start_offset;
                    // Add in the calculated offset for wiping.
                    // Time offset is computed by actual column being wiped * the time offset for each column.
                    start_pack += static_cast<int>( static_cast<double>(curr_actual_col) * col_time_adv );
                    // Set the initial x,y offset values (block is 6 wide, 12 high).
                    int x_offset = x_tile + x_block_offset;
                    int y_offset = y_tile + y_block_offset;
                    // Set up a temporary block
                    CDGMagic_FontBlock *curr_fontblock = new CDGMagic_FontBlock(x_offset, y_offset, start_pack);
                    // And set the z/track/compositing/whatever index, but only if the object exists (can return NULL).
                    if ( incoming_clip->track_options() )
                    {
                        curr_fontblock->z_location( incoming_clip->track_options()->track() );
                        curr_fontblock->channel( incoming_clip->track_options()->channel() );
                    };
                    // Then add the offset offsets (which can be negative).
                    x_offset = x_offset *  6 - bmp_object->x_offset();
                    y_offset = y_offset * 12 - bmp_object->y_offset();
                    // Now pluck out the pixel values.
                    curr_fontblock->color_fill(0);
                    for (int y_pxl = 0; y_pxl < 12; y_pxl++ )
                    {
                        for (int x_pxl = (x_wipe_col*px_per_col); x_pxl < ((x_wipe_col*px_per_col)+px_per_col); x_pxl++ )
                        {
                            curr_fontblock->pixel_value(x_pxl, y_pxl, bmp_object->pixel(x_pxl+x_offset, y_pxl+y_offset) );
                        }; // X pixel column.
                    }; // Y pixel column.
                    curr_fontblock->xor_only( bmp_object->xor_only() );
                    // In the case of XOR only, it would be useless to draw a "0" block...
                    if ( (curr_fontblock->num_colors() == 2) || (curr_fontblock->prominent_color() == 1) )
                    {
                        // Add the current block to the block deque.
                        font_deque_return->push_back(curr_fontblock);
                        number_of_fontblocks++;
                    }
                    else { delete curr_fontblock; }; // Delete the unused block.
                }; // Y Tiles.
                curr_actual_col++;
            };
            start_x_col = 0;
        }; // X Tiles.
    }; // Event
    // Return the new font queue.
    return number_of_fontblocks;
}

int CDGMagic_GraphicsEncoder::copy_compare_fontblock(CDGMagic_FontBlock *block_to_compare)
{
    int is_block_different = 0;
    // Calculate the offset for the top left pixel of the current block location.
    // This is block offset into the compositing buffer, so need to use the comp_width in this calculation.
    int max_x_block = (comp_width / 6); // Silence the comparison between signed/unsigned warning...
    int max_y_block = (comp_height/12); // Silence the comparison between signed/unsigned warning...
    if (block_to_compare->x_location() < 0) { return 0; };
    if (block_to_compare->x_location() >= max_x_block) { return 0; };
    if (block_to_compare->y_location() < 0) { return 0; };
    if (block_to_compare->y_location() >= max_y_block) { return 0; };
    int comp_block_offset = block_to_compare->x_location() * 6 + block_to_compare->y_location() * comp_width * 12;
    // Calculate whether this block is within the confines of CDG VRAM, or if we should just composite it.
    // NOTE: The VRAM is always 300x216, or 50x18 font blocks.
    // However, the compositing area may be that size or larger to accomodate scrolling/panning effects.
    // If the block is outside the screen area, this stays set to -1, so we don't compare it to the CDG VRAM,
    // nor do we have any need to write to the current screen... It "lies dormant" until scrolled in to view.
    int vram_block_offset = -1;
    if (  (block_to_compare->x_location() >= 0) && (block_to_compare->x_location() < 50)
       && (block_to_compare->y_location() >= 0) && (block_to_compare->y_location() < 18)  )
    {
        // Need to use VRAM_WIDTH here, which is 300.
        vram_block_offset = block_to_compare->x_location() * 6 + block_to_compare->y_location() * VRAM_WIDTH * 12;
    };

  if (block_to_compare->xor_only() == 0)
  {
    // Run through the lines/Y.
    for (int y_pix = 0; y_pix < 12; y_pix++)
    {
      // Run through the columns/X.
      for (int x_pix = 0; x_pix < 6; x_pix++)
      {
          // Set the current pixel starting offset for the block location.
          int pixel_offset = comp_block_offset + x_pix + y_pix * comp_width;
          // Set the layer offset.
          // NOTE: Track, which ranges from 0 to 7, offsets are thus compositing area * track + pixel_offset.
          int layer_offset = comp_width * comp_height * block_to_compare->z_location() + pixel_offset;
          // Replacement transparency forces the current layer pixel to be transparent.
          // Overlay transparency leaves whatever index is currently in the layer untouched.
          // Get the index value of the current pixel.
          int new_index = block_to_compare->pixel_value(x_pix, y_pix);
          // If the pixel value equals the index value to REPLACE the pixel with transparent, then do so.
          if ( new_index == block_to_compare->replacement_transparent_color() )
          {
              comp_buffer[layer_offset] = 256; // Magic internal transparent number.
          }
          // Otherwise, if the pixel index value is NOT the value to OVERLAY transparent, then update it.
          else if ( new_index != block_to_compare->overlay_transparent_color() )
          {
              comp_buffer[layer_offset] = new_index; // Put the pixel value in to the layer.
          };
          // Now step through the layers, starting with the last used MEMORY PRESET (which is a "virtual layer 0").
          // Always setting the top most index if it's not transparent.
          block_to_compare->pixel_value(x_pix, y_pix, last_preset_index);
          for (int z_loc = 0; z_loc < 8; z_loc++)
          {
              layer_offset = comp_width * comp_height * z_loc + pixel_offset;
              // If the pixel value of the current layer at the current location is less than 256, it's opaque.
              // So set the value of the incoming block's pixel to that value instead.
              if ( comp_buffer[layer_offset] < 256 )  {  block_to_compare->pixel_value(x_pix, y_pix, comp_buffer[layer_offset]);  };
          };
          // Now, determine if the pixel we have is equal to the pixel we used to have.
          new_index = block_to_compare->pixel_value(x_pix, y_pix);
          // If the value differs from what's already on screen, we need to redraw this block.
          if ( vram_block_offset >= 0 )
          {
              pixel_offset = vram_block_offset + x_pix + y_pix * VRAM_WIDTH;
              if ( vram[pixel_offset] != new_index )
              {
                  vram[pixel_offset] = new_index;
                  is_block_different = 1;
              };
          };
//            printf("x: %i, y: %i, old: %i, new: %i\n", x_pix, y_pix, vram[px_offset], new_index);
      };
    };
  }
  else // Hacky hack for XOR karaoke highlighting.
  {
    // Run through the lines/Y.
    for (int y_pix = 0; y_pix < 12; y_pix++)
    {
      // Run through the columns/X.
      for (int x_pix = 0; x_pix < 6; x_pix++)
      {
          // Set the current pixel starting offset for the block location.
          int pixel_offset = comp_block_offset + x_pix + y_pix * comp_width;
          // Set the layer offset.
          // NOTE: Track, which ranges from 0 to 7, offsets are thus compositing area * track + pixel_offset.
          int layer_offset = comp_width * comp_height * block_to_compare->z_location() + pixel_offset;
          // Replacement transparency forces the current layer pixel to be transparent.
          // Overlay transparency leaves whatever index is currently in the layer untouched.
          // Get the index value of the current pixel.
          int new_index = block_to_compare->pixel_value(x_pix, y_pix);
          // If the pixel is 1/on/set, then we need to XOR with the current color index in that layer.
          // (This should maybe check for transparent, too?...)
          if ( new_index == 1 )
          {
              comp_buffer[layer_offset] ^= block_to_compare->xor_only();
          };
          // Now step through the layers, starting at the layer ABOVE the highlighting clip.
          // If there's a non-transparent layer above the highlighting layer, then DON'T XOR that pixel (set to 0).
          for (int z_loc = (block_to_compare->z_location()+1); z_loc < 8; z_loc++)
          {
              layer_offset = comp_width * comp_height * z_loc + pixel_offset;
              // If the pixel value of the current layer at the current location is less than 256, it's opaque.
              // So set the value of the incoming block's pixel to that value instead.
              if ( comp_buffer[layer_offset] < 256 )  {  block_to_compare->pixel_value(x_pix, y_pix, 0);  };
          };
          // Now, determine if the pixel we have will change what's onscreen.
          // If so, then we need to draw this block.
          // TODO: Count the ending amount of colors in the XORed VRAM block, and if it's <=2, then convert the XOR to a regular copy block.
          if (  (block_to_compare->pixel_value(x_pix, y_pix) == 1)
             && (vram_block_offset >= 0) )
          {
              pixel_offset = vram_block_offset + x_pix + y_pix * VRAM_WIDTH;
              vram[pixel_offset] ^= block_to_compare->xor_only();
              is_block_different = 1;
          };
//            printf("x: %i, y: %i, old: %i, new: %i\n", x_pix, y_pix, vram[px_offset], new_index);
      };
    };
  };
    // Now that the block has been composited, it's always FULLY OPAQUE.
    block_to_compare->replacement_transparent_color(256);
    block_to_compare->overlay_transparent_color(256);
    block_to_compare->x_location( block_to_compare->x_location() + x_block_offset );
    block_to_compare->y_location( block_to_compare->y_location() + y_block_offset );
    // Return whether this block MUST be written. (NOTE: A forced update write can still happen when FALSE.)
    return is_block_different;
}

bool CDGMagic_GraphicsEncoder::timeline_sorter(CDGMagic_MediaClip *entry_a, CDGMagic_MediaClip *entry_b)
{
    return (entry_a->start_pack() < entry_b->start_pack());
}

bool CDGMagic_GraphicsEncoder::timeline_event_sorter(Timeline_EventQueue entry_a, Timeline_EventQueue entry_b)
{
    return (entry_a.start_pack < entry_b.start_pack);
}

bool CDGMagic_GraphicsEncoder::xor_fontblock_sorter(CDGMagic_FontBlock* entry_a, CDGMagic_FontBlock* entry_b)
{
    return ( entry_a->start_pack() < entry_b->start_pack() );
}
