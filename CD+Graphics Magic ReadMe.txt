+----------------------------------------------------------+
|        CD+Graphics Magic Read Me  /  May 14, 2011        |
+----------------------------------------------------------+

http://cdgmagic.sourceforge.net/

CD+Graphics Magic is a cross-platform application for creation of CD+Graphics
subcodes using a (hopefully straightforward) timeline based interface.

It is currently in the early, experimental stage.

Many planned and desired features have not yet been implemented, while those
that have may be either incomplete, or not work correctly.

The included Windows build should now be "reasonably" stable.
That is, suitable for hobbyist use in non-production environments.

However, this release is still not recommended for critical work!

Linux builds (not included) may be significantly less stable.
(Possibly due to bugs in FLTK, differences in the way FLTK functions
under X11 versus Windows, or undetermined application bugs.)


+-----------------+
|     License     |
+-----------------+

Please see "copying.txt" for license information.


+-----------------------+
|     Minimal Usage     |
+-----------------------+

Unzip the full archive contents to a new directory/folder.

Sample files are in the "Sample_Files" directory/folder.

Basic tool tips are provided when hovering over most controls.

Run the program (CDG_Magic.exe).

Click the "Set WAVE..." button and select a "CD Quality" WAVE file.
     Only uncompressed 44.1KHz/16bit/Stereo wave files are supported.
     Wait while the file is scanned to generate a "peaks" display.
     You can use the included "60sec_silence.wav" for testing.

Click the "Add BMP..." button and select an indexed color bitmap file.
     Multiple files may be selected, which will be displayed in succession.
     Only uncompressed 4bit and 8bit indexed color images are supported.
     Only the first 16 colors are used (CD+G only supports 16 colors).
     Best results are obtained when palette RGB values are divisible by 17.
     You can use the included "simple_sky_2+14.bmp" for testing.

Drag the newly added clip to around the 00:01-00:02 second mark.

Click the "Recalculate" button, or right-click the "playback head".
     This can be done at any time to rerender the internal subcodes stream
     used for playback in the "CD+G Preview" window.

Click in the "scrubbing area" (above the timeline) to position the
"playback head" at a time before the start location of the bitmap clip.

Click the "Play / Pause" button to begin preview playback.
     You should see the image being drawn on the screen in the
     "CD+G Preview" window.

Click "Export CDG..." to save a .cdg subcode packets file.
     Note: This will save the current preview stream as a file.
           Click "Recalculate" first if you have made changes to the
	   timeline which are not reflected by the preview.

Select "File --> Save Project" to save the current project.
     Note: Currently, "Save" and "Save As" execute the same code.
           You will always be asked to provide a filename.
     The file format is very bad and will not be frozen for some time.
     Older projects will not be compatible with newer versions!
     Some options may not be saved.
     Again, this is not recommended for critical production work.

Select "File --> Exit" and click "Yes" to exit.
     Note: This does not yet check for any project modifications.
           You will always be prompted, even if no changes have been made.


+-------------------------+
|     Example Project     |
+-------------------------+

For a more advanced example:

Note: Due to deficiencies of the current project file format,
      the example project may not open/render correctly on Linux.

First, please try the program by following the Minimal Usage instructions.

Run the program (CDG_Magic.exe).

Select "File --> Open Project" and open "sample_project_04.cmp".

     Note: "sample_project_01.cmp" was included in release 000.
           The project file format has been changed.
           Release 000 projects can not be loaded with any other version.

           "sample_project_02.cmp" was included in release 001.
           "sample_project_03.cmp" was included in release 001a.

	   "sample_project_03b.cmp" is based on "sample_project_03.cmp",
	   but has been slightly altered to work with this release.

	   Release 001[a] projects are mostly compatible with this version.
	   However, the karaoke draw/erase event order has been changed.
	   Older karaoke clips will need to have a "Fix Timing" performed,
	   and any manual draw/erase timing changes will need readjustment.

Click the "Play / Pause" button to begin preview playback.


+----------------------------------------+
|     General Usage for Karaoke Mode     |
+----------------------------------------+

First, please try the program by following the Minimal Usage instructions.

Run the program (CDG_Magic.exe).

Click the "Set WAVE..." button to load your backing track.

Click the "Add BMP..." button to load your title/introduction graphic(s).

Click the "Add Text..." button to create a new text clip.

