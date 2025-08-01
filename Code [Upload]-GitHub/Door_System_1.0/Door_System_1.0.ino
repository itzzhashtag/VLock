/*
  Hello This is me Hashtag .....
  This is my Personal Home Automation Work,
  I use this project in my room to auto lock my door or open it..!
  I added 3 Buttons in 3 different places in my room and one one the Door handel which opens the door.
  There is a Display outside to type in Passcode and (Secret Passcode) to Enter Room or change Current Passcode, with Display Light toggle option.
  I also added a Safety Feature where in case the Keypad is malfunctioning the door can open directly form Secret Button.

  Name: Aniket Chowdhury [Hashtag]
  Email: micro.aniket@gmail.com
  GitHub: https://github.com/itzzhashtag
  Instagram: https://instagram.com/itzz_hashtag
  LinkedIn: https://www.linkedin.com/in/itzz-hashtag/
*/

// For taking Key Data from Other system (Keypad in Nano Board) use Keypad_Nano_v1.0

//===============================================
// --- Libraries Used ---
//===============================================
#include <Servo.h>                  // For controlling the servo motor (used for door mechanism)
#include <LiquidCrystal_I2C.h>      // For interfacing with I2C LCD display (20x4)
#include <SoftwareSerial.h>         // For creating a secondary serial communication port
//===============================================
// --- Matrix Pins ---
//===============================================
SoftwareSerial nanoSerial(2, 3);    // Nano-RX to D2, TX to D3
LiquidCrystal_I2C lcd(0x27, 20, 4); // Set the LCD I2C address to 0x27, and define 20 columns and 4 rows
//===============================================
// --- IO Pins ---
//===============================================
const int LP1 = A3;                 // Red LED
const int LP2 = A2;                 // Green LED
const int LP3 = A1;                 // Blue LED (buzzer indicator)
const int LP4 = 8;                  // Button LED
const int buttonPin = 10;           // Door toggle button pin
const int hingeSwitchPin = 5;       // Hinge safety switch pin
const int buzzerPin = 12;           // Buzzer pin
const int srPin = 9;                // Servo motor pin
const int EMGPin = 4;               // Emergency Button pin
//===============================================
// --- Global State Variables ---
//===============================================
uint8_t passIndex = 0;                              // Index for current position in entered passcode
char mappedKey;                                     // Last key pressed from the keypad
char enteredPasscode[7] = "";                       // store entered passcode
const char passcode[] = "123456";                   // Default system passcode (can be changed later)
const int DoorOpen = 95, DoorClose = 180;           // Servo positions for door open and close (degrees)
bool servoState = false;                            // Keeps track of door lock/unlock state (true = unlocked)
bool isUnlocking = false;                           // Flag indicating if door is currently being unlocked
unsigned long startTime;                            // Used to measure time durations for servo unlock timeout or UI
unsigned long passcodeEntryStartTime = 0;           // Used to track time since user started entering passcode (for timeout)
int f1 = 0, f2 = 0;                                 // Generic flags (used for Print control, logic states, etc.)
int retryCount = 0;                                 // Number of incorrect passcode attempts
unsigned long holdTime1 = 300, holdTime2 = 10000 ;  // Hold durations for button functions or double press (ms)
Servo myServo;                                      // Create a Servo object to control the door lock

