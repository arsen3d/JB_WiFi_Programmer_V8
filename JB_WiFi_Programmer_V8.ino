/*
this is WiFI config sketch for  JuiceBox

The first version of this did not do anything - the only purpose of it was to disable all serial comms from Arduino
so there is no conflict with the commands from the PC
*/

#include <SoftwareSerial.h>
#include <WiFlyHQ.h>
#include <avr/pgmspace.h>

#define VerStr "V8"

#define WFCFG_VAL_MOBILE
// #define WFCFG_VAL_HOME
//#define HTTP // using HTTP protocol for bidirectional control
//#define NOCONFIG

#ifdef WFCFG_VAL_MOBILE
  const char mySSID[] = "bzk-mobile"; // test network at  JuiceBox lab
  const char myPWD[] = "kolobok27"; // test network at  JuiceBox lab
#endif

#ifdef WFCFG_VAL_HOME
  const char mySSID[] = "BZI-OLD"; // test network at  JuiceBox lab
  const char myPWD[] = "kolobok27"; // test network at  JuiceBox lab
#endif


SoftwareSerial wifiSerial(2,4);
WiFly wifly;

const char myHOST[] = "50.21.181.240"; // main EMW server
// const char myHOST[] = "38.111.141.40"; // test server #1
const int myPORT=8042; // matches UDP listener on the server
const int cmddelay=300; // 300ms delay between commands

int i=0;
long timer=0;
char sym;
char str[10];
char datastr[228];


void terminal();


void setup() {
    Serial.begin(9600);
    Serial.println("Starting");
    Serial.print("Free memory: ");
    Serial.println(wifly.getFreeMemory(),DEC);

    wifiSerial.begin(9600);
    
#ifndef NOCONFIG    
    if (!wifly.begin(&wifiSerial, NULL)) { // by default. no error output into &Serial
        Serial.println("Failed to start wifly");
    } else {
      Serial.println("Success starting wifly");
      
      // now init
      wifly.factoryRestore();
      delay(1000);
      wifly.save();
      delay(1000);
      wifly.reboot();
      delay(5000);
    
      boolean success=1;
      int nc=1;
  
      Serial.println(); Serial.print(nc++);
      if(wifly.sendCommand(PSTR("set ip dhcp"), 1)) {
        Serial.print(F(". DHCP NOW ON")); 
      } else success=0;
      delay(cmddelay);
      
      Serial.println(); Serial.print(nc++);
      if(wifly.sendCommand(PSTR("set wlan auth"), 4)) {
        Serial.print(F(". WLAN AUTH NOW 4 (WPA2)")); // pp31 - 0=open, 4=WPA2
      } else success=0;
      delay(cmddelay);
  
      Serial.println(); Serial.print(nc++);
      if(wifly.sendCommand(PSTR("set ip host"), myHOST, 0)) {
        Serial.print(F(". IP HOST NOW ")); 
        Serial.print(myHOST);  
      } else success=0;
      delay(cmddelay);
  
      // set the button to run WPS app
      Serial.println(); Serial.print(nc++);
      if(wifly.sendCommand(PSTR("set sys launch_string"), "wps_app", 0)) {
        Serial.print(F(". WPS IS NOW ON")); // enable WPS app on button press (pp22) 
      } else success=0;
      delay(cmddelay);
      
      // set the module to pull real time from the tine server (default is 64.90.182.55:123 - Pacific time zone)
      Serial.println(); Serial.print(nc++);
      if(wifly.sendCommand(PSTR("set time enable"), 10)) { // every 10 minutes
        Serial.print(F(". RTC IS NOW ON from 64.90.182.55:123")); // pp27,68 
      } else success=0;
      delay(cmddelay);
  
      // set communication options 
      Serial.println(); Serial.print(nc++);
      if(wifly.sendCommand(PSTR("set comm time"), 0)) {
        Serial.print(F(". COMM TIME NOW 0")); // no timeout for packet
      } else success=0;
      delay(cmddelay);
  
      Serial.println(); Serial.print(nc++);
      if(wifly.sendCommand(PSTR("set comm match"), 0xD)) {
        Serial.print(F(". COMM MATCH NOW 0xD")); // send on carriage return 
      } else success=0;
      delay(cmddelay);
  
  #ifdef HTTP 
  
      Serial.println(); Serial.print(nc++);
      if(wifly.sendCommand(PSTR("set ip protocol"), 18)) {
        Serial.print(F(". IP PROTOCOL NOW 18 (HTML)")); // HTML client, p. 77
      } else success=0;
      delay(cmddelay);
      
      Serial.println(); Serial.print(nc++);
      if(wifly.sendCommand(PSTR("set ip remote"), 80)) { // website
        Serial.print(F(". IP REMOTE PORT NOW set to 80")); 
      } else success=0;
      delay(cmddelay);
  
      Serial.println(); Serial.print(nc++);
      if(wifly.sendCommand(PSTR("set uart mode"), 2)) { // auto-send on UART data
        Serial.print(F(". UART MODE set to 2")); 
      } else success=0;
      delay(cmddelay);
  
      Serial.println(); Serial.print(nc++);
      if(wifly.sendCommand(PSTR("set option format"), 1)) { // 1=HTML header, 16=all params, p.19
        Serial.print(F(". OPT FORMAT set to 1")); 
      } else success=0;
      delay(cmddelay);
  
      Serial.println(); Serial.print(nc++);
      if(wifly.sendCommand(PSTR("set comm remote"), "GET$/cgi-bin/JB_HTTP_test.pl?DATA=", 0)) {
        Serial.print(F(". set Web URL to call")); // 
      } else success=0;
      delay(cmddelay);
      
  #else
  
      Serial.println(); Serial.print(nc++);
      if(wifly.sendCommand(PSTR("set ip remote"), myPORT)) { // custom UDP port
        Serial.print(F(". IP REMOTE PORT NOW set")); // 
      } else success=0;
      delay(cmddelay);
  
      Serial.println(); Serial.print(nc++);
      if(wifly.sendCommand(PSTR("set ip protocol"), 1)) {
        Serial.print(F(". IP PROTOCOL NOW 1")); // UDP=1, pp18
      } else success=0;
      delay(cmddelay);  

  #endif
          
      Serial.println(); Serial.print(nc++);
      if(wifly.sendCommand(PSTR("set wlan ssid"), mySSID, 0)) { // pp35
        Serial.print(F(". SSID NOW "));
        Serial.print(mySSID); 
      } else success=0;
      delay(cmddelay);
  
      Serial.println(); Serial.print(nc++);
      if(wifly.sendCommand(PSTR("set wlan phrase"), myPWD, 0)) {
        Serial.print(F(". PASSPHRASE NOW "));
        Serial.print(myPWD); 
      } else success=0;
      delay(cmddelay);
  
  
      //================ prepare the WiFly module for accepting commands from JuiceBox =================
      // THIS IS NOT YET FUNCTIONAL - DO NOT RELY!
      // disable LED use by the module's firmware (to enable their use by JuiceBox to indicate status etc)
      Serial.println(); Serial.print(nc++);
      if(wifly.sendCommand(PSTR("set sys iofunc"), 7)) { // pp27
        Serial.print(F(". LEDs are now available for JuiceBox's use")); 
      } else success=0;
      delay(cmddelay);
  
      Serial.println(); Serial.print(nc++);
      if(wifly.sendCommand(PSTR("set wlan join"), 1)) {
        Serial.print(F(". WLAN JOIN NOW 1")); // join the wifi network with stored params, SSID, pass, etc.
      } else success=0;
      delay(cmddelay);
      
      Serial.println("\n");
  
      
      // there will be two main ways to send commands to the JuiceBox via network: 
      //     1. Local commands from the JuiceBox smartphone app (to be developed) - the smartphone is 
      //        connected to the same local wifi router as JuiceBox. Latency will be ~zero for both 
      //        monitoring stats and commands
      //     2. Remote commands from the JuiceBox website or smartphone app when the smartphone is NOT
      //        connected to the same local network. In these cases, to avoid tricky port forwarding setup
      //        on the local routers, JuiceBox will use HTTP pulls from EMW server to get commands
      //        This means that there will be high latency in executing commands (up to 10 minutes - to
      //        avoid overloading JuiceBox servers with too many requests for comments). Therefore, 
      //        you should think about remote commands as batch commands to be executed 'soon', not 'right now'
      //        You will also experience similar latency in display of the JuiceBox status / charging stats
      //        when connected remotely
      // IN THE INITIAL VERSIONS OF JUICEBOX, ONLY REMOTE MONITORING WILL BE IMPLEMENTED - this means
      // NO commands can be sent via network to JuiceBox and NO local smartphone monitoring is possible
      // We expect these features to be implemented in V9.0+ of the JuiceBox  
      //================ END preparing WiFly for commands from JuiceBox  ===============================
     
      if(success) {
        Serial.println("WiFly initialization COMPLETE!\n");
      } else {
        Serial.println("\nWiFi FAILED to initialize in full!");
        Serial.println("Close & restart Serial Monitor to re-launch the initialization script.");
        Serial.println("If failed 10 times in a row, contact us at JB-support@emotorwerks.com\n");
      }
      
      // activate the settings now
      wifly.save();
      delay(1000);
      wifly.reboot();
      
      while(!wifly.isAssociated()) {
        Serial.print(".");
        delay(100);
      }
      Serial.println("Associated!\n");
      delay(5000);
     
       terminal();
    }
    
#endif // NOCONFIG

}



