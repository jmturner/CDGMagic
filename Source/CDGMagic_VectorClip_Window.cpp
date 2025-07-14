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

#include "CDGMagic_VectorClip_Window.h"

CDGMagic_VectorClip_Window::CDGMagic_VectorClip_Window(int X, int Y) : Fl_Double_Window(0, 0, 0)
{
    const char *bmpclipwindow_title  = "Vector Clip Settings";
    const int   bmpclipwindow_width  = 700;
    const int   bmpclipwindow_height = 500;

    label( bmpclipwindow_title );
    size( bmpclipwindow_width, bmpclipwindow_height );
    position( X-w()/2, Y );

    begin();

        PreviewBox = new Fl_Box(10, 10, 600, 432);
        PreviewBox->color(FL_BLACK);
        PreviewBox->box(FL_FLAT_BOX);

        min_x = PreviewBox->x();
        min_y = PreviewBox->y();
        max_x = PreviewBox->x()+PreviewBox->w();
        max_y = PreviewBox->y()+PreviewBox->h();

        Fl_Button *OK_button = new Fl_Button(10, h()-40, 100, 30, "OK");
        OK_button->callback(DoOK_callback_static, this);

        Fl_Button *Record_button = new Fl_Button(240, h()-40, 100, 30, "Record");
        Record_button->callback(DoRecord_callback_static, this);

        Fl_Button *Play_button = new Fl_Button(360, h()-40, 100, 30, "Play");
        Play_button->callback(DoPlay_callback_static, this);

        Fl_Button *Clear_button = new Fl_Button(480, h()-40, 100, 30, "Clear");
        Clear_button->callback(DoClear_callback_static, this);

    end();

    record_mode = 0;
};

CDGMagic_VectorClip_Window::~CDGMagic_VectorClip_Window()
{
    printf("CDGMagic_VectorClip_Window::~CDGMagic_VectorClip_Window()\n");
};

void CDGMagic_VectorClip_Window::DoOK_callback(Fl_Widget *CallingWidget)
{
    Fl::delete_widget(this);
};

void CDGMagic_VectorClip_Window::Do_Vector_Update(void *IncomingObject)
{
    CDGMagic_VectorClip_Window *win_p = ((CDGMagic_VectorClip_Window*)IncomingObject);

    int new_time = win_p->get_time();
    static int old_time = 0;
    static unsigned int current_vector = 0;
    if (new_time < old_time)
    {
        win_p->make_current();
        fl_draw_box(FL_FLAT_BOX, 10, 10, 600, 432, FL_BLACK);
        current_vector = 0;
    };
    old_time = new_time;

    while ( (current_vector < win_p->record_buffer_deque.size())
         && (win_p->record_buffer_deque.at(current_vector).time <= new_time) )
    {
        win_p->make_current();
        fl_color(FL_WHITE);
        fl_line_style(FL_SOLID, 2, NULL);
        float x_correction = static_cast<float>(win_p->max_x-win_p->min_x)*0.5;
        float y_correction = static_cast<float>(win_p->max_y-win_p->min_y)*0.5;
        fl_line( static_cast<int>(win_p->record_buffer_deque.at(current_vector).x1*x_correction)+win_p->min_x,
                 static_cast<int>(win_p->record_buffer_deque.at(current_vector).y1*y_correction)+win_p->min_y,
                 static_cast<int>(win_p->record_buffer_deque.at(current_vector).x2*x_correction)+win_p->min_x,
                 static_cast<int>(win_p->record_buffer_deque.at(current_vector).y2*y_correction)+win_p->min_y );
        fl_line_style(0);
        current_vector++;
    };

    Fl::repeat_timeout(0.01, Do_Vector_Update, IncomingObject);
};

void CDGMagic_VectorClip_Window::DoRecord_callback(Fl_Widget *CallingWidget)
{
    printf("CDGMagic_VectorClip_Window:: record mode ON\n");
    Fl::remove_timeout(Do_Vector_Update);
    record_mode = 1;
};

void CDGMagic_VectorClip_Window::DoPlay_callback(Fl_Widget *CallingWidget)
{
    printf("CDGMagic_VectorClip_Window:: record mode OFF\n");
    record_mode = 0;

    CDGMagic_VectorClip_Window::Do_Vector_Update(this);
};

void CDGMagic_VectorClip_Window::DoClear_callback(Fl_Widget *CallingWidget)
{
    record_buffer_deque.clear();
};