Drag the newly created text clip to a position at least
a few seconds prior to the first lyrics being sung.

Double-click the text clip to show the edit/settings window.

Please select the following settings for the purposes of this demonstration:

      BMP: 1
Font Face: BArial
       AA: Checked
Font Size: 20
     Type: Karaoke (5/Bottom/Line)
  Sq Size: 0
     Wipe: Both
 Rnd Size: 3
  Palette: Preset 1
   Fg Idx: 2
   Ol Idx: 1
   Bg Idx: 0
  Box Idx: 0
 Fill Idx: 16
  Frm Idx: 8 (or 4, or 12)
 Comp Idx: 16
    Comp?: Checked
       Bw: 3/4

Enter your song lyrics in the text editing box.
     Standard CTRL+X/C/V/A/Z shortcuts are supported.
     Text is shown actual size with the selected font face.
     Make sure all words are visible, breaking lines as appropriate.
     The current line, page, and row is displayed to the right of the preview.

Click the "Render Text" button to render the entered text.

Click the "Fix Timing" button.

If this is a fast song, you may want to set SR: 32,000 Hz (in the main window)
to play back at a slower rate (about 73% of real-time).

Click the "Play" button (or press F12) to begin playback.

The "Set Start" button will be given focus automatically after clicking "Play".

Press the space bar to mark the beginning of each word.

Press CTRL to set the wipe end time for the current word
to a time earlier than the start of the next word.

Click the "Play / Pause" button (in the main window) to stop playback,
or wait until the selected audio file reaches its natural conclusion.

Click the "Fix Timing" button to automatically set the text draw/erase times.
    Note: In page-by-page modes, "Fix Timing" may shorten some highlight
          durations when there is insufficient spacing between pages.
	  It is recommended to always begin synchronizing using
	  the equivalent (5, 6, or 8 line) line-by-line mode,
	  then save your project before rerendering in page-by-page mode.
    SHIFT+Click in line-by-line modes will set times using an
    alternate method where new lines are not drawn until the
    last line of the current page begins highlighting.

You can manually adjust the draw, erase, and highlight timing by dragging
the vertical line at the left side of the words on the timeline clip.
    Black: Draw Line
    Green: Highlight Word
      Red: Erase Line     [or Reveal Line in "Line Palette" modes.]
    CTRL+Drag moves the timing of all same type events including
    and to the right of the selected event by an equal amount.
    CTRL+SHIFT+Drag additionally limits movement to events on the same page.
    This is useful for adjusting draw/highlight timing in page-by-page modes.

Highlight duration is indicated by a green horizontal line above the lyric.
    SHIFT+Drag adjusts the duration of the selected word highlight.

Click the "Recalculate" button.

Position the "playback head" at the start of the song.

Click the "Play / Pause" button to begin preview playback.
     You should see the graphics playing in the "CD+G Preview" window.
     If any highlighting is "scrambled", try lowering the Bw: setting.
     For very fast songs, try setting both Sq Size: and Rnd Size: to 0.

Click "Export CDG..." to save a .cdg subcode packets file.

Select "File --> Save Project" to save the current project.

Select "File --> Exit" and click "Yes" to exit.


+-------------------+
|     Compiling     |
+-------------------+

Code::Blocks project files are included for Windows and Linux.

They have been tested on XP SP3 and PCLinuxOS KDE 2010.12, respectively.

You will also need to obtain and compile the following libraries:

    FLTK v1.3 : http://www.fltk.org/software.php
PortAudio v19 : http://www.portaudio.com/download.html

The long-term goal is to support all platforms which have both
FLTK and PortAudio available.
Presently, however, only the mentioned platforms have been tested.

For Windows, it is recommended to use the included Windows_FLTK__config.h
file (renamed to config.h and placed in the root FLTK directory)
when compiling the FLTK library.

For Linux, the standard FLTK configure script should be used.
However, to significantly reduce (but not eliminate) some
stability issues, it is recommended to first alter the following file:

FLTK_Root/src/Fl_File_Chooser.cxx

(xxx: refers to the line number and is not in the actual file.)

307: delete window;
308: delete favWindow;

to

307: Fl::delete_widget(favWindow);
308: Fl::delete_widget(window);

If that code is no longer present in the version of FLTK you're using,
please try the library as is, but there are no guarantees as to
compatibility or stability.
