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

#include "ch.h"
#include "hal.h"

#include "usb_hid.h"


// The USB driver to use
#define USB_DRIVER USBD1

// HID specific constants
#define USB_DESCRIPTOR_HID 0x21
#define USB_DESCRIPTOR_HID_REPORT 0x22
#define HID_GET_REPORT 0x01
#define HID_SET_REPORT 0x09

// Endpoints
#define USBD1_IN_EP  1
#define USBD1_OUT_EP 1

InputQueue usb_input_queue;
static uint8_t usb_input_queue_buffer[USB_INPUT_QUEUE_BUFFER_SIZE];

OutputQueue usb_output_queue;
static uint8_t usb_output_queue_buffer[USB_OUTPUT_QUEUE_BUFFER_SIZE];

static uint8_t in_report_sequence_number = 0;

// IN EP1 state
static USBInEndpointState ep1instate;
// OUT EP1 state
static USBOutEndpointState ep1outstate;

// USB Device Descriptor
static const uint8_t usb_device_descriptor_data[] = {
  USB_DESC_DEVICE (0x0200,  // bcdUSB (1.1)
           0x00,    // bDeviceClass (defined in later in interface)
           0x00,    // bDeviceSubClass
           0x00,    // bDeviceProtocol
           0x40,    // bMaxPacketSize (64 bytes)
           0x046D,  // idVendor (ST)
           0xC299,  // idProduct (STM32)
           0x1222,  // bcdDevice
           0x00,       // iManufacturer
           0x02,       // iProduct
           0x00,       // iSerialNumber
           0x01)       // bNumConfigurations
};

// Device Descriptor wrapper
static const USBDescriptor usb_device_descriptor = {
  sizeof usb_device_descriptor_data,
  usb_device_descriptor_data
};

/*
 * Configuration Descriptor tree for a HID device
 *
 * The HID Specifications version 1.11 require the following order:
 * - Configuration Descriptor
 * - Interface Descriptor
 * - HID Descriptor
 * - Endpoints Descriptors
 */
#define HID_DESCRIPTOR_OFFSET 18
#define HID_DESCRIPTOR_SIZE 9

static const uint8_t hid_configuration_descriptor_data[] = {
  0x09, 0x02, 0x29, 0x00, 0x01, 0x01, 0x00, 0x80, 0x28, 0x09, 0x04, 0x00, 0x00, 0x02, 0x03, 0x00,
  0x00, 0xfe, 0x09, 0x21, 0x11, 0x01, 0x21, 0x01, 0x22, 0x6f, 0x00, 0x07, 0x05, 0x81, 0x03, 0x10,
  0x00, 0x02, 0x07, 0x05, 0x01, 0x03, 0x10, 0x00, 0x02
};

// Configuration Descriptor wrapper
static const USBDescriptor hid_configuration_descriptor = {
  sizeof hid_configuration_descriptor_data,
  hid_configuration_descriptor_data
};

// HID descriptor wrapper
static const USBDescriptor hid_descriptor = {
  HID_DESCRIPTOR_SIZE,
  &hid_configuration_descriptor_data[HID_DESCRIPTOR_OFFSET]
};

/*
 * HID Report Descriptor
 *
 * This is the description of the format and the content of the
 * different IN or/and OUT reports that your application can
 * receive/send
 *
 * See "Device Class Definition for Human Interface Devices (HID)"
 * (http://www.usb.org/developers/hidpage/HID1_11.pdf) for the
 * detailed descrition of all the fields
 */
static const uint8_t hid_report_descriptor_data[] = {
  0x05, 0x01, 0x09, 0x04, 0xa1, 0x01, 0x15, 0x00, 0x25, 0x07, 0x35, 0x00, 0x46, 0x3b, 0x01, 0x65,
  0x14, 0x09, 0x39, 0x75, 0x04, 0x95, 0x01, 0x81, 0x42, 0x65, 0x00, 0x25, 0x01, 0x45, 0x01, 0x05,
  0x09, 0x19, 0x01, 0x29, 0x13, 0x75, 0x01, 0x95, 0x13, 0x81, 0x02, 0x06, 0x00, 0xff, 0x09, 0x01,
  0x95, 0x03, 0x81, 0x02, 0x26, 0xff, 0x3f, 0x46, 0xff, 0x3f, 0x75, 0x0e, 0x95, 0x01, 0x05, 0x01,
  0x09, 0x30, 0x81, 0x02, 0x26, 0xff, 0x00, 0x46, 0xff, 0x00, 0x75, 0x08, 0x95, 0x03, 0x09, 0x32,
  0x09, 0x35, 0x09, 0x31, 0x81, 0x02, 0x06, 0x00, 0xff, 0x09, 0x04, 0x95, 0x03, 0x81, 0x02, 0x95,
  0x07, 0x06, 0x00, 0xff, 0x09, 0x02, 0x91, 0x02, 0x95, 0x90, 0x09, 0x03, 0xb1, 0x02, 0xc0,
};

