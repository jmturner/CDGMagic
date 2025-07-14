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

#include "CDGMagic_ScrollClip_Window.h"

CDGMagic_ScrollClip_Window::CDGMagic_ScrollClip_Window(CDGMagic_ScrollClip *clip_to_edit, int X, int Y) : Fl_Double_Window(0, 0, 0)
{
    const char *scrollclipwindow_title  = "Scroll Clip Settings";
    const int   scrollclipwindow_width  = 600;
    const int   scrollclipwindow_height = 400;

    label( scrollclipwindow_title );
    size( scrollclipwindow_width, scrollclipwindow_height );
    position( X-w()/2, Y );

    preview_buffer = new unsigned char[300 * 216 * 3];

    begin();

        // Create a new 24bit image and set the pointer to the CDG screen data pointer.
        PreviewImage = new Fl_RGB_Image( preview_buffer, 300, 216, 3, 0 );
        // Create a new empty box and associate the image with it.
        PreviewBox = new Fl_Box(10, 10, 300+Fl::box_dw(FL_DOWN_BOX), 216+Fl::box_dh(FL_DOWN_BOX));
        PreviewBox->box(FL_DOWN_BOX);
        PreviewBox->image(PreviewImage);

        PreviewLabel = new Fl_Box(PreviewBox->x(), PreviewBox->y()+PreviewBox->h(), PreviewBox->w(), 20);
        PreviewLabel->label("Preview Image");
        PreviewLabel->box(FL_FLAT_BOX);
/*
        Fl_Choice *Transition_choice = new Fl_Choice(PreviewBox->x()+PreviewBox->w()+100, 10, 150, 30, "Transition:");
        Transition_choice->add("Left->Right->Top->Bottom", 0, NULL);
        Transition_choice->add("Right->Left->Top->Bottom", 0, NULL);
        Transition_choice->add("Left->Right->Bottom->Top", 0, NULL);
        Transition_choice->value(0);

        Fl_Choice *Border_choice = new Fl_Choice(PreviewBox->x()+PreviewBox->w()+100, 50, 150, 30, "Border:");
        Border_choice->add("None", 0, NULL);
        Border_choice->add("Transition", 0, NULL);
        Border_choice->add("First", 0, NULL);
        Border_choice->add("Last", 0, NULL);
        Border_choice->value(0);

        Fl_Choice *Remap_choice = new Fl_Choice(PreviewBox->x()+PreviewBox->w()+100, 90, 150, 30, "Color Remap:");
        Remap_choice->add("None", 0, NULL);
        Remap_choice->add("Add 4", 0, NULL);
        Remap_choice->add("Add 8", 0, NULL);
        Remap_choice->add("Add 10", 0, NULL);
        Remap_choice->add("Add 12", 0, NULL);
        Remap_choice->add("Rotate 4", 0, NULL);
        Remap_choice->add("Rotate 8", 0, NULL);
        Remap_choice->add("Reverse", 0, NULL);
        Remap_choice->value(0);
*/
        const int ctrl_x_pos  = PreviewBox->x()+PreviewBox->w();
        const int ctrl_y_pos  = 30;
        const int ctrl_height = 25;
        const int ctrl_y_add  = 50;


        Trans_button = new Fl_Button(ctrl_x_pos+20, ctrl_y_pos+ctrl_y_add*1, 250, ctrl_height, "Load transition...");
        Trans_button->callback(DoTransition_callback_static, this);

        fill_idx_counter = new Fl_Counter(ctrl_x_pos+20, ctrl_y_pos+ctrl_y_add*2, 100, ctrl_height, "Fill Idx");
        fill_idx_counter->align(FL_ALIGN_TOP);
        fill_idx_counter->step(1.0, 10.0);
        fill_idx_counter->bounds(0.0, 255.0);
        fill_idx_counter->value(0);
        fill_idx_counter->callback( FillIdx_cb_static, this );

        comp_idx_counter = new Fl_Counter(ctrl_x_pos+150, ctrl_y_pos+ctrl_y_add*2, 100, ctrl_height, "Comp Idx");
        comp_idx_counter->align(FL_ALIGN_TOP);
        comp_idx_counter->step(1.0, 10.0);
        comp_idx_counter->bounds(0.0, 255.0);
        comp_idx_counter->value(0);
        comp_idx_counter->callback( CompIdx_cb_static, this );

        Composite_choice = new Fl_Choice(ctrl_x_pos+20, ctrl_y_pos+ctrl_y_add*3, 200, ctrl_height, "Composite Type");
        Composite_choice->align(FL_ALIGN_TOP);
        Composite_choice->add("None (Opaque Layer Replace)");
        Composite_choice->add("Replace Layer");
        Composite_choice->add("Composite Into Layer");
        Composite_choice->value(0);
        Composite_choice->callback( ShouldComp_cb_static, this );

        Fl_Button *Add_button = new Fl_Button(ctrl_x_pos+50, ctrl_y_pos+ctrl_y_add*5, 100, ctrl_height, "Add BMP...");
        Add_button->callback(DoAdd_callback_static, this);

        Fl_Button *OK_button = new Fl_Button(10, h()-40, 100, 30, "OK");
        OK_button->callback(DoOK_callback_static, this);
    end();

    current_clip = clip_to_edit;

    update_current_values();
};

CDGMagic_ScrollClip_Window::~CDGMagic_ScrollClip_Window()
{
    current_clip->null_edit();
    delete[] preview_buffer;
};

