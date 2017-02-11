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
#include "adccfg.h"

static struct usb_hid_in_report_s usb_hid_in_report;
static struct usb_hid_out_report_s usb_hid_out_report;


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

#define PEDAL_AVG 4
#define PAD_FRAC_STEER 0.15
#define PAD_FRAC_BRAKE 0.15

icucnt_t last_period;
unsigned char data_pedal;
unsigned char pedal_val;


uint8_t pedal_fired;
uint8_t pedal_arr[PEDAL_AVG];
uint16_t count = 0;


uint32_t angle_max;
uint32_t angle_min;
uint32_t output_brake;
uint32_t output_angle;
uint32_t output_pedal;
uint8_t  output_handbrake;


static void icuperiodcb(ICUDriver *icup) {
  double tmp;
  palSetPad(GPIOD, 13);
  last_period = icuGetPeriod(icup);
  tmp = (double)(1.0 / (double)(last_period / 10000.0)); //freq
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


void refresh_outputs() {
  double tmp;
  if (pedal_fired) pedal_arr[count % PEDAL_AVG] = pedal_val;
  else pedal_arr[count % PEDAL_AVG] = 0;
  pedal_fired = 0;
  tmp = 0;
  for (int i=0; i< PEDAL_AVG; i++) {
    tmp = tmp+pedal_arr[i];
  }
  tmp = tmp / (double)PEDAL_AVG;
  tmp = tmp / 3.5;
  tmp = tmp * tmp;
  if (tmp > 255) tmp = 255;
  if (tmp < 0) tmp = 0;
  output_pedal = (uint8_t)tmp;
}

/*
 * Button thread
 *
 * This thread regularely checks the value of the wkup
 * pushbutton. When its state changes, the thread sends a IN report.
 */
static WORKING_AREA (buttonThreadWorkingArea, 128);

static uint8_t wkup_old_state, wkup_cur_state;

static msg_t buttonThread (void __attribute__ ((__unused__)) * arg) {


  while (1) {

    // palClearPad(GPIOD, 13);

    chSysLock ();

    if (usbGetDriverStateI (&USBD1) == USB_ACTIVE) {
      chSysUnlock ();

      // Build the IN report to be sent
      usb_build_in_report (&usb_hid_in_report);

      // Send the IN report
      usb_send_hid_report (&usb_hid_in_report);
    }
    else chSysUnlock ();

    // ++count;

    chThdSleepMilliseconds (50);
  }
  return 0;
}

int main(void) {
	halInit();
	chSysInit();

  palSetPadMode(GPIOD, 13, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPadMode(GPIOD, 14, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPadMode(GPIOD, 15, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPadMode(GPIOE, 7, PAL_MODE_INPUT_PULLUP);

  init_usb_queues();

  init_usb_driver();

  palSetPad(GPIOD, 13);




  // Wait until the USB is active
  while (USBD1.state != USB_ACTIVE) chThdSleepMilliseconds (1000);



  for (int i=0; i< PEDAL_AVG; i++) {
    pedal_arr[i] = 0;
  }
  pedal_fired = 0;
  angle_max = 0;
  angle_min = 10000000;

  chThdSleepMilliseconds (500);


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

  // double ret;
  // double tmp;
  // uint8_t handbrake;

  // for (int i=0; i< PEDAL_AVG; i++) {
  //   pedal_arr[i] = 0;
  // }
  // pedal_fired = 0;

  // angle_max = 0;
  // angle_min = 10000000;


  // Indicate we're ready to run
  palSetPad(GPIOD, 15);




  // Start the button thread
  chThdCreateStatic (buttonThreadWorkingArea,
         sizeof (buttonThreadWorkingArea),
         NORMALPRIO, buttonThread, NULL);



	while (TRUE) {

    if (palReadPad(GPIOE, 7)) {
      output_handbrake = 0x04;
      palSetPad(GPIOD, 15);
    }
    else {
      output_handbrake = 0;
      palClearPad(GPIOD, 15);
    }

    refresh_outputs();

    // if (chIQReadTimeout (&usb_input_queue, (uint8_t *) & usb_hid_out_report, USB_HID_OUT_REPORT_SIZE, TIME_INFINITE) == USB_HID_OUT_REPORT_SIZE) {
    //   palSetPad(GPIOD, 13);
    //   // Not really sure what to do in here. I don't care what comes back from the host.
    // }
	}

}


