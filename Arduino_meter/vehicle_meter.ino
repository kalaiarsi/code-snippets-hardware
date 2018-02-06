
// Libraries Used...........................//
#include <Arduino.h>
#include <LiquidCrystal.h>      // Library for using LCD with Arduino (Predefined)
#include <SoftwareSerial.h>        
#include <GxxxxL.h>               // Library to integrate GPS with Arduino (Created)
#include <avr/pgmspace.h>

GxxxxxL xyz;                      // Initializing GPS Library by creating an instance
SoftwareSerial gprs_Serial(5,6);  // Rx,Tx pins of Arduino, Rx of Arduino->Tx of GPRS and vice versa

LiquidCrystal lcd(12,2, 7, 8, 9, 10); // Pin connections of Arduino with LCD (RS (Register Select), En(Enable), D4,D5,D6,D7 (Data Pins));

const int odo = 3;      // Arduino Pin using Odometer as an Interrupt
int odo_now = HIGH;     // Odometer new state
int odo_last = HIGH;    // Odometer Last State
int i=0;                // Variable for loops
int min_fare=25;        // Minimum Fare 
float min_distance=0.3;  // Minimum Distance
int fare=0;              // Initial Fare By default
float distance=0.0;       // Initialize Variable to store distance 
int addl_distance=0;        
int fare_per_addl_km=10;   
boolean gprs_status=false;  // GPRS indicator Status
boolean gps_status=false;   // GPS Indicator Status
const int led_gprs=A5;       // LED to indicate GPRS status (Analog pin 5)
float km_per_odo_pulse=0.01;       // Distance to be added for one odometer pulse
float overall_distance=0.0;        

const int led_gps_post=A4;         // LED indicating the GPS status attached to Analog pin 4
int value_accuracy=1;
boolean inside_gps_post=false;     // variable to show the status of GPS post
char imei[15];                     // variable to store IMEI of GPRS chip (Using as unique ID of meter)

const int panic=A0;                // Initializing Panic variable on Analog Pin 0 
const int led_panic=A1;            // LED to show the status of panic connected to Analog pin 1
const int hire=A2;                 // Variable to store the status of hire
const int led_hire=A3;            // Led to show hire status connected to Analog pin 3
boolean hired=false;              // hire status indicator
boolean h_now = true;             
boolean h_last = false;
boolean off_hire_to_on_hire=false;     // variable to store off->on hire status ( As printer will print a receipt when status will be true)     
//char gps_data[63]="-99999-999,9999-9999,N,99999.9999,E,0,0,9.99,123456789012345,t";
char gps_data[63]="00..--.---,----.----,N,-----.----,E,-,-,-.--,---------------,t"; // Variable used to store gps data
//char gps_data[63]="000051.000,1259.5389,N,08014.4333,E,1,4,2.31,---------------,t";
char server_response[16]="------,Q-------";
boolean IPstatus=true;  // Variable to store the IP staus of GPRS (whether IP is obtained or not)

void lcd_display(char line1[],char line2[])  // Function to print something on LCD line by line (sometext (line 1), sometext (line 2))
{                                            // Because, when we print something on lcd more than 16 characeters (without using this function) text will get cut after 16 characters
  lcd.setCursor(0, 1);
  lcd.print(line1);
  lcd.print(line2);
  delay(40);
}

