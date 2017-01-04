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


// Main

#define PEDAL_AVG 2

icucnt_t last_period;
unsigned char data_pedal;
unsigned char pedal_val;
uint8_t pedal_fired;
uint8_t pedal_arr[PEDAL_AVG];

static void icuperiodcb(ICUDriver *icup) {
  double tmp;
  palSetPad(GPIOD, 13);
  last_period = icuGetPeriod(icup);
  tmp = (double)(1.0 / (double)(last_period / 10000.0)); //freq
  // data_pedal = (255.0 / (last_period));
  pedal_val = (uint8_t)tmp;
  pedal_fired = TRUE;
}


static ICUConfig icucfg = {
  ICU_INPUT_ACTIVE_HIGH,
  10000,                                    /* 10kHz ICU clock frequency.   */
  NULL,
  icuperiodcb,
  NULL,
  ICU_CHANNEL_1,
  0
};

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

  icuStart(&ICUD3, &icucfg);
  palSetPadMode(GPIOC, 6, PAL_MODE_ALTERNATE(2));
  icuEnable(&ICUD3);
  chThdSleepMilliseconds(200);

	/*
	 * Normal main() thread activity, in this demo it does nothing except
	 * sleeping in a loop and check the button state.
	 */
      /*
   * Setting up analog inputs used by the demo.
   */
  myADCinit();

  palSetGroupMode(GPIOA, PAL_PORT_BIT(1) | PAL_PORT_BIT(2)| PAL_PORT_BIT(3)| PAL_PORT_BIT(4)| PAL_PORT_BIT(5)| PAL_PORT_BIT(6)| PAL_PORT_BIT(7),0, PAL_MODE_INPUT);

	//hid_transmit(&USBD1);

  double ret;
  double tmp;
  for (int i=0; i< PEDAL_AVG; i++) {
    pedal_arr[i] = 0;
  }
  pedal_fired = 0;

  // Indicate we're ready to run
  palSetPad(GPIOD, 15);

	while (TRUE) {

    if (pedal_fired) pedal_arr[count % PEDAL_AVG] = pedal_val;
    else pedal_arr[count % PEDAL_AVG] = 0;
    pedal_fired = 0;
    tmp = 0;
    for (int i=0; i< PEDAL_AVG; i++) {
      tmp = tmp+pedal_arr[i];
    }
    tmp = tmp / (double)PEDAL_AVG;
    tmp = tmp / 4;
    tmp = tmp * tmp;
    if (tmp > 255) tmp = 255;
    if (tmp < 0) tmp = 0;
    data_pedal = (uint8_t)tmp;

		chThdSleepMilliseconds(50);
    ++count;
    ret = (sin(count / 20.0) * 128.0) + 128;
    hid_in_data.a0 = 0x00;
    hid_in_data.a1 = 0x00;
    hid_in_data.a2 = 0x00;
    hid_in_data.a3 = data_brake;
    hid_in_data.a4 = ((unsigned char)ret);
    hid_in_data.a5 = data_pedal;
    hid_in_data.a6 = data_angle;
    hid_in_data.a7 = data_adjust;
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

    // Steering wheel
    //a4 = axis0

    // Accelerator
    //a5 = axis2

    // Brake
    //a6 = axis3

    // Clutch
    //a7 = axis1



      hid_transmit(&USBD1);

	}

}


