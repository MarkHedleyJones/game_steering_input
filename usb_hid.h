// /*
//     Logitech G25 controller on STM32F407
//     Copyright (C) 2017, markjones112358.

//     EMAIL: markjones112358@gmail.com

//     This piece of code is FREE SOFTWARE and is released
//     under the Apache License, Version 2.0 (the "License");
// */

// /*
//     USB-HID Gamepad for ChibiOS/RT
//     Copyright (C) 2014, +inf Wenzheng Xu.

//     EMAIL: wx330@nyu.edu

//     This piece of code is FREE SOFTWARE and is released
//     under the Apache License, Version 2.0 (the "License");
// */

// /*
//     ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio
//     Licensed under the Apache License, Version 2.0 (the "License");
//     you may not use this file except in compliance with the License.
//     You may obtain a copy of the License at
//         http://www.apache.org/licenses/LICENSE-2.0
//     Unless required by applicable law or agreed to in writing, software
//     distributed under the License is distributed on an "AS IS" BASIS,
//     WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//     See the License for the specific language governing permissions and
//     limitations under the License.
// */



/*

  Copyright (c) 2014 Guillaume Duc <guillaume@guiduc.org>

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

*/

#ifndef _USB_HID_H_
#define _USB_HID_H_

#include "ch.h"
#include "hal.h"

// Size in bytes of the IN report (board->PC)
#define USB_HID_IN_REPORT_SIZE 11
// Size in bytes of the OUT report (PC->board)
#define USB_HID_OUT_REPORT_SIZE 7

// Number of OUT reports that can be stored inside the input queue
#define USB_INPUT_QUEUE_CAPACITY 10
// Number of IN reports that can be stored inside the output queue
#define USB_OUTPUT_QUEUE_CAPACITY 10

#define USB_INPUT_QUEUE_BUFFER_SIZE (USB_INPUT_QUEUE_CAPACITY * USB_HID_OUT_REPORT_SIZE)
#define USB_OUTPUT_QUEUE_BUFFER_SIZE (USB_OUTPUT_QUEUE_CAPACITY * USB_HID_IN_REPORT_SIZE)

extern uint8_t output_handb;
extern uint8_t output_steer;
extern uint8_t output_brake;
extern uint8_t output_pedal;


// extern uint32_t angle_max;
// extern uint32_t angle_min;
// extern uint32_t output_brake;
// extern uint32_t output_angle;
// extern uint32_t output_pedal;
// extern uint8_t  output_handbrake;


/*
 * Content of the IN report (board->PC)
 * - 1 byte: sequence number
 * - 1 byte: current value of the WKUP pushbutton
 *
 * => 2 bytes
 */
struct usb_hid_in_report_s
{
  int8_t a0;
  int8_t a1;
  int8_t a2;
  int16_t a3;
  int8_t a4;
  int8_t a5;
  int8_t a6;
  int8_t a7;
  int8_t a8;
  int8_t a9;
};

/*
 * Content of the OUT report (PC->board)
 * - 1 byte: sequence number
 * - 1 byte: LED value
 *
 * => 2 bytes
 */
struct usb_hid_out_report_s
{
  uint8_t a0;
  uint8_t a1;
  uint8_t a2;
  uint8_t a3;
  uint8_t a4;
  uint8_t a5;
  uint8_t a6;
};


// The reception queue
extern InputQueue usb_input_queue;
// The emission queue
extern OutputQueue usb_output_queue;

// Initialize the USB serial string from STM32 unique serial number
void init_usb_serial_string (void);

// Initialize the USB Input/Output queues
void init_usb_queues (void);

// Initialize the USB driver and bus
void init_usb_driver (void);

// Queue a report to be sent
int usb_send_hid_report (struct usb_hid_in_report_s *report);

// Prepare an IN report
void usb_build_in_report (struct usb_hid_in_report_s *report);

#endif /* _USB_HID_H_ */
