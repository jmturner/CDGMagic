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

#include "CDGMagic_MediaClip.h"

CDGMagic_MediaClip::CDGMagic_MediaClip(int requested_start, int requested_duration = 0)
{
    // Set up default values.
    start_pack(requested_start);
    duration(requested_duration);
    // Set the (optional) track options object to null.
    internal_trackoptions_object = NULL;
    // Create an internal queue of MediaEvent struct pointers.
    internal_queue = new MediaEvent_Queue;
}

CDGMagic_MediaClip::~CDGMagic_MediaClip()
{
    // Loop through and delete all the struct pointers.
    for (unsigned int current_event = 0; current_event < internal_queue->size(); current_event++)
    {
        delete (internal_queue->at(current_event));
    };
    // Then deallocate the internal event queue.
    delete internal_queue;
}

void CDGMagic_MediaClip::start_pack(int requested_start)
{
    internal_start = requested_start;
}

void CDGMagic_MediaClip::duration(int requested_duration)
{
    internal_duration = requested_duration;
}

MediaEvent_Queue* CDGMagic_MediaClip::event_queue()
{
    return internal_queue;
}

void CDGMagic_MediaClip::sort_event_queue()
{
    // Only need to sort if we have 2 or more events.
    if (internal_queue->size() > 1)
    {
        std::sort(internal_queue->begin(), internal_queue->end(), internal_queue_sorter);
    };
}

bool CDGMagic_MediaClip::internal_queue_sorter(CDGMagic_MediaEvent *event_a, CDGMagic_MediaEvent *event_b)
{
    return (event_a->start_offset < event_b->start_offset);
}

int CDGMagic_MediaClip::start_pack() const {  return internal_start;  }
int CDGMagic_MediaClip::duration()   const {  return internal_duration;  }
int CDGMagic_MediaClip::end_pack()   const {  return (internal_start + internal_duration);  }

void CDGMagic_MediaClip::track_options(CDGMagic_TrackOptions* requested_options)
{
    internal_trackoptions_object = requested_options;
}

CDGMagic_TrackOptions* CDGMagic_MediaClip::track_options()
{
    return internal_trackoptions_object;
}

int CDGMagic_MediaClip::serialize(char_vector* incoming_save_vector)
{
    return 0;
}

// ##### Serialize helper functions... These should be moved to a "project file save/open" class of some sort! ##### //

void CDGMagic_MediaClip::add_text(char_vector* incoming_vector, const char* data_to_add)
{
    if (data_to_add)  // Don't try to copy if pointer is NULL.
    {
        const char* tmp_pointer = data_to_add;
        while (*tmp_pointer)  { incoming_vector->push_back( *tmp_pointer++ ); };
    };
    incoming_vector->push_back( 0 ); // Always ensure there's a NULL at end.
}

void CDGMagic_MediaClip::add_int(char_vector* incoming_vector, const int data_to_add)
{
    incoming_vector->push_back( (data_to_add >> 030) & 0xFF );
    incoming_vector->push_back( (data_to_add >> 020) & 0xFF );
    incoming_vector->push_back( (data_to_add >> 010) & 0xFF );
    incoming_vector->push_back( (data_to_add >> 000) & 0xFF );
}

void CDGMagic_MediaClip::add_short(char_vector* incoming_vector, const short data_to_add)
{
    incoming_vector->push_back( (data_to_add >> 010) & 0xFF );
    incoming_vector->push_back( (data_to_add >> 000) & 0xFF );
}

void CDGMagic_MediaClip::add_char(char_vector* incoming_vector, const char data_to_add)
{
    incoming_vector->push_back( data_to_add );
}

int CDGMagic_MediaClip::get_int(char* incoming_stream, int &data_to_get) const
{
    data_to_get = 0;
    data_to_get |= (incoming_stream[0] << 030) & 0xFF000000;
    data_to_get |= (incoming_stream[1] << 020) & 0x00FF0000;
    data_to_get |= (incoming_stream[2] << 010) & 0x0000FF00;
    data_to_get |= (incoming_stream[3] << 000) & 0x000000FF;
    return 4;
}

int CDGMagic_MediaClip::get_short(char* incoming_stream, short &data_to_get) const
{
    data_to_get = 0;
    data_to_get |= (incoming_stream[0] << 010) & 0xFF00;
    data_to_get |= (incoming_stream[1] << 000) & 0x00FF;
    return 2;
}

int CDGMagic_MediaClip::get_char(char* incoming_stream, char &data_to_get) const
{
    data_to_get = incoming_stream[0];
    return 1;
}
