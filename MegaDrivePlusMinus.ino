/*******************************************************************************
 * This file is part of MegaDrive++.                                           *
 *                                                                             *
 * Copyright (C) 2015-2016 by SukkoPera <software@sukkology.net>               *
 *                                                                             *
 * MegaDrive++ is free software: you can redistribute it and/or modify         *
 * it under the terms of the GNU General Public License as published by        *
 * the Free Software Foundation, either version 3 of the License, or           *
 * (at your option) any later version.                                         *
 *                                                                             *
 * MegaDrive++ is distributed in the hope that it will be useful,              *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the               *
 * GNU General Public License for more details.                                *
 *                                                                             *
 * You should have received a copy of the GNU General Public License           *
 * along with MegaDrive++. If not, see <http://www.gnu.org/licenses/>.         *
 *******************************************************************************
 *
 * MegaDrive++ - Universal Region mod, 50/60 Hz switch and In-Game-Reset (IGR)
 * for Sega Mega Drive (AKA Genesis)
 *
 * Please refer to the GitHub page and wiki for any information:
 * https://github.com/SukkoPera/MegaDrivePlusPlus
 */

/*******************************************************************************
 * PLATFORM SELECTION
 ******************************************************************************/

// Check if we should disable some features because of low flash space
#if FLASHEND < 2048
    /* We only have 2 kb flash, let's take special measures:
     * - Remove PWM support for leds. This saves us the use of analogWrite(),
     *   which saves enough flash memory.
     * - Don't check if video mode has changed before saving it to EEPROM.
     */
    #warning Low flash space mode enabled
    #define LOW_FLASH
#endif


#if defined __AVR_ATtinyX5__
/*
 * On ATtinyX5's we only support Reset-From-Pad.
 *                  ,-----_-----.
 *                  |1 (5)     8| +5V
 *         Reset In |2   3  2  7| Pad Port Pin 7
 *        Reset Out |3   4  1  6| Pad Port Pin 9
 *              GND |4      0  5| Pad Port Pin 6
 *                  `-----------'
 */
#define RESET_IN_PIN 3
#define RESET_OUT_PIN 4

#elif defined __AVR_ATtinyX4__
/*
 * On ATtinyX4's most features are supported. The only exception is that RIGHT
 * and LEFT cannot be used in combos.
 *
 * The connection layout is derived from that of the Seb/D4s mod, so that if you
 * already have a socket wired properly in you console, you will just need to
 * add a few wires and replace the chip to get the new features. The wires to be
 * added are all those coming from the controller pad port.
 *
 *                  ,-----_-----.
 *              +5V |1        14| GND
 *   Pad Port Pin 1 |2   0 10 13| Reset In
 *   Pad Port Pin 2 |3   1  9 12| Pad Port Pin 6
 *                  |4 (11) 8 11| Pad Port Pin 9
 *          LED Red |5   2  7 10| JP1/2 (Language)
 *        LED Green |6   3  6  9| JP3/4 (Video Mode)
 *   Pad Port Pin 7 |7   4  5  8| Reset Out
 *                  `-----------'
 */
#define RESET_IN_PIN 10
#define RESET_OUT_PIN 5
#define VIDEOMODE_PIN 6
#define LANGUAGE_PIN 7

#define MODE_LED_R_PIN 2
#define MODE_LED_G_PIN 3
// No blue pin!

#elif defined __AVR_ATtinyX61__
/*
 * On ATtinyX61's all features are supported. We even read all buttons with a
 * single instruction.
 *
 * The connection layout puts the SELECT signal on the INT1 pin. This will
 * probably be needed if we ever want to read 6-button pads. LED is connected to
 * PWM-capable pins.
 *
 *                    ,-----_-----.
 *           Reset In |1   9  0 20| Pad Port Pin 1
 *            LED Red |2   8  1 19| Pad Port Pin 2
 *          Reset Out |3   7  2 18| Pad Port Pin 7
 *          LED Green |4   6 14 17| Pad Port Pin 3
 *                +5V |5        16| GND
 *                GND |6        15| +5V
 * JP3/4 (Video Mode) |7   5 10 14| Pad Port Pin 4
 *           LED Blue |8   4 11 13| Pad Port Pin 6
 *   JP1/2 (Language) |9   3 12 12| Pad Port Pin 9
 *                    |10(15)13 11|
 *                    `-----------'
 */
