#include <FastLED.h> //Library for WS2812 LEDS
#define LED_PIN 11 //WS2812 pin
#define NUM_LEDS 8 //number of LEDs
#define SHIFT_PIN 5 //Shift button
#define TURN1_PIN 4 //1st button
#define TURN2_PIN 3 //2nd button
#define TURN3_PIN 2 //3rd button
CRGB leds[NUM_LEDS]; //object to control LEDs
byte state[NUM_LEDS] = {0, 0, 0, 0, 0, 0, 0, 0}; //Array with colours of the LEDs
byte mask[] = {0b00001111, 0b00110011, 0b01010101}; //Three binary masks for three buttons
CRGB colors[] = {CRGB(0, 0, 0), CRGB(25, 0, 0), CRGB(25, 7, 0), CRGB(25, 22, 0), CRGB(0, 25, 0), CRGB(0, 25, 25), CRGB(0, 0, 25), CRGB(15, 0, 21)};//game colours
byte level; //number of current "world" or level
byte sublevel; //number of current sublevel
byte animationFrame; //number of animation frame (used for some visual effects)
bool btn1_pressed, btn2_pressed, btn3_pressed, shift_pressed; //state of each button
byte animationMask = 0; //binary mask used to animate selected LEDS (may be one of three primary masks or inverted primary masks)

void startLevel();//start next sublevel (the actual function is further)

//Initialization
void setup() {
  //Initializing buttons
  pinMode(SHIFT_PIN, INPUT_PULLUP);
  pinMode(TURN1_PIN, INPUT_PULLUP);
  pinMode(TURN2_PIN, INPUT_PULLUP);
  pinMode(TURN3_PIN, INPUT_PULLUP);
  pinMode(13, OUTPUT); //You don't really need it. Just for debug purposes
  pinMode(LED_PIN, OUTPUT); //Initialize LEDs pin
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS); //Initialize WS2812
  //Reset level and sublevel
  level = 1;
  sublevel = 1;
  animationFrame = 0; //Reset animation
  randomSeed(analogRead(0));//Initializing RNG
  startLevel();//Now start the level
}

//Update LEDs
void update()
{
  animationFrame++;
  if (animationFrame > 100)
    animationFrame = 0;
  for (byte i = 0; i<NUM_LEDS; i++) //Loop over all the LEDs
    if(animationMask & 1<<i) //if this LED is highlighted (user pressed but not released the button)
      leds[i] = CRGB(colors[state[i]].r*(animationFrame)/50,colors[state[i]].g*(animationFrame)/50,colors[state[i]].b*(animationFrame)/50); //highlighted diode has "pulsing" colour
    else
      leds[i] = colors[state[i]]; //non-selected LED has regular colour
  FastLED.show(); //Update LEDs
}

//What happens when you finished the game (magic)
void gameCompleted()
{
  for(;;)
    for (animationFrame = 0; animationFrame<8; animationFrame++){
      for (byte i = 0; i<NUM_LEDS; i++)
        leds[i] = colors[(i+animationFrame)%7+1];
      FastLED.show();
      delay(40);
    }
}

//Animation when a sublevel is completed
void winAnimation()
{
  //Make all leds blink in world colour several times
  for (animationFrame = 0; animationFrame<255; animationFrame++){
    for (byte i = 0; i<NUM_LEDS; i++)
      leds[i] = CRGB(colors[level].r*(animationFrame%20)/10,colors[level].g*(animationFrame%20)/10,colors[level].b*(animationFrame%20)/10);
    FastLED.show();
    delay(5);
  }
}

//Check if a sublevel is completed
void checkWin()
{
  bool win = true;
  for(byte i = 0; i<NUM_LEDS; i++) //Check all LEDs
    if (state[i]!=level) //If any of them has incorrect colour
      win = false; //then continue playing
  if (win){ //If player finished the sublevel
    winAnimation(); //Show animation
    sublevel++; //and go to next sublevel
    if (sublevel > 8){ //If the world is completed
      sublevel = 1; //reset the sublevel
      level++; //and go to next level (world)
        if (level==8) //If the game is completed
          gameCompleted();
    }
    startLevel();
  }
}