// HID report descriptor wrapper
static const USBDescriptor hid_report_descriptor = {
  sizeof hid_report_descriptor_data,
  hid_report_descriptor_data
};

// U.S. English language identifier
static const uint8_t usb_string_langid[] = {
  USB_DESC_BYTE (4),        // bLength
  USB_DESC_BYTE (USB_DESCRIPTOR_STRING),    // bDescriptorType
  USB_DESC_WORD (0x0409)    // wLANGID (U.S. English)
};

// Vendor string
static const uint8_t usb_string_vendor[] = {
  USB_DESC_BYTE (38),       // bLength
  USB_DESC_BYTE (USB_DESCRIPTOR_STRING),    // bDescriptorType
  'S', 0, 'T', 0, 'M', 0, 'i', 0, 'c', 0, 'r', 0, 'o', 0, 'e', 0,
  'l', 0, 'e', 0, 'c', 0, 't', 0, 'r', 0, 'o', 0, 'n', 0, 'i', 0,
  'c', 0, 's', 0
};

// Device Description string
static const uint8_t usb_string_description[] = {
  USB_DESC_BYTE(34),                    /* bLength.                         */
  USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType.                 */
  'G', 0, '2', 0, '5', 0, ' ', 0, 'R', 0, 'a', 0, 'c', 0, 'i', 0,
  'n', 0, 'g', 0, ' ', 0, 'W', 0, 'h', 0, 'e', 0, 'e', 0, 'l', 0
};