void CDGMagic_ScrollClip_Window::DoOK_callback(Fl_Widget *CallingWidget)
{
    Fl::delete_widget(this);
};

int CDGMagic_ScrollClip_Window::can_delete()
{
    show();
    return fl_choice("This clip has a window open for editing.\nDelete anyway?", "No (don't delete)", "Yes (delete this clip)", NULL);
};

void CDGMagic_ScrollClip_Window::DoAdd_callback(Fl_Widget *CallingWidget)
{
    current_clip->add_bmp_file();
    update_current_values();
};

void CDGMagic_ScrollClip_Window::DoTransition_callback(Fl_Widget *CallingWidget)
{
    current_clip->set_transition_file(0);
    if ( current_bmp_object->transition_file() )
    {
        CallingWidget->copy_label( current_bmp_object->transition_file() );
    }
    else
    {
        CallingWidget->copy_label( "Load transition..." );
    };
};

void CDGMagic_ScrollClip_Window::update_current_values()
{
    // Set the temp values to the requested page's pointers.
    current_bmp_object = current_clip->event_queue()->at(0)->BMPObject;
    // Update the controls.
    fill_idx_counter->value( current_bmp_object->fill_index() );
    comp_idx_counter->value( current_bmp_object->composite_index() );
    Composite_choice->value( current_bmp_object->should_composite() );
    if ( current_bmp_object->transition_file() )  {  Trans_button->copy_label( current_bmp_object->transition_file() );  }
    else  {  Trans_button->copy_label( "Load transition..." );  };
    // Update the preview display.
    update_preview();
};

void CDGMagic_ScrollClip_Window::update_preview()
{
    int x_offset = current_bmp_object->x_offset();
    int y_offset = current_bmp_object->y_offset();

    char temp_label[32];
    snprintf(temp_label, 32, "X: %i, Y: %i", x_offset, y_offset);
    PreviewLabel->copy_label(temp_label);

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

int CDGMagic_ScrollClip_Window::handle(int incoming_event)
{
    static int orig_x = 0, orig_y = 0;
    static int x_off_a = 0, y_off_a = 0;

    if (Fl::belowmouse() == PreviewBox)
    {
        switch ( incoming_event )
        {
            case FL_PUSH:
                if (Fl::event_button() != FL_LEFT_MOUSE) { break; };
                orig_x = Fl::event_x();
                orig_y = Fl::event_y();
                x_off_a = current_bmp_object->x_offset();
                y_off_a = current_bmp_object->y_offset();
                return 1;

            case FL_DRAG:
                if (Fl::event_button() != FL_LEFT_MOUSE) { break; };
                current_bmp_object->x_offset( x_off_a + Fl::event_x() - orig_x );
                current_bmp_object->y_offset( y_off_a + Fl::event_y() - orig_y );
                update_preview();
                return 1;
        };
    };

    static int previous_event = -1;
    if ( (incoming_event == FL_HIDE) && (previous_event != FL_UNFOCUS) )
    {
        printf("Deleting CDGMagic_ScrollClip_Window object!\n");
        Fl::delete_widget( this );
        return 1;
    };
    previous_event = incoming_event;

    return Fl_Double_Window::handle(incoming_event);
};

void CDGMagic_ScrollClip_Window::FillIdx_cb_static(Fl_Widget *CallingWidget, void *IncomingObject)
{
    CDGMagic_ScrollClip_Window*  this_win = static_cast<CDGMagic_ScrollClip_Window*>( IncomingObject );
    Fl_Counter*            idx_counter = static_cast<Fl_Counter*>( CallingWidget );
    this_win->current_bmp_object->fill_index( static_cast<int>(idx_counter->value()) );
};

void CDGMagic_ScrollClip_Window::CompIdx_cb_static(Fl_Widget *CallingWidget, void *IncomingObject)
{
    CDGMagic_ScrollClip_Window* this_win = static_cast<CDGMagic_ScrollClip_Window*>( IncomingObject );
    Fl_Counter*           idx_counter = static_cast<Fl_Counter*>( CallingWidget );
    this_win->current_bmp_object->composite_index( static_cast<int>(idx_counter->value()) );
};

void CDGMagic_ScrollClip_Window::ShouldComp_cb_static(Fl_Widget *CallingWidget, void *IncomingObject)
{
    CDGMagic_ScrollClip_Window*  this_win = static_cast<CDGMagic_ScrollClip_Window*>( IncomingObject );
    Fl_Choice*             type_choice = static_cast<Fl_Choice*>( CallingWidget );
    this_win->current_bmp_object->should_composite( type_choice->value() );
    printf("CDGMagic_ScrollClip_Window # should_composite # %i\n", this_win->current_bmp_object->should_composite() );
};

//##### Static Callbacks Below this Line #####//

void CDGMagic_ScrollClip_Window::DoOK_callback_static(Fl_Widget *CallingWidget, void *IncomingObject)
{
    ((CDGMagic_ScrollClip_Window*)IncomingObject)->DoOK_callback(CallingWidget);
};

void CDGMagic_ScrollClip_Window::DoAdd_callback_static(Fl_Widget *CallingWidget, void *IncomingObject)
{
    ((CDGMagic_ScrollClip_Window*)IncomingObject)->DoAdd_callback(CallingWidget);
};

void CDGMagic_ScrollClip_Window::DoTransition_callback_static(Fl_Widget *CallingWidget, void *IncomingObject)
{
    ((CDGMagic_ScrollClip_Window*)IncomingObject)->DoTransition_callback(CallingWidget);
};
