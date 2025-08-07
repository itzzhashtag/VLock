/*
  Hello This is me Hashtag .....
  This is my Personal Home Automation Work,
  I use this project in my room to auto lock my door or open it..!
  I added 3 Buttons in 3 different places in my room and one one the Door handel which opens the door.
  There is a Display outside to type in Passcode and (Secret Passcode) to Enter Room or change Current Passcode, with Display Light toggle option.
  I also added a Safety Feature where in case the System Frezes or something happens i can Mannually Reset using button hidden in my Door. 

  Name: Aniket Chowdhury [Hashtag]
  Email: micro.aniket@gmail.com
  GitHub: https://github.com/itzzhashtag
  Instagram: https://instagram.com/itzz_hashtag
  LinkedIn: https://www.linkedin.com/in/itzz-hashtag/
*/

// For taking Key Data from Other system (Keypad in Nano Board) use Keypad_Nano_v1.0

//==============================================================================================
// --- Libraries Used ---
//==============================================================================================
#include <Servo.h>                  // For controlling the servo motor (used for door mechanism)
#include <Keypad.h>                 // For reading user input from a matrix keypad
#include <LiquidCrystal_I2C.h>      // For interfacing with I2C LCD display (20x4)
#include <SoftwareSerial.h>         // For creating a secondary serial communication port
#include <EEPROM.h>
//==============================================================================================
// --- Matrix Pins ---
//==============================================================================================

SoftwareSerial nanoSerial(2, 3);    // Nano-RX to D2, TX to D3
LiquidCrystal_I2C lcd(0x27, 20, 4); // Set the LCD I2C address to 0x27, and define 20 columns and 4 rows
//==============================================================================================
// --- IO Pins ---
//==============================================================================================
const int LP1 = A3;                 // Red LED
const int LP2 = A2;                 // Green LED
const int LP3 = A1;                 // Blue LED (buzzer indicator)
const int LP4 = 8;                  // Button LED
const int TriggerPin = 7;           // Pin to control solenoid trigger
const int buttonPin = 10;           // Door toggle button pin
const int hingeSwitchPin = 5;       // Hinge safety switch pin
const int buzzerPin = 12;           // Buzzer pin
const int srPin = 9;                // Servo motor pin
const int TriggerSwitchPin = 4;     // Triger to close door
//==============================================================================================
// --- Global State Variables ---
//==============================================================================================
uint8_t passIndex = 0;                              // Index for current position in entered passcode
char mappedKey;                                     // Last key pressed from the keypad
const char changepasscode[] = "#2002#";             // Secret code to initiate the passcode change process
const int SystemStartPass = "120202";               // Starting Default Passcode
const int PASSCODE_LENGTH = 6;                      // 🔐 Length of the passcode
const int EEPROM_ADDR = 0;                          // 💾 EEPROM starting address for passcode storage
char passcode[PASSCODE_LENGTH + 1];                 // Holds current passcode
char tempNewPasscode[PASSCODE_LENGTH + 1];          // Buffer for new passcode
char enteredPasscode[PASSCODE_LENGTH + 1];          // Buffer for user entry  

bool servoState = false;                            // Keeps track of door lock/unlock state (true = unlocked)
bool isUnlocking = false;                           // Flag indicating if door is currently being unlocked
bool isChangingPasscode = false;                    // Flag indicating user is trying to change the passcode
bool waitingForOldPass = false;                     // Flag for waiting for the old passcode before changing
bool waitingForNewPass = false;                     // Flag for waiting to enter new passcode
bool waitingForConfirmPass = false;                 // Flag for waiting to confirm new passcode
unsigned long startTime;                            // Used to measure time durations for servo unlock timeout or UI
unsigned long passcodeEntryStartTime = 0;           // Used to track time since user started entering passcode (for timeout)
unsigned long holdTime1 = 300, holdTime2 = 1000 ;   // Hold durations for button functions or double press (ms)
int f1 = 0, f2 = 0, f3 = 0, f4 = 0, f5 = 0;         // Generic flags (used for Print control, logic states, etc.)
int c = 0, c2 = 0;                                  // Generic counters (used for logic states, etc.)
int retryCount = 0;                                 // Number of incorrect passcode attempts
const int DoorOpen = 95, DoorClose = 180;           // Servo positions for door open and close (degrees)

const bool RELAY_ON = HIGH;                         // or LOW, depending on your relay
const bool RELAY_OFF = LOW;