//===============================================
// --- Setup ---
//===============================================
void setup()
{
  Serial.begin(9600);                               // Initialize Serial Monitor for debugging (USB connection)
  nanoSerial.begin(9600);                           // Initialize communication with Nano (likely using SoftwareSerial)
  Serial.println(F("✅ Initiated : Serial baud to 9600"));
  Serial.println(F("✅ Initiated : NanoSerial baud to 9600"));
  // --------------------------------------------------
  // Input Pin Configurations
  // --------------------------------------------------
  pinMode(buttonPin, INPUT_PULLUP);                 // Button to unlock/trigger (pulled HIGH when idle)
  pinMode(hingeSwitchPin, INPUT_PULLUP);            // Detects if the door is physically closed
  pinMode(EMGPin, INPUT_PULLUP);                    // Emergency Button Pin
  pinMode(buzzerPin, OUTPUT);                       // Buzzer for feedback sounds
  pinMode(13, OUTPUT);                              // Built-in LED on many boards (used as general indicator)
  digitalWrite(13, LOW);                            // Turn off built-in LED
  pinMode(LP1, OUTPUT);                             // Red LED (e.g., Error or Lock status)
  pinMode(LP2, OUTPUT);                             // Green LED (Success or Ready)
  pinMode(LP3, OUTPUT);                             // Blue LED (Optional use)
  pinMode(LP4, OUTPUT);                             // Button LED (illuminates button or keypad area)
  Serial.println("✅ Initiated : Input Pins");
  delay(150);

  // Initial LED states
  digitalWrite(LP2, HIGH);   // Green LED ON
  digitalWrite(LP4, HIGH);   // Button LED ON

  // --------------------------------------------------
  // Servo Initialization
  // --------------------------------------------------
  myServo.attach(srPin);                            // Attach the servo to its control pin
  myServo.write(DoorOpen);
  Serial.println("✅ Initiated : Servo Pins");
  delay(150);
  // --------------------------------------------------
  // LCD Initialization
  // --------------------------------------------------
  lcd.init();                                       // Initialize the LCD display
  lcd.backlight();                                  // Turn on LCD backlight
  lcd.setCursor(0, 0);                              // Set cursor to top-left
  delay(150);
  Serial.println("✅ Initiated : Display ");
  delay(150);
  // --------------------------------------------------
  // Startup Display & Status
  // --------------------------------------------------
  startup();                                        // Custom startup function (welcome screen, logo, etc.)
}

//===============================================
// --- Main Loop ---
//===============================================
void loop()
{
  DoorToggler();                                    // Function to check for external door open request or button press
  keypad();                                         // Function to read input from keypad and handle passcode logic
}

