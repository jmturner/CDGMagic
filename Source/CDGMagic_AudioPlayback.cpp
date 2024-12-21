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

#include <math.h>

#include "CDGMagic_AudioPlayback.h"

CDGMagic_AudioPlayback::CDGMagic_AudioPlayback(const char *file_path)
{
    InputFile = NULL;
    Current_Play_Position = 0;
    Current_Read_Position = 0;
    internal_current_interval = 1000;

    // Make sure the file_path we were given isn't NULL.
    if ( file_path == NULL )  { throw (int)NO_PATH; };
    // Attempt to open the file_path for binary reading.
    InputFile = fopen( (const char*)file_path, "rb" );
    // Check to make sure we got a valid file handle.
    if ( InputFile == NULL )  { throw (int)OPEN_FAIL; };
    // Put the file cursor at the end of the file.
    fseek(InputFile, 0, SEEK_END);
    // Get the end of file cursor position (i.e. the file size).
    WAVE_filesize = ftell(InputFile);
    // Set the file cursor back to the beginning of the file.
    rewind(InputFile);
    // Make sure the file isn't too small (that is, less than 100 bytes).
    if ( WAVE_filesize < 100 )
    {
        if ( InputFile != NULL )  { fclose( InputFile ); };
        throw (int)TO_SMALL;
    };
#ifdef _DEBUG_AUDIO_
    printf("Opened %i bytes WAVE file.\n", WAVE_filesize);
#endif
    // Make a temp buffer for the header data.
    unsigned char *wav_header = new unsigned char[44];
    // Read the header from the file.
    fread(wav_header, 44, 1, InputFile);
    // Set the file cursor back to the beginning of the file.
    rewind(InputFile);
    // Check for the RIFF, WAVE, fmt header magic.
    unsigned int header_chunksize = 0;
    header_chunksize |= (wav_header[0x10] << 000);
    header_chunksize |= (wav_header[0x11] << 010);
    header_chunksize |= (wav_header[0x12] << 020);
    header_chunksize |= (wav_header[0x13] << 030);
    if (  (memcmp(&wav_header[0x00], "RIFF", 4) != 0)
       || (memcmp(&wav_header[0x08], "WAVE", 4) != 0)
       || (memcmp(&wav_header[0x0C], "fmt ", 4) != 0)
       || (header_chunksize != 0x10)
       || (memcmp(&wav_header[0x24], "data", 4) != 0)  )
    {
        if ( InputFile != NULL )  { fclose( InputFile ); };
        delete[] wav_header;
        throw (int)NOT_WAVE;
    };
    // Check that the header data sizes match the actual file size.
    unsigned int header_filesize = 0;
    header_filesize |= (wav_header[0x04] << 000);
    header_filesize |= (wav_header[0x05] << 010);
    header_filesize |= (wav_header[0x06] << 020);
    header_filesize |= (wav_header[0x07] << 030);
    unsigned int data_filesize = 0;
    data_filesize |= (wav_header[0x28] << 000);
    data_filesize |= (wav_header[0x29] << 010);
    data_filesize |= (wav_header[0x2A] << 020);
    data_filesize |= (wav_header[0x2B] << 030);
    int header_file_sz = (header_filesize > 0) ? header_filesize : 0; // Silence the comparison between signed/unsigned warning...
    int   data_file_sz = (data_filesize   > 0) ? data_filesize   : 0; // Silence the comparison between signed/unsigned warning...
    if (  (header_file_sz != (WAVE_filesize - 0x08))
       || (data_file_sz   != (WAVE_filesize - 0x2C))  )
    {
        if ( InputFile != NULL )  { fclose( InputFile ); };
        delete[] wav_header;
        throw (int)BAD_SIZE;
    };
    // Check that the format is LPCM.
    unsigned int header_codec = 0;
    header_codec |= (wav_header[0x14] << 000);
    header_codec |= (wav_header[0x15] << 010);
    if ( header_codec != 0x01 )
    {
        if ( InputFile != NULL )  { fclose( InputFile ); };
        delete[] wav_header;
        throw (int)UNSUPPORTED_CODEC;
    };
    // Check that there's two channels (stereo).
    unsigned int header_channels = 0;
    header_channels |= (wav_header[0x16] << 000);
    header_channels |= (wav_header[0x17] << 010);
    if ( header_channels != 0x02 )
    {
        if ( InputFile != NULL )  { fclose( InputFile ); };
        delete[] wav_header;
        throw (int)UNSUPPORTED_CHANNELS;
    };
    // Check that the samplerate is 44,100Hz.
    unsigned int header_samplerate = 0;
    header_samplerate |= (wav_header[0x18] << 000);
    header_samplerate |= (wav_header[0x19] << 010);
    header_samplerate |= (wav_header[0x1A] << 020);
    header_samplerate |= (wav_header[0x1B] << 030);
    unsigned int header_datarate = 0;
    header_datarate |= (wav_header[0x1C] << 000);
    header_datarate |= (wav_header[0x1D] << 010);
    header_datarate |= (wav_header[0x1E] << 020);
    header_datarate |= (wav_header[0x1F] << 030);
    unsigned int header_blockalign = 0;
    header_blockalign |= (wav_header[0x20] << 000);
    header_blockalign |= (wav_header[0x21] << 010);
    if (  (header_samplerate !=  44100)
       || (header_datarate   != 176400)
       || (header_blockalign !=   0x04)  )
    {
        if ( InputFile != NULL )  { fclose( InputFile ); };
        delete[] wav_header;
        throw (int)UNSUPPORTED_SAMPLERATE;
    };
    // Check that the depth is 16bit.
    unsigned int header_bitdepth = 0;
    header_bitdepth |= (wav_header[0x22] << 000);
    header_bitdepth |= (wav_header[0x23] << 010);
    if ( header_bitdepth != 16 )
    {
        if ( InputFile != NULL )  { fclose( InputFile ); };
        delete[] wav_header;
        throw (int)UNSUPPORTED_BITDEPTH;
    };
    // The header check is done, so delete the header buffer.
    delete[] wav_header;
    // Allocate buffers for WAVE file read and raw PCM audio data.
    // Note: Since this is all single threaded, having multiple buffers may not be optimal?...
    // It does, however, make playing across "read sector boundaries" a bit more straightforward.
    WAVE_File_Buffer = new signed short[8*1024*2];
    PCM_Audio_Buffer = new signed short[16*1024*2];
#ifdef _CDGMAGIC_USEPCLINUXAUDIO_
    Float_Audio_Buffer = new float[8*1024*2];
#else
    Float_Audio_Buffer = new float[FRAMES_PER_BUFFER*2];
#endif
    // Attempt to initialize the PortAudio library.
    PortAudio_LastError = Pa_Initialize();
    if( PortAudio_LastError != paNoError )  { throw (int)PORTAUDIO_ERROR; };
    // Open the stream.
    Open_Stream();
    // Generate the wave display data.
    generate_wave_bmp();
    // Allocate and copy the filename.
    internal_filepath = new char[ strlen(file_path)+1 ];
    strcpy(internal_filepath, file_path);
}