#define RESET_IN_PIN 9
#define RESET_OUT_PIN 7
#define VIDEOMODE_PIN 5
#define LANGUAGE_PIN 3

#define MODE_LED_R_PIN 8
#define MODE_LED_G_PIN 6
#define MODE_LED_B_PIN 4


#elif defined __AVR_ATtinyX313__
/*
 * On ATtinyX13's all features are supported. We even read all buttons with a
 * single instruction.
 *
 * Again, the connection layout puts the SELECT signal on the INT1 pin. LED is
 * connected to PWM-capable pins.
 *
 *                    ,-----_-----.
 *                    |1 (17)   20| +5V
 *     Pad Port Pin 1 |2   0 16 19| JP3/4 (Video Mode)
 *     Pad Port Pin 2 |3   1 15 18| JP1/2 (Language)
 *                    |4   2 14 17| Reset Out
 *                    |5   3 13 16| Reset In
 *     Pad Port Pin 7 |6   4 12 15| LED Blue
 *     Pad Port Pin 3 |7   5 11 14| LED Green
 *     Pad Port Pin 4 |8   6 10 13| LED Red
 *     Pad Port Pin 6 |9   7  9 12|
 *                GND |10(15) 8 11| Pad Port Pin 9
 *                    `-----------'
 */
#define RESET_IN_PIN 13
#define RESET_OUT_PIN 14
#define VIDEOMODE_PIN 16
#define LANGUAGE_PIN 15
#define MODE_LED_R_PIN 10
#define MODE_LED_G_PIN 11
#define MODE_LED_B_PIN 12

#elif defined __AVR_ATmega328__ || defined __AVR_ATmega328P__ || defined __AVR_ATmega168__
/*
 * Arduino Uno/Nano/Micro/Whatever, use a convenience #define till we come up
 * with something better
 */
#define ARDUINO328

/*
 * On an full Arduino board all features are supported. Unfortunately, there is
 * no port fully available, so we resort again to reading UP and DOWN from a
 * different port. Technically we could use PORTD, but since working on a full
 * Arduino board is mainly useful to get debugging messages through the serial
 * port, we don't do that (PD0 and PD1 are used by the hardware serial port).
 * But if you put a single ATmega328 on a board and use its internal clock you
 * also get PORTB, so we might support that in the future. On a side note, PORTD
 * also has INT1 on pin2, so we could easily use the X61 read function...
 *
 *                    ,-----_-----.
 *                    |1     A5 28| JP1/2 (Language)
 *                    |2   0 A4 27| JP3/4 (Video Mode)
 *                    |3   1 A3 26| Reset In
 *     Pad Port Pin 7 |4   2 A2 25| Reset Out
 *     Pad Port Pin 3 |5   3 A1 24| Pad Port Pin 2
 *     Pad Port Pin 4 |6   4 A0 23| Pad Port Pin 1
 *                +5V |7        22| GND
 *                GND |8        21| +5V
 *                    |9        20| +5V
 *                    |10    13 19| (Built-in LED)
 *     Pad Port Pin 6 |11  5 12 18|
 *     Pad Port Pin 9 |12  6 11 17| LED Blue
 *                    |13  7 10 16| LED Green
 *                    |14  8  9 15| LED Red
 *                    `-----------'
 */
#define RESET_IN_PIN A3
#define RESET_OUT_PIN A2
#define VIDEOMODE_PIN A4
#define LANGUAGE_PIN A5
#define MODE_LED_R_PIN 9
#define MODE_LED_G_PIN 10
#define MODE_LED_B_PIN 11
#define PAD_LED_PIN LED_BUILTIN
#define ENABLE_SERIAL_DEBUG

#else
  #error "Unsupported platform!"
#endif


/*******************************************************************************
 * BUTTON COMBO SETTINGS
 ******************************************************************************/

/* DON'T TOUCH THIS! Just look at it for the button names you can use below!
 *
 * Technical note: This has been organized (together with the controller port
 * wiring) to minimize bit twiddling in the controller reading function.
 */
