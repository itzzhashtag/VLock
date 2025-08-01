#include <Keypad.h>
#include <SoftwareSerial.h>
// Serial to UNO (Nano TX = D3 → UNO RX = D2)
SoftwareSerial mySerial(2, 3); // Nano TX (D3), RX (D2, unused)
// Keypad setup
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = 
{
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {11, 10, 9, 8};       // connect to keypad rows
byte colPins[COLS] = {7, 6, 5, 4};     // connect to keypad cols
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
void setup() 
{
  Serial.begin(9600);     // For debugging
  mySerial.begin(9600);   // To UNO
}
void loop() 
{
  char key = keypad.getKey();
  if (key) 
  {
    Serial.print("Pressed: ");  // ← Only for USB debugging
    Serial.println(key);
    mySerial.write(key);  // ← ONLY send raw key char to UNO, nothing else
  }
}
