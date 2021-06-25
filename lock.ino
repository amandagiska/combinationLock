#define HELP 1
#define ASSIGN 2
#define EEADDR 500

#include <LiquidCrystal.h>
#include <Servo.h>
#include <EEPROM.h>

//LCD
const int rs = 13, en = 12, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
//piezo
int piezoPin = 11;
//serwo
Servo serwo;

//buttons
const int buttonPin1 = 8, buttonPin2 = 7, buttonPin3 = 14, buttonPin4 = 15;
const int correctLedPin = 10, wrongLedPin = 9;     
int buttonPushCounter = 0;   // counter for the number of button presses
int bS1 = 0, lBS1 = 0, bS2 = 0, lBS2 = 0, bS3 = 0, lBS3 = 0, bS4 = 0, lBS4 = 0;
char entered[5]; // entered code

// command processing
bool readComplete = false;
bool masterPsswd = false;
uint8_t type;
char value[64];
char command[64];
char inputBuffer[64];
char outputBuffer[64];
int tempTries;

int EEAddr = EEADDR;
//changeable variables
char code[5];
char passwordMaster[9];
char tries[5];
char tLock[5];

//================================================================================ 
void setup() {      
  lcd.begin(16, 2);
  lcd.setCursor(0,0);
  lcd.print("Enter password: ");
  pinMode(buttonPin1, INPUT_PULLUP);
  pinMode(buttonPin2, INPUT_PULLUP);
  pinMode(buttonPin3, INPUT_PULLUP);
  pinMode(buttonPin4, INPUT_PULLUP);
  pinMode(wrongLedPin, OUTPUT);
  pinMode(correctLedPin, OUTPUT);
  Serial.begin(115200);
  Serial.println(F("**** Welcome to the device settings dashboard! ****\n    Enter master password to make some changes...\n"));        
  EEPROM.get(EEAddr,code);              EEAddr += sizeof(code);
  EEPROM.get(EEAddr,passwordMaster);    EEAddr += sizeof(passwordMaster);
  EEPROM.get(EEAddr,tries);             EEAddr += sizeof(tries);
  EEPROM.get(EEAddr,tLock);             EEAddr += sizeof(tLock);
  tempTries = atoi(tries);
}