unsigned long triggerStartTime = 0;
bool triggerActive = false;
unsigned long triggerDuration = 5000;               // 5 seconds (change if needed)

Servo myServo;                                      // Create a Servo object to control the door lock
//==============================================================================================
// --- Setup ---
//==============================================================================================
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
  pinMode(TriggerSwitchPin, INPUT_PULLUP);          // Optional trigger (external switch)
  pinMode(buzzerPin, OUTPUT);                       // Buzzer for feedback sounds
  pinMode(13, OUTPUT);                              // Built-in LED on many boards (used as general indicator)
  digitalWrite(13, LOW);                            // Turn off built-in LED
  pinMode(LP1, OUTPUT);                             // Red LED (e.g., Error or Lock status)
  pinMode(LP2, OUTPUT);                             // Green LED (Success or Ready)
  pinMode(LP3, OUTPUT);                             // Blue LED (Optional use)
  pinMode(LP4, OUTPUT);                             // Button LED (illuminates button or keypad area)
  pinMode(TriggerPin, OUTPUT);                      // Output to trigger something (e.g., relay, motor, secondary board)
  digitalWrite(LP2, HIGH);                          // Green LED ON (system ready or idle)
  digitalWrite(LP4, HIGH);                          // Button LED ON (highlight button area)
  Serial.println(F("✅ Initiated : Input Pins"));
  delay(150);
  loadOrInitPasscode();                            // 👈 Load saved passcode

  Serial.print("🔐 Current passcode: ");
  Serial.println(passcode);

  // --------------------------------------------------
  // LCD Initialization
  // --------------------------------------------------
  lcd.init();                                       // Initialize the LCD display
  lcd.backlight();                                  // Turn on LCD backlight
  lcd.setCursor(0, 0);                              // Set cursor to top-left
  delay(150);
  Serial.println(F("✅ Initiated : Display "));
  delay(150);

  // --------------------------------------------------
  // Servo Initialization
  // --------------------------------------------------
  myServo.attach(srPin);                            // Attach the servo to its control pin
  Serial.println(F("✅ Initiated : Servo Pins"));
  delay(150);

  // -------------------------
  // Startup Display & Status
  // -------------------------
  startup();                                        // Custom startup function (welcome screen, logo, etc.)
}

//==============================================================================================
// --- Main Loop ---
//==============================================================================================
void loop()
{
  DoorToggler();                                    // Function to check for external door open request or button press
  keypad();                                         // Function to read input from keypad and handle passcode logic
  check_Relay();
}

//==============================================================================================
// --- Accepting Keypad Data from Different System [Nano Board] ---
//==============================================================================================
void keygen()
{
  /*  
  if (Serial.available())                           // Detect Data incomming from Serial Monitor
  {
    char incoming = Serial.read();                  // Read the Data Incomming from Serial 
    
  */
  if (nanoSerial.available())                       // Detect Data incomming from Different Board [Nano]
    {
    char incoming = nanoSerial.read();              // Read the Data Incomming from Board
    if ((incoming >= 'A' && incoming <= 'D') || (incoming >= '0' && incoming <= '9') || incoming == '#' || incoming == '*')
    {
      mappedKey = incoming;
      Serial.print(F("📥 From Nano: "));
      Serial.println(mappedKey);
    }
  }
}