CDGMagic_AudioPlayback::~CDGMagic_AudioPlayback()
{
    // Delete the various buffers.
    delete[] Float_Audio_Buffer;
    delete[] PCM_Audio_Buffer;
    delete[] WAVE_File_Buffer;
    delete[] WAVE_View_BMP;
    // We allocated the filepath, so delete it.
    delete[] internal_filepath;
    // Close the file.
    if ( InputFile != NULL )  {  fclose( InputFile );  };
    // Close and terminate PortAudio.
    Close_Stream();
    Pa_Terminate();
}

void CDGMagic_AudioPlayback::Open_Stream(int requested_sample_rate)
{
#ifdef WIN32
    // Find the PortAudio device index for the default WinMME output -- which seems to work best for scrubbing on Windows.
    const PaHostApiInfo* preferred_pa_host = Pa_GetHostApiInfo( Pa_HostApiTypeIdToHostApiIndex(paMME) );
#else
  #ifdef __APPLE__
    // Find the PortAudio device index for the default CoreAudio output...
    // This is COMPLETELY UNTESTED... May or may not work at all!
    // Just here to give Mac users at least a chance to compile a working binary.
    const PaHostApiInfo* preferred_pa_host = Pa_GetHostApiInfo( Pa_HostApiTypeIdToHostApiIndex(paCoreAudio) );
  #else
    // Find the PortAudio device index for the default OSS output -- which seems to work best for scrubbing on Linux.
    const PaHostApiInfo* preferred_pa_host = Pa_GetHostApiInfo( Pa_HostApiTypeIdToHostApiIndex(paOSS) );
  #endif
#endif
    // Set up the initial stream parameters.
    PortAudio_OutputParameters.device = preferred_pa_host->defaultOutputDevice; // Use the default audio device for preferred host.
    PortAudio_OutputParameters.channelCount = 2;                     // Only support stereo.
    PortAudio_OutputParameters.sampleFormat = paFloat32;             // Use floating point, because it seems to work best.
    // Changed the latency from default to fixed 160ms, which should slightly reduce playback "judder" due to the now exact integer timing relationships.
    // Although it also/probably increases the CPU consumed by Pa_WriteStream(...)... Maybe make it an option?
    // (Or better yet, rip out all the blocking IO code, and reimplement using the standard callback interface to PortAudio.)
    //PortAudio_OutputParameters.suggestedLatency = Pa_GetDeviceInfo( PortAudio_OutputParameters.device )->defaultLowOutputLatency;
    PortAudio_OutputParameters.suggestedLatency = 0.160;
    PortAudio_OutputParameters.hostApiSpecificStreamInfo = NULL;
    Current_Latency = (int)(PortAudio_OutputParameters.suggestedLatency*static_cast<float>(requested_sample_rate));

    PortAudio_LastError = Pa_OpenStream(
              &PortAudio_Stream,
              NULL,
              &PortAudio_OutputParameters,
              requested_sample_rate,
              FRAMES_PER_BUFFER,
              paClipOff,
              NULL,
              NULL );
    if ( PortAudio_LastError != paNoError )  { throw (int)PORTAUDIO_ERROR; };
#ifdef WIN32
    internal_current_interval = 0.20;
#else
    // Set the callback interval to the actual time between buffer size callbacks given the current playback rate.
    internal_current_interval = static_cast<float>(FRAMES_PER_BUFFER) / static_cast<float>(requested_sample_rate);
#endif
#ifdef _DEBUG_AUDIO_
    printf("PortAudio successfully initialized!\n");
#endif
    return;
}

