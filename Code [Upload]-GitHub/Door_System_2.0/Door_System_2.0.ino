//===============================================
// --- Libraries Used ---
//===============================================
#include <Servo.h>
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
SoftwareSerial nanoSerial(2, 3);
//===============================================
// --- Matrix Pins ---
//===============================================
LiquidCrystal_I2C lcd(0x27, 20, 4);
//===============================================
// --- IO Pins ---
//===============================================
const int LP1 = A3; // Red LED
const int LP2 = A2; // Green LED
const int LP3 = A1; // Blue LED (buzzer indicator)
const int LP4 = 8;  // Button LED
const int TriggerPin = 7;
const int buttonPin = 10;          // Door toggle button pin
const int hingeSwitchPin = 5;      // Hinge safety switch pin
const int buzzerPin = 12;          // Buzzer pin
const int srPin = 9;               // Servo motor pin
const int TriggerSwitchPin = 4;              // Triger to close door
//===============================================
// --- Global State Variables ---
//===============================================
uint8_t passIndex = 0;
char mappedKey;
char enteredPasscode[7] = ""; // store entered passcode
char passcode[] = "123456";
const char changepasscode[]="#1234#";
bool servoState = false;
bool isUnlocking = false;
bool isChangingPasscode = false;
unsigned long startTime;
unsigned long passcodeEntryStartTime = 0;
unsigned long holdTime1 = 300, holdTime2 = 1000 ;
int f1 = 0, f2 = 0, f3=0,f4=0, c=0,c2=0;
int retryCount = 0;
const int DoorOpen = 95, DoorClose = 180;

Servo myServo;
//===============================================
// --- Setup ---
//===============================================
void setup()
{
  Serial.begin(9600);       // for Serial Monitor
  nanoSerial.begin(9600);   // for Nano
  Serial.println("✅ Initiated : Serial baud to 9600");
  Serial.println("✅ Initiated : NanoSerial baud to 9600");
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(hingeSwitchPin, INPUT_PULLUP);
  pinMode(TriggerSwitchPin, INPUT_PULLUP);
  pinMode(buzzerPin, OUTPUT);
  pinMode(13, OUTPUT); digitalWrite(13, LOW);
  pinMode(LP1, OUTPUT);         // Red
  pinMode(LP2, OUTPUT);         // Green
  pinMode(LP3, OUTPUT);         // Blue
  pinMode(LP4, OUTPUT);         // Button LED
  pinMode(TriggerPin, OUTPUT);  //
  Serial.println("✅ Initiated : Input Pins");
  delay(150);

  // Initial LED states
  digitalWrite(LP2, HIGH);   // Green LED ON
  digitalWrite(LP4, HIGH);   // Button LED ON

  // Init Servo
  myServo.attach(srPin);
  Serial.println("✅ Initiated : Servo Pins");
  delay(150);
  // Init LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  delay(150);
  Serial.println("✅ Initiated : Display ");
  delay(150);
  startup();
}

//===============================================
// --- Main Loop ---
//===============================================
void loop()
{
  DoorToggler();
  keypad();
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
        lcd.print(F("  Shut The Door...  "));
        //door_close();
        delay(1000);
        homescreen();
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
      else if (f2 == 0)
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
      if (strcmp(enteredPasscode, changepasscode) == 0)
      {
        Serial.println("🛠 Enter New Passcode:");
        f4=1;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(F("===================="));
        lcd.setCursor(0, 1);
        lcd.print(F("  --New Passcode--  "));
        lcd.setCursor(0, 2);
        lcd.print(F("       ______       "));
        lcd.setCursor(0, 3);
        lcd.print(F("===================="));
        passIndex = 0;
        enteredPasscode[0] = '\0';
        isChangingPasscode = true;  // Now expecting new passcode next
        return;
      }

      if (isChangingPasscode)
      {
        strcpy(passcode, enteredPasscode);
        Serial.print("🔐 New Passcode Set: ");
        Serial.println(passcode); 
        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print(" Passcode Changed");
        warningBuzz(2, 1000, 100);
        isChangingPasscode = false;
      }
      else if (strcmp(enteredPasscode, passcode) == 0)
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
  /*        //Original ONe
  if (nanoSerial.available())
  {
    char incoming = nanoSerial.read();
  */
  if (Serial.available())
  {
    char incoming = Serial.read();
    if ((incoming >= 'A' && incoming <= 'D') || (incoming >= '0' && incoming <= '9') || incoming == '#' || incoming == '*')
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
  bool buttonTriggered = debounceButton(buttonPin, holdTime1) && (f2==0);
  bool doorTriggered = debounceButton(TriggerSwitchPin, holdTime2) && (f2 == 1);
  if (buttonTriggered || doorTriggered)
  {
    for (int i = 0; i < 3; i++)         // Blink LP4 three times
    {
      digitalWrite(LP4, LOW);
      delay(100);
      digitalWrite(LP4, HIGH);
      delay(100);
    }
    servoState = !servoState;           // Toggle servo state
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
  digitalWrite(TriggerPin, HIGH); // Power solenoid to unlock
  delay(200); // Wait for piston to retract
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
  digitalWrite(TriggerPin, LOW);
  homescreen();
}

void door_close()
{
  f2 = 0;
  digitalWrite(TriggerPin, LOW); // Power solenoid to unlock
  delay(200); // Wait for piston to retract
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
          digitalWrite(TriggerPin, LOW);
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
        f2=1;
        myServo.write(DoorOpen);
        digitalWrite(TriggerPin, HIGH); // Power solenoid to unlock
        delay(1000);
        warningBuzz(1, 800, 100);
        warningBuzz(1, 1200, 150);
        digitalWrite(LP1, LOW);
        digitalWrite(LP2, HIGH);
        servoState = true;
        digitalWrite(TriggerPin, LOW);
        return;
      }
    }
  }
}
//===============================================
// --- Debounce Function for Button ---
//===============================================
bool debounceButton(int pin, int holdTime) 
{
  static unsigned long pressStartTimes[2] = {0};
  static bool longPressStates[2] = {false};
  static unsigned long lastBuzzTimes[2] = {0};  // For controlling buzz interval

  int index = (pin == TriggerSwitchPin) ? 1 : 0;  // Use index 1 for TriggerSwitchPin, 0 for buttonPin

  if (digitalRead(pin) == LOW) 
  {
    // Buzz only for TriggerSwitchPin while held
    if (pin == TriggerSwitchPin) 
    {
      unsigned long now = millis();
      if (now - lastBuzzTimes[index] >= 300) { // Buzz every 300ms
        if(f2==1)
        {
          c++;
          if(c<=2)
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
      c=0;  // Stop buzzer when TriggerSwitchPin is released
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
  c2=0;
  f4=0;
  lcd.setCursor(0, 0);
  lcd.print(F("-------VLock+-------"));
  lcd.setCursor(0, 1);
  lcd.print(F("A - Lock  B - Unlock"));
  lcd.setCursor(0, 2);
  lcd.print(F("C - DpLgt D - AReset"));
  lcd.setCursor(0, 3);
  lcd.print(F("--------2.0--------"));
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
  digitalWrite(TriggerPin,HIGH);
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
  if(f4==1)
  {
    lcd.print(F("  --New Passcode--  "));
  }
  else
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

