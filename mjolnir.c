#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <getopt.h>
#include <wchar.h>
#include <hidapi/hidapi.h>

#define GENESIS_VID 0x258a
#define THOR300_PID 0x0090
#define INTERFACE   1
#define CMD_LENGTH  8

/* Mode setting
 * 0x0a,
 * 0x0a,
 * 0x07 -- effect type:
 *          0x00 prismo
 *          0x01 breathing
 *          0x02 wave LTR
 *          0x03 flowers blossom
 *          0x04 rainbow
 *          0x05 wave 2
 *          0x06 cw rotation
 *          0x07 wave 3
 *          0x08 response
 *          0x09 ccw rotation
 *          0x0a snake
 *          0x0b wave 4
 *          0x0c tornado
 *          0x0d neon
 *          0x0e stars twinkling
 *          0x0f single color response
 *          0x10 steady
 *          0x11 raindrops
 *          0x12 wave 5
 *          0x13 custom
 * 0x03 -- brightness (0x00 -- 0x04)
 * 0x02 -- speed (0x00 -- 0x04)
 * 0x06 -- color:
 *          0x00 red,
 *          0x01 green,
 *          0x02 blue,
 *          0x03 orange,
 *          0x04 purple,
 *          0x05 cyan,
 *          0x06 "white",
 *          0x07 rainbow
 * 0x00,
 * 0x00
 */

/* Custom key settings
 * 0x0a
 * 0x0c,
 * 0x01,
 * 0x00 -- key number
 * 0x00 -- red value (0x00 -- 0xff)
 * 0x00 -- green value (0x00 -- 0xff)
 * 0x00 -- blue value (0x00 -- 0xff)
 * 0x00
 *
 * Needs to end with "key" 0x89
 */

static int verbose;

static struct option long_options[] = {
    {"verbose", no_argument, 0, 'v'},
    {"effect", required_argument, 0, 'e'},
    {"brightness", required_argument, 0, 'b'},
    {"speed", required_argument, 0, 's'},
    {"color", required_argument, 0, 'c'},
    { 0 }
};

struct mode {
    uint8_t effect;
    uint8_t brightness;
    uint8_t speed;
    uint8_t color;
};

void print_mode(struct mode *mode) {
    wprintf(L"Setting effect 0x%02x, with brightness 0x%02x, speed 0x%02x and color 0x%02x\n",
            mode->effect, mode->brightness, mode->speed, mode->color);
}

hid_device* get_keyboard_device() {
    int r;
    r = hid_init();
    if (r) {
        fwprintf(stderr, L"Unable to initialize HIDAPI library\n");
        return NULL;
    }

    struct hid_device_info *info, *list;
    hid_device *handle;
    list = hid_enumerate(GENESIS_VID, THOR300_PID);
    info = list;
    while (info) {
        if (info->vendor_id == GENESIS_VID && info->product_id == THOR300_PID && info->interface_number == INTERFACE) {
            if (verbose)
                wprintf(L"Opening device %s (%04x:%04x)...\n", info->path, info->vendor_id, info->product_id);
            handle = hid_open_path(info->path);
            break;
        }
        info = info->next;
    }
    hid_free_enumeration(list);
    return handle;
}

void send_commands(hid_device *dev, unsigned char data[][CMD_LENGTH], unsigned int length) {
    int r;
    for (int i = 0; i < length; i++) {
        if (verbose) {
            wprintf(L"Sending command: ");
            for (int x = 1; x < sizeof(data[i]); x++)
                wprintf(L"0x%02x ", data[i][x]);
            wprintf(L"as report %d\n", data[i][0]);
        }
        r = hid_send_feature_report(dev, data[i], sizeof(data[i]));
        if (r < 0)
            fwprintf(stderr, L"Error %d during sending command: %ls\n", r, hid_error(dev));
    }
}

int set_keyboard_mode(struct mode *mode) {
    if (verbose)
        print_mode(mode);

    hid_device *dev;
    dev = get_keyboard_device();
    if (!dev) {
        fwprintf(stderr, L"Unable to open correct device\n");
        return -10;
    }

    unsigned char command[][CMD_LENGTH] = {
        //{ 0x0a, 0x03, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00 },
        { 0x0a, 0x0a, mode->effect, mode->brightness, mode->speed, mode->color, 0x00, 0x00 }
    };

    send_commands(dev, command, sizeof(command) / CMD_LENGTH);
    hid_close(dev);
    return hid_exit();
}

int main(int argc, char* argv[]) {

    struct mode mode = {
        0x00,
        0x02,
        0x02,
        0x07,
    };

    while(1) {
        int c;
        int option_index = 0;
        c = getopt_long(argc, argv, "ve:b:s:c:", long_options, &option_index);

        if (c == -1)
            break;

        switch(c) {
            case 'v':
                verbose = 1;
                break;
            case 'e':
                mode.effect = strtoul(optarg, NULL, 0);
                break;
            case 'b':
                mode.brightness = strtoul(optarg, NULL, 0);
                break;
            case 's':
                mode.speed = strtoul(optarg, NULL, 0);
                break;
            case 'c':
                mode.color = strtoul(optarg, NULL, 0);
                break;
            case '?':
                break;
            default:
                return -1;
        }
    }

    int r;
    r = set_keyboard_mode(&mode);
    return r;
}
