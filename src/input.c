// VOLATILE - this must match the number of buttons defined in input_state.
#define INPUT_BUTTONS_LEN 20

typedef struct
{
	uint32_t held;
	uint32_t pressed;
	uint32_t released;
} InputButton;

typedef struct
{
	int32_t mouse_delta_x;
	int32_t mouse_delta_y;
	int32_t mouse_x;
	int32_t mouse_y;

	bool mouse_scroll_up;
	bool mouse_scroll_down;

	union 
	{
    	InputButton buttons[INPUT_BUTTONS_LEN];
    	struct 
    	{
	    	InputButton mouse_left;
	    	InputButton mouse_right;
        	InputButton move_forward;
        	InputButton move_back;
        	InputButton move_left;
        	InputButton move_right;
        	InputButton move_up;
        	InputButton move_down;
        	InputButton move_ana;
        	InputButton move_kata;
        	InputButton move_up_a;
        	InputButton move_up_b;
        	InputButton move_up_c;
        	InputButton move_up_d;
        	InputButton move_down_a;
        	InputButton move_down_b;
        	InputButton move_down_c;
        	InputButton move_down_d;
        	InputButton change_mode;
        	InputButton bang_center;
    	};
	};
} Input;

void input_reset_buttons(Input* input)
{
    for(uint32_t i = 0; i < INPUT_BUTTONS_LEN; i++) {
        input->buttons[i].pressed = 0;
        input->buttons[i].released = 0;
    }
}

void input_button_press(InputButton* btn) 
{
    btn->held = 1;
    btn->pressed = 1;
}

void input_button_release(InputButton* btn) 
{
    btn->held = 0;
    btn->released = 1;
}