//Make one turn (checkwin should be false if this function is used for colours shuffling at the begining of sublevel)
void turn(byte mask, bool shift, bool checkwin=true)
{
  for (byte i = 0; i < NUM_LEDS; i++) //Loop over all the LEDs
    if ( ( (mask & 1<<i) && (!shift) ) || ( (~mask & 1<<i) && (shift) ) ){ //If LED corresponds the mask
        state[i]++; //switch it's colour to the next one
        if (state[i]>level) //if it's the last colour in this world
          state[i] = 0; //reset to blank colour
      }
  if (checkwin)
    checkWin(); //Check if the sublevel is completed
}

//Shuffle the colours at the beginning of sublevel
void shuffle(byte count)
{
  for (byte i = 0; i < count; i++){
    turn(mask[random(3)],random(5),false); //Choose rangom mask and Shift modifier (5 makes Shift modifier probability asymmetric, it makes previous turn undo less probable)
    update();//Update LEDs
    delay(20);//Delay to make user see shuffle process
  }
  //Chech if all LEDs are accidentally same-coloured
  bool win = true; 
  for(byte i = 0; i<NUM_LEDS; i++)
    if (state[i]!=level)
      win = false;
  if(win){ //If so, make another shuffle step to prevent auto-win
    turn(mask[random(3)],random(5),false);
    update();
  }
    
}

//Staret a sublevel
void startLevel()
{
  //Animation which shows world progress
  for (animationFrame = 0; animationFrame<255; animationFrame++){
    for (byte i = 0; i<NUM_LEDS; i++)
      leds[i] = (i+1<=sublevel) ? CRGB(colors[level].r*(animationFrame%10)/10,colors[level].g*(animationFrame%10)/10,colors[level].b*(animationFrame%10)/10) : colors[0];
    FastLED.show();
    delay(5);
  }
  //Reset LEDs state
  for (byte i = 0; i < NUM_LEDS; i++)
    state[i] = 0;
  shuffle ((level+sublevel)<<2); //Shuffle colours (<<2 means *2)
  //Reset button states
  btn1_pressed = false;
  btn2_pressed = false;
  btn3_pressed = false;
  shift_pressed = false;
  //Reset animation
  animationMask = 0;
  animationFrame = 0;
}

//Main loop
void loop() {
  if(digitalRead(SHIFT_PIN)==LOW) //If button Shift is pressed
    shift_pressed = true; //Turn on the flag
  else 
    shift_pressed = false; //Turn off the flag
  if(digitalRead(TURN1_PIN)==LOW){ //If button 1 is pressed
    btn1_pressed = true; //Turn on the flag
    animationMask = (shift_pressed ? ~mask[0] : mask[0]); //Calculate current mask
  }
  else{
    if (btn1_pressed){
      turn(mask[0],shift_pressed);
      animationMask &= (shift_pressed ? mask[0] :~mask[0]); //Calculate current mask
    }
    btn1_pressed = false; //Turn off the flag
  }
  if(digitalRead(TURN2_PIN)==LOW){ //If button 2 is pressed
    btn2_pressed = true; //Turn on the flag
    animationMask = (shift_pressed ? ~mask[1] : mask[1]); //Calculate current mask
  }
  else{
    if (btn2_pressed){
      turn(mask[1],shift_pressed);
      animationMask &= (shift_pressed ? mask[1] :~mask[1]); //Calculate current mask
    }
    btn2_pressed = false; //Turn off the flag
  }
  if(digitalRead(TURN3_PIN)==LOW){ //If button 3 is pressed
    btn3_pressed = true; //Turn on the flag
    animationMask = (shift_pressed ? ~mask[2] : mask[2]); //Calculate current mask
  }
  else{
    if (btn3_pressed){
      turn(mask[2],shift_pressed);
      animationMask &= (shift_pressed ? mask[2] :~mask[2]); //Calculate current mask
    }
    btn3_pressed = false; //Turn off the flag
  }
  update();
}
