//STEPPER AZ '''' SERVO EL
#include <Stepper.h>
//#include <AccelStepper.h>
#include <Servo.h>           //include servo library
#include <Wire.h> // Library for I2C communication
#include <LiquidCrystal_I2C.h> // https://www.arduinolibraries.info/libraries/liquid-crystal-i2-c (Library for LCD)
#include <EEPROM.h>
// Wiring: SDA pin is connected to A4 and SCL pin to A5.
// Connect to LCD via I2C, default address 0x27 (A0-A2 not jumpered)
LiquidCrystal_I2C lcd(0x3F, 16, 2); // address, chars, rows.

Servo servo1;                // creating servo object

#define STEPS 200
#define rom_var1 0

//Encoder variables

// #define CLK 6
// #define DT 7

// int counter = 0; 
// float angle = 0;
// int aState;
// int aLastState;

  
int joystick_x = A2;              // joystick x direction pin                                          
int joystick_y = A3;              // joystick y direction pin                                         
int pos_x;                   // declaring variable to stroe horizontal value
int pos_y;                   // declaring variable to stroe vertical value
int ComElev=180;         // storing servo position
int ComAzim=0;         // storing stepper position
int previous = 0;             //stepper preveous steps count
int Resetangle=0 ;            //Resetting stepper position from EEPROM
int offset = 0;

// create an instance of the stepper class, specifying
// the number of steps of the motor and the pins it's
// attached to
Stepper stepper(STEPS, 8, 9, 10, 11);

// declaring custom symbol for up/down arrow
 byte DownArrow[8] = {
  B00000,
  B00100,
  B00100,
  B00100,
  B10101,
  B01110,
  B00100,
  B00000
};
 byte UpArrow[8] = {
  B00000,
  B00100,
  B01110,
  B10101,
  B00100,
  B00100,
  B00100,
  B00000
};

/**********************************THIS IS WHERE YOU REALY TWEAK THE ANTENNA MOVEMENT**************/
// ANTENNA potentiometers CALIBRATION
 // int AzMin = 1;         //begining of the potentiometer
 // int AzMax = 1023;      //end of the potentiometer 
  int ElMin = 1;
  int ElMax = 1023;

// Allowed error for >< LCD
  int AzErr = 8;
  int ElErr = 4;

// Angle difference where soft stop begins
  int Amax = 25;        //azimuth
  int Emax = 15;        //elevation

// min and max power for motors, percents;
//  int PwAzMin = 30;     //minimum power for which the motor doesn't stall and starts under load
//  int PwAzMax = 100;    //full power for the fastest speed
//  int PwElMin = 30;
//  int PwElMax = 100;

/***************************************************************************************************/

  enum PinAssignments {
  ElPotPin = A1,        // input pin for the elev. potentiometer    
  ElPWMPin = 3,         // out pin for elevation rotation PWM command
  };
  
  
// movement variables
  int CurrentAzim = 0;      // calculated real azimuth value
  int CurrentElev = 90;      // calculated real elevation value
  int OldCurrentAzim = 0;   // to store previous azimuth value
  int OldComAzim = 0;
  int OldCurrentElev = 0;   // to store previous elevation value
  int OldComElev = 0;
  unsigned long NowTime;         // store the current millis() for timing purposes
  

// variables for serial comm
  String Azimuth = "";
  String Elevation = "";
  String ComputerRead;
  String ComputerWrite;

/*************** END VARIABLE DECLARATION  ************/

void setup() {
  Serial.begin(9600);
  Serial.setTimeout(50);           // miliseconds to wait for USB sata. Default 1000
  servo1.attach (ElPWMPin) ;          // servo 1 attached pin 3
  servo1.write (ComElev) ;

// Initiate the LCD:
  lcd.init();
  lcd.backlight();

//reseting stepper position to 0 degree
  stepper.setSpeed(30);  // set the speed of the motor to 10 RPMs
  Resetangle=EEPROM.read(rom_var1);
  stepper.step(Resetangle);
  EEPROM.write(rom_var1 , 0);

////Encoder
//   pinMode (CLK,INPUT);
//   pinMode (DT,INPUT);

//   aLastState = digitalRead(CLK); 
// write on display name and version
  lcd.setCursor(0, 0);           
  lcd.print("Orbitron"); 
  lcd.setCursor(0, 1);           
  lcd.print("Auto. Control");
  delay(3000);

//creating custom symbol for up/dwn arrow
  lcd.createChar(1, DownArrow);
  lcd.createChar(2, UpArrow);
  
// pin declaration
  pinMode(ElPWMPin, OUTPUT);
  pinMode(ElPotPin, INPUT);

// Interrupt Service Routine for Az and El encoder
//  attachInterrupt(0, ReadAzimEncoder, CHANGE);                               // Az encoder
//  attachInterrupt(1, ReadAzimEncoder, CHANGE);                               // El Encoder

//  CurrentElev = map(analogRead(A1), 320, 451, 5, 90);
  CurrentElev = map(analogRead(A1), 315, 455, 0, 90);

/* keep command values in range  */
  ComAzim = constrain(CurrentAzim, 0, 359);
  ComElev = constrain(CurrentElev, 0, 90);
  OldCurrentAzim = CurrentAzim;
  OldComAzim = ComAzim;
  OldCurrentElev = CurrentElev;
  OldComElev = CurrentElev;

  delay(1500);                     // keep for 1.5 seconds
// display Azim. and Elev. values
  lcd.setCursor(0, 0);
  lcd.print("Azm.***" + String(char(223)) + "=Cd.***" + String(char(223)));  // char(223) is degree symbol
  lcd.setCursor(0, 1); 
  lcd.print("Elv.***" + String(char(223)) + "=Cd.***" + String(char(223)));
  LCD_Display(CurrentAzim, 4,0);
  LCD_Display(ComAzim,12,0);
  LCD_Display(CurrentElev, 4,1);
  LCD_Display(ComElev,12,1);
}
// end SETUP