enum PadButton {
  MD_BTN_START = 1 << 7,
  MD_BTN_A =     1 << 6,
  MD_BTN_C =     1 << 5,
  MD_BTN_B =     1 << 4,
  MD_BTN_RIGHT = 1 << 3,
  MD_BTN_LEFT =  1 << 2,
  MD_BTN_DOWN =  1 << 1,
  MD_BTN_UP =    1 << 0
};

/* Button combo that enables the other combos
 *
 * Note: That vertical bar ("pipe") means that the buttons must be pressed
 *       together.
 */
#define TRIGGER_COMBO (MD_BTN_START | MD_BTN_B)

/* Button combos to perform other actions. These are to be considered in
 * addition to TRIGGER_COMBO.
 *
 * Note that we cannot detect certain buttons on some platforms
 */
#define RESET_COMBO (MD_BTN_A | MD_BTN_C)

#if defined __AVR_ATtinyX4__
  /* On ATtinyX4's we can't use LEFT and RIGHT, so just use UP and DOWN to
   * cycle through modes
   */
  #define NEXT_MODE_COMBO MD_BTN_DOWN
  #define PREV_MODE_COMBO MD_BTN_UP
#elif defined __AVR_ATtinyX61__ || defined __AVR_ATtinyX313__ || defined ARDUINO328
  /* On ATtinyX61's, ATtinyX313's and Arduinos we can detect all buttons, so we
   * can make up a specific combo for every mode that switches straight to it,
   * no need to cycle among modes.
   */
  #define USA_COMBO MD_BTN_RIGHT
  #define JAP_COMBO MD_BTN_LEFT
#endif


/*******************************************************************************
 * ADVANCED SETTINGS
 ******************************************************************************/

#if !defined __AVR_ATtinyX5__
/* Offset in the EEPROM at which the current mode should be saved. Undefine to
 * disable mode saving.
 */
#define MODE_ROM_OFFSET 42

// Time to wait after mode change before saving the new mode (milliseconds)
#define MODE_SAVE_DELAY 5000L

// Force the reset line level when active. Undefine to enable auto-detection.
//#define FORCE_RESET_ACTIVE_LEVEL LOW

#endif // !__AVR_ATtinyX5__

/* Colors to use to indicate the video mode, in 8-bit RGB componentes. You can
 * use any value here if your led is connected to PWM-capable pins, otherwise
 * values specified here will be interpreted as either fully off (if 0) or fully
 * on (if anything else).
 *
 * Note that using PWM-values here sometimes causes unpredictable problems. This
 * happened to me on an ATtiny861, and it's probably due to how pins and timers
 * interact. It seems to work fine on a full Arduino, but unless you really want
 * weird colors, use only 0x00 and 0xFF.
 *
 * Oh, and good luck trying to fit a 5mm RGB led in the MegaDrive ;).
 */
#if defined __AVR_ATtinyX4__
/* We only have two LED pins, so let's use a dual-color led and stick to the
 * D4s/Seb colors
 */
#define MODE_LED_USA_COLOR {0xFF, 0x00}  // Red
#define MODE_LED_JAP_COLOR {0x00, 0xFF}  // Green

#elif !defined __AVR_ATtinyX5__

#define MODE_LED_USA_COLOR {0xFF, 0x00, 0x00}  // Red
#define MODE_LED_JAP_COLOR {0x00, 0xFF, 0x00}  // Green

#endif

// Define this if your led is common-anode, comment out for common-cathode
//#define MODE_LED_COMMON_ANODE

/* Use a single led to indicate the video mode. This is enabled automatically
 * in place of the RGB led when low flash space is detected, but since this
 * does NOT disable the RGB led, it can be used together with it, provided that
 * you have a free pin.
 *
 * Basically, the single led is blinked 1-2 times according to which mode is set
 * (1 is USA, see enum VideoMode below).
 */
//#define MODE_LED_SINGLE_PIN 3

/* Presses of the reset button longer than this amount of milliseconds will
 * switch to the next mode, shorter presses will reset the console.
 */
#define LONGPRESS_LEN 700

// Debounce duration for the reset button
#define DEBOUNCE_MS 20