//=================================================================================
void loop() {
  lcd.setCursor(0,1);
  lcd.blink();
  delay(100);
  lcd.noBlink();

  if(readComplete) // dashboard (RS232) ===========================================
  {
    readComplete = false; 
    if(!masterPsswd) {
      if(strcmp(inputBuffer, passwordMaster) == 0) {
        masterPsswd = true;
        Serial.println(F("Now we can start. Type 'help?' if you want to see command list. \n"));
      }
      else {
        Serial.println(F("Wrong password"));
      }
    }
    else {
      if (processCommand(inputBuffer, command, &type, value) > 0) {
        implementCommand(command, type, value);
        EEAddr = EEADDR;
        EEPROM.get(EEAddr,code);              EEAddr += sizeof(code);
        EEPROM.get(EEAddr,passwordMaster);    EEAddr += sizeof(passwordMaster);
        EEPROM.get(EEAddr,tries);             EEAddr += sizeof(tries);
        EEPROM.get(EEAddr,tLock);             EEAddr += sizeof(tLock); 
        strcpy(command, "");
      }
      else {
        Serial.println(F("Error! Problem processing the command."));
      }
    }
    strcpy(inputBuffer, "");  
  }

  // push button sequence =====================
  static int buttonPushCounter = 0; 
  bS1 = digitalRead(buttonPin1);
  bS2 = digitalRead(buttonPin2);
  bS3 = digitalRead(buttonPin3);
  bS4 = digitalRead(buttonPin4);
 
  if (bS1 != lBS1) {   // check difference between last time and now
    if (bS1 == LOW) {  // button just pressed
      buttonPushCounter = buttonPushCounter + 1;
      digitalWrite(correctLedPin, HIGH);
      delay(100);
      digitalWrite(correctLedPin, LOW);
      entered[buttonPushCounter-1] = '1';
    }
  }
  else if (bS2 != lBS2) {
      if (bS2 == LOW) {
        buttonPushCounter = buttonPushCounter + 1;
        digitalWrite(correctLedPin, HIGH);
        delay(100);
        digitalWrite(correctLedPin, LOW);
        entered[buttonPushCounter-1] = '2';
      }
  }
  else if (bS3 != lBS3) {
      if (bS3 == LOW) {
        buttonPushCounter = buttonPushCounter + 1;
        digitalWrite(correctLedPin, HIGH);
        delay(100);
        digitalWrite(correctLedPin, LOW);
        entered[buttonPushCounter-1] = '3';
      }
  }
  else if (bS4 != lBS4) {
      if (bS4 == LOW) {
        buttonPushCounter = buttonPushCounter + 1;
        digitalWrite(correctLedPin, HIGH);
        delay(100);
        digitalWrite(correctLedPin, LOW);
        entered[buttonPushCounter-1] = '4';
      }
  }

  lBS1 = bS1;
  lBS2 = bS2;
  lBS3 = bS3;
  lBS4 = bS4;
  
  if(buttonPushCounter == 4) {
    if(strcmp(entered, code) == 0)
    {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Device unlocked!");
      delay(100);
      serwo.attach(6);      
      serwo.write(0);
      delay(300);
      serwo.detach(); 
      for(int i=0; i<3; i++) {
        digitalWrite(correctLedPin, HIGH);   
        delay(300);                    
        digitalWrite(correctLedPin, LOW);   
        delay(300);   
      }
      lcd.clear();
      lcd.print("Enter password: "); 
      tempTries = atoi(tries);
      serwo.attach(6);      
      serwo.write(90);  // 90 degree
      delay(300);
      serwo.detach(); 
    }    
    else
    {
      tempTries -= 1;
      if((tempTries)>0) {
        lcd.clear();
        sprintf(outputBuffer, "Tries left: %i", tempTries);
        lcd.print(outputBuffer);
        for(int i=0; i<3; i++) {
          digitalWrite(wrongLedPin, HIGH);   
          delay(300);                    
          digitalWrite(wrongLedPin, LOW);   
          delay(300);    
        } 
        lcd.clear();
        lcd.print("Enter password: ");
      }             
      else
      {        
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Device locked...  ");        
        Serial.end();
        int t = atoi(tLock);
        for(int i=0; i<t; i++) {
          lcd.setCursor(0,1);
          lcd.print("  ");
          int tLeft = t-i;
          lcd.setCursor(0,1);
          lcd.print(tLeft);
          tone(piezoPin, 5000, 500);
          digitalWrite(wrongLedPin, HIGH);
          digitalWrite(correctLedPin, HIGH);   
          delay(500);                    
          digitalWrite(wrongLedPin, LOW);
          digitalWrite(correctLedPin, LOW);   
          delay(500);    
        } 
        noTone(piezoPin);
        Serial.begin(115200);
        tempTries = atoi(tries);
      }
      lcd.clear();
      lcd.print("Enter password: ");      
    }     
    buttonPushCounter = 0;
  }
}
//==========================================================================
//==========================================================================
void serialEvent()
{
  while (Serial.available()) {
    char inChar = Serial.read();
    if (inChar == '\n' || inChar == '\r') {
      readComplete = true;
    }
    else {
      uint8_t len = strlen(inputBuffer);
      inputBuffer[len] = inChar;
      inputBuffer[len + 1] = '\0';
    }
  }
}

int8_t processCommand(char* inputBuffer, char* command, uint8_t *type, char* value) 
{  
  if(strchr(inputBuffer, '?') != NULL)
    *type = HELP;
  else if(strchr(inputBuffer, '=') != NULL)
    *type = ASSIGN;

  char* temp, tokens[64];
  strcpy(tokens, inputBuffer);
  temp = strtok(tokens, "=?");

  if (temp == NULL) {
    Serial.println("Empty command");
    return -1;
  }
  strcpy(command, temp);
  if (*type == ASSIGN) {
    temp = strtok(NULL, "=");
    if (temp == NULL) {
      Serial.println("No value to assign.");
      return -1;
    }
    strcpy(value,temp); 
  }
  return 1;   
}

