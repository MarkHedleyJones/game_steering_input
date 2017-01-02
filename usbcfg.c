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
#include "usb_hid.h"
#define USB_DESCRIPTOR_REPORT 0x22
/*===========================================================================*/
/* USB related stuff.                                                        */
/*===========================================================================*/

/* For a better understanding look at http://www.beyondlogic.org/usbnutshell/usb1.shtml */
/* Reference: [DCDHID] USB - Device Class Definition for Human Interface Devices (HID)  */
/*
 * USB Device Descriptor.
 */

static const uint8_t hid_device_descriptor_data[] = {
  USB_DESC_DEVICE       (0x0200,        /* bcdUSB (1.1).                    						*/
                         0x00,          /* bDeviceClass (in interface).     						*/
                         0x00,          /* bDeviceSubClass.                 						*/
                         0x00,          /* bDeviceProtocol.                 						*/
                         0x40,          /* bMaxPacketSize. Maximum Packet Size for Zero Endpoint (8,16,32,64) 		*/
                         0x046d,        /* idVendor (ST). Assigned by USB.org 						*/
                         0xc299,        /* idProduct. Assigned by the manufacture 					*/
                         0x1222,        /* bcdDevice. Device Version Number assigned by the developer 			*/
                         0x00,             /* iManufacturer.                   						*/
                         0x02,             /* iProduct.                        						*/
                         0x00,             /* iSerialNumber.                   						*/
                         0x01)             /* bNumConfigurations. The system has only one configuration             	*/
};

/*
 * Device Descriptor wrapper.
 */

static const USBDescriptor hid_device_descriptor = {
  sizeof hid_device_descriptor_data,
  hid_device_descriptor_data
};


/*
 * USB Configuration Descriptor.
 */
static const uint8_t hid_generic_joystick_reporter_data[] ={
// HID_USAGE_PAGE	(HID_USAGE_PAGE_GENERIC_DESKTOP),
// HID_USAGE		(HID_USAGE_JOYSTICK),
// HID_COLLECTION		(HID_COLLECTION_APPLICATION),
//   	HID_COLLECTION		(HID_COLLECTION_PHYSICAL),
//   	    HID_USAGE_PAGE	(HID_USAGE_PAGE_GENERIC_DESKTOP),
//   	    HID_USAGE		(HID_USAGE_X),
//   		HID_USAGE		(HID_USAGE_Y),
// 		HID_LOGICAL_MINIMUM     (-127),
// 		HID_LOGICAL_MAXIMUM	    (127),
// 		HID_REPORT_SIZE		(8),
// 		HID_REPORT_COUNT	(2),
// 		HID_INPUT	(HID_INPUT_DATA_VAR_ABS),
//         HID_USAGE_PAGE(HID_USAGE_PAGE_BUTTON),
// 	    HID_USAGE_MINIMUM(HID_USAGE_BUTTON1),
//   	    HID_USAGE_MAXIMUM(HID_USAGE_BUTTON8),
// 	    HID_LOGICAL_MINIMUM     (0),
// 	    HID_LOGICAL_MAXIMUM	    (1),
// 	    HID_REPORT_SIZE		(1),
// 	    HID_REPORT_COUNT	(8),
// 	    HID_INPUT	(HID_INPUT_DATA_VAR_ABS),
// 	HID_END_COLLECTION ,
// HID_END_COLLECTION,
// Copied from the Logitech G25 HID Report
  0x05, 0x01, 0x09, 0x04, 0xa1, 0x01, 0x15, 0x00, 0x25, 0x07, 0x35, 0x00, 0x46, 0x3b, 0x01, 0x65,
  0x14, 0x09, 0x39, 0x75, 0x04, 0x95, 0x01, 0x81, 0x42, 0x65, 0x00, 0x25, 0x01, 0x45, 0x01, 0x05,
  0x09, 0x19, 0x01, 0x29, 0x13, 0x75, 0x01, 0x95, 0x13, 0x81, 0x02, 0x06, 0x00, 0xff, 0x09, 0x01,
  0x95, 0x03, 0x81, 0x02, 0x26, 0xff, 0x3f, 0x46, 0xff, 0x3f, 0x75, 0x0e, 0x95, 0x01, 0x05, 0x01,
  0x09, 0x30, 0x81, 0x02, 0x26, 0xff, 0x00, 0x46, 0xff, 0x00, 0x75, 0x08, 0x95, 0x03, 0x09, 0x32,
  0x09, 0x35, 0x09, 0x31, 0x81, 0x02, 0x06, 0x00, 0xff, 0x09, 0x04, 0x95, 0x03, 0x81, 0x02, 0x95,
  0x07, 0x06, 0x00, 0xff, 0x09, 0x02, 0x91, 0x02, 0x95, 0x90, 0x09, 0x03, 0xb1, 0x02, 0xc0,
};
static const USBDescriptor hid_generic_joystick_reporter = {
  sizeof hid_generic_joystick_reporter_data,
  hid_generic_joystick_reporter_data
};