//===============================================
// --- Keypad Input System ---
//===============================================
void keypad()
{
  // Auto-exit passcode entry after 1 minute timeout
  if (isUnlocking && millis() - passcodeEntryStartTime > 60000)
  {
    Serial.println("⏳ Passcode entry timeout. Returning to home screen.");
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print(F("Timed Out! Back Home"));
    warningBuzz(2, 1000, 100);
    delay(2000);
    isUnlocking = false;
    passIndex = 0;
    enteredPasscode[0] = '\0';
    mappedKey = ' ';
    homescreen();
  }
  // Button override during passcode entry only
  if (isUnlocking && (debounceButton(buttonPin, holdTime1)))
  {
    Serial.println("🛑 BUTTON override detected! Opening door directly...");
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print(F("Override : Door Open"));
    delay(1000);
    isUnlocking = false;
    passIndex = 0;
    enteredPasscode[0] = '\0';
    mappedKey = ' ';
    homescreen();
    door_open();
  }
  // Handle key input
  keygen();
  // If unlocking state → restrict keypad behavior
  if (isUnlocking)
  {
    if (mappedKey == 'C')
    {
      /* allow C */
    }
    else if (mappedKey == 'A' || mappedKey == 'B' || mappedKey == 'D')
    {
      mappedKey = ' '; // ignore other keys
      return;
    }
    else
    {
      unlocking();
      return;
    }
  }
  // Normal keypad actions
  switch (mappedKey)
  {
    case 'A': // Lock Door
      if (!servoState)
      {
        Serial.println("🚪🔒 Door is already closed!");
        lcd.clear();
        lcd.print(F("===================="));
        lcd.setCursor(0, 1);
        lcd.print(F(" Door Stats : Close "));
        lcd.setCursor(0, 2);
        lcd.print(F("===================="));
        warningBuzz(1, 1000, 100);
        homescreen();
      }
      else
      {
        Serial.println("🚪🔒 Locking door...");
        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print(F("   Locking Door...  "));
        door_close();
      }
      mappedKey = ' ';
      break;

    case 'B': // Unlock Request → Enter passcode
      if (servoState)
      {
        Serial.println("🚪🔓 Door is already open!");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(F("===================="));
        lcd.setCursor(0, 1);
        lcd.print(F(" Door Stats : Open! "));
        lcd.setCursor(0, 2);
        lcd.print(F("===================="));
        warningBuzz(1, 1000, 100);
        homescreen();
      }
      else
      {
        Serial.println("⚙ Unlock requested. Enter passcode:");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(F("===================="));
        lcd.setCursor(0, 1);
        lcd.print(F("  ----Passcode----  "));
        lcd.setCursor(0, 2);
        lcd.print(F("       ______       "));
        lcd.setCursor(0, 3);
        lcd.print(F("===================="));
        warningBuzz(1, 1500, 50);
        isUnlocking = true;
        passcodeEntryStartTime = millis();  // <<--- start 1 minute timer
        passIndex = 0;
        enteredPasscode[0] = '\0';
      }
      mappedKey = ' ';
      break;

    case 'C': // Toggle LCD backlight
      f1 = !f1;
      if (f1 == 1) lcd.noBacklight();
      else lcd.backlight();
      warningBuzz(1, 500, 100);
      delay(600);
      mappedKey = ' ';
      break;

    case 'D': // System restart (if door closed)
      if (f2 == 1)
      {
        Serial.println("♻ RESTARTING The System....!!");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(F("===================="));
        lcd.setCursor(0, 1);
        lcd.print(F(" Restarting System! "));
        lcd.setCursor(0, 2);
        lcd.print(F("     Please Wait    "));
        lcd.setCursor(0, 3);
        lcd.print(F("===================="));
        RestartBuzz();
        asm volatile ("jmp 0"); // AVR Soft reset
      }
      else
      {
        Serial.println("⚠ RESTART Error... Door is Closed..!");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(F("===================="));
        lcd.setCursor(0, 1);
        lcd.print(F("  Resetting Error!  "));
        lcd.setCursor(0, 2);
        lcd.print(F("Open The Door First!"));
        lcd.setCursor(0, 3);
        lcd.print(F("===================="));
        warningBuzz(2, 2000, 250);
        delay(1000);
        homescreen();
      }
      mappedKey = ' ';
      break;
  }
}

//===============================================
// --- Passcode Unlocking ---
//===============================================
void unlocking()
{
  if (mappedKey != ' ' && mappedKey != 'A' && mappedKey != 'B' && mappedKey != 'C' && mappedKey != 'D')
  {
    if (passIndex < 6)
    {
      enteredPasscode[passIndex++] = mappedKey;
      enteredPasscode[passIndex] = '\0';
      warningBuzz(1, 1000, 50);
      Serial.print("🔢 Passcode so far: ");
      Serial.println(enteredPasscode);
      // Update LCD display
      displayPasscodeProgress();
    }
    mappedKey = ' ';
    if (passIndex >= 6)
    {
      if (strcmp(enteredPasscode, passcode) == 0)
      {
        Serial.println("✅ ACCESS GRANTED!");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(F("--------------------"));
        lcd.setCursor(0, 1);
        lcd.print(F("   Access Granted!  "));
        lcd.setCursor(0, 2);
        lcd.print(F("--------------------"));
        warningBuzz(2, 2500, 250);
        door_open();
      }
      else
      {
        Serial.println("❌ ACCESS DENIED!");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(F("--------------------"));
        lcd.setCursor(0, 1);
        lcd.print(F("   Access Denied!   "));
        lcd.setCursor(0, 2);
        lcd.print(F("--------------------"));
        warningBuzz(2, 1500, 250);
        homescreen();
      }

      isUnlocking = false;
      passIndex = 0;
      enteredPasscode[0] = '\0';
      mappedKey = ' ';
    }
  }
}

