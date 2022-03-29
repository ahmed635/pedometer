/*---------------------------------------Pedometer Device ----------------------------*/
/*------------------------------------ Libraries --------------------------------------*/
#include <Wire.h>
#include <DHT.h>
// Import the Liquid Crystal library
#include <LiquidCrystal.h>
/*------------------------------------------------------------------------------------*/

 /*------------------------------- Macros --------------------------------------------*/
#define DEVICE (0x53)    //ADXL345 device address
#define TO_READ (6)        //num of bytes we are going to read each time (two bytes for each axis)
 
#define offsetX   -10.5       // OFFSET values
#define offsetY   -2.5
#define offsetZ   -4.5
 
#define gainX     257.5        // GAIN factors
#define gainY     254.5
#define gainZ     248.5


#define PIN 8 // pin whtich connected to arduino
#define TYPE 11  // type of the sensor 

/*-----------------------------------------------------------------------------------*/

/*--------------------------------- objects -----------------------------------------*/
//Initialise the LCD with the arduino. LiquidCrystal(rs, enable, d4, d5, d6, d7)
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
DHT dht(PIN , TYPE); // creating an object to control the sensor

/*-----------------------------------------------------------------------------------*/


/*--------------------------------------- Variables ---------------------------------*/
float hum , temp_c , temp_f = 0;

byte buff[TO_READ] ;    //6 bytes buffer for saving data read from the device
char str[512];                      //string buffer to transform data before sending it to the serial port
 

int x,y,z;
 
int xavg, yavg,zavg, steps=0, flag=0;
int xval[15]={0}, yval[15]={0}, zval[15]={0};
int threshhold = 30.0;

float comprimento_do_passo;
float calories = 0;
float peso = 57;
float altura = 168;
float calories_lost_per_km;
float calories_burned; 
float distance;
float passos_por_milha;

const int buttonPin = 13; // connect button to make the DHT turned on
const int resetPin = 7 ;  // connect button to reset the steps
/*----------------------------------------------------------------------------------------------*/


void setup() {
  
  Wire.begin();        // join i2c bus (address optional for master)
  Serial.begin(9600);  // start serial for output
    // Switch on the LCD screen
  lcd.begin(16, 2);
  dht.begin();

   digitalWrite (buttonPin , INPUT);
   digitalWrite (resetPin , INPUT);
   
  comprimento_do_passo=0.30*altura; // Height in cm
  calories_lost_per_km  =(0.57*peso*1.6)/0.453; // Weight in kg
  passos_por_milha = 160000.0/comprimento_do_passo; // 16000.0 CM = 16 KM

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Pedometer");
  //lcd.setCursor(0, 1);
  //lcd.print("Made by Group A");
  delay(3000);

  
  //Turning on the ADXL345
  writeTo(DEVICE, 0x2D, 0);      
  writeTo(DEVICE, 0x2D, 16);
  writeTo(DEVICE, 0x2D, 8);
}