int freeRam () {                          // Function to check free ram of arduino board at a paticular instant of time (defined in arduino)
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

prog_char register_url[] PROGMEM = "AT+HTTPPARA=\"URL\",http://121.242.232.160:8080/ard/registration";  
//prog_char cood_url[] PROGMEM = "AT+HTTPPARA=\"URL\",http://121.242.232.160:8080/ard/addcood";
prog_char cood_url[] PROGMEM = "AT";
prog_char register_data[] PROGMEM = "{\"username\":\"123456789011111\",\"password\":\"87654821\",\"name\":\"w\",\"emailId\":\"8@gmail.com\"}";
prog_char panic_format[] PROGMEM = "Panic at : 0034.58746,N,08080.4578,E,help reqd:888456789012345\x1A";
//char gps_data[63]="104051.000,1259.5389,N,08014.4333,E,1,4,2.31,---------------,t";

prog_char fetch_rule[] PROGMEM = "AT+HTTPPARA=\"URL\",http://121.242.232.160:8080/ard/fetch_rule";

PROGMEM const char *list[] = 	   // change "string_table" name to suit
{
register_url,  
cood_url,  
register_data,  
panic_format,
fetch_rule};


void send_panic_sms_and_call()     // function called at the time of emergencies (panic)...will send message to a particular number having location (latitude, longitude) and Unique Id (IMEI)) 
{
  //Serial.println(F("inside send_panic_sms_and_call"));
  //char msg1[75];
  //strcpy_P(msg1, (char*)pgm_read_word(&(list[3])));
  //Serial.println(F("msg from progmem is :"));Serial.print(msg1);
  char* msg1="Panic at : 0034.58746,N,08080.4578,Ehelp reqd888456789012345 \x1A";  // Variable to store the message to be send at the time of panic ( \x1A indicates that message has end)
  for (i=11;i<=35;i++){msg1[i]=gps_data[i];}  // Replace the location with the current location obtained from gps
  for (i=45;i<=59;i++){msg1[i]=gps_data[i];}   // Replace the IMEI with IMEI of the device (GPRS)
  //Serial.println(msg1);
  //Serial.read();
  int counter=0;  // Storing the number of bytes available on gps
  char from_cell=0; // variable to store message/text sent to gprs
  //delay(200);
  Serial.println(F("SENDING SMS"));         // Message on Serial Monitor
  //delay(200);
  gprs_Serial.write("AT+CMGF = 1 \r");        // command to declare that message sent will be in text format
  delay(1000);
  gprs_Serial.write("AT+CMGS=\"+919xxxxxxxx8\" \r");   //Command to add number to which message will be sent
  delay(1000);

  gprs_Serial.print(msg1); // writing message to GPRS, that has to sent
  delay(1200); 
  //Serial.println(freeRam());
  counter=gprs_Serial.available  // get the number of bytes available on gprs ( using available function defined in arduino ) and store it in counter
  for(int i=0;i<counter;i++)   // run the loop for number of times stored in counter
  {from_cell = gprs_Serial.read(); // read data from gprs buffer (using read function defined in arduino) and store it in from_cell variable 
   Serial.print(from_cell);}   // Print the data on Serial monitor 
  delay(1000);
  //Serial.println(F("\n\nMAKING A CALL"));
  ////delay(1000);
  //gprs_Serial.write("ATD+919500834468;\r");
  //delay(3000);
  //counter=gprs_Serial.available();
  //for(int i=0;i<counter;i++){from_cell = gprs_Serial.read();Serial.print(from_cell);}
  //counter=gprs_Serial.available();
}

boolean get_gps_cood()    // Function to collect gps data
{
  //Serial.println(freeRam());
  xyz.init();  // It is defined under GxxxxxL library and will initialize the GPS software serial pins (Check GxxxxxL library for details)
  delay(500);
  xyz.listen();  // Under GxxxxxL, will enable the gps pins to listen ( As using multiple Software serial pins only one can listen at a time) 
  //delay(500);
  char* collected_gps=xyz.collectdata();  // Collecting data using collectdata function (Under GxxxxxL) and storing it in a variable
  //if(!collected_gps){delay(700);collected_gps=xyz.collectdata();}
  //Serial.print(F("GPSdata from Library is:  "));Serial.println(collected_gps);
  Serial.print(F("gpsdata before changing..:  "));  
  Serial.println(gps_data); // Printing the collected data on Serial Monitor
// if(!collected_gps){xyz.end();return false;} 
  if(collected_gps) // If GPS has Collected Data  (Fixed)  
  
  {for(int i=0;i<44;i++)   
  gps_data[i]=collected_gps[i]; } // Change the first 44 characters of GPS data with the data collected just now
  else  {Serial.println(F("GPS not set"));return false;}  // If gps has not fixed yet
  Serial.print(F("gpsdata after  changing..:  "));  
  Serial.println(gps_data);  // New gps data with IMEI added 
  //Serial.println(F("exiting get_gps_cood"));
  xyz.end();  // disable the gps software serial pins
  return true;  
}

void odo_reading()   // function to read odometer pulses
{
  odo_now = digitalRead(odo);   // Read odometer status and store it in odo_now

  if((odo_now==LOW)&&(odo_last==HIGH))
  {    
    overall_distance=overall_distance+km_per_odo_pulse;
    if((digitalRead(hire)==LOW)&&(hired==true))//&&(i==10))
    {
      distance=distance+km_per_odo_pulse;
      if(distance<min_distance)
      {
        fare=min_fare;
      }
      else
      {
        addl_distance=int(distance-min_distance)+1;
        fare=min_fare+(addl_distance*fare_per_addl_km);
      }
      }
  }
  odo_last = odo_now;

  delay(10);
}

void panic_generated()            // Function to check the status of panic
{
  boolean is_panic_pressed=true;   // Variable to store panic status
  //Serial.println(freeRam());
  //Serial.println(F("panic func"));
  detachInterrupt(1);             // Disabling the Odometer Interrupt
  delay(100);
  digitalWrite(led_panic,HIGH);      // Blink the led corresponds to Panic status
  //Serial.println(freeRam());
  lcd_display("Panic Situation","help!");  // Display on LCD
  send_panic_sms_and_call();   // Call sms_panic_and_call_function ( to send sms in case of panic generation)
//  while(1!=0)
//  {
//    delay(5000);
//  }
  Serial.println(F("getting back from panic sms and call"));  // Status on Serial monitor
  attachInterrupt(1,odo_reading,FALLING); // Initialize iterrupt again
}

void printer_flush()   // function to flush out the data stored in printer's buffer
{
  SoftwareSerial printer(1,4);//Rx, Tx of Arduino , Rx of Arduino-> Tx of Printer and vice versa
  printer.begin(9600); // Initialize software serial pins of printer 
  delay(500);
  printer.flush();   
  while(printer.available()!=0){printer.read();}  // Read all the data stored in printer's buffer and flush it out 
  delay(500);
  printer.end();  // Disable the software serial pins of printer 
  delay(200);  
}

void fare_printer(float distance,int fare)  //Function to print the fare receipt using Printer 
{  
  //Serial.println(freeRam());
  SoftwareSerial printer(1,4);//rx,tx of arduino
  printer.begin(9600); // Initialize software serial pins of printer
  delay(300);
//  printer.flush();
//  while(printer.available()!=0){printer.read();}
//  delay(300);
  printer.println(F("\n\n\n\t\t Welcome to \n"));
  printer.println(F("\t\t Intellimeter\n"));
  printer.print(F("\t\t\t distance : "));
  printer.print(distance,1);    // Print the distance covered during journey on receipt
  printer.println(F(" KM"));
  printer.print(F("\t\t\t fare : "));
  printer.print(F("RS "));
  printer.println(fare);     // print the fare of journey on receipt
  printer.println(F("\n\n\n"));
  delay(700);
  lcd_display("After bill print","Vehicle Ready  "); // Print status on LCD
  printer.end();  // disable the printer software serial pins
}

void check_for_hire_status()  // Function to check hire status
{  //Serial.println(freeRam());
  h_now=digitalRead(hire); // read the status of hire pin and store it in h_now variable

  if(h_now==LOW) // if h_now is low, it means auto has gone to on hire status 
  {
    hired=true;   // hire status is true as auto is hired now
   gps_data[61]='f';  // Indicator to server that vehicle has gone to on hire state 
    digitalWrite(led_hire,HIGH); // Blink the led corresponds to hire status
    if(h_last==HIGH)   // if auto was in off hire status previously
    {
      off_hire_to_on_hire=true; // auto has gone from off hire to on hire
      distance=0.0;fare=min_fare;  // initializing initial distance and fare 
      lcd.setCursor(0,0);
      lcd.print(distance,1);      // Print distance on lcd
      lcd.print(" KM ");
      lcd.print("RS ");
      lcd.print(fare);           // printing fare on lcd
      lcd.print("   ");
//      Serial.println(F("last point flag is: "));
    }
  }
  else
  {
    hired=false;        //  else means auto has gone to off hire from on hire (time for printer receipt) 
    gps_data[61]='t';   // indicator to server that vehicle has gone to off hire
    digitalWrite(led_hire,LOW);  // Stop led blinking as vehicle is in off hire position now
    if(h_last==LOW){
      if(off_hire_to_on_hire==true){ 
       printer_flush();   // flush out the previous parameters stored in printer's buffer
        fare_printer(distance,fare); // call printer to print the receipt (defined earlier)
        lcd_display("Printing bill:","Fare & distance"); // display the status on lcd
        delay(500);
        fetch_fare_rule(); // fetch fare paramters from server
      }
    }
  }
  h_last = h_now;
}
  
void gprs_ShowSerialData() // function to show the data stored gprs's buffer on serial monitor
{
  gprs_Serial.listen();   // listen to the gprs software serial pins
  while(gprs_Serial.available()!=0)
  {
    Serial.write(gprs_Serial.read());
  }
}

boolean gprs_ShowSerialData_1() // function to show the IP status of gprs on serial monitor (whether it has ip or not)
{
  gprs_Serial.listen();  // listen to gprs software serial pins
  char c;           
  int i=0; 
  boolean IP=false;      // variable to store the ip status (false means ip is not set, true means ip is set)
  while(gprs_Serial.available()!=0)
  {
    c=gprs_Serial.read();
    Serial.write(c);//Serial.print(i);Serial.print(' ');
  //if(i==34)
  if(i==26) 
      if(c=='1')
      {IP=true;//Serial.println(F("char is "));Serial.write(c);
    }
    i=i+1;
  }
  return IP;
}

void to_set_IP()    // Function to Set Ip in case if it nor set
{
  //Serial.println(freeRam());
  gprs_Serial.println(F("AT+CPIN"));  // Check the status of SIM 
  delay(3500);
  gprs_ShowSerialData();
  gprs_Serial.println(F("AT+CSQ"));  // checking signal strength
  delay(500);
  gprs_ShowSerialData();
  gprs_Serial.println(F("AT+CGATT?"));   // check the status of gprs , 0 means its not attached, 1 means its attached
  delay(1000);
  gprs_ShowSerialData(); 
  set_bearer();  // 
}

void set_bearer()  // Setting bearer parameters for applications based on ip (like post and get)
{
  gprs_Serial.println(F("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\""));   // Connection type is GPRS
  delay(1000);
  gprs_ShowSerialData();
  gprs_Serial.println(F("AT+SAPBR=3,1,\"APN\",\"www\""));  // APN(Access point Name) of vodafone is www (Change it according to the Sim you use)
  delay(1500);
  gprs_ShowSerialData();
  gprs_Serial.println(F("AT+SAPBR=1,1"));     // Set IP
  delay(2000);
  gprs_ShowSerialData();
  gprs_Serial.println(F("AT+SAPBR=2,1"));    // Query for IP
  delay(2000);
  IPstatus=gprs_ShowSerialData_1();
}

boolean GPRS_initialise()  // Function to Initialise gprs
{
  //Serial.println(freeRam());
  gprs_Serial.begin(9600);   // initialize GPRS software serial 
  delay(500);//gprs_Serial.flush();//added flush on 04042014
  if(hired==false)
  {
    lcd_display("GPRS being ","initiali");  // Display status on lcd
  }
  delay(200);
  //Serial.println(freeRam());
  check_for_hire_status(); // checking hired status
   if(hired==true)  
 {lcd_fare_display();} // if hired status is true print the fare receipt of journey
  if(digitalRead(panic)==LOW) 
  {
   panic_generated();
  }
  gprs_Serial.listen();   // listen to gprs software serial pins
  gprs_Serial.println(F("AT+SAPBR=2,1"));// Query for IP
  delay(3000);
  IPstatus=gprs_ShowSerialData_1(); 
  check_for_hire_status();   // checking hired status
  if(hired==true)
   {lcd_fare_display();} // if hired status is true print the fare receipt of journey

  if(IPstatus==false)   // If ip is not set, call the function to set ip
  {
    to_set_IP();    // function to set ip
  }
  if(IPstatus==true)
  {
    if(hired==false){
      lcd_display("GPRS connected"," plz wait");
    }
    digitalWrite(led_gprs,LOW);
  }
  if(IPstatus==false)
  {
    if(hired==false)
    {
      lcd_display("GPRS not","connected");
    }
    digitalWrite(led_gprs,HIGH);
  }
  //Serial.println(freeRam());
  check_for_hire_status(); // call function to check hire status
  if(hired==true){lcd_fare_display();}
  gprs_status=IPstatus;
  gprs_Serial.end();
  return gprs_status;
}

void http_read(int wait)    // Function to read http parameters
{
  gprs_Serial.println(F("AT+HTTPREAD"));   // At command to read http parameters
  gprs_Serial.listen();  // listen to gprs software 
  gprs_ShowSerialData();
  delay(wait);
}

void http_term(int wait)
{
  gprs_Serial.println(F("AT+HTTPTERM")); // Terminate http service
  gprs_Serial.listen();   // listen to GPRS software serial pins
  gprs_ShowSerialData();   
  delay(wait);
}

void http_init(int wait)   // initialize http service
{

  gprs_Serial.println(F("AT+HTTPTERM"));   // at command to terminate http service
  delay(wait);
  gprs_Serial.println(F("AT+HTTPINIT"));   // at command to terminate http service
  delay(wait);
  gprs_Serial.listen();    // listen to GPRS software serial pins
  gprs_ShowSerialData();
}

boolean gprs_ShowSerialData_3()
{
  gprs_Serial.listen();  
  char c;
  int i=0;
  boolean gprs=true;
  while(gprs_Serial.available()!=0)
  {
    c=gprs_Serial.read();
    Serial.write(c);
    if(i==0)
      if(c!='A')
      {
        gprs=false;
        //Serial.print(F("..gprs..not..fine.."));
      }
    i=i+1;
  }
  return gprs;
}

void http_url_gps(int wait)  // Add url To send gps coordinates
{
  gprs_Serial.println(F("AT+HTTPPARA=\"URL\",http://1xxxxxxxx0:8080/ard/addcood"));  // AT command to add URL
  delay(wait);
  gprs_Serial.listen();
  gprs_status=gprs_ShowSerialData_3();
  if(gprs_status==false)
  {
    digitalWrite(led_gprs,HIGH);
    GPRS_initialise();    // Initialize GPRS
  }
}

void http_url(char url[],int wait)  
{
  gprs_Serial.println(url);
  delay(wait);
  gprs_Serial.listen();
  gprs_status=gprs_ShowSerialData_3();
  if(gprs_status==false)
  {
    digitalWrite(led_gprs,HIGH);
    GPRS_initialise();
  }
}

void http_para_cid_content(int wait)   // enter the type of content for http service like application/json or application/text
{
  gprs_Serial.println(F("AT+HTTPPARA=\"CID\",1"));
  delay(wait);
  gprs_Serial.listen();
  gprs_ShowSerialData();
  gprs_Serial.println(F("AT+HTTPPARA=\"CONTENT\",application/json"));   // AT command to add content type
  delay(wait);
  gprs_Serial.listen();
  gprs_ShowSerialData();
}


int check_signal() // Check signal strength
{
  char c[3];
  int csq;
  gprs_Serial.println(F("AT+CSQ"));  // AT command to check signal strength
  delay(500);
  //gprs_ShowSerialData();
  gprs_Serial.listen();
  //while((gprs_Serial.available()!=0)&&(c!=' ')){c=gprs_Serial.read();Serial.write(c);}
  //i=8;
  //while(gprs_Serial.available()!=0){server_response[i]=gprs_Serial.read();Serial.write(server_response[i]);i++;}
  csq=gprs_Serial.parseInt(); 
 // Serial.print(F("signal strength is"));Serial.println(csq);
while(gprs_Serial.available()!=0){gprs_Serial.read();}  

if(csq<10){server_response[8]='0';server_response[9]='0'+csq;return(10-csq);}
  else{server_response[8]='0'+(csq/10);server_response[9]='0'+(csq%10);return(0);}
  
}

void http_action_post(int wait)    // posting the data
{
  gprs_Serial.println(F("AT+HTTPACTION=1"));//AT command to post the data
  delay(wait);
  gprs_Serial.listen();delay(2*wait);
  gprs_ShowSerialData();
  
 // Serial.println(F("a-over, read"));
}

void http_read_fare(int wait)   // Read fare parameters from server
{
  Serial.println(freeRam());
  gprs_Serial.println(F("AT+HTTPREAD"));  // AT command to read the data
  delay(2*wait);//delay(700);
  //00201.8000201430002500025020.2 //  Serial.println(F("read."));
  char fare_details[31];  // variable to store fare details 
  // gprs_Serial.listen();
  //if(gprs_Serial.available()==0){delay(1000);}
  //if(gprs_Serial.available()==0){delay(1000);}
  //while(gprs_Serial.read()!='\n')
  //delay(3000);
  gprs_Serial.listen();
  if(gprs_Serial.available()!=0)
  {
  delay(2000);
  //while(gprs_Serial.read()!='\n'){}    //gprs_Serial.read();gprs_Serial.read();    //while(gprs_Serial.read()!='\n'){}
  char c=',';
  while(c!=':'){c=gprs_Serial.read();Serial.write(c);delay(5);}
  Serial.write(c);//delay(100);
  while(c!='\n'){c=gprs_Serial.read();Serial.write(c);}
  Serial.write(c);
  //for(int i=0;i<19;i++)  {Serial.write(gprs_Serial.read());delay(20);}
  //delay(1000);gprs_Serial.listen();
  //for(int i=19;i<29;i++)  {Serial.write(gprs_Serial.read());delay(20);}
  //delay(1000);gprs_Serial.listen();
  
  
  // for(int i=33;i<53;i++)  {Serial.write(gprs_Serial.read());delay(20);}
  //gprs_Serial.listen();
  // Serial.println("\n\n\n");
  // while(gprs_Serial.available()<31){delay(100);}
  delay(2000);
  i=0;
  while(i<31){fare_details[i]=gprs_Serial.read();i++;}//delay(10);}
  //Serial.println(F("extracted fares:"));
  }
  Serial.println(F(" Fare params : "));//....................................................
  Serial.println(fare_details);//Serial.println(F("over"));
 //00251.2000201430002500025020.2 - Response from server (200,OK)
  //Serial.print("\n\n\n");
  int min_fare1=0;
  for (i=0;i<4;i++){min_fare1=min_fare1+((int(fare_details[i]-'0'))*(int(pow(10,3-i))));}//Serial.println(min_fare1);}
 // Serial.print(int(pow(10,1)));
  float min_distance1=0.0;
  min_distance1=min_distance1+(fare_details[4]-'0');
  min_distance1=min_distance1+(0.1*(fare_details[6]-'0'));
  int fare_per_addl_km1=0;
  for (i=0;i<5;i++){fare_per_addl_km1=fare_per_addl_km1+((fare_details[7+i]-'0')*(int(pow(10,4-i))));}
  Serial.println(F("min_fare,min_distance,fare_per_addl_km.."));
  Serial.println(min_fare1);
  Serial.println(min_distance1);
  Serial.println(fare_per_addl_km1);
  min_fare=min_fare1;
  min_distance=min_distance1;
  fare_per_addl_km=fare_per_addl_km1;  
//  update_fare_params_in_code();
  
}

void fetch_fare_url()    // Fetching fare url from program memory, declared earlier
{
 //Serial.println(freeRam());
 char fetch_fare_url[65];
 strcpy_P(fetch_fare_url, (char*)pgm_read_word(&(list[4])));
 http_url(fetch_fare_url,300);
}

void send_trip_data()  
{ 
  //Serial.println(freeRam());
  char trip_end_data[28]="";
  char imei[16]="";
  for(int i=0;i<15;i++){imei[i]=gps_data[45+i];}
  char temp[7]="000000";
  strcat(trip_end_data,imei);
  strcat(trip_end_data,",");
  ltoa((distance*1000),temp,10);
  Serial.println(F("distance : "));Serial.print(temp);
  for(int n=strlen(temp);n<6;++n)strcat(trip_end_data,"0");
  strcat(trip_end_data,temp);
  strcat(trip_end_data,",");
  itoa(fare,temp,10);
  for(int n=strlen(temp);n<4;++n)strcat(trip_end_data,"0");
  strcat(trip_end_data,temp);Serial.println(trip_end_data);
  gprs_Serial.println(trip_end_data);
  delay(500);
  gprs_ShowSerialData();
  delay(300);
}
 
void fetch_fare_rule()
{
  //Serial.println(F("inside fetch_fare_rule"));Serial.println(freeRam());
  gprs_Serial.begin(9600);
  delay(100);
  lcd_display("Fare params"," retriving..");
  if(digitalRead(panic)==LOW){panic_generated();}
  gprs_Serial.listen();
  http_init(300);
  fetch_fare_url();
  http_para_cid_content(300);
  gprs_Serial.println(F("AT+HTTPDATA=28,1000"));
  delay(700);
  send_trip_data();
  http_action_post(1200);
  http_read_fare(1200);
  //Serial.println(F("fares list"));Serial.print(fare_params);
  http_term(300);
  gprs_Serial.end();
}

//void user_register()
//{
//  Serial.println(freeRam());
//  gprs_Serial.begin(9600);
//  delay(500);
//
//  char register_url[64];
//  char register_data[80];
//  strcpy_P(register_url, (char*)pgm_read_word(&(list[0])));
//  strcpy_P(register_data, (char*)pgm_read_word(&(list[2])));
//  Serial.println(freeRam());
//  
//  lcd_display("User being"," registered");
//  if(digitalRead(panic)==LOW){panic_generated();}
//  gprs_Serial.listen();
//  http_init(500);
//  http_url(register_url,500);
//  http_para_cid_content(500);
//  gprs_Serial.println(F("AT+HTTPDATA=50,3500"));//90..50
//  delay(300);
//  gprs_Serial.println(register_data);
//  delay(500);
//  gprs_ShowSerialData();
//  delay(300);
//
//  http_action_post(2000);
//  http_read(1500);
//  http_term(600);
//  gprs_Serial.end();
//  //Serial.println(freeRam());
//}

void http_action_post_gpscood(int wait)
{
  Serial.println(freeRam());
  char c; //int resp;
  gprs_Serial.println(F("AT+HTTPACTION=1"));//submit the request
  delay(wait);
   
  gprs_Serial.listen();
  c='.';  
  delay(2*wait);
  int g=300;
  while((c!=',')&&(g>0))
    {
      c=gprs_Serial.read();
     if((c>='A')&&(c<='Z')){Serial.write(c);}
     else{g--;}
    }
  i=11;
  while(gprs_Serial.available()!=0)
  {
    server_response[i]=gprs_Serial.read();
    Serial.write(server_response[i]);
    i++;
  }
//while((gprs_Serial.available()!=0)&&(c!='1')){c=gprs_Serial.read();Serial.write(c);}
//  
//  resp=gprs_Serial.parseInt();
//  resp=gprs_Serial.parseInt();
//  Serial.print(F("resp is"));Serial.print(resp);
//  server_response[11]='0'+int(resp/100);
//  resp=resp%100;
//  server_response[12]='0'+int(resp/10);
//  resp=resp%10;
//  server_response[13]='0'+resp;  


    //Serial.print(F("server response b4 exiting hapgc"));Serial.print(server_response);

}

void http_gps_post()  //  Function to post gps data to server
{
  //Serial.println(F("inside httppost"));
 //Serial.println(gps_data);
  gprs_Serial.begin(9600);
  delay(100);
  char gps_url[65];
  strcpy_P(gps_url,(char*)pgm_read_word(&(list[1])));
  check_for_hire_status();if(hired==true){lcd_fare_display();} 
  int w=check_signal();
  Serial.println(w);
  if (w>4)
    {digitalWrite(led_gprs,HIGH);Serial.println(F("Weak signal.. csq<6"));
    lcd.setCursor(0,1);lcd.print("WeakSignal str<6");return;}//since returned value is 10-csq
  else{digitalWrite(led_gprs,LOW);}
  http_init(500+(w*250));
  //  http_url(gps_url,500);
  http_url_gps(250+(w*50));
  http_para_cid_content(200+(w*50));
  digitalWrite(led_gps_post,HIGH);
  check_for_hire_status();if(hired==true){lcd_fare_display();}   
  if(digitalRead(panic)==LOW){ panic_generated();}
  
  //delay(200);
  
  inside_gps_post=true;
//  gprs_Serial.println(F("AT+HTTPDATA=235,5000"));
  gprs_Serial.println(F("AT+HTTPDATA=62,2000"));
  delay(300);

  gprs_Serial.println(gps_data);
  delay(1000+(w*150));
  gprs_ShowSerialData();
  delay(300+(w*50));//200
  //
  check_for_hire_status();if(hired==true){lcd_fare_display();} 
  if(digitalRead(panic)==LOW){
   panic_generated();
  }
  w=check_signal();
  http_action_post_gpscood(1000+(w*250));
  
  lcd.setCursor(0,1);lcd.print("next GPS cood...");delay(200);
  
  digitalWrite(led_gps_post,LOW);
  inside_gps_post=false;
  http_read(800+(w*150));
  http_term(700);
  
  Serial.println(freeRam());
  
  for(i=0;i<6;i++){server_response[i]=gps_data[i];} 
  server_response[15]=gps_data[61];
  if((server_response[11]=='6')&&(server_response[13]=='4'))

//flush
    {gprs_Serial.flush();delay(300);
//restart Serial???
    gprs_Serial.end();delay(500);
     gprs_Serial.begin(9600);delay(500);}
   
  
//close and open bearer..query bearer
//{ gprs_Serial.println(F("AT+SAPBR=0,1"));//close  
//  delay(500);
//  gprs_ShowSerialData();
//  gprs_Serial.println(F("AT+SAPBR=1,1"));//open
//  delay(500);
//  gprs_ShowSerialData();
//  gprs_Serial.println(F("AT+SAPBR=2,1"));//query
//  delay(500);
//  gprs_ShowSerialData();
//}


  if((server_response[11]=='6')&&(server_response[13]=='1')){GPRS_initialise();}
  Serial.print(F("Server Response : "));Serial.println(server_response);
  lcd.setCursor(0,1);lcd.print(server_response);
  //lcd_display("GPS cood","Posted");
  //Serial.println(F("completed posting GPS cood"));
  gprs_Serial.end();delay(100);
}

void lcd_setup()   // function to set up
{
  lcd.begin(2, 16);   // initialize lcd 
  delay(200); 
  lcd.clear();   // clear lcd 
  delay(100);
}


void get_imei()   // function to get IMEI of gprs shield
{  
  //Serial.println(F("inside imei"));
  gprs_Serial.begin(9600);delay(200);
  gprs_Serial.println(F("AT+CGSN")); // AT command to get IMEI of gprs shield
  delay(200);
  int i=0;
  gprs_Serial.listen();
  while((gprs_Serial.available()!=0)&&(i<=10)){gps_data[45]=gprs_Serial.read();i++;}  i=0;
//  Serial.print(F("pi"));Serial.print(gprs_Serial.parseInt(),DEC);Serial.print(gprs_Serial.parseInt(),DEC);
  while((gprs_Serial.available())&&(i<15))  {gps_data[i+45]=gprs_Serial.read();i++;}
  //Serial.println();Serial.println(gps_data);
  gprs_Serial.end();
}

//.................................Starting of Program.............................!

void setup()  // this function only once
{
  Serial.begin(9600); // initialize the serial monitor
  Serial.println(F("booting.."));
  pinMode(odo,INPUT); // Initializing the pins as INPUT/ OUTPUT
  pinMode(led_gprs,OUTPUT);  
  digitalWrite(led_gprs,HIGH); 
  pinMode(led_gps_post,OUTPUT); 
  pinMode(panic,INPUT);
  digitalWrite(panic,HIGH);
  pinMode(led_panic,OUTPUT);
  pinMode(hire,INPUT);
  digitalWrite(hire,HIGH);
  pinMode(led_hire,OUTPUT);
  digitalWrite(led_gprs,LOW);
  lcd_setup(); // Setup lcd (Initialize)
  printer_flush(); // flush out the data from printer's buffer
//  Serial.println(gps_data);
  lcd.setCursor(0,1);
  lcd.print("ArduinoRebooting");
  delay(700);
  gprs_status=GPRS_initialise();// initialize gprs and store the status in variable
  get_imei();  // collect IMEI of GPRS shield
  fetch_fare_rule();  // fetch fare parameters from server
  delay(100);
//  user_register();//getting 200 OK for user register.. but response more than 3000 bytes, and so, crashes..
  attachInterrupt(1,odo_reading,FALLING); // initialize the interrupt (odometer)
  lcd_display("Vehicle","Ready !!  "); // Print the status on LCD
  //Serial.println(freeRam());
}

void lcd_fare_display() // this funtion will print the distance and fare on lcd
{
    lcd.setCursor(0,0);
    lcd.print(distance,1);
    lcd.print(" KM ");
    lcd.print("RS ");
    lcd.print(fare);
    lcd.print("   ");
}


void loop() // this function keeps on running unless the arduino is powered off
{
  //get_gps_cood();
  if(!get_gps_cood())  // if gps is not fixed try once again to fix it
  {Serial.println(F("calling gps again"));
   get_gps_cood();} // try collecting gps data again
  
  check_for_hire_status(); // call the function to check for hire status 
  if(hired==true){lcd_fare_display();}   
  if(digitalRead(panic)==LOW)
  {
    delay(20);
    if(digitalRead(panic)==LOW)
    {
      panic_generated();
    }
  }
  if(hired==false)
  {
    lcd_display("Vehicle","Not hired");
  }
  check_for_hire_status();if(hired==true){lcd_fare_display();} 
  if(gprs_status==false)
  {
    gprs_status=GPRS_initialise();
    digitalWrite(led_gprs,HIGH);
  }
  else{
    digitalWrite(led_gprs,LOW);
  }
  
  http_gps_post(); // post gps data to server
  check_for_hire_status();if(hired==true){lcd_fare_display();} 
  
}