void CDGMagic_AudioPlayback::Close_Stream()
{
    // Close the PortAudio stream.
    if( Pa_CloseStream(PortAudio_Stream) != paNoError  )
    {
        printf("Error closing PortAudio stream inside CDGMagic_AudioPlayback::Close_Stream().\n");
    };
    return;
}

void CDGMagic_AudioPlayback::Set_SampleRate(int requested_sample_option)
{
    Stop_Playback();
    Close_Stream();
    int the_sample_rate = 44100;
    switch ( requested_sample_option )
    {
        case 0: the_sample_rate = 22050; break;
        case 1: the_sample_rate = 32000; break;
        case 2: the_sample_rate = 44100; break;
        case 3: the_sample_rate = 48000; break;
    };
    Open_Stream(the_sample_rate);
    Start_Playback();
    return;
}

void CDGMagic_AudioPlayback::Start_Playback()
{
#ifdef _DEBUG_AUDIO_
    printf("PortAudio Start_Playback\n");
#endif
    if( Pa_StartStream(PortAudio_Stream) != paNoError )
    {
        printf("Error starting PortAudio stream!\n");
    };
}

void CDGMagic_AudioPlayback::Stop_Playback()
{
#ifdef _DEBUG_AUDIO_
    printf("PortAudio Stop_Playback\n");
#endif
    if( Pa_StopStream(PortAudio_Stream) != paNoError )
    {
        printf("Error stopping PortAudio stream!\n");
    };
}

int CDGMagic_AudioPlayback::Seek_To_Sample( int sample_pos )
{
    // Clear the PCM buffer to silence.
    for (int sample = 0; sample < 32*1024; sample++)  { PCM_Audio_Buffer[sample] = 0; };
    // Update to the requested play sample position.
    Current_Play_Position = sample_pos*2;
    // Seek the file to the sample.
    fseek(InputFile, Current_Play_Position * 2, SEEK_SET);
    // Get where we actually are.
    Current_Read_Position = ftell(InputFile) / 2;
#ifdef _DEBUG_AUDIO_
    printf("Seek [ Requested: %i, Read: %i, Play: %i ]\n", sample_pos, Current_Read_Position, Current_Play_Position);
#endif
    // Return the current play sample position.
    return Current_Play_Position/2;
}