//==============================================================================================
// --- Keypad + Button System with Passcode Entry ---
//==============================================================================================
void keypad()
{
  // ⏳ Auto-exit passcode entry after 1 minute
  if (isUnlocking && millis() - passcodeEntryStartTime > 60000)
  {
    Serial.println(F("⏳ Passcode entry timeout. Returning to home screen."));
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print(F("Timed Out! Back Home"));
    warningBuzz(2, 1000, 100);     // Buzz twice for timeout
    delay(2000);                   // Brief delay before returning home
    isUnlocking = false;           // Exit unlock mode
    passIndex = 0;                 // Reset passcode entry index
    enteredPasscode[0] = '\0';     // Clear passcode buffer
    mappedKey = ' ';               // Clear key
    homescreen();                  // Go back to main screen
  }
  if (isUnlocking && (debounceButton(buttonPin, holdTime1)))  // 🛑 Manual button override while entering passcode
  {
    Serial.println(F("🛑 BUTTON override detected! Opening door directly..."));
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print(F("Override : Door Open"));
    delay(1000);
    isUnlocking = false;           // Cancel passcode entry
    passIndex = 0;
    enteredPasscode[0] = '\0';
    mappedKey = ' ';
    homescreen();                  // Return to home screen
    door_open();                   // Force door open
  }
  keygen();                        // 📥 Check if a key is received via Serial (from Nano)
  if (isUnlocking)                 // Restrict keypad usage during unlocking (passcode entry)
  {
    if (mappedKey == 'C')
    {
      // Allow 'C' to toggle LCD even during passcode
    }
    else if (mappedKey == 'A' || mappedKey == 'B' || mappedKey == 'D')
    {
      mappedKey = ' ';            // Ignore commands that don’t make sense while unlocking
      return;
    }
    else
    {
      unlocking();                // Process digit entry for passcode
      return;
    }
  }
  switch (mappedKey)              // Normal operation for non-unlock modes
  {
    case 'A':                     // 🔒 Lock the door
      if (!servoState)            // Door already closed
      {
        Serial.println(F("🚪🔒 Door is already closed!"));
        lcd.clear();
        lcd.print(F("===================="));
        lcd.setCursor(0, 1);
        lcd.print(F(" Door Stats : Close "));
        lcd.setCursor(0, 2);
        lcd.print(F("===================="));
        warningBuzz(1, 1000, 100); // Single buzz
        homescreen();
      }
      else                         // Door is open → close it
      {
        Serial.println(F("🚪🔒 Locking door..."));
        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print(F("  Shut The Door...  "));
        door_close();             // Close the door (servo + solenoid)
        delay(1000);
        homescreen();
      }
      mappedKey = ' ';            // Reset key
      break;

    case 'B':                     // 🔓 Unlock request → enter passcode
      if (servoState)             // Already open
      {
        Serial.println(F("🚪🔓 Door is already open! But Returying Again"));
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(F("===================="));
        lcd.setCursor(0, 1);
        lcd.print(F(" Door Stats : Open! "));
        lcd.setCursor(0, 2);
        lcd.print(F(" But Retrying Again "));
        lcd.setCursor(0, 3);
        lcd.print(F("===================="));
        warningBuzz(1, 1000, 100); // Single buzz
        delay(1500);
        door_open();
      }
      else                        // Door closed → ask for passcode
      {
        Serial.println(F("⚙ Unlock requested. Enter passcode:"));
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(F("===================="));
        lcd.setCursor(0, 1);
        lcd.print(F("  ----Passcode----  "));
        lcd.setCursor(0, 2);
        lcd.print(F("       ______       "));
        lcd.setCursor(0, 3);
        lcd.print(F("===================="));
        warningBuzz(1, 1500, 50);   // Entry tone
        isUnlocking = true;         // Enter unlock state
        passcodeEntryStartTime = millis();  // Start 1-min timeout
        passIndex = 0;              // Reset entry index
        enteredPasscode[0] = '\0';  // Clear previous input
      }
      mappedKey = ' ';
      break;

    case 'C':                         // 💡 Toggle LCD backlight
      f1 = !f1;
      if (f1 == 1) lcd.noBacklight(); // Turn OFF backlight
      else lcd.backlight();           // Turn ON backlight
      warningBuzz(1, 500, 100);       // Short buzz
      delay(600);                     // Short delay to avoid accidental rapid toggles
      mappedKey = ' ';
      break;

    case 'D': // ♻ System restart
      if (f2 == 1) // Only allow restart when door is OPEN
      {
        Serial.println(F("♻ RESTARTING The System....!!"));
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
        asm volatile ("jmp 0");       // Soft reset using assembly
      }
      else if (f2 == 0)               // Don't allow restart if door is closed
      {
        Serial.println(F("⚠ RESTART Error... Door is Closed..!"));
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(F("===================="));
        lcd.setCursor(0, 1);
        lcd.print(F("  Resetting Error!  "));
        lcd.setCursor(0, 2);
        lcd.print(F("Open The Door First!"));
        lcd.setCursor(0, 3);
        lcd.print(F("===================="));
        warningBuzz(2, 2000, 250);    // Error buzz
        delay(1000);
        homescreen();
      }
      mappedKey = ' ';
      break;
  }
}