void loop() {
  // as of 3/31/14, for some reason this fails to post any data to server 
  // but after reboot of the WiFly, everything works...
  wifiSerial.end();
  wifiSerial.begin(9600);

  // to see this on the server, (if in UDP mode) launch UDP listener on the server, OR 
  //                            (if in HTTP mode) look at the logs for the webserver
  // in HTTP mode, this will call JB_HTTP_test.pl script with DATA=<whatever is sent here>
  wifiSerial.print("test data: ");
  wifiSerial.print(i++);
  wifiSerial.print("\n\r\n\n");
   
#ifndef HTTP

  Serial.print("*");
  if(i%50==0) Serial.println();
  delay(1000);

#else

  int ii=0;
  int start_data=0;
  str[0]=str[1]=str[2]=str[3]=str[4]=0;
  timer=millis();
  
  // sending things faster than every 2.5 seconds results in total failure to communicate - 
  // this is partially due to the baud rate on the wifi unit
  while(millis()-timer<3000) { 
    if(wifiSerial.available()>0) {
      sym=wifiSerial.read();
//      Serial.write(sym); // echo to serial monitor
      if(sym=='\n') start_data=0;
      
      if(start_data) {
        datastr[ii]=sym;
        ii++;
      }
      datastr[ii]=0; // terminate string
      
      str[0]=str[1]; str[1]=str[2]; str[2]=str[3]; str[3]=sym; str[4]=0;
      if(!strcmp(str, "CMD:")) { // this would signal start of real data
        start_data=1;
      }
    }
  }
  
  Serial.print("Received: ---[");
  Serial.print(datastr);
  Serial.println("]---");

#endif

}


void terminal() 
{
    Serial.println("Terminal ready");
    while (1) {
	if (wifiSerial.available() > 0) {
	    Serial.write(wifiSerial.read());
	}

	if (Serial.available()) {
	    wifiSerial.write(Serial.read());
	}
    }
}