//===============================================
// --- Keypad Hardware Events ---
//===============================================
void keygen()
{
  /*
  if (nanoSerial.available())
  {
    char incoming = nanoSerial.read();
  */
  if (Serial.available()) 
  {
    char incoming = Serial.read();
    if ((incoming >= 'A' && incoming <= 'D') || (incoming >= '0' && incoming <= '9') || incoming == 'C')
    {
      mappedKey = incoming;
      Serial.print("📥 From Nano: ");
      Serial.println(mappedKey);
    }
  }
}
//===============================================
// --- Door Toggle via Button ---
//===============================================
void DoorToggler()
{
  if (debounceButton(buttonPin, holdTime1) || debounceButton(EMGPin, holdTime2))
  {
    for (int i = 0; i < 3; i++)
    {
      digitalWrite(LP4, LOW);
      delay(100);
      digitalWrite(LP4, HIGH);
      delay(100);
    }
    servoState = !servoState;
    if (servoState)
      door_open();
    else
      door_close();
  }
}

//===============================================
// --- Door Control ---
//===============================================
void door_open()
{
  f2 = 1;
  servoState = true;
  myServo.write(DoorOpen);
  delay(1000);
  Serial.println("🚪🔓 Door Opening...");
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print(F("   Door Opening...  "));
  digitalWrite(LP1, LOW);
  digitalWrite(LP2, HIGH);
  warningBuzz(1, 800, 100);
  warningBuzz(1, 1200, 150);
  homescreen();
}

void door_close()
{
  f2 = 0;
  servoState = false;
  myServo.write(DoorClose);
  delay(1000);
  Serial.println("🚪🔒 Door Closing...");
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print(F("  Door Closing...."));
  digitalWrite(LP1, HIGH);
  digitalWrite(LP2, LOW);
  warningBuzz(1, 1200, 100);
  warningBuzz(1, 800, 150);
  CheckDoorSafety();
  startTime = millis();
  homescreen();
}

void CheckDoorSafety()
{
  retryCount = 0;
  bool doorClosed = false;
  unsigned long hingeActivatedTime = 0;

  while (retryCount < 3)
  {
    unsigned long waitStart = millis();
    doorClosed = false;
    while (millis() - waitStart < 800)
    {
      digitalWrite(LP2, LOW);
      if (digitalRead(hingeSwitchPin) == LOW)
      {
        if (hingeActivatedTime == 0)
          hingeActivatedTime = millis();

        if (millis() - hingeActivatedTime > 1000)
        {
          Serial.println("✅🔒 Door locked successfully... Returning to idle.");
          lcd.clear();
          lcd.setCursor(0, 1);
          lcd.print(F("    Door Locked !   "));
          digitalWrite(LP2, LOW);
          digitalWrite(LP1, HIGH);
          servoState = false;
          warningBuzz(2, 800, 150);
          return;
        }
      }
      else
      {
        hingeActivatedTime = 0;
      }
    }

    if (digitalRead(hingeSwitchPin) == HIGH)
    {
      Serial.println("⚠ WARNING! Door did not close properly.");
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print(F("Warning! Door Issue"));
      warningBuzz(3, 900, 100);
      retryCount++;

      if (retryCount < 3)
      {
        Serial.print("♻ RETRY Attempt: ");
        Serial.println(retryCount);
        myServo.write(DoorOpen);
        delay(1000);
        myServo.write(DoorClose);
        delay(1000);
      }
      else
      {
        Serial.println("❌🔒 Maximum retries reached! Reopening door...");
        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print(F(" Max Retries! OPEN "));
        myServo.write(DoorOpen);
        delay(1000);
        warningBuzz(1, 800, 100);
        warningBuzz(1, 1200, 150);
        digitalWrite(LP1, LOW);
        digitalWrite(LP2, HIGH);
        servoState = true;
        return;
      }
    }
  }
}
//===============================================
// --- Debounce Function for Button ---
//===============================================
bool debounceButton(int pin, int holdTime) {
  static unsigned long pressStartTimes[2] = {0};
  static bool longPressStates[2] = {false};
  static unsigned long lastBuzzTimes[2] = {0};  // For controlling buzz interval

  int index = (pin == EMGPin) ? 1 : 0;  // Use index 1 for EMGPin, 0 for buttonPin

  if (digitalRead(pin) == LOW) {
    // Buzz only for EMGPin while held
    if (pin == EMGPin) {
      unsigned long now = millis();
      if (now - lastBuzzTimes[index] >= 300) { // Buzz every 300ms
        tone(buzzerPin, 1000, 100);            // 1kHz tone for 100ms
        lastBuzzTimes[index] = now;
      }
    }

    if (!longPressStates[index]) {
      if (pressStartTimes[index] == 0) {
        pressStartTimes[index] = millis();
      } else if (millis() - pressStartTimes[index] >= holdTime) {
        longPressStates[index] = true;
        return true;
      }
    }
  } else {
    pressStartTimes[index] = 0;
    longPressStates[index] = false;
    if (pin == EMGPin) {
      noTone(buzzerPin);  // Stop buzzer when EMGPin is released
    }
  }

  return false;
}