void implementCommand(char *command, uint8_t type, char *value)
{
  char tempCode[30];
  int v = 0;
  bool wrongCommand= false;
  
  // HELP ================================================================
  if (strcmp(command, "help") == 0) {
    if (type == HELP)
    {
      //Serial.println("**** Combination lock **** \n\
      List of commands: \n\
      - help? - see Combination lock manual, \n\
      - CHP - change 4 digits password with 'CHP=', use numbers 1 to 4, e.g. CHP=1211, \n\
      - CHPMaster - change 6 (up to 20) characters master password with 'CHPMaster=', use all characters, e.g. CHPMaster=Password123, \n\
      - Ntries - change number of tries 'Ntries=' before blockade (up to 9), \n\
      - LockTime - change duration of the lock's blockade in seconds 'LockTime=' (up to 300s). \n\
      ");
      for (int i = 0; i < 450; i++)
      {
        Serial.print((char)EEPROM.read(i));    
      }
    }
  }
  // Change Password =====================================================
  else if (strcmp(command, "CHP") == 0)
  {
    if(type==ASSIGN)
    {
      snprintf(tempCode, sizeof(tempCode), "%s", value);
      bool wrongCode = false;
      if(strlen(tempCode) != 4) {
        sprintf(outputBuffer,"Wrong code. Keep 4 digits length. \n");
        wrongCode = true;
      }
      else {
        for(int i=0; i<strlen(tempCode); i++) {
          int a = tempCode[i]-48; 
          if(a < 1 || a > 4) {  
            sprintf(outputBuffer,"Wrong code. Use digits 1 to 4. \n");
            wrongCode = true;
            break;
          }
        }
      }
      if(!wrongCode) {
        sprintf(code, "%s", tempCode);
        sprintf(outputBuffer, "New code is: %s \n", code);
      }
    }  
  }
  // Change Master Password ====================================================
  else if (strcmp(command, "CHPMaster") == 0)
  {
    if(type==ASSIGN)
    {      
      snprintf(tempCode, sizeof(tempCode), "%s", value);
      if(strlen(tempCode) < 6 || strlen(tempCode) > 20) {
        sprintf(outputBuffer,"Wrong password. Keep minimum 6 (up to 20) characters length. \n");
      }
      else {
        sprintf(passwordMaster, "%s", tempCode);
        sprintf(outputBuffer, "New master password is: %s. \n Note: characters '=' and '?' won't be considered as password elements. \n", passwordMaster);
      }
    }  
  }
  // Change number of tries ===================================================
  else if (strcmp(command, "Ntries") == 0)
  {
    v = atoi(value);
    if(v < 10 && v > 0) {
      sprintf(tries, "%s", value);
      sprintf(outputBuffer, "Number of tries changed to %s. \n", tries);
    }
    else {
      sprintf(outputBuffer,"Wrong value. Number of tries can be 1 to 9. \n");
    }     
  }
  // Change duration of lock ====================================================
  else if (strcmp(command, "LockTime") == 0)
  {
    v = atoi(value);
    if(v <= 300 && v > 0) {
       sprintf(tLock, "%s", value);
       sprintf(outputBuffer,"Lock time changed to %s s. \n", tLock);
    }
    else {
      sprintf(outputBuffer,"Wrong value. Maximum lock time is 300 s and it can't be 0 s. \n");
    } 
  }
  // wrong command =============================================================
  else
  {
    sprintf(outputBuffer,"Wrong command. \n");
    wrongCommand = true;
  }
  
  Serial.println(outputBuffer);
  
  if(!wrongCommand)
  {
    EEAddr = EEADDR;
    EEPROM.put(EEAddr,code);              EEAddr += sizeof(code);
    EEPROM.put(EEAddr,passwordMaster);    EEAddr += sizeof(passwordMaster);
    EEPROM.put(EEAddr,tries);             EEAddr += sizeof(tries);
    EEPROM.put(EEAddr,tLock);             EEAddr += sizeof(tLock); 
  }
}