// Duration of the reset pulse (milliseconds)
#define RESET_LEN 350

/* Duration of the pad read function (microseconds). The pad signals must be
 * stable for this amount of time for the read to be valid. This used to be 1,
 * but then it was discovered that the 6-button pad needs more time, so it was
 * brought to 5. Feel free to increase it a bit (up to 10 or thereabots) if you
 * are experiencing unwanted resets when you keep A+B pressed.
 */
#define PORT_READ_TIME 5

// Print the controller status on serial. Useful for debugging.
#ifdef ENABLE_SERIAL_DEBUG
//#define DEBUG_PAD
#endif

/*******************************************************************************
 * END OF SETTINGS
 ******************************************************************************/


#ifdef ENABLE_SERIAL_DEBUG
  #define debug(...) Serial.print (__VA_ARGS__)
  #define debugln(...) Serial.println (__VA_ARGS__)
#else
  #define debug(...)
  #define debugln(...)
#endif

#ifdef MODE_ROM_OFFSET
  #include <EEPROM.h>
#endif

enum __attribute__ ((__packed__)) VideoMode {
  USA,
  JAP,
  MODES_NO // Leave at end
};

// This will be handy
#if (defined MODE_LED_R_PIN || defined MODE_LED_G_PIN || defined MODE_LED_B_PIN)
  #define ENABLE_MODE_LED_RGB

const byte mode_led_colors[][MODES_NO] = {
  MODE_LED_USA_COLOR,
  MODE_LED_JAP_COLOR
};
#endif

#ifdef LOW_FLASH
  // A bit of hack, but seems to work fine and saves quite a bit of flash memory
  #define analogWrite digitalWrite
#endif



VideoMode current_mode;
unsigned long mode_last_changed_time;

// Reset level when NOT ACTIVE
byte reset_inactive_level;

inline void save_mode () {
#ifdef MODE_ROM_OFFSET
  if (mode_last_changed_time > 0 && millis () - mode_last_changed_time >= MODE_SAVE_DELAY) {
    debug ("Saving video mode to EEPROM: ");
    debugln (current_mode);

#ifndef LOW_FLASH
    byte saved_mode = EEPROM.read (MODE_ROM_OFFSET);
    if (current_mode != saved_mode) {
#endif
      EEPROM.write (MODE_ROM_OFFSET, static_cast<byte> (current_mode));
#ifndef LOW_FLASH
    } else {
      debugln ("Mode unchanged, not saving");
    }
#endif
    mode_last_changed_time = 0;    // Don't save again

    // Blink led to tell the user that mode was saved
#ifdef ENABLE_MODE_LED_RGB
    byte c = 0;

#ifdef RGB_LED_COMMON_ANODE
    c = 255 - c;
#endif

#ifdef MODE_LED_R_PIN
    analogWrite (MODE_LED_R_PIN, c);
#endif

#ifdef MODE_LED_G_PIN
    analogWrite (MODE_LED_G_PIN, c);
#endif

#ifdef MODE_LED_B_PIN
    analogWrite (MODE_LED_B_PIN, c);
#endif

    // Keep off for a bit
    delay (200);

    // Turn leds back on
    update_mode_leds ();
#endif  // ENABLE_MODE_LED_RGB

#ifdef MODE_LED_SINGLE_PIN
    // Make one long flash
    digitalWrite (MODE_LED_SINGLE_PIN, LOW);
    delay (500);
    digitalWrite (MODE_LED_SINGLE_PIN, HIGH);
#endif
  }
#endif  // MODE_ROM_OFFSET
}

#if !defined __AVR_ATtinyX5__
inline void change_mode (int increment) {
  // This also loops in [0, MODES_NO) backwards
  VideoMode new_mode = static_cast<VideoMode> ((current_mode + increment + MODES_NO) % MODES_NO);
  set_mode (new_mode);
}

inline void next_mode () {
  change_mode (+1);
}

inline void prev_mode () {
  change_mode (-1);
}