static const uint8_t usb_string_serial[] = {
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

// Serial Number string (will be filled by the function init_usb_serial_string)
static uint8_t usb_string_other[] = {
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

// Strings wrappers array
static const USBDescriptor usb_strings[] = {
  {sizeof usb_string_langid, usb_string_langid} ,
  {sizeof usb_string_vendor, usb_string_vendor} ,
  {sizeof usb_string_description, usb_string_description} ,
  {sizeof usb_string_serial, usb_string_serial},
  {sizeof usb_string_other, usb_string_other}
};

/*
 * Handles the GET_DESCRIPTOR callback
 *
 * Returns the proper descriptor
 */
static const USBDescriptor *
usb_get_descriptor_cb (USBDriver __attribute__ ((__unused__)) * usbp,
               uint8_t dtype,
               uint8_t dindex,
               uint16_t __attribute__ ((__unused__)) lang)
{

  switch (dtype)
    {
      // Generic descriptors
    case USB_DESCRIPTOR_DEVICE: // Device Descriptor
      return &usb_device_descriptor;
    case USB_DESCRIPTOR_CONFIGURATION:  // Configuration Descriptor
      return &hid_configuration_descriptor;
    case USB_DESCRIPTOR_STRING: // Strings
      return &usb_strings[dindex % 5];
      break;

      // HID specific descriptors
    case USB_DESCRIPTOR_HID:    // HID Descriptor
      return &hid_descriptor;
    case USB_DESCRIPTOR_HID_REPORT: // HID Report Descriptor
      return &hid_report_descriptor;
    }

  return NULL;
}

/*
 * EP1 IN callback handler
 *
 * Data (IN report) have just been sent to the PC. Check if there are
 * remaining reports to be sent in the output queue and in this case,
 * schedule the transmission
 */
static void
ep1in_cb (USBDriver __attribute__ ((__unused__)) * usbp,
      usbep_t __attribute__ ((__unused__)) ep)
{
  chSysLockFromIsr ();

  // Check if there is data to send in the output queue
  if (chOQGetFullI (&usb_output_queue) >= USB_HID_IN_REPORT_SIZE)
    {
      chSysUnlockFromIsr ();
      // Prepare the transmission
      usbPrepareQueuedTransmit (&USB_DRIVER, USBD1_IN_EP, &usb_output_queue,
                USB_HID_IN_REPORT_SIZE);
      chSysLockFromIsr ();
      usbStartTransmitI (&USB_DRIVER, USBD1_IN_EP);
    }

  chSysUnlockFromIsr ();
}

/*
 * EP1 OUT callback handler
 *
 * Data (OUT report) have just been received. Check if the input queue
 * is not full and in this case, prepare the reception of the next OUT
 * report.
 */
static void
ep1out_cb (USBDriver __attribute__ ((__unused__)) * usbp,
       usbep_t __attribute__ ((__unused__)) ep)
{
  chSysLockFromIsr ();

  // Check if there is still some space left in the input queue
  if (chIQGetEmptyI (&usb_input_queue) >= USB_HID_OUT_REPORT_SIZE)
    {
      chSysUnlockFromIsr ();
      // Prepares the reception of new data
      usbPrepareQueuedReceive (&USB_DRIVER, USBD1_OUT_EP, &usb_input_queue,
                   USB_HID_OUT_REPORT_SIZE);
      chSysLockFromIsr ();
      usbStartReceiveI (&USB_DRIVER, USBD1_OUT_EP);
    }

  chSysUnlockFromIsr ();
}

// EP1 initialization structure (both IN and OUT)
static const USBEndpointConfig ep1config = {
  USB_EP_MODE_TYPE_INTR,    // Interrupt EP
  NULL,             // SETUP packet notification callback
  ep1in_cb,         // IN notification callback
  ep1out_cb,            // OUT notification callback
  0x0040,           // IN maximum packet size
  0x0040,           // OUT maximum packet size
  &ep1instate,          // IN Endpoint state
  &ep1outstate,         // OUT endpoint state
  2,                // IN multiplier
  NULL              // SETUP buffer (not a SETUP endpoint)
};

// Handles the USB driver global events
static void
usb_event_cb (USBDriver * usbp, usbevent_t event)
{

  switch (event)
    {
    case USB_EVENT_RESET:
      return;
    case USB_EVENT_ADDRESS:
      return;
    case USB_EVENT_CONFIGURED:
      chSysLockFromIsr ();

      // Enable the endpoints specified into the configuration.
      usbInitEndpointI (usbp, USBD1_IN_EP, &ep1config);

      // Start the reception immediately
      chIQResetI (&usb_input_queue);
      usbPrepareQueuedReceive (&USB_DRIVER, USBD1_OUT_EP, &usb_input_queue,
                   USB_HID_OUT_REPORT_SIZE);
      usbStartReceiveI (&USB_DRIVER, USBD1_OUT_EP);

      chSysUnlockFromIsr ();
      return;
    case USB_EVENT_SUSPEND:
      return;
    case USB_EVENT_WAKEUP:
      return;
    case USB_EVENT_STALLED:
      return;
    }
}

// Callback for SETUP request on the endpoint 0 (control)
static bool_t
usb_request_hook_cb (USBDriver * usbp)
{
  const USBDescriptor *dp;

  // Handle HID class specific requests
  // Only GetReport is mandatory for HID devices
  if ((usbp->setup[0] & USB_RTYPE_TYPE_MASK) == USB_RTYPE_TYPE_CLASS)
    {
      if (usbp->setup[1] == HID_GET_REPORT)
    {
      /* setup[3] (MSB of wValue) = Report ID (must be 0 as we
       * have declared only one IN report)
       * setup[2] (LSB of wValue) = Report Type (1 = Input, 3 = Feature)
       */
      if ((usbp->setup[3] == 0) && (usbp->setup[2] == 1))
        {
          struct usb_hid_in_report_s in_report;
          usb_build_in_report (&in_report);
          usbSetupTransfer (usbp, (uint8_t *) & in_report,
                USB_HID_IN_REPORT_SIZE, NULL);
        }
    }
      if (usbp->setup[1] == HID_SET_REPORT)
    {
      // Not implemented (yet)
    }
    }

  // Handle the Get_Descriptor Request for HID class (not handled by the default hook)
  if ((usbp->setup[0] == 0x81) && (usbp->setup[1] == USB_REQ_GET_DESCRIPTOR))
    {
      dp =
    usbp->config->get_descriptor_cb (usbp, usbp->setup[3], usbp->setup[2],
                     usbFetchWord (&usbp->setup[4]));
      if (dp == NULL)
    return FALSE;

      usbSetupTransfer (usbp, (uint8_t *) dp->ud_string, dp->ud_size, NULL);
      return TRUE;
    }

  return FALSE;
}

// USB driver configuration
static const USBConfig usbcfg = {
  usb_event_cb,         // USB events callback
  usb_get_descriptor_cb,    // Device GET_DESCRIPTOR request callback
  usb_request_hook_cb,      // Requests hook callback
  NULL              // Start Of Frame callback
};

/*
 * Notification of data removed from the input queue
 *
 * If there is sufficient space in the input queue to receive a new
 * OUT report from the PC, prepare the reception of the next OUT
 * report
 */
static void
usb_input_queue_inotify (GenericQueue __attribute__ ((__unused__)) * qp)
{
  if (usbGetDriverStateI (&USB_DRIVER) != USB_ACTIVE)
    return;

  if (chIQGetEmptyI (&usb_input_queue) >= USB_HID_OUT_REPORT_SIZE)
    {
      chSysUnlock ();
      usbPrepareQueuedReceive (&USB_DRIVER, USBD1_OUT_EP, &usb_input_queue,
                   USB_HID_OUT_REPORT_SIZE);

      chSysLock ();
      usbStartReceiveI (&USB_DRIVER, USBD1_OUT_EP);
    }
}


/*
 * Notification of data inserted into the output queue
 *
 * If the transmission is not active, prepare the transmission.
 */
static void
usb_output_queue_onotify (GenericQueue __attribute__ ((__unused__)) * qp)
{
  if (usbGetDriverStateI (&USB_DRIVER) != USB_ACTIVE)
    return;

  if (!usbGetTransmitStatusI (&USB_DRIVER, USBD1_IN_EP)
      && (chOQGetFullI (&usb_output_queue) >= USB_HID_IN_REPORT_SIZE))
    {
      chSysUnlock ();

      usbPrepareQueuedTransmit (&USB_DRIVER, USBD1_IN_EP, &usb_output_queue,
                USB_HID_IN_REPORT_SIZE);

      chSysLock ();
      usbStartTransmitI (&USB_DRIVER, USBD1_IN_EP);
    }
}


/*
 * Initialize the USB serial number from the STM32F4xx device
 * electronic signature
 */
void
init_usb_serial_string (void)
{
  const uint8_t *ptr = (const uint8_t *) 0x1FFF7A1B;
  uint8_t *str = &(usb_string_serial[2]);

  uint8_t val, tmp;

  do
    {
      val = *ptr--;

      // Most significant nibble
      tmp = (val & 0xF0U) >> 4;

      *str = (tmp < 10) ? ('0' + tmp) : ('A' + (tmp - 10));

      str += 2;         // Skip the UTF-16 second byte

      // Least significant nibble
      tmp = val & 0x0FU;

      *str = (tmp < 10) ? ('0' + tmp) : ('A' + (tmp - 10));

      str += 2;         // Skip the UTF-16 second byte

    }
  while (ptr >= (const uint8_t *) 0x1FFF7A10);
}


/*
 * Initialize the USB input and output queues
 */
void
init_usb_queues (void)
{
  chIQInit (&usb_input_queue, usb_input_queue_buffer,
        sizeof (usb_input_queue_buffer), usb_input_queue_inotify, NULL);

  chOQInit (&usb_output_queue, usb_output_queue_buffer,
        sizeof (usb_output_queue_buffer), usb_output_queue_onotify, NULL);
}


/*
 * Initialize the USB driver
 */
void
init_usb_driver (void)
{
  /*
   * Activates the USB driver and then the USB bus pull-up on D+.
   * Note, a delay is inserted in order to not have to disconnect the cable
   * after a reset.
   */
  usbDisconnectBus (&USB_DRIVER);
  chThdSleepMilliseconds (1500);
  usbStart (&USB_DRIVER, &usbcfg);
  usbConnectBus (&USB_DRIVER);
  return;
}

/*
 * Queue a IN report to be sent
 */
int
usb_send_hid_report (struct usb_hid_in_report_s *report)
{
  int res;

  chSysLock ();
  if (usbGetDriverStateI (&USB_DRIVER) != USB_ACTIVE)
    {
      chSysUnlock ();
      return 0;
    }

  res = chOQGetEmptyI (&usb_output_queue);
  chSysUnlock ();

  if (res > USB_HID_IN_REPORT_SIZE)
    {
      chOQWriteTimeout (&usb_output_queue, (uint8_t *) report,
            USB_HID_IN_REPORT_SIZE, TIME_INFINITE);

      // TODO : check error condition

      return 1;
    }
  else
    return 0;
}

/*
 * Prepare an IN report
 */
void usb_build_in_report (struct usb_hid_in_report_s *report) {
  report->a0 = 0x80;
  report->a1 = output_handb;
  report->a2 = 0x00;
  report->a3 = output_steer;
  // report->a4 = output_steer;
  report->a4 = output_pedal;
  report->a5 = output_brake;
  report->a6 = 0xFF;
  report->a7 = 0x79;
  report->a8 = 0x6C;
  report->a9 = 0x13;

  if (output_steer > 128) palSetPad(GPIOD, 15);
  else palClearPad(GPIOD, 15);
}
