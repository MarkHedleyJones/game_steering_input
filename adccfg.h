/*
    Logitech G25 controller on STM32F407
    Copyright (C) 2017, markjones112358.

    EMAIL: markjones112358@gmail.com

    This piece of code is FREE SOFTWARE and is released
    under the Apache License, Version 2.0 (the "License");
*/

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


#ifndef _ADCCFG_H_
#define _ADCCFG_H_

extern uint32_t angle_max;
extern uint32_t angle_min;
extern uint32_t output_brake;
extern uint32_t output_angle;
extern uint32_t output_pedal;
extern uint8_t  output_handbrake;

// extern uint32_t dataX;
// extern uint32_t dataY;
// extern uint32_t output_brake;
// extern uint32_t output_angle;
extern void myADCinit(void);
uint8_t scale_steering_angle(uint32_t angle);
uint8_t scale_brake_effort(uint32_t brake);
#endif  /* _ADCCFG_H_ */

/** @} */
