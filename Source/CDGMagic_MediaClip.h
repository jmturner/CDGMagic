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

#ifndef CDGMAGIC_MEDIACLIP_H
#define CDGMAGIC_MEDIACLIP_H

#include "CDGMagic_TrackOptions.h"
#include "CDGMagic_PALObject.h"
#include "CDGMagic_BMPObject.h"

#include <algorithm>
#include <deque>
#include <vector>

struct CDGMagic_MediaEvent
{
    int start_offset;
    int duration;
    int actual_start_offset;
    int actual_duration;
    CDGMagic_PALObject *PALObject;
    CDGMagic_BMPObject *BMPObject;
    unsigned char border_index;
    unsigned char memory_preset_index;
    signed char x_scroll;
    signed char y_scroll;
    void *user_obj;
};

typedef std::deque<CDGMagic_MediaEvent*> MediaEvent_Queue;

typedef std::vector<char> char_vector;

class CDGMagic_MediaClip
{
private:
    MediaEvent_Queue* internal_queue;
    CDGMagic_TrackOptions* internal_trackoptions_object;
    int internal_start, internal_duration;
    int internal_x_offset, internal_y_offset;
    static bool internal_queue_sorter(CDGMagic_MediaEvent *event_a, CDGMagic_MediaEvent *event_b);

public:
    CDGMagic_MediaClip(int requested_start, int requested_duration);
    ~CDGMagic_MediaClip();

    int start_pack() const;
    void start_pack(int requested_start);
    int duration() const;
    void duration(int requested_duration);
    int end_pack() const;
    MediaEvent_Queue* event_queue();
    void track_options(CDGMagic_TrackOptions* requested_options);
    CDGMagic_TrackOptions* track_options();
    void sort_event_queue();

    virtual int serialize(char_vector* incoming_save_vector);

    void add_text (char_vector* incoming_vector, const char* data_to_add);
    void add_int  (char_vector* incoming_vector, const int   data_to_add);
    void add_short(char_vector* incoming_vector, const short data_to_add);
    void add_char (char_vector* incoming_vector, const char  data_to_add);

    int get_int(char* incoming_stream, int   &data_to_get) const;
    int get_short(char* incoming_stream, short &data_to_get) const;
    int get_char(char* incoming_stream, char  &data_to_get) const;
};

#endif