/*
void CDGMagic_BMPClip_Window::update_preview()
{
    unsigned int tmp_color = 0x00;
    unsigned int src_x = 0, src_y = 0, dst_pxl = 0;

    for (int y_inc = 0; y_inc < 216; y_inc++)
    {
        src_y = (y_inc - y_offset);
        for (int x_inc = 0; x_inc < 300; x_inc++)
        {
            src_x = (x_inc - x_offset);
            dst_pxl = (x_inc + y_inc * 300) * 3;
            tmp_color = current_bmp_object->get_rgb_pixel(src_x, src_y);
            preview_buffer[dst_pxl+0] = (tmp_color >> 030) & 0xFF;
            preview_buffer[dst_pxl+1] = (tmp_color >> 020) & 0xFF;
            preview_buffer[dst_pxl+2] = (tmp_color >> 010) & 0xFF;
        };
    }
    PreviewImage->uncache();
    PreviewBox->redraw();
};
*/
int CDGMagic_VectorClip_Window::handle(int incoming_event)
{
    static int orig_x = 0, orig_y = 0;
//    static int x_off_a = 0, y_off_a = 0;

    if (Fl::belowmouse() == PreviewBox)
    {
        switch ( incoming_event )
        {
            case FL_PUSH:
                if (Fl::event_button() != FL_LEFT_MOUSE) { break; };
                orig_x = Fl::event_x();
                orig_y = Fl::event_y();

                add_and_draw_line(orig_x, orig_y, orig_x, orig_y);

//                x_off_a = current_bmp_object->x_offset();
//                y_off_a = current_bmp_object->y_offset();
                return 1;

            case FL_DRAG:
                if (Fl::event_button() != FL_LEFT_MOUSE) { break; };

                add_and_draw_line(orig_x, orig_y, Fl::event_x(), Fl::event_y());

                orig_x = Fl::event_x();
                orig_y = Fl::event_y();
//                current_bmp_object->x_offset( x_off_a + Fl::event_x() - orig_x );
//                current_bmp_object->y_offset( y_off_a + Fl::event_y() - orig_y );
//                update_preview();
                return 1;

            case FL_RELEASE:
                fl_line_style(0);
                return 1;
        };
    };

    return Fl_Double_Window::handle(incoming_event);
};

void CDGMagic_VectorClip_Window::add_and_draw_line(int x1, int y1, int x2, int y2)
{
    if (  (record_mode == 1)
       && (x1 >= min_x) && (x1 < max_x)
       && (y1 >= min_y) && (y1 < max_y)
       && (x2 >= min_x) && (x2 < max_x)
       && (y2 >= min_y) && (y2 < max_y) )
    {
        line_cords temp_cords;
        temp_cords.time = get_time();
        temp_cords.x1 = static_cast<float>(x1-min_x) / static_cast<float>(max_x-min_x);
        temp_cords.y1 = static_cast<float>(y1-min_y) / static_cast<float>(max_y-min_y);
        temp_cords.x2 = static_cast<float>(x2-min_x) / static_cast<float>(max_x-min_x);
        temp_cords.y2 = static_cast<float>(y2-min_y) / static_cast<float>(max_y-min_y);
        record_buffer_deque.push_back(temp_cords);
        make_current();
        fl_color(FL_WHITE);
        fl_line_style(FL_SOLID, 4, NULL);
        fl_line(x1, y1, x2, y2);
        //printf("%i: %i,%i:%i,%i\n", temp_cords.time, temp_cords.x1, temp_cords.y1, temp_cords.x2, temp_cords.y2 );
        printf("%i: %f,%f:%f,%f\n", temp_cords.time, temp_cords.x1, temp_cords.y1, temp_cords.x2, temp_cords.y2 );
    };
}

void CDGMagic_VectorClip_Window::set_time_var(int *time)
{
   current_time = time;
}

int CDGMagic_VectorClip_Window::get_time()
{
    return *current_time;
};

//##### Static Callbacks Below this Line #####//

void CDGMagic_VectorClip_Window::DoOK_callback_static(Fl_Widget *CallingWidget, void *IncomingObject)
{
    ((CDGMagic_VectorClip_Window*)IncomingObject)->DoOK_callback(CallingWidget);
};

void CDGMagic_VectorClip_Window::DoRecord_callback_static(Fl_Widget *CallingWidget, void *IncomingObject)
{
    ((CDGMagic_VectorClip_Window*)IncomingObject)->DoRecord_callback(CallingWidget);
};

void CDGMagic_VectorClip_Window::DoPlay_callback_static(Fl_Widget *CallingWidget, void *IncomingObject)
{
    ((CDGMagic_VectorClip_Window*)IncomingObject)->DoPlay_callback(CallingWidget);
};

void CDGMagic_VectorClip_Window::DoClear_callback_static(Fl_Widget *CallingWidget, void *IncomingObject)
{
    ((CDGMagic_VectorClip_Window*)IncomingObject)->DoClear_callback(CallingWidget);
};
