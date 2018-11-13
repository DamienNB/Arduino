/*
 * Copyright 2018 Damien Bobrek
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to 
 * deal in the Software without restriction, including without limitation the 
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or 
 * sell copies of the Software, and to permit persons to whom the Software is 
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in 
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
*/


// Code for LCD configuration modified from code found at
// http://learning.grobotronics.com/2013/07/controlling-lcd-displays-with-the-hitachi-hd44780-driver/ 
// Can also be found on Github at
// https://gist.github.com/grobotronics/6062488
/*
  The circuit:
 * LCD RS pin to digital pin 12
 * LCD Enable pin to digital pin 11
 * LCD D4 pin to digital pin 9
 * LCD D5 pin to digital pin 8
 * LCD D6 pin to digital pin 7
 * LCD D7 pin to digital pin 6
 * LCD R/W pin to ground
 * 10K potentiometer:
 * ends to +5V and ground
 * wiper to LCD VO pin (pin 3)
 */
// include the library code:
#include <LiquidCrystal.h>
// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 9, 8, 7, 6);


// Code for Joystick configuration modified from code found at
// https://www.brainy-bits.com/arduino-joystick-tutorial/
// Arduino pin numbers
const int SW_pin = 2; // digital pin connected to switch output
const int X_pin = 0; // analog pin connected to X output
const int Y_pin = 1; // analog pin connected to Y output


// Code for 7-segment display modified from code found at
// https://docs.labs.mediatek.com/resource/linkit7697-arduino/en/tutorial/driving-7-segment-displays-with-74hc595
//
// Use one 74HC595 to control a common-anode seven-segment display
//

// pin 11 of 74HC595 (SHCP)
const int bit_clock_pin = 5;
// pin 12 of 74HC595 (STCP)
const int digit_clock_pin = 4;
// pin 14 of 74HC595 (DS)
const int data_pin = 3;

// digit pattern for a 7-segment display
const byte digit_pattern[16] =
{
  B00111111,  // 0
  B00000110,  // 1
  B01011011,  // 2
  B01001111,  // 3
  B01100110,  // 4
  B01101101,  // 5
  B01111101,  // 6
  B00000111,  // 7
  B01111111,  // 8
  B01101111,  // 9
  B01110111,  // A
  B01111100,  // b
  B00111001,  // C
  B01011110,  // d
  B01111001,  // E
  B01110001   // F
};

void update_one_digit(int data)
{
  int i;
  byte pattern;
  
  // get the digit pattern to be updated
  pattern = digit_pattern[data];

  // turn off the output of 74HC595
  digitalWrite(digit_clock_pin, LOW);
  
  // update data pattern to be outputed from 74HC595
  // because it's a common anode LED, the pattern needs to be inverted
  shiftOut(data_pin, bit_clock_pin, MSBFIRST, pattern);
  
  // turn on the output of 74HC595
  digitalWrite(digit_clock_pin, HIGH);
}



// Joystick poll code
typedef enum {no_action = 0, button_pressed, up, upright, right, downright, 
              down, downleft, left, upleft} command_t;

int poll_joystick()
{
  if (digitalRead(SW_pin) == 0)
    return button_pressed;

  unsigned int x_val = analogRead(X_pin);
  unsigned int y_val = analogRead(Y_pin);

  if(y_val < 23)
  {
    if(x_val < 23)
      return upleft;
    else if (x_val > 1000)
      return upright;

    return up; // if there's no serious x-axis tilt, just return the y-axis one
  }
  else if (y_val > 1000)
  {
    if(x_val < 23)
      return downleft;
    else if (x_val > 1000)
      return downright;

    return down; // if there's no serious x-axis tilt, just return the y-axis one
  }
  
  if(x_val < 23)
    return left;
  else if (x_val > 1000)
    return right;

  return no_action;
}

void setup()
{
  // LCD 
  // set up the LCD's number of columns and rows: 
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("hello, world!");

  // Joystick setup
  pinMode(SW_pin, INPUT);
  digitalWrite(SW_pin, HIGH);
  Serial.begin(115200); // used for sending serial data to the Arduino IDE
  
  // 7-segment display setup
  pinMode(data_pin, OUTPUT);
  pinMode(bit_clock_pin, OUTPUT);
  pinMode(digit_clock_pin, OUTPUT);  
}

void loop()
{
  command_t jc = (command_t)poll_joystick();

  if ( jc == button_pressed )
    Serial.print("Switch: Pressed\n");
  else
    Serial.print("Switch: Not Pressed\n");

  if ( jc==left | jc==upleft | jc==downleft ) 
    Serial.print("X-axis: Left\n");
  else if ( jc==right | jc==upright | jc==downright )
    Serial.print("X-axis: Right\n");
  else
    Serial.print("X-axis: None\n");
  
  if ( jc==up | jc==upleft | jc==upright ) 
    Serial.print("Y-axis: Up\n");
  else if ( jc==down | jc==downleft | jc==downright )
    Serial.print("Y-axis: Down\n");
  else
    Serial.print("Y-axis: None\n");
  
  Serial.print("\n\n");

  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
  lcd.setCursor(0, 1);
  // print the number of seconds since reset:
  lcd.print(millis()/1000);
  
  int i;
  unsigned int digit_base;

  static unsigned char counter = 0;
  
  counter++;
  
  digit_base = 16;

  // get the value to be displayed and update one digit
  update_one_digit(counter % digit_base);
  
  delay(500);
}