void loop() {
  
 /*-------------------------------- accelerometer code ----------------------------*/
 
  int regAddress = 0x32;    //first axis-acceleration-data register on the ADXL345
  
  readFrom(DEVICE, regAddress, TO_READ, buff); //read the acceleration data from the ADXL345
  
   //each axis reading comes in 10 bit resolution, ie 2 bytes.  Least Significat Byte first!!
   //thus we are converting both bytes in to one int
  x = (((int)buff[1]) << 8) | buff[0];   
  y = (((int)buff[3])<< 8) | buff[2];
  z = (((int)buff[5]) << 8) | buff[4];
  
//we send the x y z values as a string to the serial port 
//  sprintf(str, "%d %d %d", x, y, z);  
//  Serial.print(str);
//  Serial.print(10, byte());
  x = ArduinoPedometer();
  Serial.print("steps=");
  Serial.println(x);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Steps : ");
  lcd.print(x);
  //It appears that delay is needed in order not to clog the port
  delay(10);
/*---------------------------------------------------------------------------------------*/  
 
/*------------------------------- code of button which turned tne DHT ---------------------*/  
 if (digitalRead(buttonPin) == HIGH) {
  
  /*---------------------------burned calories ----------------------------------------*/
 calories_burned = steps*(steps/passos_por_milha);
 lcd.clear();
 lcd.setCursor(0, 0);
 lcd.print("Calories B.: "); 
 lcd.setCursor(0, 1);
 lcd.print(calories_burned); 
 lcd.print(" Kcal");
 delay(2000);

/*-----------------------------------------------------------------------------------------*/

/*-------------------------------Distance ------------------------------------------------*/
  distance = (comprimento_do_passo*steps)/100; // Distance in meters

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Distance: ");
  lcd.setCursor(0, 1);
  lcd.print(distance);
  lcd.print(" Meters");
  delay(2000);
/*---------------------------------------------------------------------------------------------*/
  
 /*------------------------------------ dht code ----------------------------------------------*/
  delay(2000); // wait a few second to work the sensor
  hum = dht.readHumidity();
  temp_c = dht.readTemperature();
  temp_f = dht.readTemperature(true); 

  // check if the sensor is working
  if (isnan(hum || temp_c || temp_f )) {
    Serial.print("Failed to read DHT sensor");
    return ;  // to repeat the reading from the sensor
  }
  
  Serial.print("Humidity ");
  Serial.print (hum);
  Serial.print("\t");
  Serial.print("temperature ");
  Serial.print(temp_c);
  Serial.print("c\t");
  Serial.print(temp_f);
  Serial.print("f");
  Serial.println();
  
  // printing by lcd
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Humidity");
  lcd.setCursor(0,1);
  lcd.print(hum);
  lcd.print(" %");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Temperature");
  lcd.setCursor(0,1);
  lcd.print(temp_c);
  lcd.print(" c");
  delay(2000);
  lcd.clear();
  /*
  lcd.setCursor(7,1);
  lcd.print(temp_f);
  lcd.print("f");
  delay(3000);
  */
  /*--------------------------------------------------------------------------------------*/
 }
 /*-----------------------------------------------------------------------------------------*/

 
/*-------------------------------------------Reset the Steps -----------------------------*/
 if (digitalRead(resetPin) == HIGH ) {
  steps = 0;
 }
/*-------------------------------------------------------------------------------------------*/
}


/*------------------------------------- Functions ------------------------------------------*/
//Writes val to address register on device
void writeTo(int device, byte address, byte val) {
   Wire.beginTransmission(device); //start transmission to device 
   Wire.write(address);        // send register address
   Wire.write(val);        // send value to write
   Wire.endTransmission(); //end transmission
}
 
//reads num bytes starting from address register on device in to buff array
void readFrom(int device, byte address, int num, byte buff[]) {
  Wire.beginTransmission(device); //start transmission to device 
  Wire.write(address);        //sends address to read from
  Wire.endTransmission(); //end transmission
  
  Wire.beginTransmission(device); //start transmission to device
  Wire.requestFrom(device, num);    // request 6 bytes from device
  
  int i = 0;
  while(Wire.available())    //device may send less than requested (abnormal)
  { 
    buff[i] = Wire.read(); // receive a byte
    i++;
  }
  Wire.endTransmission(); //end transmission
}
 
 
/*------------------------------ Get pedometer.-------------------------------------*/
 
int ArduinoPedometer(){
    int acc=0;
    int totvect[15]={0};
    int totave[15]={0};
    int xaccl[15]={0};
    int yaccl[15]={0};
    int zaccl[15]={0};
    for (int i=0;i<15;i++)
    {
      xaccl[i]= x;
      delay(1);
      yaccl[i]= y;
      delay(1);
      zaccl[i]= z;
      delay(1);
      //totvect[i] = sqrt(((xaccl[i]-xavg)* (xaccl[i]-xavg))+ ((yaccl[i] - yavg)*(yaccl[i] - yavg)) + ((zval[i] - zavg)*(zval[i] - zavg)));
      totvect[i] = sqrt(((xaccl[i]-xavg)* (xaccl[i]-xavg))+ ((yaccl[i] - yavg)*(yaccl[i] - yavg)) + ((zaccl[i] - zavg)*(zaccl[i] - zavg)));
      totave[i] = (totvect[i] + totvect[i-1]) / 2 ;
      delay(150);
  
      //cal steps 
      if (totave[i]>threshhold && flag==0)
      {
         steps=steps+1;
         flag=1;
      }
      else if (totave[i] > threshhold && flag==1)
      {
          //do nothing 
      }
      if (totave[i] <threshhold  && flag==1)
      {
        flag=0;
      }
     return(steps);
    }
  delay(10); 
 }
 /*---------------------------------------------------------------------------------*/