void update_mode_leds () {
#ifdef ENABLE_MODE_LED_RGB
  const byte *colors = mode_led_colors[current_mode];
  byte c;

#ifdef MODE_LED_R_PIN
  c = colors[0];
#ifdef MODE_LED_COMMON_ANODE
  c = 255 - c;
#endif
  analogWrite (MODE_LED_R_PIN, c);
#endif

#ifdef MODE_LED_G_PIN
  c = colors[1];
#ifdef MODE_LED_COMMON_ANODE
  c = 255 - c;
#endif
  analogWrite (MODE_LED_G_PIN, c);
#endif

#ifdef MODE_LED_B_PIN
  c = colors[2];
#ifdef MODE_LED_COMMON_ANODE
  c = 255 - c;
#endif
  analogWrite (MODE_LED_B_PIN, c);
#endif

#endif  // ENABLE_MODE_LED_RGB

#ifdef MODE_LED_SINGLE_PIN
  // WARNING: This loop must be reasonably shorter than LONGPRESS_LEN in the worst case!
  for (byte i = 0; i < current_mode + 1; ++i) {
    digitalWrite (MODE_LED_SINGLE_PIN, LOW);
    delay (40);
    digitalWrite (MODE_LED_SINGLE_PIN, HIGH);
    delay (80);
  }
#endif
}

void set_mode (VideoMode m) {
  switch (m) {
    default:
    case USA:
      digitalWrite (VIDEOMODE_PIN, HIGH);   // NTSC 60Hz
      digitalWrite (LANGUAGE_PIN, HIGH);    // ENG
      break;
    case JAP:
      digitalWrite (VIDEOMODE_PIN, HIGH);   // NTSC 60Hz
      digitalWrite (LANGUAGE_PIN, LOW);     // JAP
      break;
  }

  current_mode = m;
  update_mode_leds ();

  mode_last_changed_time = millis ();
}
#endif

inline void handle_reset_button () {
  static byte debounce_level = LOW;
  static bool reset_pressed_before = false;
  static long last_int = 0, reset_press_start = 0;
  static unsigned int hold_cycles = 0;

  byte reset_level = digitalRead (RESET_IN_PIN);
  if (reset_level != debounce_level) {
    // Reset debouncing timer
    last_int = millis ();
    debounce_level = reset_level;
  } else if (millis () - last_int > DEBOUNCE_MS) {
    // OK, button is stable, see if it has changed
    if (reset_level != reset_inactive_level && !reset_pressed_before) {
      // Button just pressed
      reset_press_start = millis ();
      hold_cycles = 0;
    } else if (reset_level == reset_inactive_level && reset_pressed_before) {
      // Button released
      if (hold_cycles == 0) {
        debugln ("Reset button pushed for a short time");
        reset_console ();
      }
#if !defined __AVR_ATtinyX5__
    } else {
      // Button has not just been pressed/released
      if (reset_level != reset_inactive_level && millis () % reset_press_start >= LONGPRESS_LEN * (hold_cycles + 1)) {
        // Reset has been held for a while
        debugln ("Reset button hold");
        ++hold_cycles;
        next_mode ();
      }
#endif
    }

    reset_pressed_before = (reset_level != reset_inactive_level);
  }
}

void reset_console () {
  debugln ("Resetting console");

  digitalWrite (RESET_OUT_PIN, !reset_inactive_level);
  delay (RESET_LEN);
  digitalWrite (RESET_OUT_PIN, reset_inactive_level);
}

