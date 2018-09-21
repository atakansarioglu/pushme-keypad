/**
 * @file      keypad.c
 * @author    Atakan S.
 * @date      01/01/2018
 * @version   1.0
 * @brief     "PushME" Event based keypad reader module with debouncing.
 *
 * @copyright Copyright (c) 2018 Atakan SARIOGLU ~ www.atakansarioglu.com
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a
 *  copy of this software and associated documentation files (the "Software"),
 *  to deal in the Software without restriction, including without limitation
 *  the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *  and/or sell copies of the Software, and to permit persons to whom the
 *  Software is furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 *  DEALINGS IN THE SOFTWARE.
 */

#include <porty.h>
#ifdef _H_KEYPAD_H // Exclude from compile when not included in projech.h

#include <keypad.h>
#include <string.h>

// Struct to keep the variables of the keypad module.
typedef struct {
	//-- Key events.
	uint16_t keysPressed;  			// PORT POLLING register
	uint16_t keysDown;  			//
	uint16_t keysReleased;  		//
	tTimeout timerDebounce;
	tTimeout timerLong;
	uint8_t counterBoost;
} tKeypad;

// Initialize the variables.
tKeypad keypad = { .keysPressed = 0, .keysDown = 0, .keysReleased = 0, .counterBoost = 0, };

// This function reads the pin values and writes to bits of a variable.
uint16_t keypadSerialize(void) {
	uint16_t result = 0;
#ifdef KEY00_pin
	result |= (mPinRead(KEY00_pin) == KEY00_active) ? KEY00 : 0;
#endif
#ifdef KEY01_pin
	result |= (mPinRead(KEY01_pin) == KEY01_active) ? KEY01 : 0;
#endif
#ifdef KEY02_pin
	result |= (mPinRead(KEY02_pin) == KEY02_active) ? KEY02 : 0;
#endif
#ifdef KEY03_pin
	result |= (mPinRead(KEY03_pin) == KEY03_active) ? KEY03 : 0;
#endif
#ifdef KEY04_pin
	result |= (mPinRead(KEY04_pin) == KEY04_active) ? KEY04 : 0;
#endif
#ifdef KEY05_pin
	result |= (mPinRead(KEY05_pin) == KEY05_active) ? KEY05 : 0;
#endif
#ifdef KEY06_pin
	result |= (mPinRead(KEY06_pin) == KEY06_active) ? KEY06 : 0;
#endif
#ifdef KEY07_pin
	result |= (mPinRead(KEY07_pin) == KEY07_active) ? KEY07 : 0;
#endif
#ifdef KEY08_pin
	result |= (mPinRead(KEY08_pin) == KEY08_active) ? KEY08 : 0;
#endif
#ifdef KEY09_pin
	result |= (mPinRead(KEY09_pin) == KEY09_active) ? KEY09 : 0;
#endif
#ifdef KEY10_pin
	result |= (mPinRead(KEY10_pin) == KEY10_active) ? KEY10 : 0;
#endif
#ifdef KEY11_pin
	result |= (mPinRead(KEY11_pin) == KEY11_active) ? KEY11 : 0;
#endif
#ifdef KEY12_pin
	result |= (mPinRead(KEY12_pin) == KEY12_active) ? KEY12 : 0;
#endif
#ifdef KEY13_pin
	result |= (mPinRead(KEY13_pin) == KEY13_active) ? KEY13 : 0;
#endif
#ifdef KEY14_pin
	result |= (mPinRead(KEY14_pin) == KEY14_active) ? KEY14 : 0;
#endif
	return result;
}

//-- Read keypad.
void keypadRead() {
	//-- All release events processed in the last cycle are cleared.
	keypad.keysReleased &= KEYLONG; //Release=0

	// Read all the keys to variable.
	keypad.keysPressed = keypadSerialize();

	//-- If pressed.
	if (keypad.keysPressed) {
		//-- If pressed and seen for the first time.
		if (keypad.keysPressed != keypad.keysDown) {
			keypad.keysReleased &= ~KEYLONG; //Long=0
			keypad.counterBoost = 0;
			PunctualTimeoutSet(&keypad.timerDebounce, KEY_DEBOUNCE_TIME);
			PunctualTimeoutSet(&keypad.timerLong, KEY_LONGPRESS_TIME);
		}

		//-- Remember what is just seen.
		keypad.keysDown |= keypad.keysPressed;

		//-- Longpress timeout check.
		//-- If this is the first Longpress event or REPEAT is enabled (KEY_LONGPRESS_REPEAT_TIME > 0).
		if (PunctualTimeoutCheck(&keypad.timerLong) && (((keypad.keysReleased & KEYLONG) == 0) || KEY_LONGPRESS_REPEAT_TIME)) {
			keypad.keysReleased = keypad.keysDown | KEYLONG; //Release=Down, Long=1

			// If number of REPEAT events are greater than KEY_LONGPRESS_BOOST_THRESHOLD and BOOST is enabled.
			if (KEY_LONGPRESS_BOOST_THRESHOLD && (++keypad.counterBoost > KEY_LONGPRESS_BOOST_THRESHOLD)) {
				keypad.counterBoost = KEY_LONGPRESS_BOOST_THRESHOLD;
				PunctualTimeoutSet(&keypad.timerLong, KEY_LONGPRESS_BOOST_TIME);
			} else {
				PunctualTimeoutSet(&keypad.timerLong, KEY_LONGPRESS_REPEAT_TIME);
			}
		}
	} else {
		//-- Keypress timeout check.
		if (PunctualTimeoutCheck(&keypad.timerDebounce)) {
			if ((keypad.keysReleased & KEYLONG) == 0) {
				keypad.keysReleased = keypad.keysDown; //Release=Down
			}
		}

		//-- Reset all.
		keypad.keysDown = 0;
		keypad.keysReleased &= ~KEYLONG; //Long=0
		keypad.counterBoost = 0;

		//-- Reset timeouts.
		PunctualTimeoutSet(&keypad.timerLong, KEY_LONGPRESS_TIME);
		PunctualTimeoutSet(&keypad.timerDebounce, KEY_DEBOUNCE_TIME);
	}
}

bool isKeyDown(uint16_t m) {
	return ((keypad.keysDown == (m)) && (keypad.keysReleased == 0));
}

bool isKeyPress(uint16_t m) {
	return ((keypad.keysReleased == (m)));
}

bool isKeyHold(uint16_t m) {
	return ((keypad.keysReleased == (m | KEYLONG)));
}

#endif
