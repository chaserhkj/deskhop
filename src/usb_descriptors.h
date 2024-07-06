/* 
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
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
 */

#ifndef USB_DESCRIPTORS_H_
#define USB_DESCRIPTORS_H_

enum
{
  REPORT_ID_KEYBOARD = 1,
  REPORT_ID_MOUSE,
  REPORT_ID_CONSUMER,
#if BOARD_ROLE == PICO_A
  REPORT_ID_GENERIC,
#endif
  REPORT_ID_COUNT,
};

enum
{
  REPORT_ID_RELMOUSE = 1,
};

#define TUD_HID_REPORT_DESC_ABSMOUSE(...) \
HID_USAGE_PAGE ( HID_USAGE_PAGE_DESKTOP      )                   ,\
HID_USAGE      ( HID_USAGE_DESKTOP_MOUSE     )                   ,\
HID_COLLECTION ( HID_COLLECTION_APPLICATION  )                   ,\
  /* Report ID */\
  __VA_ARGS__ \
  HID_USAGE      ( HID_USAGE_DESKTOP_POINTER )                   ,\
  HID_COLLECTION ( HID_COLLECTION_PHYSICAL   )                   ,\
    HID_USAGE_PAGE  ( HID_USAGE_PAGE_BUTTON  )                   ,\
      HID_USAGE_MIN   ( 1                                      ) ,\
      HID_USAGE_MAX   ( 5                                      ) ,\
      HID_LOGICAL_MIN ( 0                                      ) ,\
      HID_LOGICAL_MAX ( 1                                      ) ,\
      \
      /* Left, Right, Middle, Backward, Forward buttons */ \
      HID_REPORT_COUNT( 5                                      ) ,\
      HID_REPORT_SIZE ( 1                                      ) ,\
      HID_INPUT       ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE ) ,\
      \
      /* 3 bit padding */ \
      HID_REPORT_COUNT( 1                                      ) ,\
      HID_REPORT_SIZE ( 3                                      ) ,\
      HID_INPUT       ( HID_CONSTANT                           ) ,\
    HID_USAGE_PAGE  ( HID_USAGE_PAGE_DESKTOP )                   ,\
      \
      /* X, Y absolute position [0, 32767] */ \
      HID_USAGE       ( HID_USAGE_DESKTOP_X                    ) ,\
      HID_USAGE       ( HID_USAGE_DESKTOP_Y                    ) ,\
      HID_LOGICAL_MIN  ( 0x00                                ) ,\
      HID_LOGICAL_MAX_N( 0x7FFF, 2                           ) ,\
      HID_REPORT_SIZE  ( 16                                  ) ,\
      HID_REPORT_COUNT ( 2                                   ) ,\
      HID_INPUT       ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE ) ,\
      \
      /* Vertical wheel scroll [-127, 127] */ \
      HID_USAGE       ( HID_USAGE_DESKTOP_WHEEL                )  ,\
      HID_LOGICAL_MIN ( 0x81                                   )  ,\
      HID_LOGICAL_MAX ( 0x7f                                   )  ,\
      HID_REPORT_COUNT( 1                                      )  ,\
      HID_REPORT_SIZE ( 8                                      )  ,\
      HID_INPUT       ( HID_DATA | HID_VARIABLE | HID_RELATIVE )  ,\
      \
      /* Mouse mode (0 = absolute, 1 = relative) */ \
      HID_REPORT_COUNT( 1                                      ), \
      HID_REPORT_SIZE ( 8                                      ), \
      HID_INPUT       ( HID_CONSTANT                           ), \
  HID_COLLECTION_END                                            , \
HID_COLLECTION_END \

// Consumer Control Report Descriptor Template
#define TUD_HID_REPORT_DESC_CONSUMER_CTRL(...) \
  HID_USAGE_PAGE ( HID_USAGE_PAGE_CONSUMER    )              ,\
  HID_USAGE      ( HID_USAGE_CONSUMER_CONTROL )              ,\
  HID_COLLECTION ( HID_COLLECTION_APPLICATION )              ,\
    /* Report ID if any */\
    __VA_ARGS__ \
    HID_LOGICAL_MIN  ( 0x01                                ) ,\
    HID_LOGICAL_MAX_N( 0x0FFF, 2                           ) ,\
    HID_USAGE_MIN    ( 0x01                                ) ,\
    HID_USAGE_MAX_N  ( 0x0FFF, 2                           ) ,\
    HID_REPORT_SIZE  ( 16                                  ) ,\
    HID_REPORT_COUNT ( 2                                   ) ,\
    HID_INPUT        ( HID_DATA | HID_ARRAY | HID_ABSOLUTE ) ,\
  HID_COLLECTION_END \

#endif /* USB_DESCRIPTORS_H_ */