static const uint8_t hid_configuration_descriptor_data[] = {
  0x09, 0x02, 0x29, 0x00, 0x01, 0x01, 0x00, 0x80, 0x28, 0x09, 0x04, 0x00, 0x00, 0x02, 0x03, 0x00,
  0x00, 0xfe, 0x09, 0x21, 0x11, 0x01, 0x21, 0x01, 0x22, 0x6f, 0x00, 0x07, 0x05, 0x81, 0x03, 0x40,
  0x00, 0x02, 0x07, 0x05, 0x01, 0x03, 0x40, 0x00, 0x02

  // /* Configuration Descriptor.*/
  // USB_DESC_CONFIGURATION(0x29,          /* wTotalLength.                    */
  //                        0x01,          /* bNumInterfaces.                  */
  //                        0x01,          /* bConfigurationValue.             */
  //                        0x00,          /* iConfiguration.                  */
  //                        0x80,          /* bmAttributes (self powered).     */
  //                        0x28),         /* bMaxPower (50mA).                */
  // /* Interface Descriptor.*/
  // USB_DESC_INTERFACE    (0x00,          /* bInterfaceNumber.                */
  //                        0x00,          /* bAlternateSetting.               */
  //                        0x02,          /* bNumEndpoints.                   */
  //                        0x03,          /* bInterfaceClass (Human Device Interface).   */
  //                        0x00,          /* bInterfaceSubClass  (DCDHID page 8)         */
  //                        0x00,          /* bInterfaceProtocol  (DCDHID page 9)         */
  //                        0xfe),            /* iInterface.                                 */

  // // Copied from G25 (USB HID Descriptor)
  // 0x09, 0x21, 0x11, 0x01, 0x21, 0x01, 0x22, 0x6f, 0x00,
  // // USB_DESC_HID		    (0x0111,	/* bcdHID 		*/
		// // 	             0x00,		/* bCountryCode 	*/
		// // 	             0x01,		/* bNumDescriptor 	*/
		// // 	             0x22,		/* bDescriptorType	*/
		// // 	             sizeof(hid_generic_joystick_reporter_data)),		/* Report Descriptor Lenght. Compute it! */
  // /* Endpoint 1 Descriptor INTERRUPT IN  */
  // USB_DESC_ENDPOINT     (0x81,   	/* bEndpointAddress.   (0x80 = Direction IN) + (0x01 = Address 1)     */
  //                        0x03,          /* bmAttributes (Interrupt).             		 	      */
  //                        0x10,          /* wMaxPacketSize.     0x40 = 64 BYTES                               */
  //                        0x02),         /* bInterval (Polling every 50ms)                                     */
  // /* Endpoint 1 Descriptor INTERRUPT OUT */
  // USB_DESC_ENDPOINT     (0x01,       	/* bEndpointAddress.   (0x00 = Direction OUT) + (0x0q = Address 1)    */
  //                        0x03,          /* bmAttributes (Interrupt).             */
  //                        0x10,          /* wMaxPacketSize.     0x40 = 64 BYTES  */
  //                        0x02),         /* bInterval (Polling every 50ms)        */
  //                         /* HID Report Descriptor */
  //                         /* Specific Class HID Descriptor */


  // // was commented from here


  //   /* Interface Descriptor.*/
  // USB_DESC_INTERFACE    (0x01,          /* bInterfaceNumber.                */
  //                        0x00,          /* bAlternateSetting.               */
  //                        0x04,          /* bNumEndpoints.                   */
  //                        0xff,          /* bInterfaceClass (Human Device Interface).   */
  //                        0x5d,          /* bInterfaceSubClass  (DCDHID page 8)         */
  //                        0x03,          /* bInterfaceProtocol  (DCDHID page 9)         */
  //                        0x00),            /* iInterface.                                 */

  // // USB_DESC_HID        (0x0111,  /* bcdHID     */
  // //                  0x00,    /* bCountryCode   */
  // //                  0x01,    /* bNumDescriptor   */
  // //                  0x22,    /* bDescriptorType  */
  // //                  sizeof(hid_generic_joystick_reporter_data)),   /* Report Descriptor Lenght. Compute it! */
  // // Copied from Xbox controller
  // 0x1b, 0x21, 0x00, 0x01, 0x01, 0x01, 0x82, 0x40, 0x01, 0x02, 0x20, 0x16, 0x83, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  // /* Endpoint 2 Descriptor INTERRUPT IN  */
  // USB_DESC_ENDPOINT     (0x82,    /* bEndpointAddress.   (0x80 = Direction IN) + (0x01 = Address 1)     */
  //                        0x03,          /* bmAttributes (Interrupt).                        */
  //                        0x20,          /* wMaxPacketSize.     0x40 = 64 BYTES                               */
  //                        0x02),         /* bInterval (Polling every 50ms)                                     */
  // /* Endpoint 2 Descriptor INTERRUPT OUT */
  // USB_DESC_ENDPOINT     (0x02,        /* bEndpointAddress.   (0x00 = Direction OUT) + (0x0q = Address 1)    */
  //                        0x03,          /* bmAttributes (Interrupt).             */
  //                        0x20,          /* wMaxPacketSize.     0x40 = 64 BYTES  */
  //                        0x04),         /* bInterval (Polling every 50ms)        */
  //                         /* HID Report Descriptor */
  //                         /* Specific Class HID Descriptor */

  // /* Endpoint 3 Descriptor INTERRUPT IN  */
  // USB_DESC_ENDPOINT     (0x83,    /* bEndpointAddress.   (0x80 = Direction IN) + (0x01 = Address 1)     */
  //                        0x03,          /* bmAttributes (Interrupt).                        */
  //                        0x20,          /* wMaxPacketSize.     0x40 = 64 BYTES                               */
  //                        0x40),         /* bInterval (Polling every 50ms)                                     */
  // /* Endpoint 3 Descriptor INTERRUPT OUT */
  // USB_DESC_ENDPOINT     (0x03,        /* bEndpointAddress.   (0x00 = Direction OUT) + (0x0q = Address 1)    */
  //                        0x03,          /* bmAttributes (Interrupt).             */
  //                        0x20,          /* wMaxPacketSize.     0x40 = 64 BYTES  */
  //                        0x10),         /* bInterval (Polling every 50ms)        */
  //                         /* HID Report Descriptor */
  //                         /* Specific Class HID Descriptor */








  //     /* Interface Descriptor.*/
  // USB_DESC_INTERFACE    (0x02,          /* bInterfaceNumber.                */
  //                        0x00,          /* bAlternateSetting.               */
  //                        0x01,          /* bNumEndpoints.                   */
  //                        0xff,          /* bInterfaceClass (Human Device Interface).   */
  //                        0x5d,          /* bInterfaceSubClass  (DCDHID page 8)         */
  //                        0x02,          /* bInterfaceProtocol  (DCDHID page 9)         */
  //                        0x00),            /* iInterface.                                 */

  // USB_DESC_HID        (0x0111,  /* bcdHID     */
  //                  0x00,    /* bCountryCode   */
  //                  0x01,    /* bNumDescriptor   */
  //                  0x22,    /* bDescriptorType  */
  //                  sizeof(hid_generic_joystick_reporter_data)),   /* Report Descriptor Lenght. Compute it! */
  // /* Endpoint 2 Descriptor INTERRUPT IN  */
  // USB_DESC_ENDPOINT     (0x84,    /* bEndpointAddress.   (0x80 = Direction IN) + (0x01 = Address 1)     */
  //                        0x03,          /* bmAttributes (Interrupt).                        */
  //                        0x20,          /* wMaxPacketSize.     0x40 = 64 BYTES                               */
  //                        0x10),         /* bInterval (Polling every 50ms)                                     */






  //     /* Interface Descriptor.*/
  // USB_DESC_INTERFACE    (0x03,          /* bInterfaceNumber.                */
  //                        0x00,          /* bAlternateSetting.               */
  //                        0x00,          /* bNumEndpoints.                   */
  //                        0xff,          /* bInterfaceClass (Human Device Interface).   */
  //                        0xfd,          /* bInterfaceSubClass  (DCDHID page 8)         */
  //                        0x13,          /* bInterfaceProtocol  (DCDHID page 9)         */
  //                        0x04),            /* iInterface.                                 */

  // 0x06, 0x41, 0x00, 0x01, 0x01, 0x03,

};

