/* DriverLib Includes */
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>





#define LEFT_THRESHOLD  1500
#define RIGHT_THRESHOLD  12000
#define UP_THRESHOLD  12000
#define DOWN_THRESHOLD  1500

/* HAL and Application includes */
#include <HAL/HAL.h>
#include <HAL/Timer.h>
#include <App_GuessTheColor.h>



extern const Graphics_Image colors8BPP_UNCOMP;
extern const Graphics_Image sad8BPP_UNCOMP;
extern const Graphics_Image happy8BPP_UNCOMP;
void initialize();
void ModifyLEDColor(bool leftButtonWasPushed, bool rightButtonWasPushed);

void initADC();
void startADC();
void initJoyStick();
void getSampleJoyStick(unsigned *X, unsigned *Y);
/**
 * The main entry point of your project. The main function should immediately
 * stop the Watchdog timer, call the Application constructor, and then
 * repeatedly call the main super-loop function. The App_GuessTheColor
 * constructor should be responsible for initializing all hardware components as
 * well as all other finite state machines you choose to use in this project.
 *
 * THIS FUNCTION IS ALREADY COMPLETE. Unless you want to temporarily experiment
 * with some behavior of a code snippet you may have, we DO NOT RECOMMEND
 * modifying this function in any way.
 */
int main(void)
{
    // Stop Watchdog Timer - THIS SHOULD ALWAYS BE THE FIRST LINE OF YOUR MAIN
    WDT_A_holdTimer();



    // Initialize the system clock and background hardware timer, used to enable
    // software timers to time their measurements properly.
    InitSystemTiming();

    // Initialize the main Application object and the HAL.
    HAL hal = HAL_construct();
    App_GuessTheColor app = App_GuessTheColor_construct(&hal);
    App_GuessTheColor_showTitleScreen(&hal.gfx);

    // Main super-loop! In a polling architecture, this function should call
    // your main FSM function over and over.


    while (true)
    {
        App_GuessTheColor_loop(&app, &hal);  //update my program, application state, output
        HAL_refresh(&hal);


        Graphics_Context g_sContext;





    }
}

void initialize()
{
    // stop the watchdog timer
    WDT_A_hold(WDT_A_BASE);

    initADC();
    initJoyStick();
    startADC();
}


// Initializing the ADC which resides on SoC
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

    // This selects the GPIO as analog input
    // A15 is multiplexed on GPIO port P6 pin PIN0
    // TODO: which one of GPIO_PRIMARY_MODULE_FUNCTION, or
    //                    GPIO_SECONDARY_MODULE_FUNCTION, or
    //                    GPIO_TERTIARY_MODULE_FUNCTION
    // should be used in place of 0 as the last argument?
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P6,
                                               GPIO_PIN0,
                                               GPIO_TERTIARY_MODULE_FUNCTION);

    // TODO: add joystick Y
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P4,
                                               GPIO_PIN4,
                                               GPIO_TERTIARY_MODULE_FUNCTION);


}

void getSampleJoyStick(unsigned *X, unsigned *Y) {
    // ADC runs in continuous mode, we just read the conversion buffers
    *X = ADC14_getResult(ADC_MEM0);

    // TODO: Read the Y channel
    *Y = ADC14_getResult(ADC_MEM1);
}


/**
 * The main constructor for your application. This function initializes each
 * state variable required for the GuessTheColor game.
 */
App_GuessTheColor App_GuessTheColor_construct(HAL* hal_p)
{
    // The App_GuessTheColor object to initialize
    App_GuessTheColor app;

    // Predetermined random numbers for this application. In an actual project,
    // you should probably use some form of noise generator instead, like the
    // noise from your ADC.
    app.randomNumbers[0] = 5;
    app.randomNumbers[1] = 2;
    app.randomNumbers[2] = 7;
    app.randomNumbers[3] = 1;
    app.randomNumbers[4] = 3;

    app.randomNumberChoice = 0;

    // Initialization of FSM variables
    app.state = TITLE_SCREEN;
    app.timer = SWTimer_construct(TITLE_SCREEN_WAIT);
    SWTimer_start(&app.timer);

    App_GuessTheColor_initGameVariables(&app, hal_p);

    app.cursor = CURSOR_0;

    // Return the completed Application struct to the user
    return app;
}