void setup () {
#ifdef ENABLE_SERIAL_DEBUG
  Serial.begin (9600);
#endif

  debugln ("Starting up...");

/* Rant: As per D4s's installation schematics out there (which we use too), it
 * seems that on consoles with an active low reset signal, the Reset In input
 * is taken before the pull-up resistor, while on consoles with active-high
 * reset it is taken AFTER the pull-down resistor. This means that detecting
 * the reset level by sampling the same line on both consoles is tricky, as in
 * both cases one of the Reset In/Out signals is left floating :(. The
 * following should work reliably, but we allow for a way to force the reset
 * line level.
 */
#ifndef FORCE_RESET_ACTIVE_LEVEL
  // Let things settle down and then sample the reset line
  delay (100);
  pinMode (RESET_IN_PIN, INPUT_PULLUP);
  reset_inactive_level = digitalRead (RESET_IN_PIN);
  debug ("Reset line is ");
  debug (reset_inactive_level ? "HIGH" : "LOW");
  debugln (" at startup");
#else
  reset_inactive_level = !FORCE_RESET_ACTIVE_LEVEL;
  debug ("Reset line is forced to active-");
  debugln (FORCE_RESET_ACTIVE_LEVEL ? "HIGH" : "LOW");
#endif

  if (reset_inactive_level == LOW) {
    // No need for pull-up
    pinMode (RESET_IN_PIN, INPUT);
#ifdef FORCE_RESET_ACTIVE_LEVEL   // If this is not defined pull-up was already enabled above
  } else {
    pinMode (RESET_IN_PIN, INPUT_PULLUP);
#endif
  }

  // Enable reset
  pinMode (RESET_OUT_PIN, OUTPUT);
  digitalWrite (RESET_OUT_PIN, !reset_inactive_level);

  // Setup leds
#ifdef MODE_LED_R_PIN
  pinMode (MODE_LED_R_PIN, OUTPUT);
#endif

#ifdef MODE_LED_G_PIN
  pinMode (MODE_LED_G_PIN, OUTPUT);
#endif

#ifdef MODE_LED_B_PIN
  pinMode (MODE_LED_B_PIN, OUTPUT);
#endif

#ifdef MODE_LED_SINGLE_PIN
  pinMode (MODE_LED_SINGLE_PIN, OUTPUT);
#endif

#ifdef PAD_LED_PIN
  pinMode (PAD_LED_PIN, OUTPUT);
#endif

#if !defined __AVR_ATtinyX5__
  // Init video mode
  pinMode (VIDEOMODE_PIN, OUTPUT);
  pinMode (LANGUAGE_PIN, OUTPUT);
  current_mode = USA;
#ifdef MODE_ROM_OFFSET
  byte tmp = EEPROM.read (MODE_ROM_OFFSET);
  debug ("Loaded video mode from EEPROM: ");
  debugln (tmp);
  if (tmp < MODES_NO) {
    // Palette EEPROM value is good
    current_mode = static_cast<VideoMode> (tmp);
  }
#endif
  set_mode (current_mode);
  mode_last_changed_time = 0;   // No need to save what we just loaded
#endif

  // Prepare to read pad
  setup_pad ();

  // Finally release the reset line
  digitalWrite (RESET_OUT_PIN, reset_inactive_level);
}

inline void setup_pad () {
  // Set port directions
#if defined __AVR_ATtinyX5__
  DDRB &= ~((1 << DDB2) | (1 << DDB1) | (1 << DDB0));
#elif defined __AVR_ATtinyX4__
  DDRA &= ~((1 << DDA6) | (1 << DDA2) | (1 << DDA1));
  DDRB &= ~((1 << DDB1) | (1 << DDB0));
#elif defined __AVR_ATtinyX61__
  DDRA &= ~((1 << DDA6) | (1 << DDA5) | (1 << DDA4) | (1 << DDA3) | (1 << DDA2) | (1 << DDA1) | (1 << DDA0));
#elif defined __AVR_ATtinyX313__
  DDRD &= ~((1 << DDD6) | (1 << DDD5) | (1 << DDD4) | (1 << DDD3) | (1 << DDD2) | (1 << DDD1) | (1 << DDD0));
#elif defined ARDUINO328
  DDRC &= ~((1 << DDC1) | (1 << DDC0));
  DDRD &= ~((1 << DDD6) | (1 << DDD5) | (1 << DDD4) | (1 << DDD3) | (1 << DDD2));
#endif
}

/******************************************************************************/

/* Reads the controller port in a safe way, making sure that values are stable
 * across a 1 us interval. This is needed to avoid false reads which occur when
 * we happen to sample the port value right after the SELECT pin has been
 * toggled and the 74HC157 still hasn't had time to update its outputs.
 *
 * This issue was identified by Nopileus, who also helped testing the fix, at
 * http://assemblergames.com/l/threads/megadrive-new-switchless-region-igr-mod.61273/page-4#post-898356
 */
inline byte read_pad_port (volatile uint8_t *pin) {
  byte port, port2;

  port2 = *pin;
  do {
    port = port2;
    delayMicroseconds (PORT_READ_TIME);
    port2 = *pin;
  } while (port != port2);

  return port;
}

