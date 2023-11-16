/*
 * Joystick.h
 *
 *  Created on: Nov 6, 2023
 *      Author: user
 */

#ifndef HAL_JOYSTICK_H_
#define HAL_JOYSTICK_H_
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

struct _Joystick
{


uint_fast16_t x;
uint_fast16_t y;

bool isTappedToLeft;
bool isTappedToRight;
bool isTappedToDown;
bool isTappedToUp;
};
typedef struct _Joystick Joystick;

/** Constructs a new Joystick object, given a valid port and pin. */
Joystick Joystick_construct();

/** Given a Joystick, determines if the joystick is currently pressed to left */
bool Joystick_isPressedToLeft(Joystick* Joystick);
bool Joystick_isPressedToRight(Joystick* Joystick);
bool Joystick_isPressedToDown(Joystick* Joystick);
bool Joystick_isPressedToUp(Joystick* Joystick);
/** Given a Joystick, determines if it was "tapped" - pressed down and released */
bool Joystick_isTappedToLeft(Joystick* Joystick);
bool Joystick_isTappedToRight(Joystick* Joystick);
bool Joystick_isTappedToDown(Joystick* Joystick);
bool Joystick_isTappedToUp(Joystick* Joystick);
/** Refreshes this Joystick so the Joystick FSM now has new outputs to interpret */
void Joystick_refresh(Joystick* Joystick);


#endif /* HAL_JOYSTICK_H_ */