/**
 * The main super-loop function of the application. We place this inside of a
 * single infinite loop in main. In this way, we can model a polling system of
 * FSMs. Every cycle of this loop function, we poll each of the FSMs one time.
 */
void App_GuessTheColor_loop(App_GuessTheColor* app_p, HAL* hal_p)
{
    switch (app_p->state)
    {
        case TITLE_SCREEN:
            App_GuessTheColor_handleTitleScreen(app_p, hal_p);
            break;

        case INSTRUCTIONS_SCREEN:
            App_GuessTheColor_handleInstructionsScreen(app_p, hal_p);
            break;

        case GAME_SCREEN:
            App_GuessTheColor_handleGameScreen(app_p, hal_p);
            break;

        case RESULT_SCREEN:
            App_GuessTheColor_handleResultScreen(app_p, hal_p);
            break;

        case SCORE_SCREEN:
                   App_GuessTheColor_handleScoreScreen(app_p, hal_p);
                   break;
        case PLAY_SCREEN:
                           App_GuessTheColor_handlePlayScreen(app_p, hal_p);
                           break;
        default:
            break;
    }
}

/**
 * Sets up the GuessTheColors game by initializing the game state to the Title
 * Screen state.
 */
void App_GuessTheColor_showTitleScreen(GFX* gfx_p)
{

    GFX_clear(gfx_p);


    GFX_print(gfx_p, "HW 9  Game  ", 0, 0);
    GFX_print(gfx_p, "---------------------", 1, 0);
    GFX_print(gfx_p, "By: Ashley Leister", 2, 0);
    GFX_print(gfx_p, "Press any key on ", 3, 0);
    GFX_print(gfx_p, "Mobaxterm to start", 3, 0);
}

/**
 * A helper function which resets all the game variables to their unselected
 * states and resets the cursor position.
 */
void App_GuessTheColor_initGameVariables(App_GuessTheColor* app_p, HAL* hal_p)
{
    // Reset the cursor
    app_p->cursor = CURSOR_0;

    // Deselect each option
    app_p->redSelected = false;
    app_p->greenSelected = false;
    app_p->blueSelected = false;

    // Turn off all LEDs - they don't turn on until a random number is generated
    LED_turnOff(&hal_p->boosterpackRed);
    LED_turnOff(&hal_p->boosterpackGreen);
    LED_turnOff(&hal_p->boosterpackBlue);
}

/**
 * Callback function for when the game is in the TITLE_SCREEN state. Used to
 * break down the main App_GuessTheColor_loop() function into smaller
 * sub-functions.
 */
void App_GuessTheColor_handleTitleScreen(App_GuessTheColor* app_p, HAL* hal_p)
{
    if (SWTimer_expired(&app_p->timer))
    {
        app_p->state = GAME_SCREEN;
        App_GuessTheColor_showGameScreen(app_p, &hal_p->gfx);

    }
}

/**
 * Callback function for when the game is in the INSTRUCTIONS_SCREEN state. Used
 * to break down the main App_GuessTheColor_loop() function into smaller
 * sub-functions.
 */
