/*
  Hello This is me Hashtag .....
  This is my Personal Home Automation Work,
  This Simply takes in Keypad data in the Nano board (i used) and sends to other Board.

  Name: Aniket Chowdhury [Hashtag]
  Email: micro.aniket@gmail.com
  GitHub: https://github.com/itzzhashtag
  Instagram: https://instagram.com/itzz_hashtag
  LinkedIn: https://www.linkedin.com/in/itzz-hashtag/
*/
//=====================================================
// --- Libraries Used ---
//=====================================================
#include <Keypad.h>                // Include the Keypad library to interface with the 4x4 matrix keypad
#include <SoftwareSerial.h>        // Include SoftwareSerial library to create a virtual serial port

//=====================================================
// --- Serial Creation ---
//=====================================================
SoftwareSerial mySerial(2, 3);     // RX = pin 2 (not used), TX = pin 3 (sends data to UNO)

//=====================================================
// --- Keypad Setup ---
//=====================================================
const byte ROWS = 4;               // Define number of rows in keypad (4)
const byte COLS = 4;               // Define number of columns in keypad (4)
// Define the character mapping of the 4x4 keypad
char keys[ROWS][COLS] = 
{
  {'1', '2', '3', 'A'},            // Row 0
  {'4', '5', '6', 'B'},            // Row 1
  {'7', '8', '9', 'C'},            // Row 2
  {'*', '0', '#', 'D'}             // Row 3
};
byte rowPins[ROWS] = {11, 10, 9, 8};   // Define the Arduino pins connected to the rows of the keypad-> Row 0 to pin 11, Row 1 to pin 10, etc.
byte colPins[COLS] = {7, 6, 5, 4};     // Define the Arduino pins connected to the columns of the keypad Col 0 to pin 7, Col 1 to pin 6, etc.
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);    // Create a Keypad object using the defined keymap and pin connections

//=====================================================
// --- Setup ---
//=====================================================
void setup() 
{
  Serial.begin(9600);            // Start hardware serial monitor for debugging via USB
  mySerial.begin(9600);          // Start software serial communication to UNO
}

//=====================================================
// --- Loop ---
//=====================================================
void loop() 
{
  char key = keypad.getKey();    // Read a key from the keypad (returns 0 if no key is pressed)
  
  if (key)                       // If a valid key is pressed
  {
    Serial.print("Pressed: ");   // Debug output to serial monitor
    Serial.println(key);         // Print the key that was pressed
    mySerial.write(key);         // Send the key character via software serial to UNO
  }
}
//=====================================================
// --- The End ---
//=====================================================
