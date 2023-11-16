/*
 * Joystick.c
 *
 *  Created on: Nov 6, 2023
 *      Author: user
 */


/**
 * Constructs a Joystick
 * Initializes the output FSMs.
 *
#include <HAL/Button.h>
 * @return a constructed Joystick with debouncing and output FSMs initialized
 */
// Initializing the ADC which resides on SoC

#define LEFT_THRESHOLD  1500
#define RIGHT_THRESHOLD  12000
#define UP_THRESHOLD  12000
#define DOWN_THRESHOLD  1500

enum _JoystickDebounceStateLeft { LEFT, NOT_LEFT };
typedef enum _JoystickDebounceStateLeft JoystickDebounceStateLeft;
enum _JoystickDebounceStateDown { DOWN, NOT_DOWN };
typedef enum _JoystickDebounceStateDown JoystickDebounceStateDown;
enum _JoystickDebounceStateRight { RIGHT, NOT_RIGHT };
typedef enum _JoystickDebounceStateRight JoystickDebounceStateRight;
enum _JoystickDebounceStateUp { UP, NOT_UP };
typedef enum _JoystickDebounceStateUp JoystickDebounceStateUp;

#include <HAL/Joystick.h>
void initADC() {
    ADC14_enableModule();

    ADC14_initModule(ADC_CLOCKSOURCE_SYSOSC,
                     ADC_PREDIVIDER_1,
                     ADC_DIVIDER_1,
                      0
                     );

    // This configures the ADC to store output results
    // in ADC_MEM0 for joystick X.
    // Todo: if we want to add joystick Y, then, we have to use more memory locations
    ADC14_configureMultiSequenceMode(ADC_MEM0, ADC_MEM1, true);//made to use memory locations mem0 to mem1

    // This configures the ADC in manual conversion mode
    // Software will start each conversion.
    ADC14_enableSampleTimer(ADC_AUTOMATIC_ITERATION);
}


void startADC() {
   // Starts the ADC with the first conversion
   // in repeat-mode, subsequent conversions run automatically
   ADC14_enableConversion();
   ADC14_toggleConversionTrigger();
}


// Interfacing the Joystick with ADC (making the proper connections in software)
void initJoyStick() {

    // This configures ADC_MEM0 to store the result from
    // input channel A15 (Joystick X), in non-differential input mode
    // (non-differential means: only a single input pin)
    // The reference for Vref- and Vref+ are VSS and VCC respectively
    ADC14_configureConversionMemory(ADC_MEM0,
                                  ADC_VREFPOS_AVCC_VREFNEG_VSS,
                                  ADC_INPUT_A15,                 // joystick X
                                  ADC_NONDIFFERENTIAL_INPUTS);

    ADC14_configureConversionMemory(ADC_MEM1,
                                     ADC_VREFPOS_AVCC_VREFNEG_VSS,
                                     ADC_INPUT_A9,                 // joystick Y
                                     ADC_NONDIFFERENTIAL_INPUTS);


    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P6,
                                               GPIO_PIN0,
                                               GPIO_TERTIARY_MODULE_FUNCTION);//joystick x


    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P4,//joysticky
                                               GPIO_PIN4,
                                               GPIO_TERTIARY_MODULE_FUNCTION);


}

Joystick Joystick_construct()
{
    // The Joystick object which will be returned at the end of construction
    Joystick Joystick;

    initADC();
    initJoyStick();
    startADC();
    // Initialize all buffered outputs of the Joystick
  //  Joystick.pushState = RELEASED;
    //Joystick.isTapped = false;

    // Return the constructed Joystick object to the user
    return Joystick;
}


void Joystick_refresh(Joystick* joystick_p)
        {
            joystick_p->x = ADC14_getResult(ADC_MEM0);
            joystick_p->y = ADC14_getResult(ADC_MEM1);

            static JoystickDebounceStateLeft Downstate = NOT_DOWN;
            joystick_p->isTappedToDown= false;//////////////////////////////added for down

            switch(Downstate) {
                     case NOT_DOWN:

                     if (joystick_p->y < DOWN_THRESHOLD){
                        Downstate = DOWN;
                         joystick_p->isTappedToDown = true;
                     }
                   break;
                     case DOWN:
                         if (joystick_p->y > DOWN_THRESHOLD){
                               Downstate = NOT_DOWN;
                             }
                         }




            static JoystickDebounceStateLeft leftstate = NOT_LEFT;
            joystick_p->isTappedToLeft= false;

             switch(leftstate) {
             case NOT_LEFT:

             if (joystick_p->x < LEFT_THRESHOLD){
                 leftstate = LEFT;
                 joystick_p->isTappedToLeft = true;
             }
           break;
             case LEFT:
                 if (joystick_p->x > LEFT_THRESHOLD){
                       leftstate = NOT_LEFT;
                     }
                 }



             static JoystickDebounceStateUp Upstate = NOT_UP;
                      joystick_p->isTappedToUp= false;

                       switch(Upstate) {
                       case NOT_UP:

                       if (joystick_p->y > UP_THRESHOLD){
                           Upstate = UP;
                           joystick_p->isTappedToUp = true;
                       }
                     break;
                       case UP:
                           if (joystick_p->y < UP_THRESHOLD){
                                 Upstate = NOT_UP;
                               }
                           }

             static JoystickDebounceStateRight Rightstate = NOT_RIGHT;
                                         joystick_p->isTappedToRight= false;

                                          switch(Rightstate) {
                                          case NOT_RIGHT:

                                          if (joystick_p->x > RIGHT_THRESHOLD){
                                              Upstate = RIGHT;
                                              joystick_p->isTappedToRight = true;
                                          }
                                        break;
                                          case RIGHT:
                                              if (joystick_p->x < RIGHT_THRESHOLD){
                                                    Rightstate = NOT_RIGHT;
                                                  }
                                              }
        }








bool Joystick_isPressedToLeft(Joystick* joystick_p){

if(joystick_p->x < LEFT_THRESHOLD)
    return true;
else
    return false;
}

bool Joystick_isTappedToLeft(Joystick* joystick_p){

  return(joystick_p->isTappedToLeft);
}



bool Joystick_isPressedToDown(Joystick* joystick_p){

if(joystick_p->y < DOWN_THRESHOLD)
    return true;
else
    return false;
}

bool Joystick_isTappedToDown(Joystick* joystick_p){

  return(joystick_p->isTappedToDown);
}





bool Joystick_isPressedToUp(Joystick* joystick_p){


if(joystick_p->y > UP_THRESHOLD)
    return true;
else
    return false;
}

bool Joystick_isTappedToUp(Joystick* joystick_p){

  return(joystick_p->isTappedToUp);
}



bool Joystick_isPressedToRight(Joystick* joystick_p){


if(joystick_p->x > RIGHT_THRESHOLD)
    return true;
else
    return false;
}

bool Joystick_isTappedToRight(Joystick* joystick_p){

  return(joystick_p->isTappedToRight);
}