/*
 * The basic idea here is to make up a byte where each bit represents the state
 * of a button, where 1 means pressed, for commodity's sake. The bit-button
 * mapping is defined in the PadButton enum above.
 *
 * To get consistent readings, we should really read all of the pad pins at
 * once, since some of them must be interpreted according to the value of the
 * SELECT signal. In order to do this we could connect all pins to a single port
 * of our MCU, but this is a luxury we cannot often afford, for various reasons.
 * Luckily, the UP and DOWN signals do not depend upon the SELECT pins, so we
 * can read them anytime, and this often takes us out of trouble.
 *
 * Note that printing readings through serial slows down the code so much that
 * it misses most of the readings with SELECT low!
 */
inline byte read_pad () {
  static byte pad_status = 0;

#if defined __AVR_ATtinyX5__
  /*
   * On ATtinyX4's all we can do is read A/B and Start/C together with SELECT:
   * - Pin 6 (A/B) -> PB0
   * - Pin 7 (SELECT) -> PB2
   * - Pin 9 (Start/C) -> PB1
   */
  byte portb = read_pad_port (&PINB);
  if (portb & (1 << PINB2)) {
    // Select is high, we have C & B
    pad_status = (pad_status & 0xCF)
               | ((~portb & ((1 << PINB1) | (1 << PINB0))) << 4);
  } else {
    // Select is low, we have Start & A
    pad_status = (pad_status & 0x3F)
               | ((~portb & ((1 << PINB1) | (1 << PINB0))) << 6);
  }
#elif defined __AVR_ATtinyX4__
  /*
   * On ATtinyX4's we read A/B and Start/C together with SELECT through PORTA.
   * Then we read UP and DOWN through PORTB.
   * Connections are made like this to only use pins left spare from the D4s/Seb
   * mod. In principle, we could read all of them through a single port (which
   * would also allow us to read LEFT and RIGHT instead of UP and DOWN) if we
   * reorganize all the connections.
   *
   * If connections are made as per the diagram above we have:
   * - Pin 1 (UP) -> PB0
   * - Pin 2 (DOWN) -> PB1
   * - Pin 6 (A/B) -> PA1
   * - Pin 7 (SELECT) -> PA6
   * - Pin 9 (Start/C) -> PA2
   */

  // Update UP and DOWN, which are always valid and on PORTB alone
  pad_status = (pad_status & 0xFC) | (~PINB & ((1 << PINB1) | (1 << PINB0)));

  // Then deal with the rest
  byte porta = read_pad_port (&PINA);
  if (porta & (1 << PINA6)) {
    // Select is high, we have C & B
    pad_status = (pad_status & 0xCF)
               | ((~porta & ((1 << PINA2) | (1 << PINA1))) << 3)
               ;
  } else {
    // Select is low, we have Start & A
    pad_status = (pad_status & 0x3F)
               | ((~porta & ((1 << PINA2) | (1 << PINA1))) << 5)
               ;
  }
#elif defined __AVR_ATtinyX61__
  /*
   * On ATtinyX61 we have all the buttons on a single port, so we can read all
   * of them at once. We still have to play a bit with the bits (pun intended)
   * since we want to have SELECT connected to INT1, which will probably help
   * with the 6-button pad.
   */

  byte porta = read_pad_port (&PINA);
  if (porta & (1 << PINA2)) {
    // Select is high, we have the 4 directions, C & B
    pad_status = (pad_status & 0xC0)
               | ((~porta & ((1 << PINA6) | (1 << PINA5) | (1 << PINA4) | (1 << PINA3))) >> 1)
               | (~porta & ((1 << PINA1) | (1 << PINA0)))
               ;
  } else {
    // Select is low, we have Up, Down, Start & A
    pad_status = (pad_status & 0x30)
               | ((~porta & ((1 << PINA6) | (1 << PINA5))) << 1)
               | (~porta & ((1 << PINA1) | (1 << PINA0)))
               ;
  }
#elif defined __AVR_ATtinyX313__
  /*
   * Same as above, but with port D instead of port A. SELECT is connected to
   * INT0.
   */

  byte portd = read_pad_port (&PIND);
  if (portd & (1 << PIND2)) {
    // Select is high, we have the 4 directions, C & B
    pad_status = (pad_status & 0xC0)
               | ((~portd & ((1 << PIND6) | (1 << PIND5) | (1 << PIND4) | (1 << PIND3))) >> 1)
               | (~portd & ((1 << PIND1) | (1 << PIND0)))
               ;
  } else {
    // Select is low, we have Up, Down, Start & A
    pad_status = (pad_status & 0x30)
               | ((~portd & ((1 << PIND6) | (1 << PIND5))) << 1)
               | (~portd & ((1 << PIND1) | (1 << PIND0)))
               ;
  }
#elif defined ARDUINO328
  // Update UP and DOWN, which are always valid and on PORTC alone
  pad_status = (pad_status & 0xFC)
             | (~PINC & ((1 << PINC1) | (1 << PINC0)))
             ;

  byte portd = read_pad_port (&PIND);
  // Signals are stable, process them
  if (portd & (1 << PIND2)) {
    // Select is high, we have Right, Left, C & B
    pad_status = (pad_status & 0xC3)
               | ((~portd & ((1 << PIND6) | (1 << PIND5) | (1 << PIND4) | (1 << PIND3))) >> 1)
               ;
  } else {
    // Select is low, we have Start & A
    pad_status = (pad_status & 0x3F)
               | ((~portd & ((1 << PIND6) | (1 << PIND5))) << 1)
               ;
  }
#endif

  return pad_status;
}

