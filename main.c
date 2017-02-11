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

uint8_t output_handb;
uint16_t output_steer;
uint8_t output_brake;
uint8_t output_pedal;

uint16_t data_brake;
uint16_t data_angle_arr[STEER_AVG];
uint16_t data_adjust;
uint32_t adc_count;

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

#define PEDAL_AVG 50
// #define STEER_AVG 10
#define PAD_FRAC_STEER 0.15
#define PAD_FRAC_BRAKE 0.15

icucnt_t last_period;
unsigned char data_pedal;
unsigned char pedal_val;
uint8_t pedal_fired;
uint8_t pedal_arr[PEDAL_AVG];
uint16_t steer_arr[STEER_AVG];


uint32_t angle_max;
uint32_t angle_min;


static void icuperiodcb(ICUDriver *icup) {
  double tmp;
  // palSetPad(GPIOD, 13);
  last_period = icuGetPeriod(icup);
  tmp = (double)(1.0 / (double)(last_period / 10000.0)); //freq
  pedal_val = (uint8_t)tmp;
  pedal_fired = TRUE;
}


uint16_t steering_angle(uint16_t angle) {
  if (angle > angle_max) {
    angle_max = angle;
    return 65535;
  }
  if (angle < angle_min) {
    angle_min = angle;
    return 0;
  }

  uint32_t range;
  double tmp;
  uint16_t out;
  range = angle_max - angle_min;
  angle = angle - angle_min;

  // Get ratio of range
  tmp = (double)angle / (double)(range);

  tmp = tmp + ((tmp - 0.5) * PAD_FRAC_STEER);
  if (tmp > 1.0) tmp = 1.0;
  if (tmp < 0.0) tmp = 0.0;

  out = (uint16_t)(65535 * tmp);

  return out;

}

uint8_t brake_effort(uint32_t brake) {
  double tmp;
  tmp = (double)brake / 255;
  tmp = (tmp + ((tmp - 0.5) * PAD_FRAC_BRAKE));
  if (tmp > 1.0) tmp = 1.0;
  if (tmp < 0.0) tmp = 0.0;
  return (uint8_t)(255 * tmp);
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

    chThdSleepMilliseconds (10);
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

  // palSetPad(GPIOD, 14);

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
  // palSetPad(GPIOD, 15);




  // Start the button thread
  chThdCreateStatic (buttonThreadWorkingArea,
         sizeof (buttonThreadWorkingArea),
         NORMALPRIO, buttonThread, NULL);


  double ret;
  double tmp;
  uint8_t handbrake;

  for (int i=0; i< PEDAL_AVG; i++) {
    pedal_arr[i] = 0;
  }
  pedal_fired = 0;

  angle_max = 43000*0.8;
  angle_min = 21500*1.2;


  // Indicate we're ready to run
  handbrake = 0;

  uint32_t count = 0;

  output_steer = 0;
  output_brake = 255;
  output_pedal = 255;

	while (TRUE) {

    // refresh_outputs(count++);
    if (pedal_fired) pedal_arr[count % PEDAL_AVG] = pedal_val;
    else pedal_arr[count % PEDAL_AVG] = 0;

    pedal_fired = 0;
    tmp = 0;

    for (uint8_t i=0; i< PEDAL_AVG; i++) {
      tmp = tmp+pedal_arr[i];
    }
    tmp = tmp / (double)PEDAL_AVG;
    // tmp = tmp / 2;
    tmp = tmp * tmp;
    if (tmp > 255) tmp = 255;
    if (tmp < 0) tmp = 0;
    data_pedal = (uint8_t)tmp;


    if (palReadPad(GPIOE, 7)) handbrake = 4;
    else handbrake = 0;

    chThdSleepMilliseconds(10);


    output_handb = 0x00 | handbrake;
    // output_steer = 65535 - steering_angle(data_angle);

    tmp = 0;
    for (uint8_t i=0; i< STEER_AVG; i++) {
      tmp = tmp+data_angle_arr[i];
    }
    output_steer = (uint16_t)(tmp / (double)STEER_AVG);
    output_steer = 65535 - steering_angle(output_steer);

    // output_steer = data_angle;
    output_brake = 255 - brake_effort(data_brake);
    output_pedal = 255 - data_pedal;

    ++count;


    if (chIQReadTimeout (&usb_input_queue, (uint8_t *) & usb_hid_out_report, USB_HID_OUT_REPORT_SIZE, 1) == USB_HID_OUT_REPORT_SIZE) {
      // if (usb_hid_out_report.a0) palClearPad(GPIOD, 15);
      // else palSetPad(GPIOD, 15);
      // palClearPad(GPIOD, 13);
      // Not really sure what to do in here. I don't care what comes back from the host.
    }
	}

}


