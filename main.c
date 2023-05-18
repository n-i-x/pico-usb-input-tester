/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *                    sekigon-gonnoc
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

// This example runs both host and device concurrently. The USB host receive
// reports from HID device and print it out over USB Device CDC interface.
// For TinyUSB roothub port0 is native usb controller, roothub port1 is
// pico-pio-usb.
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/bootrom.h"
#include <pico/time.h>

#include "pio_usb.h"
#include "tusb.h"
#include "xinput_host.h"

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+

// uncomment if you are using colemak layout
// #define KEYBOARD_COLEMAK

#ifdef KEYBOARD_COLEMAK
const uint8_t colemak[128] = {
  0  ,  0,  0,  0,  0,  0,  0, 22,
  9  , 23,  7,  0, 24, 17,  8, 12,
  0  , 14, 28, 51,  0, 19, 21, 10,
  15 ,  0,  0,  0, 13,  0,  0,  0,
  0  ,  0,  0,  0,  0,  0,  0,  0,
  0  ,  0,  0,  0,  0,  0,  0,  0,
  0  ,  0,  0, 18,  0,  0,  0,  0,
  0  ,  0,  0,  0,  0,  0,  0,  0,
  0  ,  0,  0,  0,  0,  0,  0,  0,
  0  ,  0,  0,  0,  0,  0,  0,  0
};
#endif

static uint8_t const keycode2ascii[128][2] =  { HID_KEYCODE_TO_ASCII };

uint64_t getTime()
{
    return time_us_64();
}

uint64_t INPUT_SENT;
int NUM_TESTS = 10;
float TEST_DATA[10];
int CURR_TEST;

void sendInput() {
  gpio_put(3, false);
}

float arrmax(float a[], int num_elements)
{
   int i;
   float max;
   max = a[0];
   for (i=1; i<num_elements; i++)
   {
      if (a[i]>max)
      {
         max=a[i];
      }
   }

   return(max);
}

float arrmin(float a[], int num_elements)
{
   int i;
   float min;
   min = a[0];
   for (i=1; i<num_elements; i++)
   {
      if (a[i]<min)
      {
         min=a[i];
      }
   }

   return(min);
}

float arravg(float a[], int num_elements)
{
   float sum;
   int i;
   float avg;
   sum=0;
   avg=0;

   for (i=0; i<num_elements;i++)
   {
      sum=sum+a[i];
      avg=(float)sum/(i+1);
   }
   return(avg);
}

/*------------- MAIN -------------*/

// core1: handle host events
void core1_main() {
  sleep_ms(10);

  // Use tuh_configure() to pass pio configuration to the host stack
  // Note: tuh_configure() must be called before
  pio_usb_configuration_t pio_cfg = PIO_USB_DEFAULT_CONFIG;
  tuh_configure(1, TUH_CFGID_RPI_PIO_USB_CONFIGURATION, &pio_cfg);

  // To run USB SOF interrupt in core1, init host stack for pio_usb (roothub
  // port1) on core1
  tuh_init(1);

  while (true) {
    tuh_task(); // tinyusb host task
  }
}

// core0: handle device events
int main(void) {
  // default 125MHz is not appropreate. Sysclock should be multiple of 12MHz.
  set_sys_clock_khz(120000, true);

  sleep_ms(10);

  multicore_reset_core1();
  // all USB task run in core1
  multicore_launch_core1(core1_main);

  gpio_init(3);             // Initialize pin
  gpio_set_dir(3, GPIO_OUT); // Set as INPUT
  gpio_pull_up(3);          // Set as PULLUP
  gpio_put(3, true);

  // init device stack on native usb (roothub port0)
  tud_init(0);

  while (true) {
    tud_task(); // tinyusb device task
    tud_cdc_write_flush();
  }

  return 0;
}

//--------------------------------------------------------------------+
// Device CDC
//--------------------------------------------------------------------+