void App_GuessTheColor_handleInstructionsScreen(App_GuessTheColor* app_p, HAL* hal_p)
{
    // Transition to start the game when B2 is pressed
    if (Button_isTapped(&hal_p->boosterpackJS))//if joystick button pressed REPLACE WITH
    {
        // Update internal logical state
        app_p->state = GAME_SCREEN;

        // Turn on LEDs based off of the lowest three bits of a random number.
        uint32_t randomNumber = app_p->randomNumbers[app_p->randomNumberChoice];

        if (randomNumber & BIT0) { LED_turnOn(&hal_p->boosterpackRed  ); }
        if (randomNumber & BIT1) { LED_turnOn(&hal_p->boosterpackGreen); }
        if (randomNumber & BIT2) { LED_turnOn(&hal_p->boosterpackBlue ); }

        // Increment the random number choice with a mod loopback to 0 when reaching
        // NUM_RANDOM_NUMBERS.
        app_p->randomNumberChoice = (app_p->randomNumberChoice + 1) % NUM_RANDOM_NUMBERS;

        // Display the next state's screen to the user
        App_GuessTheColor_showGameScreen(app_p, &hal_p->gfx);
    }
}
void App_GuessTheColor_handleScoreScreen(App_GuessTheColor* app_p, HAL* hal_p)
{
    // Transition to start the game when B2 is pressed
    if (Button_isTapped(&hal_p->boosterpackJS))//if joystick button pressed REPLACE WITH
    {
        // Update internal logical state
        app_p->state = GAME_SCREEN;

        // Turn on LEDs based off of the lowest three bits of a random number.
        uint32_t randomNumber = app_p->randomNumbers[app_p->randomNumberChoice];

        if (randomNumber & BIT0) { LED_turnOn(&hal_p->boosterpackRed  ); }
        if (randomNumber & BIT1) { LED_turnOn(&hal_p->boosterpackGreen); }
        if (randomNumber & BIT2) { LED_turnOn(&hal_p->boosterpackBlue ); }

        // Increment the random number choice with a mod loopback to 0 when reaching
        // NUM_RANDOM_NUMBERS.
        app_p->randomNumberChoice = (app_p->randomNumberChoice + 1) % NUM_RANDOM_NUMBERS;

        // Display the next state's screen to the user
        App_GuessTheColor_showGameScreen(app_p, &hal_p->gfx);
    }
}
/**
 * Callback function for when the game is in the GAME_SCREEN state. Used to
 * break down the main App_GuessTheColors_loop() function into smaller
 * sub-functions.
 */
void App_GuessTheColor_handleGameScreen(App_GuessTheColor* app_p, HAL* hal_p)
{
// check the inputs
    // If B2 is pressed, increment the cursor and circle it around to 0 if it
    // reaches the bottom
    if (Button_isTapped(&hal_p->boosterpackS2)) {
            app_p->cursor = (Cursor) (((int) app_p->cursor + 1) % NUM_TEST_OPTIONS);
            App_GuessTheColor_updateGameScreen(app_p, &hal_p->gfx);
        }

    // If B1 is pressed, either add a selection to the proper color choice OR
    // transition to the SHOW_RESULT state if the user chooses to end the test.
    if (Button_isTapped(&hal_p->boosterpackJS))
    {
        switch (app_p->cursor)
        {
            // In the first three choices, we need to re-display the game screen
            // to reflect updated choices.
            // -----------------------------------------------------------------
            case CURSOR_0: // Red choice

                App_GuessTheColor_updateGameScreen(app_p, &hal_p->gfx);
                app_p->state = PLAY_SCREEN;
                App_GuessTheColor_showPlayScreen(app_p, &hal_p->gfx,hal_p);
                break;

            case CURSOR_1: // Green choice
                App_GuessTheColor_updateGameScreen(app_p, &hal_p->gfx);
                app_p->state = INSTRUCTIONS_SCREEN;
                App_GuessTheColor_showInstructionsScreen(app_p, &hal_p->gfx);
                break;

            case CURSOR_2: // Blue choice
                App_GuessTheColor_updateGameScreen(app_p, &hal_p->gfx);
                               app_p->state = SCORE_SCREEN;
                               App_GuessTheColor_showScoreScreen(app_p, &hal_p->gfx);
                break;

            // In the final choice, we must setup a transition to RESULT_SCREEN
            // by starting a timer and calling the proper draw function.
            // -----------------------------------------------------------------
            case CURSOR_3:
                app_p->state = RESULT_SCREEN;

                app_p->timer = SWTimer_construct(RESULT_SCREEN_WAIT);//sets timer length
                SWTimer_start(&app_p->timer);//starts the timer

                App_GuessTheColor_showResultScreen(app_p, hal_p);
                break;

            default:
                break;
        }
    }
}
void App_GuessTheColor_handlePlayScreen(App_GuessTheColor* app_p, HAL* hal_p)
{

    // check the inputs
        // If B2 is pressed, increment the cursor and circle it around to 0 if it
        // reaches the bottom




        Graphics_Context g_sContext;

        initialize();
        InitGraphics(&g_sContext);
        draw_Base(&g_sContext);

        unsigned vx, vy;

        while (1)
        {

            getSampleJoyStick(&vx, &vy);
            bool joyStickPushedtoRight = false;
            bool joyStickPushedtoLeft = false;
            bool joyStickPushedtoUp = false;//added these
            bool joyStickPushedtoDown = false;

            drawXY(&g_sContext, vx, vy);

            if (vx < LEFT_THRESHOLD)
            {
                joyStickPushedtoLeft = true;
            }
            if (vx > RIGHT_THRESHOLD)//added this for right threshold
                  {
                      joyStickPushedtoRight = true;
                  }
            if (vy > UP_THRESHOLD)
                    {
                        joyStickPushedtoUp = true;
                    }
            if (vy < DOWN_THRESHOLD)//added this for right threshold
              {
               joyStickPushedtoDown = true;
              }



            MoveCircle(&g_sContext, joyStickPushedtoLeft,joyStickPushedtoRight,joyStickPushedtoDown,joyStickPushedtoUp);

         }



}

