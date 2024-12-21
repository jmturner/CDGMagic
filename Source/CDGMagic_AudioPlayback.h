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

#ifndef CDGMAGIC_AUDIOPLAYBACK_H
#define CDGMAGIC_AUDIOPLAYBACK_H

#include <cstdio>
#include <cstring>

#include <string>

#include <portaudio.h>

class CDGMagic_AudioPlayback
{
private:
    static const int FRAMES_PER_BUFFER = 1176;
    static const int SAMPLE_RATE = 44100;

    PaStreamParameters PortAudio_OutputParameters;
    PaStream *PortAudio_Stream;
    PaError PortAudio_LastError;

    FILE *InputFile; int WAVE_filesize;
    char *internal_filepath;
    signed short *WAVE_File_Buffer;
    signed short *PCM_Audio_Buffer;
    float *Float_Audio_Buffer;
    float *WAVE_View_BMP;
    int WAVE_View_Length;
    int Current_Play_Position;
    int Current_Read_Position;
    int Current_Latency;
    float internal_current_interval;

    void Open_Stream(int requested_sample_rate = SAMPLE_RATE);
    void Close_Stream();
    void generate_wave_bmp();

public:
    enum fail_exceptions
	{
        NO_PATH,
        OPEN_FAIL,
        TO_SMALL,
        NOT_WAVE,
        BAD_SIZE,
        UNSUPPORTED_CODEC,
        UNSUPPORTED_SAMPLERATE,
        UNSUPPORTED_CHANNELS,
        UNSUPPORTED_BITDEPTH,
        INSUFFICIENT_DATA,
        PORTAUDIO_ERROR
    };

    CDGMagic_AudioPlayback(const char *file_path);
    ~CDGMagic_AudioPlayback();

    void Start_Playback();
    void Stop_Playback();
    void Set_SampleRate(int requested_sample_rate);
    int Play_Next_Buffer();
    int Play_Single_Buffer();
    int Seek_To_Sample( int sample_pos );
    int TotalFrames();
    int ViewFrames();
    float* Get_WAVE_BMP();
    float get_audio_interval();
    const char* file_path();
    static const char* error_to_text(int error_number);
};

#endif