// Invoked when CDC interface received data from host
void tud_cdc_rx_cb(uint8_t itf)
{
  (void) itf;

  char buf[64];
  uint32_t count = tud_cdc_read(buf, sizeof(buf));

  // TODO control LED on keyboard of host stack
  (void) count;
}

//--------------------------------------------------------------------+
// Host HID
//--------------------------------------------------------------------+

// Invoked when device with hid interface is mounted
// Report descriptor is also available for use. tuh_hid_parse_report_descriptor()
// can be used to parse common/simple enough descriptor.
// Note: if report descriptor length > CFG_TUH_ENUMERATION_BUFSIZE, it will be skipped
// therefore report_desc = NULL, desc_len = 0
void tuh_hid_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* desc_report, uint16_t desc_len)
{
  (void)desc_report;
  (void)desc_len;

  // Interface protocol (hid_interface_protocol_enum_t)
  const char* protocol_str[] = { "None", "Keyboard", "Mouse" };
  uint8_t const itf_protocol = tuh_hid_interface_protocol(dev_addr, instance);

  uint16_t vid, pid;
  tuh_vid_pid_get(dev_addr, &vid, &pid);

  char tempbuf[256];
  int count = sprintf(tempbuf, "[%04x:%04x][%u] HID Interface%u, Protocol = %s\r\n", vid, pid, dev_addr, instance, protocol_str[itf_protocol]);

  tud_cdc_write(tempbuf, count);
  tud_cdc_write_flush();

  // Receive report from boot keyboard & mouse only
  // tuh_hid_report_received_cb() will be invoked when report is available
  if (itf_protocol == HID_ITF_PROTOCOL_KEYBOARD || itf_protocol == HID_ITF_PROTOCOL_MOUSE)
  {
    if ( !tuh_hid_receive_report(dev_addr, instance) )
    {
      tud_cdc_write_str("Error: cannot request report\r\n");
    }
  }
}

// Invoked when device with hid interface is un-mounted
void tuh_hid_umount_cb(uint8_t dev_addr, uint8_t instance)
{
  char tempbuf[256];
  int count = sprintf(tempbuf, "[%u] HID Interface%u is unmounted\r\n", dev_addr, instance);
  tud_cdc_write(tempbuf, count);
  tud_cdc_write_flush();
}

// look up new key in previous keys
static inline bool find_key_in_report(hid_keyboard_report_t const *report, uint8_t keycode)
{
  for(uint8_t i=0; i<6; i++)
  {
    if (report->keycode[i] == keycode)  return true;
  }

  return false;
}


// convert hid keycode to ascii and print via usb device CDC (ignore non-printable)
static void process_kbd_report(uint8_t dev_addr, hid_keyboard_report_t const *report)
{
  (void) dev_addr;
  static hid_keyboard_report_t prev_report = { 0, 0, {0} }; // previous report to check key released
  bool flush = false;

  for(uint8_t i=0; i<6; i++)
  {
    uint8_t keycode = report->keycode[i];
    if ( keycode )
    {
      if ( find_key_in_report(&prev_report, keycode) )
      {
        // exist in previous report means the current key is holding
      }else
      {
        // not existed in previous report means the current key is pressed

        // remap the key code for Colemak layout
        #ifdef KEYBOARD_COLEMAK
        uint8_t colemak_key_code = colemak[keycode];
        if (colemak_key_code != 0) keycode = colemak_key_code;
        #endif

        bool const is_shift = report->modifier & (KEYBOARD_MODIFIER_LEFTSHIFT | KEYBOARD_MODIFIER_RIGHTSHIFT);
        uint8_t ch = keycode2ascii[keycode][is_shift ? 1 : 0];

        if (ch)
        {
          if (ch == '\n') tud_cdc_write("\r", 1);
          tud_cdc_write(&ch, 1);
          flush = true;
        }
      }
    }
    // TODO example skips key released
  }

  if (flush) tud_cdc_write_flush();

  prev_report = *report;
}