/**
 * Callback function for when the game is in the RESULT_SCREEN state. Used to
 * break down the main App_GuessTheColor_loop() function into smaller
 * sub-functions.
 */
void App_GuessTheColor_handleResultScreen(App_GuessTheColor* app_p, HAL* hal_p)
{
    // Transition to instructions and reset game variables when the timer expires

    if (SWTimer_expired(&app_p->timer))
      {

          app_p->state = GAME_SCREEN;
          App_GuessTheColor_initGameVariables(app_p, hal_p);
          App_GuessTheColor_showGameScreen(app_p, &hal_p->gfx);

      }
}

/**
 * A helper function which clears the screen and prints the instructions for how
 * to play the game.
 */
void App_GuessTheColor_showScoreScreen(App_GuessTheColor* app_p, GFX* gfx_p)
{
    // Clear the screen from any old text state
    GFX_clear(gfx_p);

    // Display the text
    GFX_print(gfx_p, "High Scores:         ", 0, 0);
    GFX_print(gfx_p, "---------------------", 1, 0);
    GFX_print(gfx_p, "1)  ", 2, 0);
    GFX_print(gfx_p, "2) ", 3, 0);
    GFX_print(gfx_p, "3) ", 4, 0);
    GFX_print(gfx_p, "4)  ", 5, 0);
    GFX_print(gfx_p, "5)  ", 6, 0);

    GFX_print(gfx_p, "Press joystick to     ", 8, 0);

    GFX_print(gfx_p, "return to menu.  ", 9, 0);
}
void App_GuessTheColor_showInstructionsScreen(App_GuessTheColor* app_p, GFX* gfx_p)
{
    // Clear the screen from any old text state
    GFX_clear(gfx_p);

    // Display the text
    GFX_print(gfx_p, "Instructions         ", 0, 0);
    GFX_print(gfx_p, "---------------------", 1, 0);
    GFX_print(gfx_p, "Move Joystick to move.  ", 2, 0);
    GFX_print(gfx_p, "Press joystick to ", 4, 0);

    GFX_print(gfx_p, "select option.  ", 5, 0);
    GFX_print(gfx_p, "Press joystick to     ", 7, 0);

    GFX_print(gfx_p, "return to menu.  ", 8, 0);
}

/**
 * A helper function which clears the screen and draws an updated display of
 * each color and its selection, intended for use when setting up the
 * GAME_SCREEN state.
 */