/*
 * Configuration Descriptor wrapper.
 */
static const USBDescriptor hid_configuration_descriptor = {
  sizeof hid_configuration_descriptor_data,
  hid_configuration_descriptor_data
};



/*
 * U.S. English language identifier.
 */
static const uint8_t hid_string0[] = {
  USB_DESC_BYTE(4),                     /* bLength.                         */
  USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType.                 */
  USB_DESC_WORD(0x0409)                 /* wLANGID (U.S. English).          */
};

/*
 * Vendor string.
 */
static const uint8_t hid_string1[] = {
  USB_DESC_BYTE(46),                    /* bLength.                         */
  USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType.                 */
  '©', 0, 'M', 0, 'i', 0, 'c', 0, 'r', 0, 'o', 0, 's', 0, 'o', 0,
  'f', 0, 't', 0, ' ', 0, 'C', 0, 'o', 0, 'r', 0, 'p', 0, 'o', 0,
  'r', 0, 'a', 0, 't', 0, 'i', 0, 'o', 0, 'n', 0
};

/*
 * Device Description string.
 */
static const uint8_t hid_string2[] = {
  USB_DESC_BYTE(34),                    /* bLength.                         */
  USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType.                 */
  'G', 0, '2', 0, '5', 0, ' ', 0, 'R', 0, 'a', 0, 'c', 0, 'i', 0,
  'n', 0, 'g', 0, ' ', 0, 'W', 0, 'h', 0, 'e', 0, 'e', 0, 'l', 0
};

