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




    app.frameIndex = 0;//index for pollen
        app.frameOffset = 60;


        app.frameIndexx = 0;//index for flowers w pollen
        app.frameOffsetx = 40;

        app.frameIndexf = 0;//index for flowers
            app.frameOffsetf = 20;

        app.gfx = GFX_construct(GRAPHICS_COLOR_WHITE, GRAPHICS_COLOR_BLACK);

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
unsigned colormix (unsigned r, unsigned g, unsigned b){
return((r&0xff)<<16)|((g&0xff)<<8)|(b& 0xff);
}

void App_GuessTheColor_loop(App_GuessTheColor* app_p, HAL* hal_p)
{

if (Joystick_isTappedToLeft(&hal_p->joystick)){
LED_toggle(&hal_p->boosterpackBlue);
}
//(hal_p->joystick.x<3000)
 //   LED_turnOn(&hal_p->boosterpackGreen)

    switch (app_p->state)
    {
        case TITLE_SCREEN:
            App_GuessTheColor_handleTitleScreen(app_p, hal_p);
            break;

        case INSTRUCTIONS_SCREEN:
            App_GuessTheColor_handleInstructionsScreen(app_p, hal_p);
            break;

        case MENU_SCREEN:
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
        app_p->state = MENU_SCREEN;
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
    if (Button_isTapped(&hal_p->boosterpackJS))//if joystick button pressed go to menu screen
    {
        // Update internal logical state
        app_p->state = MENU_SCREEN;

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
    if (Button_isTapped(&hal_p->boosterpackJS))//if joystick button pressed go to menu
    {
        // Update internal logical state
        app_p->state = MENU_SCREEN;

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
 * Callback function for when the game is in the MENU_SCREEN state. Used to
 * break down the main App_GuessTheColors_loop() function into smaller
 * sub-functions.
 */
void App_GuessTheColor_handleGameScreen(App_GuessTheColor* app_p, HAL* hal_p)
{
// check the inputs
    // If B2 is pressed, increment the cursor and circle it around to 0 if it
    // reaches the bottom
    if (Joystick_isTappedToDown(&hal_p->joystick)) {
            app_p->cursor = (Cursor) (((int) app_p->cursor + 1) % NUM_TEST_OPTIONS);
            App_GuessTheColor_updateGameScreen(app_p, &hal_p->gfx);
        }
    if (Joystick_isTappedToUp(&hal_p->joystick)) {
              app_p->cursor = (Cursor) (((int) app_p->cursor - 1) % NUM_TEST_OPTIONS);
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


        if (vx < LEFT_THRESHOLD)//if joystick reaches threshold pushed to left turns true
        {
            joyStickPushedtoLeft = true;
        }

        if (vx > RIGHT_THRESHOLD)////if joystick reaches threshold pushed to right turns true
              {
                  joyStickPushedtoRight = true;
              }

        if (vy > UP_THRESHOLD)///if joystick reaches threshold pushed to Up turns true
                {
                    joyStickPushedtoUp = true;
                }

        if (vy < DOWN_THRESHOLD)///if joystick reaches threshold pushed to down turns true
          {
           joyStickPushedtoDown = true;
          }
        static unsigned int x = 63;
        static unsigned int y = 63;

          static unsigned int moveCount = 0;
       static int count3 = 9;
       unsigned char lifeString[6];
       //////////////////clear previous circle instances

               Graphics_setForegroundColor(&app_p->gfx.context,GRAPHICS_COLOR_BLUE );
               Graphics_fillCircle(&app_p->gfx.context,  70, (app_p->frameIndexf + app_p->frameOffsetf)%90, 5);//wipe previous flower circles drawn



               Graphics_setForegroundColor(&app_p->gfx.context,GRAPHICS_COLOR_BLUE );
               Graphics_fillCircle(&app_p->gfx.context,  20, (app_p->frameIndex + app_p->frameOffset)%90,2);//wipe previous pollen circles

               Graphics_setForegroundColor(&app_p->gfx.context,GRAPHICS_COLOR_BLUE );
               Graphics_fillCircle(&app_p->gfx.context,  100, (app_p->frameIndexx + app_p->frameOffsetx)%90, 2);//wipe previous pollen+flower circles

               Graphics_setForegroundColor(&app_p->gfx.context,GRAPHICS_COLOR_BLUE );
               Graphics_fillCircle(&app_p->gfx.context,  101, (app_p->frameIndexx + app_p->frameOffsetx)%90, 5);//wipe previous pollen+flower circles

               ///////////////////////////////adds one to frame index for each new cycle
               app_p->frameIndex++;
               app_p->frameIndexx++;
               app_p->frameIndexf++;

          /////////////////makes circles go back to the top once they reach the bottom
               if (app_p->frameIndex==90)//pollen index
                     {
                         app_p->frameIndex = 0;
                         app_p->frameOffset++;

                         if (app_p->frameOffset==90)
                             app_p->frameOffset = 60;
                     }

                     if (app_p->frameIndexx==90)//index for flower with pollen
                     {
                         app_p->frameIndexx = 0;
                         app_p->frameOffsetx++;

                         if (app_p->frameOffsetx==90)
                             app_p->frameOffsetx = 40;
                     }
                     if (app_p->frameIndexf==90)//index for flower
                          {
                              app_p->frameIndexf = 0;
                              app_p->frameOffsetf++;

                          if (app_p->frameOffsetf==90)
                              app_p->frameOffsetf = 20;

                              snprintf((char *) lifeString, 10, "life %d",count3--);//subtract from life if flower unpollinated touches gorund
                              GFX_print(&app_p->gfx.context, (char*) lifeString, 13, 11);


                          }

///////////////////////////////////////
    //    MoveCircle(&g_sContext,joyStickPushedtoLeft,joyStickPushedtoRight,joyStickPushedtoUp,joyStickPushedtoDown,&hal_p->gfx);

        Graphics_setForegroundColor(&app_p->gfx.context,GRAPHICS_COLOR_PINK );
        Graphics_fillCircle(&app_p->gfx.context,  70, (app_p->frameIndexf + app_p->frameOffsetf)%90, 5);//flower

        Graphics_setForegroundColor(&app_p->gfx.context,GRAPHICS_COLOR_GREEN );
        Graphics_fillCircle(&app_p->gfx.context,  20, (app_p->frameIndex + app_p->frameOffset)%90, 2);//pollen


        Graphics_setForegroundColor(&app_p->gfx.context,GRAPHICS_COLOR_PINK);//flower and pollen
        Graphics_fillCircle(&app_p->gfx.context,  101, (app_p->frameIndexx + app_p->frameOffsetx)%90, 5);
        Graphics_setForegroundColor(&app_p->gfx.context,GRAPHICS_COLOR_GREEN );//flower and pollen
        Graphics_fillCircle(&app_p->gfx.context,  100, (app_p->frameIndexx + app_p->frameOffsetx)%90, 2);

////////////////////////////Yellow circle movement


               if ((joyStickPushedtoLeft && (x>20)) || (joyStickPushedtoRight && (x<110))||(joyStickPushedtoDown && (y<75)) || (joyStickPushedtoUp && (y>45)))
               {

                   Graphics_setForegroundColor(&app_p->gfx.context, GRAPHICS_COLOR_BLUE);

                   Graphics_fillCircle(&app_p->gfx.context, x, y, 10);//get rid of previous circle

                   if (joyStickPushedtoLeft)//if boolean movetoleft is true
                       x = x-10;

                   if(joyStickPushedtoRight)//if boolean movetoright is true
                       x = x+10;

                   if (joyStickPushedtoDown)//if boolean movetodown is true
                      y = y+10;

                   if(joyStickPushedtoUp)//if boolean movetoup is true
                      y = y-10;

                   Graphics_setForegroundColor(&app_p->gfx.context, GRAPHICS_COLOR_YELLOW);//draw new circle in new location
                   Graphics_fillCircle(&app_p->gfx.context, x, y, 10);//draw new circle in new location

                   moveCount++;
                   static int count1 = 0;//displays moves done
                   unsigned char MoveString[6];

                   snprintf((char *) MoveString, 10, "Moves %d",count1++);
                                    GFX_print(&app_p->gfx.context, (char*) MoveString, 12, 11);

               }


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

          app_p->state = MENU_SCREEN;
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
 * MENU_SCREEN state.
 */
void App_GuessTheColor_showPlayScreen(App_GuessTheColor* app_p, GFX* gfx_p,HAL* hal_p)
{
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

    GFX_print(gfx_p, "JS: Select choice    ", 7, 0);
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