void App_GuessTheColor_showPlayScreen(App_GuessTheColor* app_p, GFX* gfx_p,HAL* hal_p)
{
    // Clear the screen from any old text state
    GFX_clear(gfx_p);

    // Display the text
    GFX_print(gfx_p, "Play screen                 ", 0, 0);
    GFX_print(gfx_p, "---------------------", 1, 0);



    // Draw the cursor
    GFX_print(gfx_p, ">", 2 + app_p->cursor, 0);

    // Draw the stars for LED guesses
    if (app_p->redSelected  ) { GFX_print(gfx_p, "*", 2, 8); }
    if (app_p->greenSelected) { GFX_print(gfx_p, "*", 3, 8); }
    if (app_p->blueSelected ) { GFX_print(gfx_p, "*", 4, 8); }





}
void App_GuessTheColor_showGameScreen(App_GuessTheColor* app_p, GFX* gfx_p)
{
    // Clear the screen from any old text state
    GFX_clear(gfx_p);

    // Display the text
    GFX_print(gfx_p, "Game                 ", 0, 0);
    GFX_print(gfx_p, "---------------------", 1, 0);
    GFX_print(gfx_p, "  Play Game                ", 2, 0);
    GFX_print(gfx_p, "  How to Play              ", 3, 0);
    GFX_print(gfx_p, "  High Scores               ", 4, 0);
    GFX_print(gfx_p, "  Game Over      ", 5, 0);

    GFX_print(gfx_p, "B1: Select choice    ", 7, 0);
    GFX_print(gfx_p, "B2: Move arrow       ", 8, 0);

    // Draw the cursor
    GFX_print(gfx_p, ">", 2 + app_p->cursor, 0);

    // Draw the stars for LED guesses
    if (app_p->redSelected  ) { GFX_print(gfx_p, "*", 2, 8); }
    if (app_p->greenSelected) { GFX_print(gfx_p, "*", 3, 8); }
    if (app_p->blueSelected ) { GFX_print(gfx_p, "*", 4, 8); }
}

/**
 * A helper function which updates the main game screen by redrawing only the
 * positions where the cursor could possibly be updated.
 */
void App_GuessTheColor_updateGameScreen(App_GuessTheColor* app_p, GFX* gfx_p)
{
    // Clear the cursors from any previous game screen
    GFX_print(gfx_p, "  ", 2, 0);
    GFX_print(gfx_p, "  ", 3, 0);
    GFX_print(gfx_p, "  ", 4, 0);
    GFX_print(gfx_p, "  ", 5, 0);

    // Draw the cursor
    GFX_print(gfx_p, ">", 2 + app_p->cursor, 0);

    // Draw the stars for LED guesses
    if (app_p->redSelected  ) { GFX_print(gfx_p, "*", 2, 813); }
    if (app_p->greenSelected) { GFX_print(gfx_p, "*", 3, 8); }
    if (app_p->blueSelected ) { GFX_print(gfx_p, "*", 4, 8); }
}
void App_GuessTheColor_updatePlayScreen(App_GuessTheColor* app_p, GFX* gfx_p)
{
    // Clear the cursors from any previous game screen
    GFX_print(gfx_p, "  ", 2, 0);
    GFX_print(gfx_p, "  ", 3, 0);
    GFX_print(gfx_p, "  ", 4, 0);
    GFX_print(gfx_p, "  ", 5, 0);

    // Draw the cursor
    GFX_print(gfx_p, ">", 2 + app_p->cursor, 0);

    // Draw the stars for LED guesses
    if (app_p->redSelected  ) { GFX_print(gfx_p, "*", 2, 813); }
    if (app_p->greenSelected) { GFX_print(gfx_p, "*", 3, 8); }
    if (app_p->blueSelected ) { GFX_print(gfx_p, "*", 4, 8); }
}

/**
 * A helper function which clears the screen and displays whether the user has
 * won or not.
 */
void App_GuessTheColor_showResultScreen(App_GuessTheColor* app_p, HAL* hal_p)
{
    // Print the splash text
    GFX_clear(&hal_p->gfx);
    GFX_print(&hal_p->gfx, "Result               ", 0, 0);
    GFX_print(&hal_p->gfx, "---------------------", 1, 0);

    // Determine if each selection matched correctly
    bool match = app_p->redSelected   == LED_isLit(&hal_p->boosterpackRed  )
              && app_p->greenSelected == LED_isLit(&hal_p->boosterpackGreen)
              && app_p->blueSelected  == LED_isLit(&hal_p->boosterpackBlue );

    // Print the correct string based on if the user won or not
    if (match) {
        GFX_print(&hal_p->gfx, "Right!", 2, 0);
        Graphics_drawImage((const Graphics_Context *)&hal_p->gfx, &happy8BPP_UNCOMP, 60, 60);
    }
    else {
        GFX_print(&hal_p->gfx, "Wrong!", 2, 0);
        Graphics_drawImage((const Graphics_Context *)&hal_p->gfx, &sad8BPP_UNCOMP, 60, 60);


    }
}