int CDGMagic_AudioPlayback::Play_Next_Buffer()
{
    if ( InputFile == NULL )
    {
        printf("InputFile is NULL inside CDGMagic_AudioPlayback::Play_Next_Buffer()\n");
        return 0;
    };
#ifdef _CDGMAGIC_USEPCLINUXAUDIO_
    // Determine how many samples are available to be queued.
    int samples_to_queue = Pa_GetStreamWriteAvailable(PortAudio_Stream);
    // Constrain to 8192 (or the OUR max buffer size).
    if (samples_to_queue > 8192)
    {
        printf("samples_to_queue was %i inside CDGMagic_AudioPlayback::Play_Next_Buffer()\n", samples_to_queue);
        samples_to_queue = 8192;
    };
    // Contrain to multiples of the frames per buffer option.
    samples_to_queue = samples_to_queue / FRAMES_PER_BUFFER * FRAMES_PER_BUFFER;
#ifdef _DEBUG_AUDIO_
    printf( "Play_Next_Buffer: %i, %i, %f\n", Pa_GetStreamWriteAvailable(PortAudio_Stream), samples_to_queue, Pa_GetStreamTime(PortAudio_Stream) );
#endif
    // Play any needed PCM audio data.
    if (samples_to_queue > 0)
    {
        // Hacky hack, limits the playback to one buffer only, unless we're >3 buffers behind.
        if (samples_to_queue <= (FRAMES_PER_BUFFER*3))  { samples_to_queue = FRAMES_PER_BUFFER; };
        // Calculate what the new position will be.
        int this_buffer_position = Current_Play_Position + (samples_to_queue * 2);
        // Read from the file until we get there.
        while ( Current_Read_Position < this_buffer_position )
        {
            int samples_read = fread( WAVE_File_Buffer, sizeof(signed short), 8*1024*2, InputFile);
            // Check for error conditions, and return if set.
            // [TODO: Maybe should pass this through and alert the user?...]
            if ( ferror(InputFile) != 0 )
            {
                perror("Play_Next_Buffer: Audio file read error: ");
                return -1;
            };
            // We've reached the end of the file and have nothing more in the buffer to play.
            if ( feof(InputFile) && (samples_read == 0) )
            {
                printf("Play_Next_Buffer: End of audio file.\n");
                return -1;
            };
            // Copy the data from the file read buffer to the PCM samples buffer...
            // (It might be best to eliminate this entirely and copy directly in to floats buffer...)
            for (int sample = 0; sample < samples_read; sample++)
            {
                PCM_Audio_Buffer[Current_Read_Position++&0x7FFF] = WAVE_File_Buffer[sample];
            };
        };
        // Copy the PCM audio data from the 16bit integer file read buffer to the 32bit float playback buffer.
        for (int sample = 0; sample < (samples_to_queue*2); sample++ )
        {
            // NOTE: Should really be 32,768, but some PortAudio hosts seem to wrap/click around 32,766 or so.
            // Proper dither, of course, would be even better... But this seems OK just for syncing.
            Float_Audio_Buffer[sample] = PCM_Audio_Buffer[Current_Play_Position++&0x7FFF] / 32770.0;
        };
        // Hand the 32bit float audio data off to PoirtAudio for playback.
        if( Pa_WriteStream(PortAudio_Stream, Float_Audio_Buffer, samples_to_queue) != paNoError )
        {
            printf("Error writing to PortAudio stream!\n");
        };
    };
#else
    // Original code, which works great on WinXP...
    // The working linux code doesn't work well on Windows, and vice-versa... Nice :-/.
    while ( Current_Read_Position < Current_Play_Position+FRAMES_PER_BUFFER*2 )
    {
         int samples_read = fread( WAVE_File_Buffer, sizeof(signed short), 16*1024, InputFile);

         if ( feof(InputFile) && (samples_read == 0) )
         {
             printf("End of file, stopping playback.\n");
             return -1;
         };

         for (int sample = 0; sample < samples_read; sample++)
         {
             PCM_Audio_Buffer[Current_Read_Position++&0x7FFF] = WAVE_File_Buffer[sample];
         };
    };

    for (int sample = 0; sample < FRAMES_PER_BUFFER*2; sample++ )
    {
        Float_Audio_Buffer[sample] = PCM_Audio_Buffer[Current_Play_Position++&0x7FFF] / 32770.0;
    };

    if( Pa_WriteStream(PortAudio_Stream, Float_Audio_Buffer, FRAMES_PER_BUFFER) != paNoError )
    {
        printf("Error writing to PortAudio stream!\n");
    };
#endif

    // Return the playback position.
    int ret_val = (Current_Play_Position/2)-Current_Latency;
    if (ret_val < 0)  { ret_val = 0; };
    return ret_val;
}

