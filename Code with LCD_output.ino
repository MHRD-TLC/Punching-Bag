float vol;
float cur;

float start;
#include "HX711.h"
 // Date and time functions using a DS3231 RTC connected via I2C and Wire lib
#include <Wire.h>
#include "RTClib.h"
#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>
#define calibration_factor -10640.0 //This value is obtained using the SparkFun_HX711_Calibration sketch


#define DOUT  3
#define CLK  2
LiquidCrystal_I2C lcd(0x27, 20, 4);

HX711 scale(DOUT, CLK);

RTC_DS3231 rtc;

//char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
int t_i = 0, i, j;
int P_count = 0;
SoftwareSerial BTserial(9, 10); // RX | TX
unsigned long time;
String dat="";
//--- Accelerometer Register Addresses---------
#define Power_Register 0x2D
#define X_Axis_Register_DATAX0 0x32 // Hexadecima address for the DATAX0 internal register.
#define X_Axis_Register_DATAX1 0x33 // Hexadecima address for the DATAX1 internal register.
#define Y_Axis_Register_DATAY0 0x34 
#define Y_Axis_Register_DATAY1 0x35
#define Z_Axis_Register_DATAZ0 0x36
#define Z_Axis_Register_DATAZ1 0x37
int ADXAddress = 0x53;  //Device address in which is also included the 8th bit for selecting the mode, read in this case.
int X0,X1,X_out;
int Y0,Y1,Y_out;
int Z1,Z0,Z_out;
bool f=1;
float Xa,Ya,Za,x,y,z,xyz,oxyz=0,dxyz=0,velocity=0,maxvelocity=0;
float wt =70.0, bwt=70.0;
float thr = 0.0;//Setting threshold 