//===============================================
// --- Starting Boot Display ---
//===============================================
void homescreen()
{
  delay(1000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("-------VLock+-------"));
  lcd.setCursor(0, 1);
  lcd.print(F("A - Lock  B - Unlock"));
  lcd.setCursor(0, 2);
  lcd.print(F("C - DpLgt D - AReset"));
  lcd.setCursor(0, 3);
  lcd.print(F("--------v1.0--------"));
}
void startup()
{
  lcd.setCursor(0, 1);
  lcd.print(F(" !.System Booting.! "));
  lcd.setCursor(0, 2);
  lcd.print(F("    Please Wait!    "));

  Serial.println("🎵 BUZZ Playing startup melody...");
  // Now play the melody using warningBuzz
  warningBuzz(1, 330, 150); delay(50); // E4
  warningBuzz(2, 392, 100); delay(50); // G4
  warningBuzz(1, 659, 150); delay(50); // E5
  warningBuzz(2, 523, 100); delay(50); // C5
  warningBuzz(1, 587, 150); delay(50); // D5
  warningBuzz(1, 784, 500); delay(50); // G5
  warningBuzz(1, 785, 200); delay(50); // G5
  delay(1000);
  door_open();
  Serial.println("Door Open...");
  delay(2000);
}
void displayPasscodeProgress()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("===================="));
  lcd.setCursor(0, 1);
  lcd.print(F("  ----Passcode----  "));
  lcd.setCursor(7, 2);
  for (int i = 0; i < 6; i++)
  {
    if (i < passIndex)
      lcd.print('*');
    else
      lcd.print('_');
  }

  lcd.setCursor(0, 3);
  lcd.print(F("===================="));
}

//===============================================
// --- Warning Buzzer ---
//===============================================
void warningBuzz(int times, int pitch, int duration)
{
  for (int i = 0; i < times; i++)
  {
    digitalWrite(LP3, HIGH);
    tone(buzzerPin, pitch, duration);
    delay(duration + 100);
    digitalWrite(LP3, LOW);
  }
}
void RestartBuzz()
{
  warningBuzz(2, 3000, 150);
  warningBuzz(2, 2500, 150);
  warningBuzz(2, 2000, 150);
  warningBuzz(2, 1500, 150);
  warningBuzz(2, 1000, 150);
  warningBuzz(2, 500, 150);
  warningBuzz(1, 500, 50);
  warningBuzz(1, 1000, 50);
  warningBuzz(1, 1500, 50);
  warningBuzz(1, 2000, 50);
  warningBuzz(1, 2500, 50);
  warningBuzz(1, 3000, 50);
  delay(2000);
}
//===============================================
// --- The End ---
//===============================================