void loop() {
/************** FYI, this loop repeats 500 times per second !!! **************/
  CurrentElev = map(analogRead(A1), 315, 455, 0, 90);

//  CurrentElev = map(analogRead(A1), 320, 451, 5, 90);
// read the command from encoder
  //ReadAzimEncoder();
  
  if (Serial.available()) {SerComm();
  
 //*****************Motors motion************************
  
    delay(300);   
//  stepper.stop();            //stops the stepper gradually before beginning a new loop
 
 if(ComElev>5 && ComElev<90) {
  servo1.write (105 + ComElev);}
  else if(ComElev<5 && ComElev>0){
    servo1.write (105 + 5);
    }
    else if(ComElev <1){
      servo1.write (180);
    }
    int steps = - map(ComAzim, 0, 360, 0, STEPS);  //no. of steps of ComAzim
                                                //STEPS -> steps per revolution=200
    
  offset=steps-previous;
  
  stepper.step(offset);
  steps=steps%200;
  CurrentAzim= -steps*1.8;
  previous =steps ;        // remember the previous value of the steps moved
  EEPROM.write(rom_var1 , abs(steps));

////if encoder is implemented
// //Forward offset
//  if (offset>0){
//  for(int i =0;  i <offset; i++){   
//  stepper.step(1);            //ComAzim/1.8 is the no of steps equivalent tostarting from 0 position , this difference means increment or decrement steps
//  steps=steps%200;
//  CurrentAzim=steps*1.8;
//  previous =steps ;        // remember the previous value of the steps moved
////  ReadAzimEncoder();
//  EEPROM.write(rom_var1 , steps);
//  
//  }
//  }
//  //Backward offset
//  if (offset<0){
//  for(int i =0; i >offset; i--){   
//  stepper.step(-1);       //ComAzim/1.8 is the no of steps equivalent tostarting from 0 position , this difference means increment or decrement steps
//  steps=steps%200;
//  CurrentAzim=steps*1.8;
//  previous =steps ;       // remember the previous value of the steps moved
// // ReadAzimEncoder();
//  EEPROM.write(rom_var1 , steps);
//  
//  }
//  }
  
    }          // read USB data + motion.....................................................................................
  
// update antenna position display only if value change
  if ((millis() - NowTime) > 1000){              //every 0.3 seconds, not to flicker the display
    if (OldCurrentAzim!=CurrentAzim) {
      LCD_Display(CurrentAzim,4,0);
      negativdeg();
      OldCurrentAzim = CurrentAzim;
    }
    if (OldCurrentElev!=CurrentElev) {
      LCD_Display(CurrentElev,4,1);
      negativdeg();
      OldCurrentElev = CurrentElev;
    }
    NowTime = millis();
  }

// update target position display only if value change....
  if (OldComAzim != ComAzim) {
    LCD_Display(ComAzim,12,0);
    OldComAzim = ComAzim;
  }
  if (OldComElev != ComElev) {
    LCD_Display(ComElev,12,1);
    OldComElev = ComElev;
  }

// this is to rotate in azimuth

  if (((CurrentAzim - ComAzim) < -AzErr)) {  // if in tolerance, but it wasn't an equal, rotate
     lcd.setCursor(8, 0);
     lcd.print(">");}
   else if ((CurrentAzim - ComAzim) > AzErr){   // if target is off tolerance

     lcd.setCursor(8, 0);
     lcd.print("<");    
     }
   else{
     lcd.setCursor(8, 0);
     lcd.print("=");
      }

// this is to rotate in elevation
  if ((CurrentElev - ComElev) < -ElErr) {                   // if equal, stop moving
    lcd.setCursor(8, 1);
    lcd.write(2);}
  

  else if ((CurrentElev - ComElev) > ElErr){     // if target is off tolerance >>
    lcd.setCursor(8, 1);
    lcd.write(1);   }
  else{
    lcd.setCursor(8, 1);
    lcd.print("=");}  
  
}        
         