/*
 * Serial Number string.
 */
static const uint8_t hid_string3[] = {
  USB_DESC_BYTE(16),                     /* bLength.                         */
  USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType.                 */
  '1', 0,
  '1', 0,
  '7', 0,
  'B', 0,
  '8', 0,
  '5', 0,
  '4', 0,
};

/*
 * Interface string.
 */


static const uint8_t hid_string4[] = {
  USB_DESC_BYTE(96),                    /* bLength.                             */
  USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType.                     */
  //'U', 0, 'S', 0, 'B', 0, ' ', 0, 'H', 0, 'I', 0, 'D', 0
  'N', 0, 'V', 0, '=', 0, '0', 0, '4', 0, '6', 0, 'D', 0, ',', 0,
  'N', 0, 'P', 0, '=', 0, 'C', 0, '2', 0, '9', 0, '9', 0, ',', 0,
  'N', 0, 'D', 0, '=', 0, '1', 0, '2', 0, '2', 0, '2', 0, ',', 0,
  'H', 0, 'V', 0, '=', 0, '0', 0, '4', 0, '6', 0, 'D', 0, ',', 0,
  'H', 0, 'P', 0, '=', 0, 'F', 0, 'E', 0, '0', 0, '1', 0, ',', 0,
  'H', 0, 'D', 0, '=', 0, '0', 0, '0', 0, '0', 0, '5', 0,
};

