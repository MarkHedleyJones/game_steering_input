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

	while (TRUE) {
		chThdSleepMilliseconds(50);
      ++count;
      // Button 0
      if (count % 13 == 0) hid_in_data.BYTE_BTN_A |= BIT_BTN_A;
      else hid_in_data.BYTE_BTN_A &= ~BIT_BTN_A;

      // Button 1
      if (count % 13 == 1) hid_in_data.BYTE_BTN_B |= BIT_BTN_B;
      else hid_in_data.BYTE_BTN_B &= ~BIT_BTN_B;

      // Button 2
      if (count % 13 == 2) hid_in_data.BYTE_BTN_X |= BIT_BTN_X;
      else hid_in_data.BYTE_BTN_X &= ~BIT_BTN_X;

      // Button 3
      if (count % 13 == 3) hid_in_data.BYTE_BTN_Y |= BIT_BTN_Y;
      else hid_in_data.BYTE_BTN_Y &= ~BIT_BTN_Y;

      // Button 4
      if (count % 13 == 4) hid_in_data.BYTE_BTN_TRIGGERLEFT |= BIT_BTN_TRIGGERLEFT;
      else hid_in_data.BYTE_BTN_TRIGGERLEFT &= ~BIT_BTN_TRIGGERLEFT;

      // Button 5
      if (count % 13 == 5) hid_in_data.BYTE_BTN_TRIGGERRIGHT |= BIT_BTN_TRIGGERRIGHT;
      else hid_in_data.BYTE_BTN_TRIGGERRIGHT &= ~BIT_BTN_TRIGGERRIGHT;

      // Button 6
      if (count % 13 == 6) hid_in_data.BYTE_BACK |= BIT_BACK;
      else hid_in_data.BYTE_BACK &= ~BIT_BACK;

      // Button 7
      if (count % 13 == 7) hid_in_data.BYTE_START |= BIT_START;
      else hid_in_data.BYTE_START &= ~BIT_START;

      // Button 8
      // if (count % 13 == 8) hid_in_data.BYTE_BTN_MODE |= BIT_BTN_MODE;
      // else hid_in_data.BYTE_BTN_MODE &= ~BIT_BTN_MODE;

      // Button 9
      if (count % 13 == 9) hid_in_data.BYTE_BTN_THUMBLEFT |= BIT_BTN_THUMBLEFT;
      else hid_in_data.BYTE_BTN_THUMBLEFT &= ~BIT_BTN_THUMBLEFT;

      // Button 10
      if (count % 13 == 10) hid_in_data.BYTE_BTN_THUMBRIGHT |= BIT_BTN_THUMBRIGHT;
      else hid_in_data.BYTE_BTN_THUMBRIGHT &= ~BIT_BTN_THUMBRIGHT;

      // hid_in_data.a6 = (int16_t) -32639 + (count * 1000);
      // hid_in_data.a7 = (int16_t) -32639 + (count * 1000);

      hid_in_data.a6 = (int16_t) (32767 * sin((double) count / 2.0));
      hid_in_data.a7 = (int16_t) (32767 * sin((double) count / 4.0));
      hid_in_data.a8 = (int16_t) (32767 * sin((double) count / 2.0));
      hid_in_data.a9 = (int16_t) (32767 * sin((double) count / 4.0));
      // hid_in_data.a7 = (int16_t) -32639 + (count * 1000);
      // hid_in_data.a8 = (int16_t) (30000 * sin((double) count / 2.0));
      // hid_in_data.a9 = (int16_t) -32639 + (count * 1000);
      // hid_in_data.a8 = (int16_t) sin((float) count /  2.0) * 100;
      // hid_in_data.a8 = (int16_t) sin((float) count) * 1000;

      hid_in_data.a1 = 0x14;




      // BTN_START(count % 2);

      // else start_depressed(&hid_in_data);

      // if (hid_in_data.a3 == 0x10) {
      //     hid_in_data.a3 = 0x00;
      // }
      // else {
      //     hid_in_data.a3 = 0x10;
      // }

      hid_transmit(&USBD1);

	}

}


