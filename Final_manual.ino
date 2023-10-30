#include <Servo.h>
#include <Stepper.h>
#include <Wire.h> // Library for I2C communication
#include <LiquidCrystal_I2C.h> // https://www.arduinolibraries.info/libraries/liquid-crystal-i2-c (Library for LCD)
#include <EEPROM.h>
LiquidCrystal_I2C lcd(0x3F, 16, 2); // address, chars, rows.
int joystick_x = A2;                                                       
int joystick_y = A3;                                                     
int pos_x;                   
int pos_y; 
int ComElev=180;          
int TruAzim=0;  
int nosteps=0;
int Resetangle=0 ;
int negflag;
int TruElev=90;

//Servo variables
int ElPWMPin = 3;        // out pin for azimuth PWM command
int ElPotPin = A1;       // servo feedback

Servo servo1;  
              
#define rom_var1 0       // storing steps
#define rom_var2 4       // storing negflag
#define STEPS 200        // steps per revolution

////Encoder variables
//
// #define outputA 6
// #define outputB 7

// int counter = 0; 
// float angle = 0;
// int aState;
// int aLastState;  

// create an instance of the stepper class, specifying
// the number of steps of the motor and the pins it's
// attached to
Stepper stepper(STEPS, 8, 9, 10, 11);

void setup() {
Serial.begin(9600);
servo1.attach(ElPWMPin) ;          // servo 1 attached pin 3
servo1.write(ComElev); 
          
pinMode(joystick_x, INPUT);                     
pinMode(joystick_y, INPUT); 

stepper.setSpeed(40);                // set the speed of the motor to 40 RPMs
negflag=EEPROM.read(rom_var2);
if (negflag==1){Resetangle=-EEPROM.read(rom_var1);}
else{
Resetangle=EEPROM.read(rom_var1);
}
stepper.step(-Resetangle);
  EEPROM.write(rom_var1 , 0);
  EEPROM.write(rom_var2 , 0);
// //Testing
//  Serial.print("flag: ");
//     Serial.println(negflag);
//     Serial.print("reset angle ");
//     Serial.println(Resetangle);            
    
  
// Initiate the LCD:
//  lcd.begin(16,2);               //select this one if the arrows are not displayed correctly
  lcd.init();
  lcd.backlight();
  

// write on display name
  lcd.setCursor(0, 0);           // Set the cursor on the first column first row.(counting starts at 0!)
  lcd.print("Antenna Manual"); // display "..."
  lcd.setCursor(0, 1);           // Set the cursor on the first column the second row
  lcd.print("Control");
  delay(3000);
  lcd.clear();
  // display Azim. and Elev. values
  lcd.setCursor(0, 0);
  lcd.print("Azm.---" + String(char(223)) );  // char(223) is degree symbol
  lcd.setCursor(0, 1); 
  lcd.print("Elv.---" + String(char(223)) );
  DisplValue(TruAzim, 4,0);
     TruElev = map(analogRead(A1), 315, 455, 0, 90);
  DisplValue(TruElev, 4,1);
 
} 
void loop() {

////testing
//stepper.step(10);
//   nosteps=nosteps+10;
//   EEPROM.write(rom_var1 , nosteps);
//   Serial.print("nosteps");  // prints a label
//   Serial.println(nosteps);         // prints a tab
//   Serial.print("TrueAzim");  // prints a label
   Serial.println(ComElev);
//   Serial.print("\n");
   delay(10);
////testing

func();

}  
//-------------Procedures-----------------
void DisplValue(int x, int y, int z) {        //ex.  DisplValue(TruAzim, 4,0);
  char displayString[7] = "";
  sprintf(displayString, "%3d", x);           // outputs a fixed lenght number (3 integer) x is variable name stored in displayString %3d: 3decimal places
  lcd.setCursor(y, z);                        // for leading zeros '007' use "%03d"
  lcd.print(displayString);
}

void negativdeg(){
   lcd.setCursor(0, 0);
  lcd.print("Azm.");
   if(TruAzim<-99){DisplValue(TruAzim, 3,0);
   lcd.setCursor(7, 0);
  lcd.print( String(char(223)));}
  }
  
void func(){
//Serial.print(pos_x);
//Serial.print("\n");
//Serial.print(pos_y);
//Serial.print("\n");
//Serial.print("\n");
//Serial.print("\n");

delay(500);
pos_x = analogRead (joystick_x) ;  
pos_y = analogRead (joystick_y) ;                      
if (pos_x > 700)            //if the horizontal value from joystick is greater than 700
{
  if (nosteps < -199)      //if stepper postion >= 360
  { 
  } 
  else
 { 
   stepper.step(-1);
   nosteps--;
      
   TruAzim=(-nosteps*1.8);
   if (nosteps<0){
    EEPROM.write(rom_var2 , 1);
    EEPROM.write(rom_var1 , -nosteps);
    }
    if(nosteps>=0){
    EEPROM.write(rom_var2 , 0);
    EEPROM.write(rom_var1 , nosteps);  
      }
//   Serial.print("nosteps");  // prints a label
//   Serial.println(nosteps);         // prints a tab
//   Serial.print("TrueAzim");  // prints a label
//   Serial.println(TruAzim);
//   Serial.print("\n");
   delay(100);
   DisplValue(TruAzim,4,0);
   negativdeg();
   //delay (100); 
  } 
}

if (pos_x <300)
{
  if (nosteps > 199)
  {  
  }  
  
else
  {
   stepper.step(1);
   nosteps++;
   TruAzim=(-nosteps*1.8);
if (nosteps<0){
    EEPROM.write(rom_var2 , 1);
    EEPROM.write(rom_var1 , -nosteps);
    }
    if(nosteps>=0){
    EEPROM.write(rom_var2 , 0);
    EEPROM.write(rom_var1 , nosteps);  
      }
   
//   Serial.print("nosteps");  // prints a label
//   Serial.println(nosteps);
//   Serial.print("TrueAzim");  // prints a label
//   Serial.println(TruAzim);
//   Serial.print("\n");
   delay(100);
   DisplValue(TruAzim,4,0);
   negativdeg();
   //delay (100) ;
  }}


if (pos_y > 700)      //if the vertical value from joystick is > 700
{
  if (ComElev >180)  //second servo moves right
  { 
  } 
  else
  { 
    ComElev = ComElev + 2; 
    servo1.write (ComElev);
    TruElev = map(analogRead(A1), 315, 455, 0, 90);
    DisplValue(TruElev, 4,1);
    delay(10);
  } 
} 

if (pos_y < 300)
{
  if (ComElev < 110)
  {  
  }        
  else
  {
  ComElev = ComElev - 2;
  servo1.write(ComElev);
  TruElev = map(analogRead(A1), 315, 455, 0, 90);
  DisplValue(TruElev, 4,1);
  delay(10);
}
}}