/*
 * Strings wrappers array.
 */
static const USBDescriptor hid_strings[] = {
  {sizeof hid_string0, hid_string0},
  {sizeof hid_string1, hid_string1},
  {sizeof hid_string2, hid_string2},
  {sizeof hid_string3, hid_string3},
  {sizeof hid_string4, hid_string4}
};

static const USBDescriptor *usb_get_descriptor_cb(USBDriver *usbp, uint8_t dtype, uint8_t dindex, uint16_t lang) {

  (void)usbp;
  (void)lang;
  switch (dtype) {
  case USB_DESCRIPTOR_DEVICE:
    return &hid_device_descriptor;
  case USB_DESCRIPTOR_CONFIGURATION:
    return &hid_configuration_descriptor;
  case USB_DESCRIPTOR_REPORT:
    //palSetPadMode(GPIOD, 12, PAL_MODE_OUTPUT_PUSHPULL);
    //palSetPad(GPIOD, 12);
    return &hid_generic_joystick_reporter;
  case USB_DESCRIPTOR_STRING:
    // if (dindex < 5) return &hid_strings[dindex];
    return &hid_strings[dindex % 5];
  }
  return NULL;
}

static USBInEndpointState ep1instate;
//static USBOutEndpointState ep1outstate;
//	EndPoint Initialization. INTERRUPT IN. Device -> Host
static const USBEndpointConfig ep1config = {
  USB_EP_MODE_TYPE_INTR,
  NULL,
  hidDataTransmitted,
  NULL,
  0x000E,
  0x0000,
  &ep1instate,
  NULL,
  1,
  NULL
};
// static const USBEndpointConfig ep1config = {
// 	USB_EP_MODE_TYPE_INTR,
// 	NULL,
// 	hidDataTransmitted,
// 	NULL,
// 	0x0004,
// 	0x0000,
// 	&ep1instate,
// 	NULL,
// 	1,
// 	NULL
// };

static USBOutEndpointState ep2outstate;
//static USBOutEndpointState ep1outstate;
//	EndPoint Initialization. INTERRUPT IN. Device -> Host
static const USBEndpointConfig ep2config = {
	USB_EP_MODE_TYPE_INTR,
	NULL,
  NULL,
	hidDataReceived,
	0x0000,
	0x0000,
	NULL,
	&ep2outstate,
	1,
	NULL
};
// static const USBEndpointConfig ep2config = {
//   USB_EP_MODE_TYPE_INTR,
//   NULL,
//     NULL,
//   hidDataReceived,
//   0x0000,
//   0x0004,
//   NULL,
//   &ep2outstate,
//   1,
//   NULL
// };

static void usb_event(USBDriver *usbp, usbevent_t event) {
	switch(event) {
	case USB_EVENT_RESET:
		return;
	case USB_EVENT_ADDRESS:
		return;
	case USB_EVENT_CONFIGURED:
		chSysLockFromIsr();
		usbInitEndpointI(usbp, HID_IN_EP_ADDRESS, &ep1config);
		usbInitEndpointI(usbp, HID_OUT_EP_ADDRESS, &ep2config);
		chSysUnlockFromIsr();
		return;
	case USB_EVENT_SUSPEND:
		return;
	case USB_EVENT_WAKEUP:
		return;
	case USB_EVENT_STALLED:
		return;
	}
	return;
}


const USBConfig usbcfg = {
	usb_event,
	usb_get_descriptor_cb,
	usb_request_hook_cb,
	NULL
};