void setup() {
  Serial.begin(9600);
 BTserial.begin(9600);
  lcd.begin();

  // Turn on the blacklight and print a message.
  lcd.backlight();

Serial.println("CLEARDATA");Serial.println("CLEARDATA");
  scale.set_scale(calibration_factor); //This value is obtained by using the SparkFun_HX711_Calibration sketch
  scale.tare(); //Assuming there is no weight on the scale at start up, reset the scale to 0
Serial.println("LABEL,Timer,t, AccY,AccX,AccZ,Velocity,voltage,current,Calories,..."); 
Serial.println("LABEL,Timer,t, AccY,AccX,AccZ,Velocity,voltage,current,Calories,..."); 

Serial.println("RESETTIMER"); //resets timer to 0
Serial.println("RESETTIMER");

//  Serial.println("Type your BodyWight >> __(kg)");
  //delay(5000);
  //-------------------RTC--------------------//
  #ifndef ESP8266
  while (!Serial); // for Leonardo/Micro/Zero
#endif

 // delay(50); // wait for console opening

  //if (! rtc.begin()) {
  //  Serial.println("Couldn't find RTC");
  //  while (1);
  //}
rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  if (rtc.lostPower()) {
    Serial.println("RTC lost power, lets set the time!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
   // rtc.adjust(DateTime(2017, 1, 27, 8, 42, 0));//リセットはifの前にカット＆ペーストし時間変更しコンパイル後、この位置に戻す
  }
  //-------------------acc--------------------//
 Wire.begin(); // Initiate the Wire library    
  Serial.begin(9600);    
 // delay(50);
  
  Wire.beginTransmission(ADXAddress);
  Wire.write(Power_Register); // Power_CTL Register
  // Enable measurement
  Wire.write(8); // Bit D3 High for measuring enable (0000 1000)
  Wire.endTransmission();
   start = millis();
}

  //sound //2kg
void sound1(int times)
{
  for (int i= 1; i<=times; i++)
  {
   digitalWrite(12,HIGH);
   delay(500);
   digitalWrite(12,LOW);
   delay(50);
  }
}
   //sound //4kg
void sound2(int times)
{
  for (int i=1; i<=times; i++)
  {
    tone(8, 523);
    delay(500);
    tone(8, 587);
    delay(500);
    tone(8, 659);
    delay(100);
     
    noTone(8);
  }
}
  // 
float t;
void loop() 

{

  t = float(millis()-start)/1000;
  lcd.setCursor(0, 0);
  lcd.print("Player : Noriko");
Serial.print("DATA,TIME,");
Serial.print(t); 
Serial.print(",");
/*
if ( (int(t)%10) ==  0)
{
    Serial.println("CLEARDATA");
    Serial.println("LABEL,Timer,t, AccY,AccX,AccZ,Velocity,voltage,current,Calories,..."); 

    
}
*/

//-------------------body wight--------------------//
delay(10);

  float wt =scale.get_units()*0.1;
   if (wt < 0){
     wt = wt * -1;
     }
  while (Serial.available() >0)  
   { 
      dat += char(Serial.read());     
 
   }  
    if (dat!="")
 {
    Serial.print("BodyWight = ");
    bwt=dat.toInt();
    Serial.print(bwt);
    Serial.println(" kg");
    dat="";//
}
  



  Wire.beginTransmission(ADXAddress); // Begin transmission to the Sensor 
  //Ask the particular registers for data
  Wire.write(X_Axis_Register_DATAX0);
  Wire.write(X_Axis_Register_DATAX1);  
  Wire.endTransmission(); // Ends the transmission and transmits the data from the two registers
  Wire.requestFrom(ADXAddress,2); // Request the transmitted two bytes from the two registers
  if(Wire.available()<=2) {  // 
    X0 = Wire.read(); // Reads the data from the register
    X1 = Wire.read();
    /* Converting the raw data of the X-Axis into X-Axis Acceleration
     - The output data is Two's complement 
     - X0 as the least significant byte
     - X1 as the most significant byte */ 
    X1=X1<<8;
    X_out =X0+X1;
    Xa=X_out/256.0; // Xa = output value from -1 to +1, Gravity acceleration acting on the X-Axis
  }
  // Y-Axis
  Wire.write(Y_Axis_Register_DATAY0);
  Wire.write(Y_Axis_Register_DATAY1);  
  Wire.endTransmission(); 
  Wire.requestFrom(ADXAddress,2);
  if(Wire.available()<=2) { 
    Y0 = Wire.read();
    Y1 = Wire.read();
    Y1=Y1<<8;
    Y_out =Y0+Y1;
    Ya=Y_out/25600.0;
  }
  // Z-Axis
  Wire.beginTransmission(ADXAddress); 
  Wire.write(Z_Axis_Register_DATAZ0);
  Wire.write(Z_Axis_Register_DATAZ1);  
  Wire.endTransmission(); 
  Wire.requestFrom(ADXAddress,2);
  if(Wire.available()<=2) { 
    Z0 = Wire.read();
    Z1 = Wire.read();
    Z1=Z1<<8;
    Z_out =Z0+Z1;
    Za=Z_out/256.0;

    float x = Xa;
    float y = Ya;
    float z = Za;
    float xyz = sqrt(x*x+y*y+z*z);
 
    if(f==0){
        dxyz += xyz - oxyz;
    }
    f=0;
    velocity = dxyz*9.8;
    velocity *= (3600.0f/1000.0f/10.0f);
    oxyz = xyz;
 
    if (maxvelocity < velocity){
        maxvelocity = velocity;
        
    }                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        
     if (wt <= 0.1){
      if (t_i > 2)
      {
      maxvelocity = 0.0;
      t_i =0;
      }
      else
      {
        t_i++;
  //      delay(50); 
      }
    }
   
  }



  
//------------------thr part----------------------//  
  if( wt>thr)
  {
  
  
  
  
  
 

delay(10);
  while (BTserial.available() >0)  
   { 
      dat += char(BTserial.read());     

   }     

    if (dat!="")
 {
    BTserial.print("BodyWight = ");
    bwt=dat.toInt();
    BTserial.print(bwt);
    BTserial.println(" kg");
    dat="";
}
//-------------------RTC--------------------//

    DateTime now = rtc.now();
/*    
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(' ');
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':'); 
    Serial.print(now.second(), DEC);
    Serial.print(" ");*/
  lcd.setCursor(0, 1);
lcd.print(now.year(), DEC);lcd.print("/"); lcd.print(now.month(), DEC);lcd.print("/"); lcd.print(now.day(), DEC); lcd.print("   ");lcd.print(now.hour(), DEC); lcd.print(":");lcd.print(now.minute(), DEC);lcd.print(":"); lcd.print(now.second(), DEC); 

//  delay(50);
//-------------------Wight--------------------//
//  Serial.print(wt, 2); //scale.get_units() returns a float
//  Serial.print(" kg"); //You can change this to kg but you'll need to refactor the calibration_factor
 // Serial.print(" ");
//-------------------acc-----------------------//
// X-axis
  // Prints the data on the Serial Monitor
  //Serial.print("accX= ");
  Serial.print(Xa);
  Serial.print(",");
 // Serial.print(" m/s^2");
 // Serial.print("   accY= ");
  Serial.print(Ya);
  Serial.print(",");
 // Serial.print(" m/s^2");
 // Serial.print("   accZ= ");
  Serial.print(Za);
  Serial.print(",");
 // Serial.print(" m/s^2");
 // Serial.print("   maxvelocity= ");
  Serial.print(maxvelocity);
  Serial.print(",");
 // Serial.print(" m/s");
 // Serial.print(" ");

// -----------------Power Generation Calcs------------//
vol = (float(analogRead(A0))*5.0)/1024;
lcd.setCursor(0, 2);
lcd.print("  Voltage:"); lcd.print(vol*1000,2);lcd.print("mV");lcd.print("    ");
  cur = vol/56;
  lcd.setCursor(0, 3);
lcd.print("  Current:"); lcd.print(cur*1000,2);lcd.print("mA");lcd.print("    ");
 
Serial.print(vol);
Serial.print(","); 
Serial.print(cur);
Serial.print(","); 


//-------------------calories--------------------//
  time = millis();
 // Serial.print("Calories Consumed ");
  //Serial.println(bwt);
  Serial.print((1.05*5.5* float(time/1000) * bwt)/60/60);//hour
  Serial.print(",");
 // Serial.print(" kcal");
 // Serial.print(" ");
 // Serial.print(time/1000/60);
//  Serial.println(" min");
//  delay(50); 
Serial.println("...");
//------------------LCD--------------------//

  if (wt>6)
  {
    sound1(6);
  }
  else if (wt>4)
  {
    sound1(4);   
  }
  else if(wt>2)
  {
    sound1(2);   
  }
 //-------------------BT--------------------//
//day & time
 /*   blt.print(now.year(), DEC);
    blt.print('/');
    blt.print(now.month(), DEC);
    blt.print('/');
    blt.print(now.day(), DEC);
    blt.print(" - ");
    blt.print(now.hour(), DEC);
    blt.print(':');
    blt.print(now.minute(), DEC);
    blt.print(':'); 
    blt.print(now.second(), DEC);
    blt.print(" - ");
 //power
    blt.print(wt, 2); 
    blt.println("kg"); 
 //speed
    blt.print("   maxvelocity= ");
    blt.print(maxvelocity);
    blt.println(" m/s"); 
 //calories
  time = millis();
  blt.print("Calories Consumed ");
  blt.print((1.05*5.5* float(time/1000) * bwt)/60/60);
  //blt.print(1.05*5.5* 1 * wt);//minute 1.05*55*1*50=228
  blt.println(" kcal");
  delay(100); // 1秒おきに送信*/
BTserial.print(maxvelocity);

BTserial.print(",");

BTserial.print((1.05*5.5* float(time/1000) * bwt)/60/60);

BTserial.print(",");

BTserial.print(Xa);

BTserial.print(",");

BTserial.print(Ya);

BTserial.print(";");

//message to the receiving device

delay(20);
  }
}