// Invoked when received report from device via interrupt endpoint
void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len)
{
  (void) len;
  uint8_t const itf_protocol = tuh_hid_interface_protocol(dev_addr, instance);

  switch(itf_protocol)
  {
    case HID_ITF_PROTOCOL_KEYBOARD:
      process_kbd_report(dev_addr, (hid_keyboard_report_t const*) report );
    break;

    default: break;
  }

  // continue to request to receive report
  if ( !tuh_hid_receive_report(dev_addr, instance) )
  {
    tud_cdc_write_str("Error: cannot request report\r\n");
  }
}

void tuh_xinput_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const *report, uint16_t len)
{
    char tempbuf[256];
    int count;
    uint64_t currTime = time_us_64();
    uint64_t tmpElapsed = currTime - INPUT_SENT;
    count = sprintf(tempbuf, "Previous: %lluusec Current: %lluusec Elapsed: %lluusec\n", INPUT_SENT, currTime, tmpElapsed);

    tud_cdc_write(tempbuf, count);
    tud_cdc_write_flush();

    float elapsedTime = (float)tmpElapsed / 1000.0;
    count = sprintf(tempbuf, "Current elapsed time: %.2fms\n", elapsedTime);

    tud_cdc_write(tempbuf, count);
    tud_cdc_write_flush();
    

    xinputh_interface_t *xid_itf = (xinputh_interface_t *)report;
    xinput_gamepad_t *p = &xid_itf->pad;

    if (xid_itf->connected && p->wButtons != 0x0000) {
        count = sprintf(tempbuf, "Current lag: %.2f\n", elapsedTime);

        tud_cdc_write(tempbuf, count);
        tud_cdc_write_flush();

        TEST_DATA[CURR_TEST] = elapsedTime;
        sleep_ms(100);
        gpio_put(3, true);

        CURR_TEST++;

        if (CURR_TEST < NUM_TESTS) {
          sleep_ms(100);
          INPUT_SENT = time_us_64();
          gpio_put(3, false);
        } else {
          float min = arrmin(TEST_DATA, 10);
          float max = arrmax(TEST_DATA, 10);
          float avg = arravg(TEST_DATA, 10);
          int count = sprintf(tempbuf, "Min:%.2f ms, Max:%.2f ms, Avg: %.2f ms\n", min, max, avg);

          tud_cdc_write(tempbuf, count);
          tud_cdc_write_flush();
        }
    }

    tuh_xinput_receive_report(dev_addr, instance);
}

void tuh_xinput_mount_cb(uint8_t dev_addr, uint8_t instance, const xinputh_interface_t *xinput_itf)
{
    char tempbuf[256];
    int count = sprintf(tempbuf, "XINPUT MOUNTED %02x %d\n", dev_addr, instance);

    tud_cdc_write(tempbuf, count);
    tud_cdc_write_flush();

    // If this is a Xbox 360 Wireless controller we need to wait for a connection packet
    // on the in pipe before setting LEDs etc. So just start getting data until a controller is connected.
    if (xinput_itf->type == XBOX360_WIRELESS && xinput_itf->connected == false)
    {
        tuh_xinput_receive_report(dev_addr, instance);
        return;
    }
    tuh_xinput_set_led(dev_addr, instance, 0, true);
    tuh_xinput_set_led(dev_addr, instance, 1, true);
    tuh_xinput_set_rumble(dev_addr, instance, 0, 0, true);
    tuh_xinput_receive_report(dev_addr, instance);

    tud_cdc_write("Beginning automatic testing...\n", count);
    tud_cdc_write_flush();

    CURR_TEST = 0;
    INPUT_SENT = time_us_64();
    gpio_put(3, false);
}

void tuh_xinput_umount_cb(uint8_t dev_addr, uint8_t instance)
{
    char tempbuf[256];
    int count = sprintf(tempbuf, "XINPUT UNMOUNTED %02x %d\n", dev_addr, instance);

    tud_cdc_write(tempbuf, count);
    tud_cdc_write_flush();

    CURR_TEST = 0;
    memset(TEST_DATA, 0, sizeof(TEST_DATA));
}