int CDGMagic_AudioPlayback::Play_Single_Buffer()
{
    if ( InputFile == NULL )
    {
        printf("InputFile is NULL inside CDGMagic_AudioPlayback::Play_Next_Buffer()\n");
        return 0;
    };

    while ( Current_Read_Position < Current_Play_Position+FRAMES_PER_BUFFER*2 )
    {
         int samples_read = fread( WAVE_File_Buffer, sizeof(signed short), FRAMES_PER_BUFFER*2, InputFile);

         if ( ((feof(InputFile) != 0) && (samples_read == 0)) || (ferror(InputFile) != 0) )
         {
             printf("Play_Single_Buffer: End of file or read error, returning error.\n");
             return -1;
         };

         for (int sample = 0; sample < samples_read; sample++)
         {
             PCM_Audio_Buffer[Current_Read_Position++&0x7FFF] = WAVE_File_Buffer[sample];
         };
    };

    for (int sample = 0; sample < FRAMES_PER_BUFFER*2; sample++ )
    {
        Float_Audio_Buffer[sample] = PCM_Audio_Buffer[Current_Play_Position++&0x7FFF] / 32770.0;
    };

    if( Pa_WriteStream(PortAudio_Stream, Float_Audio_Buffer, FRAMES_PER_BUFFER) != paNoError )
    {
#ifdef _DEBUG_AUDIO_
        printf("Error writing to PortAudio stream!\n");
#endif
    };

    // Return the playback position.
    int ret_val = (Current_Play_Position/2)-Current_Latency;
    if (ret_val < 0)  { ret_val = 0; };
    return ret_val;
}

void CDGMagic_AudioPlayback::generate_wave_bmp()
{
#ifdef _DEBUG_AUDIO_
        printf("Now generating WAVE view data...\n");
#endif
    WAVE_View_Length = TotalFrames();
    WAVE_View_BMP = new float[WAVE_View_Length*2];

    int bmp_pixel_counter = 0;

    rewind(InputFile);

    while ( (ferror(InputFile) == 0) && (feof(InputFile) == 0) )
    {
        // Set the average accumulator to 0.
        double accum_pos = 0.0, accum_neg = 0.0;
        // Get 1 CD frame of audio data.
        int samples_read = fread(WAVE_File_Buffer, sizeof(signed short), 588*2, InputFile);
        // Add the read samples together.
        for (int samp = 0; samp < samples_read; samp += 2)
        {
            if (WAVE_File_Buffer[samp] >= 0.0)  { accum_pos += (float)WAVE_File_Buffer[samp]; }
            else                                { accum_neg += (float)-WAVE_File_Buffer[samp]; };
        };
        // Make sure we don't overrun the buffer.
        if (bmp_pixel_counter+1 < WAVE_View_Length*2)
        {
            // Then set the current pixel to the accumulated frame average value.
            WAVE_View_BMP[bmp_pixel_counter++] = pow(accum_pos / (float)samples_read / 32770.0 * 2.0, 0.5);
            WAVE_View_BMP[bmp_pixel_counter++] = pow(accum_neg / (float)samples_read / 32770.0 * 2.0, 0.5);
        }
#ifdef _DEBUG_AUDIO_
        else
        {
            printf("Requested buffer overrun: bmp_pixel_counter=%i, WAVE_View_Length=%i\n", bmp_pixel_counter, WAVE_View_Length);
        };
#endif
    };
#ifdef _DEBUG_AUDIO_
        printf("Successfully generated WAVE view data...\n");
#endif
}

int CDGMagic_AudioPlayback::TotalFrames()
{
    return WAVE_filesize / 2352;
}

int CDGMagic_AudioPlayback::ViewFrames()
{
    return WAVE_View_Length;
}

float* CDGMagic_AudioPlayback::Get_WAVE_BMP()
{
    return WAVE_View_BMP;
}

float CDGMagic_AudioPlayback::get_audio_interval()  {  return internal_current_interval;  }

const char* CDGMagic_AudioPlayback::file_path()  {  return internal_filepath;  }

const char* CDGMagic_AudioPlayback::error_to_text(int error_number)
{
    switch (error_number)
    {
        case                 NO_PATH: return("No path was given!");
        case               OPEN_FAIL: return("Could not open file!");
        case                TO_SMALL: return("File is smaller than arbitrary 100B limit!");
        case                NOT_WAVE: return("This file's header does not contain RIFF/WAVE/fmt/data!");
        case                BAD_SIZE: return("This file's header size and actual size are inconsistent!");
        case       UNSUPPORTED_CODEC: return("This file's codec type is not LPCM (uncompressed)!");
        case  UNSUPPORTED_SAMPLERATE: return("This file's sample rate is not 44,100Hz!");
        case    UNSUPPORTED_CHANNELS: return("This file is not stereo!");
        case    UNSUPPORTED_BITDEPTH: return("This file is not 16bit!");
        case       INSUFFICIENT_DATA: return("Insufficient data, this file may have been truncated!");
        case         PORTAUDIO_ERROR: return("PortAudio Library returned an error code!\n[TODO: Pass the error description through so the user can correct the problem.]");
    };
    return("Unknown error number!");
}
