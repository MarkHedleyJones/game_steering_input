/*
    USB-HID Gamepad for ChibiOS/RT
    Copyright (C) 2014, +inf Wenzheng Xu.

    EMAIL: wx330@nyu.edu

    This piece of code is FREE SOFTWARE and is released
    under the Apache License, Version 2.0 (the "License");
*/

/*
    ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
        http://www.apache.org/licenses/LICENSE-2.0
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/


#include "ch.h"
#include "hal.h"
#include <math.h>

#include "usb_hid.h"
#include "usbcfg.h"
#include "adccfg.h"


// Possible Threads

#define BYTE_START a2
#define BYTE_BACK a2
#define BYTE_BTN_THUMBLEFT a2
#define BYTE_BTN_THUMBRIGHT a2
#define BYTE_BTN_A a3
#define BYTE_BTN_B a3
#define BYTE_BTN_X a3
#define BYTE_BTN_Y a3
#define BYTE_BTN_TRIGGERLEFT a3
#define BYTE_BTN_TRIGGERRIGHT a3
#define BYTE_BTN_MODE a3
#define BYTE_TRIGGER_LEFT a4
#define BYTE_TRIGGER_RIGHT a5

#define BIT_START 0x10
#define BIT_BACK 0x20
#define BIT_BTN_THUMBLEFT 0x40
#define BIT_BTN_THUMBRIGHT 0x80
#define BIT_BTN_A 0x10
#define BIT_BTN_B 0x20
#define BIT_BTN_X 0x40
#define BIT_BTN_Y 0x80
#define BIT_BTN_TRIGGERLEFT 0x01
#define BIT_BTN_TRIGGERRIGHT 0x02
#define BIT_BTN_MODE 0x04

// void start_press(hid_data_in *data) {
//   data->a3 |= 0x40;
// }

// void start_depressed(hid_data_in *data) {
//   data->a3 &= ~0x40;
// }


// static WORKING_AREA(waThread1, 128);
// static msg_t Thread1(void *arg) {
//   (void)arg;
//   chRegSetThreadName("blinker");
//   while (TRUE) {
//     palClearPad(GPIOD, 15);
//     chThdSleepMilliseconds(50);
//     palSetPad(GPIOD, 15);
//     chThdSleepMilliseconds(50);
//   }
//   return (msg_t)0;
// }

// Main

int main(void) {
	halInit();
	chSysInit();

  palSetPadMode(GPIOD, 13, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPadMode(GPIOD, 14, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPadMode(GPIOD, 15, PAL_MODE_OUTPUT_PUSHPULL);

	usbInitState=0;
  uint16_t count = 0;
	usbDisconnectBus(&USBD1);
  chThdSleepMilliseconds(1000);
  usbStart(&USBD1, &usbcfg);
  usbConnectBus(&USBD1);
  chThdSleepMilliseconds(1000);
	/*
	 * Creates the blinker thread.
	 */
	/*
	chThdSleepMilliseconds(3000);
    usbDisconnectBus(&USBD1);
    usbStop(&USBD1);
    chThdSleepMilliseconds(1500);
    usbStart(&USBD1, &usbcfg);
    usbConnectBus(&USBD1);
    */


	// chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);
  // chThdCreateStatic(waThread2, sizeof(waThread2), NORMALPRIO, Thread2, NULL);


	/*
	 * Normal main() thread activity, in this demo it does nothing except
	 * sleeping in a loop and check the button state.
	 */
      /*
   * Setting up analog inputs used by the demo.
   */
   // myADCinit();

   palSetGroupMode(GPIOA, PAL_PORT_BIT(1) | PAL_PORT_BIT(2)| PAL_PORT_BIT(3)| PAL_PORT_BIT(4)| PAL_PORT_BIT(5)| PAL_PORT_BIT(6)| PAL_PORT_BIT(7),0, PAL_MODE_INPUT);

	//hid_transmit(&USBD1);

   double ret;
	while (TRUE) {
		chThdSleepMilliseconds(50);
      ++count;
      ret = (sin(count / 20.0) * 128.0) + 128;
      hid_in_data.a0 = 0x00;
      hid_in_data.a1 = 0x00;
      hid_in_data.a2 = 0x00;
      hid_in_data.a3 = 0x00;
      hid_in_data.a4 = ((unsigned char)ret);
      hid_in_data.a5 = 0x00;
      hid_in_data.a6 = 0x00;
      hid_in_data.a7 = 255 - ((unsigned char)ret);
      hid_in_data.a8 = 0x04;
      hid_in_data.a9 = 0x00;
      hid_in_data.a10 = 0x00;

      //a0.1 = left-box up (dpad up)
      //a0.2 = left-box right (dpad right)
      //a0.3 = left-box down (dpad down)
      //a0.4 = left-box left (dpad left)
      //a0.5 = btn 0 (cross lower)
      //a0.6 = btn 1 (cross left)
      //a0.7 = btn 2 (cross right)
      //a0.8 = btn 3 (cross top)

      //a1.1 = btn 4 (paddle right)
      //a1.2 = btn 5 (paddle left)
      //a1.3 = btn 6 (wheel btn right)
      //a1.4 = btn 7 (wheel btn left)
      //a1.5 = btn 8 (red row - btn2)
      //a1.6 = btn 9 (red row - btn3)
      //a1.7 = btn 10 (red row - btn4)
      //a1.8 = btn 11 (red row - btn1)

      //a2.1 = btn 12 (gearstick 1)
      //a2.2 = btn 13 (gearstick 2)
      //a2.3 = btn 14 (gearstick 3)
      //a2.4 = btn 15 (gearstick 4)
      //a2.5 = btn 16 (gearstick 5)
      //a2.6 = btn 17 (gearstick 6)
      //a2.7 = btn 18 (gearstick 7 - reverse?)
      //a2.8 = nill

      //a3.1 = nill
      //a3.2 = nill
      //a3.3 = nill
      //a3.4 = nill
      //a3.5 = nill
      //a3.6 = nill
      //a3.7 = nill
      //a3.8 = nill

      // Steering wheel
      //a4.1 = axis0
      //a4.2 = axis0
      //a4.3 = axis0
      //a4.4 = axis0
      //a4.5 = axis0
      //a4.6 = axis0
      //a4.7 = axis0
      //a4.8 = axis0

      // Accelerator
      //a5.1 = axis2
      //a5.2 = axis2
      //a5.3 = axis2
      //a5.4 = axis2
      //a5.5 = axis2
      //a5.6 = axis2
      //a5.7 = axis2
      //a5.8 = axis2

      // Brake
      //a6.1 = axis3
      //a6.2 = axis3
      //a6.3 = axis3
      //a6.4 = axis3
      //a6.5 = axis3
      //a6.6 = axis3
      //a6.7 = axis3
      //a6.8 = axis3

      // Clutch
      //a7.1 = axis1
      //a7.2 = axis1
      //a7.3 = axis1
      //a7.4 = axis1
      //a7.5 = axis1
      //a7.6 = axis1
      //a7.7 = axis1
      //a7.8 = axis1

      //a8.1 =
      //a8.2 =
      //a8.3 =
      //a8.4 =
      //a8.5 =
      //a8.6 =
      //a8.7 =
      //a8.8 =

      //a9.1 =
      //a9.2 =
      //a9.3 =
      //a9.4 =
      //a9.5 =
      //a9.6 =
      //a9.7 =
      //a9.8 =

      //a10.1 =
      //a10.2 =
      //a10.3 =
      //a10.4 =
      //a10.5 =
      //a10.6 =
      //a10.7 =
      //a10.8 =


      hid_transmit(&USBD1);

	}

}