//==============================================================================================
// --- Passcode Unlocking System ---
//==============================================================================================
void unlocking()
{
  if (mappedKey != ' ' && mappedKey != 'A' && mappedKey != 'B' && mappedKey != 'C' && mappedKey != 'D')
  {
    if (passIndex < 6)
    {
      enteredPasscode[passIndex++] = mappedKey;
      enteredPasscode[passIndex] = '\0';
      warningBuzz(1, 1000, 50);
      f5 = 0;
      Serial.print(F("🔢 Passcode so far: "));
      Serial.println(enteredPasscode);
      // Update LCD display
      displayPasscodeProgress();
    }
    mappedKey = ' ';
    if (passIndex >= 6)
    {
      if (!isChangingPasscode && strcmp(enteredPasscode, changepasscode) == 0)
      {
        isChangingPasscode = true;
        waitingForOldPass = true;
        Serial.println(F("🔐 Enter OLD Passcode:"));
        f4 = 1; //for printing in passcode printer
        lcd.clear();
        warningBuzz(1, 1500, 100);
        lcd.setCursor(0, 0);
        lcd.print(F("===================="));
        lcd.setCursor(0, 1);
        lcd.print(F(" -- OLD Passcode -- "));
        lcd.setCursor(0, 2);
        lcd.print(F("       ______       "));
        lcd.setCursor(0, 3);
        lcd.print(F("===================="));
        passIndex = 0;
        enteredPasscode[0] = '\0';
        return;
      }

      // Step 2: Enter OLD Passcode
      if (isChangingPasscode && waitingForOldPass)
      {
        if (strcmp(enteredPasscode, passcode) == 0)
        {
          Serial.println(F("✅ Old Pass Verified!"));
          f4 = 2;                     //for printing in passcode printer
          lcd.setCursor(0, 0);
          lcd.clear();
          warningBuzz(1, 1500, 100);
          lcd.print(F("===================="));
          lcd.setCursor(0, 1);
          lcd.print(F(" -- New Passcode -- "));
          lcd.setCursor(0, 2);
          lcd.print(F("       ______       "));
          lcd.setCursor(0, 3);
          lcd.print(F("===================="));

          waitingForOldPass = false;
          waitingForNewPass = true;
          passIndex = 0;
          enteredPasscode[0] = '\0';
          return;
        }
        else
        {
          Serial.println(F("❌ Wrong Old Pass!"));
          lcd.clear();
          lcd.setCursor(0, 1);
          lcd.print(F(" Wrong OLD Passcode "));
          warningBuzz(2, 1000, 150);
          resetPassFlags();
          delay(1000);
          homescreen();
        }
      }

      // Step 3: Enter New Passcode
      if (isChangingPasscode && waitingForNewPass)
      {
        strcpy(tempNewPasscode, enteredPasscode);
        waitingForNewPass = false;
        waitingForConfirmPass = true;
        warningBuzz(1, 1500, 100);
        Serial.println(F("🔁 Confirm New Passcode:"));
        f4 = 3;                               //for printing in passcode printer
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(F("===================="));
        lcd.setCursor(0, 1);
        lcd.print(F("- Confirm Passcode -"));
        lcd.setCursor(0, 2);
        lcd.print(F("       ______       "));
        lcd.setCursor(0, 3);
        lcd.print(F("===================="));

        passIndex = 0;
        enteredPasscode[0] = '\0';
        return;
      }

      // Step 4: Confirm New Passcode
      if (isChangingPasscode && waitingForConfirmPass)
      {
        if (strcmp(tempNewPasscode, enteredPasscode) == 0)
        {
          strcpy(passcode, tempNewPasscode);
          for (int i = 0; i < PASSCODE_LENGTH; i++)         // Save new passcode to EEPROM
          {         
            EEPROM.update(EEPROM_ADDR + i, passcode[i]);    // 👈 Save new passcode
          }
          Serial.print(F("✅ Passcode Changed: "));
          Serial.println(passcode);
          lcd.clear();
          lcd.setCursor(0, 1);
          f5 = 1;
          lcd.print(F(" Passcode Changed! "));
          warningBuzz(2, 1000, 100);
          delay(500);
        }
        else
        {
          Serial.println(F("❌ Mismatch! Try again."));
          lcd.clear();
          lcd.setCursor(0, 1);
          lcd.print(F("Mismatch! Start Over"));
          warningBuzz(3, 1000, 200);
          delay(1000);
          homescreen();
        }
        resetPassFlags();
      }
      if (!isChangingPasscode)
      {
        if (strcmp(enteredPasscode, passcode) == 0)
        {
          Serial.println(F("✅ ACCESS GRANTED!"));
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
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print(F("--------------------"));
          lcd.setCursor(0, 1);
          if (f5 == 1)
          {
            lcd.print(F("   Unlock Again.!   "));
            Serial.println(F("Unlock Again with new Code..!"));
          }
          else
          {

            lcd.print(F("   Access Denied!   "));
            Serial.println(F("❌ ACCESS DENIED!"));

          }
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
}
void resetPassFlags()
{
  isChangingPasscode = false;
  waitingForOldPass = false;
  waitingForNewPass = false;
  waitingForConfirmPass = false;
  passIndex = 0;
  enteredPasscode[0] = '\0';
  mappedKey = ' ';
}
void loadOrInitPasscode() 
{
  bool eepromIsBlank = true;                            //Assume EEPROM is blank initially  
  for (int i = 0; i < PASSCODE_LENGTH; i++)             // 🔍 Step 1: Check if EEPROM contains data or is blank (all bytes are 0xFF)
  {
    if (EEPROM.read(EEPROM_ADDR + i) != 0xFF) 
    {
      eepromIsBlank = false;                            // Found a non-blank byte, so EEPROM is not empty
      break;                                            // No need to check further
    }
  }
  if (eepromIsBlank)                                    // 💾 EEPROM is blank, so store the default system passcode in it
  {
    strcpy(passcode, SystemStartPass);                  // Copy the default passcode string to the working passcode variable
    for (int i = 0; i < PASSCODE_LENGTH; i++)            // Save each character of the default passcode into EEPROM
    {
      EEPROM.update(EEPROM_ADDR + i, passcode[i]);      // Writes only if the value is different
    }
  } 
  else                                                     // 📥 EEPROM has a saved passcode, so load it into memory
  {
    for (int i = 0; i < PASSCODE_LENGTH; i++) 
    {
      passcode[i] = EEPROM.read(EEPROM_ADDR + i);         // Read stored characters from EEPROM
    }
    passcode[PASSCODE_LENGTH] = '\0';                     // Ensure string is null-terminated for safety
  }
}

//==============================================================================================
// --- Door and Button Toggle System ---
//==============================================================================================
void DoorToggler()                    // Check if manual button or trigger switch is pressed
{
  bool buttonTriggered = debounceButton(buttonPin, holdTime1);
  bool doorTriggered   = debounceButton(TriggerSwitchPin, holdTime2) && (f2 == 1);
  if (buttonTriggered || doorTriggered)
  {
    for (int i = 0; i < 3; i++)       // Blink LP4 three times for visual feedback
    {
      digitalWrite(LP4, LOW);
      delay(100);
      digitalWrite(LP4, HIGH);
      delay(100);
    }
    if (doorTriggered)                // --- CASE 1: Trigger Switch Pressed (for normal toggling) ---
    {
      servoState = !servoState;       // Toggle door (from open to close)
      if (servoState) door_open();
      else door_close();
    }
    if (buttonTriggered)              // --- CASE 2: Manual Button Pressed ---
    {
      if (f2 == 1)                    // Door is already open
      {
        if (servoState)               // Just force-open the piston if it's stuck
        {
          servoState = true;
          door_open();
        }
      }
      else                            // Door is closed → toggle open
      {
        servoState = !servoState;
        if (servoState) door_open();
      }
    }
  }
}

//==============================================================================================
// --- Debounce Function for Button ---
//==============================================================================================
bool debounceButton(int pin, int holdTime)
{
  static unsigned long pressStartTimes[2] = {0};
  static bool longPressStates[2] = {false};
  static unsigned long lastBuzzTimes[2] = {0};    // For controlling buzz interval

  int index = (pin == TriggerSwitchPin) ? 1 : 0;  // Use index 1 for TriggerSwitchPin, 0 for buttonPin

  if (digitalRead(pin) == LOW)                    // Buzz only for TriggerSwitchPin while held
  {

    if (pin == TriggerSwitchPin)
    {
      unsigned long now = millis();
      if (now - lastBuzzTimes[index] >= 300)      // Buzz every 300ms
      {
        if (f2 == 1)
        {
          c++;
          if (c <= 2)
            warningBuzz(1, 1000, 100);            // 1kHz tone for 100ms
        }
        lastBuzzTimes[index] = now;
      }
    }
    if (!longPressStates[index])
    {
      if (pressStartTimes[index] == 0)
      {
        pressStartTimes[index] = millis();
      } else if (millis() - pressStartTimes[index] >= holdTime)
      {
        longPressStates[index] = true;
        return true;
      }
    }
  }
  else
  {
    pressStartTimes[index] = 0;
    longPressStates[index] = false;
    if (pin == TriggerSwitchPin) {
      c = 0;                                      // Stop buzzer when TriggerSwitchPin is released
    }
  }
  return false;
}

//==============================================================================================
// --- Door Control ---
//==============================================================================================
void door_open()
{
  f2 = 1;
  Serial.println(F("🚪🔓 Door Opening..."));
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print(F("   Door Opening...  "));                                
  servoState = true;
  digitalWrite(LP1, LOW);
  digitalWrite(LP2, HIGH);
  digitalWrite(TriggerPin, RELAY_ON);    // Start relay (TriggerPin) for limited time
  triggerStartTime = millis();
  triggerActive = true;
  delay(800);
  myServo.write(DoorOpen);
  delay(500);
  warningBuzz(1, 800, 100);
  warningBuzz(1, 1200, 150);
  homescreen();
}

void door_close()
{
  f2 = 0;
  Serial.println(F("🚪🔒 Door Closing..."));
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print(F("  Door Closing...."));
  servoState = false;
  myServo.write(DoorClose);
  delay(1500);
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
          Serial.println(F("✅🔒 Door locked successfully... Returning to idle."));
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
      Serial.println(F("⚠ WARNING! Door did not close properly."));
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print(F("Warning! Door Issue"));
      warningBuzz(3, 900, 100);
      retryCount++;
      if (retryCount < 3)
      {
        Serial.print(F("♻ RETRY Attempt: "));
        Serial.println(retryCount);
        myServo.write(DoorOpen);
        delay(1000);
        myServo.write(DoorClose);
        delay(1000);
      }
      else
      {
        Serial.println(F("❌🔒 Maximum retries reached! Reopening door..."));
        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print(F(" Max Retries! OPEN "));
        f2 = 1;
        myServo.write(DoorOpen);
        delay(1200);
        warningBuzz(1, 800, 100);
        warningBuzz(1, 1200, 150);
        digitalWrite(LP1, LOW);
        digitalWrite(LP2, HIGH);
        servoState = true;
        digitalWrite(TriggerPin, RELAY_ON);
        triggerStartTime = millis();
        triggerActive = true;
        delay(800);
        return;
      }
    }
  }
}

void check_Relay()
{
  if (triggerActive && (millis() - triggerStartTime >= triggerDuration))
  {
    digitalWrite(TriggerPin, RELAY_OFF);  // Relay OFF
    triggerActive = false;
    warningBuzz(1, 1200, 100);
  }
}
//==============================================================================================
// --- Starting, Boot and Display ---
//==============================================================================================
void homescreen()
{
  delay(1000);
  lcd.clear();
  c2 = 0;
  f4 = 0;
  lcd.setCursor(0, 0);
  lcd.print(F("-------VLock+-------"));
  lcd.setCursor(0, 1);
  lcd.print(F("A - Lock  B - Unlock"));
  lcd.setCursor(0, 2);
  lcd.print(F("C - DpLgt D - AReset"));
  lcd.setCursor(0, 3);
  lcd.print(F("--------v3.1--------"));
}

void startup()
{
  lcd.setCursor(0, 1);
  lcd.print(F(" !.System Booting.! "));
  lcd.setCursor(0, 2);
  lcd.print(F("    Please Wait!    "));
  Serial.println(F("🎵 BUZZ Playing startup melody..."));// Now play the melody using warningBuzz
  warningBuzz(1, 330, 150); delay(50); // E4
  warningBuzz(2, 392, 100); delay(50); // G4
  warningBuzz(1, 659, 150); delay(50); // E5
  warningBuzz(2, 523, 100); delay(50); // C5
  warningBuzz(1, 587, 150); delay(50); // D5
  warningBuzz(1, 784, 500); delay(50); // G5
  warningBuzz(1, 785, 200); delay(50); // G5
  door_open();
  delay(2000);
}

void displayPasscodeProgress()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("===================="));
  lcd.setCursor(0, 1);
  if (f4 == 1)
    lcd.print(F(" -- OLD Passcode -- "));
  else if (f4 == 2)
    lcd.print(F(" -- New Passcode -- "));
  else if (f4 == 3)
    lcd.print(F("- Confirm Passcode -"));
  else
    lcd.print(F("  --- Passcode ---  "));
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

//==============================================================================================
// --- Warning Buzzer Functions ---
//==============================================================================================
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
//==============================================================================================
// --- The End ---
//==============================================================================================
