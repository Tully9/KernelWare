#include "input.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include "../shared/kw_ioctl.h"


// Where input is read from
#define KBD_DEVICE "/dev/input/event18"

static int drv_fd = -1;

// To update the menu screen
extern volatile int currentScreen;

void input_init(int fd)
{
    drv_fd = fd;
}

// Maps linux keycodes to a button index and character
struct key_mapping {
    int  keycode;
    int  btn_index;
    char character;
};
// Same but for mapping number keycodes to screens
struct nav_mapping {
    int keycode;
    int screen;
};

//Maps asdf keys, to add more keys ADD TO STRUCT
static struct key_mapping keymap[] = {
    { KEY_A, 0, 'a' },
    { KEY_S, 1, 's' },
    { KEY_D, 2, 'd' },
    { KEY_F, 3, 'f' },
};
static int keymap_size = sizeof(keymap) / sizeof(keymap[0]);


//For Nav keys
static struct nav_mapping navmap[] = {
    { KEY_1, 1 },
    { KEY_2, 2 },
    { KEY_3, 3 },
};
static int navmap_size = sizeof(navmap) / sizeof(navmap[0]);


void *kw_input_thread(void *arg) // arg isn't used but compiler will give a waring as in pthread signature
{
    (void)arg;

    struct input_event ev;

    int kbd_fd = open(KBD_DEVICE, O_RDONLY);
    if (kbd_fd < 0) {
        perror("input: failed to open " KBD_DEVICE);
        return NULL;
    }
    printf("input: listening on " KBD_DEVICE "\n");

    while (1)
    {
        ssize_t n = read(kbd_fd, &ev, sizeof(ev));
        if (n < (ssize_t)sizeof(ev)) {
            perror("input: read error");
            break;
        }

        if (ev.type != EV_KEY || ev.value != 1)
            continue;

        // Checks keypressa, sets last key,and writes to driver
        for (int i = 0; i < keymap_size; i++)
        {
            if (keymap[i].keycode != ev.code)
                continue;

            last_key = keymap[i].character;

            if (drv_fd >= 0) {
                unsigned char event_byte = KW_EVENT_BTN(keymap[i].btn_index);
                if (write(drv_fd, &event_byte, 1) < 0)
                    perror("input: write to driver");
            }

            break;
        }

        // checks nav keys
        for (int i = 0; i < navmap_size; i++) {
            if (navmap[i].keycode == ev.code) {
                currentScreen = navmap[i].screen;
                break;
            }
}
    }

    close(kbd_fd);
    return NULL;
}