#define IGNORE_COMBO_MS LONGPRESS_LEN

inline void handle_pad () {
  static long last_combo_time = 0;

  byte pad_status = read_pad ();

#ifdef PAD_LED_PIN
  digitalWrite (PAD_LED_PIN, pad_status);
#endif

#ifdef DEBUG_PAD
  static byte last_pad_status = 0x00;

  if (pad_status != last_pad_status) {
    debug (F("Pressed: "));
    if (pad_status & MD_BTN_UP)
      debug (F("Up "));
    if (pad_status & MD_BTN_DOWN)
      debug (F("Down "));
    if (pad_status & MD_BTN_LEFT)
      debug (F("Left "));
    if (pad_status & MD_BTN_RIGHT)
      debug (F("Right "));
    if (pad_status & MD_BTN_A)
      debug (F("A "));
    if (pad_status & MD_BTN_B)
      debug (F("B "));
    if (pad_status & MD_BTN_C)
      debug (F("C "));
    if (pad_status & MD_BTN_START)
      debug (F("Start "));
    debugln ();

    last_pad_status = pad_status;
  }
#endif

  if ((pad_status & TRIGGER_COMBO) == TRIGGER_COMBO && millis () - last_combo_time > IGNORE_COMBO_MS) {
    if ((pad_status & RESET_COMBO) == RESET_COMBO) {
      debugln ("Reset combo detected");
      reset_console ();
      pad_status = 0;     // Avoid continuous reset (pad_status might keep the last value during reset!)
      last_combo_time = millis ();
#ifdef USA_COMBO
    } else if ((pad_status & USA_COMBO) == USA_COMBO) {
      debugln ("USA mode combo detected");
      set_mode (USA);
      last_combo_time = millis ();
#endif
#ifdef JAP_COMBO
    } else if ((pad_status & JAP_COMBO) == JAP_COMBO) {
      debugln ("JAP mode combo detected");
      set_mode (JAP);
      last_combo_time = millis ();
#endif
#ifdef NEXT_MODE_COMBO
    } else if ((pad_status & NEXT_MODE_COMBO) == NEXT_MODE_COMBO) {
      debugln ("Next mode combo detected");
      next_mode ();
      last_combo_time = millis ();
#endif
#ifdef PREV_MODE_COMBO
    } else if ((pad_status & PREV_MODE_COMBO) == PREV_MODE_COMBO) {
      debugln ("Previous mode combo detected");
      prev_mode ();
      last_combo_time = millis ();
#endif
    }
  }
}

void loop () {
  handle_reset_button ();
  handle_pad ();
  save_mode ();
}