// end main LOOP

//_____________________________________________________
// __________procedures definitions_________________


void LCD_Display(int x, int y, int z) {
  if(x>=0){
  char displayString[7] = "";
  sprintf(displayString, "%3d", x);           // outputs a fixed lenght number (3 integer)
  lcd.setCursor(y, z);                        // for leading zeros '007' use "%03d"
  lcd.print(displayString);}
// ************** FOR CALIBRATION PURPOSES **************
/*
  Serial.print ("Az ");
  Serial.println (analogRead(AzPotPin));
  Serial.print ("El ");
  Serial.println (analogRead(ElPotPin));
*/
}  // end LCD_Display()
  
void negativdeg(){
   lcd.setCursor(0, 0);
  lcd.print("Azm.");
   if(CurrentAzim<-99){LCD_Display(CurrentAzim, 3,0);
   lcd.setCursor(7, 0);
  lcd.print( String(char(223)));}
  }

//void ReadAzimEncoder() {
//aState = digitalRead(CLK); // Reads the "current" state of the outputA
//   // If the previous and the current state of the outputA are different, that means a Pulse has occured
//   if (aState != aLastState){     
//     // If the outputB state is different to the outputA state, that means the encoder is rotating clockwise
//     if (digitalRead(DT) != aState) { 
//       counter ++;
//       angle=map(counter,0,32,0,360);
//       angle=counter*11.25;                        //Closed loop angle feedback
//     } 
//     else {
//       counter --;
//     angle=map(counter,0,32,0,360);
//     angle=counter*11.25;
//     }           
//   } 
//   aLastState = aState;  
//   CurrentAzim=angle;
//}

void SerComm() {
  // initialize readings
  ComputerRead = "";
  Azimuth = "";
  Elevation = "";

  while(Serial.available()) {
    ComputerRead= Serial.readString();  // read the incoming data as string
//    Serial.println(ComputerRead);     // echo the reception for testing purposes
  }
  
// Detecting Azimuth angle <AZxxx.x>
    for (int i = 0; i <= ComputerRead.length(); i++) {
     if ((ComputerRead.charAt(i)  == 'A')&&(ComputerRead.charAt(i+1) == 'Z')){ 
      for (int j =  i+2; j <= ComputerRead.length(); j++) {
        if (isDigit(ComputerRead.charAt(j)))  {                   // if the character is number
          Azimuth  = Azimuth + ComputerRead.charAt(j);
        }
        else {break;}
      }
     }
    }
   
// Detecting Azimuth angle <AZxxx.x>
    for (int i = 0; i <= (ComputerRead.length()-2); i++) {
      if ((ComputerRead.charAt(i) == 'E')&&(ComputerRead.charAt(i+1) == 'L')){
        if ((ComputerRead.charAt(i+2)) == '-') {
          ComElev = 0;                  // if elevation negative
          break;
        }
        for (int j = i+2; j <= ComputerRead.length(); j++) {
          if (isDigit(ComputerRead.charAt(j))) {                               // if the character is number
            Elevation = Elevation + ComputerRead.charAt(j);
          }
          else {break;}
        }
      }
    }
    
//  if Azimuth is filtered <casting & limiting>
    if (Azimuth != ""){
      ComAzim = Azimuth.toInt();
      if(ComAzim > 359){ComAzim = ComAzim-360;}
      ComAzim = ComAzim%360;          // keeping values between limits
      }

//  if Elevation is filtered <casting & limiting>
    if (Elevation != ""){
      ComElev = Elevation.toInt();
      if (ComElev > 165) { ComElev = 165;}
      if (ComElev < 0) { ComElev = 180;}


      }



//// looking for <AZ EL> interogation for antenna position
//  for (int i = 0; i <= (ComputerRead.length()-4); i++) {
//    if ((ComputerRead.charAt(i) == 'A')&&(ComputerRead.charAt(i+1) == 'Z')&&(ComputerRead.charAt(i+3) == 'E')&&(ComputerRead.charAt(i+4) == 'L')){
//    // send back the antenna position <+xxx.x xx.x>
//      ComputerWrite = "+"+String(CurrentAzim)+".0 "+String(CurrentElev)+".0";
// // ComputerWrite = "AZ"+String(CurrentAzim)+".0 EL"+String(CurrentElev)+".0"; //that's for Gpredict and HamLib
//      Serial.println(ComputerWrite);
//    }
//  }
}  // end SerComm()
