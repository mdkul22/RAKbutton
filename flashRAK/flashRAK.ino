/*
 This sketch shows how to request flash memory larger than defaut 4K, and read/write one specific word.
 */
#include <WiFi.h>
#include <FlashMemory.h>
#include <PubSubClient.h>
#include <PowerManagement.h>
#define LED1  0
#define LED2  1
#define LED3  2
#define LED4  3
#define RED   0
#define GREEN 1
#define BLUE  2
#define OFF   3
void led_ctrl(uint8_t led_num, uint8_t rgb);
void led_off();
void led_red();

String v1 = "0";
String v2 = "0";
String v3 = "0";
String v4 = "0";

void reconnect();
void printWifiStatus();

void serverLoop();
void flasher(String);
void extractFlash();
void ConfigureMode();
void clearFlash();
void beginBootUp();

// board pins
int pwr_en = 15;
/* leds */
int led1_r = 25;
int led1_g = 24;
int led1_b = 19;
int led2_r = 0;
int led2_g = 2;
int led2_b = 6;
int led4_r = 12;
int led4_g = 11;
int led4_b = 13;
int led3_r = 22;
int led3_g = 21;
int led3_b = 1;
/* keys */
int key1 = 23;
int key2 = 14;
int key3 = 10;
int key4 = 20;


void setup() {
  //clearFlash();
  pinMode(pwr_en, OUTPUT);
  digitalWrite(pwr_en, 1);
  pinMode(led2_r, OUTPUT);
  pinMode(led2_g, OUTPUT);
  pinMode(led2_b, OUTPUT);
  pinMode(led1_b, OUTPUT);
  pinMode(led3_r, OUTPUT);
  pinMode(led3_g, OUTPUT);
  pinMode(led3_b, OUTPUT);
  pinMode(led4_b, OUTPUT);
  pinMode(key2, INPUT_PULLUP);
  pinMode(key3, INPUT_PULLUP);
  pinMode(key4, INPUT_PULLUP);
  #if 1
  /*
  *   Pin D21-D25 can not be used as digital IO ,when in debug mode(Enable JTAG).
  *   D21-D25 can be used as digital IO when in factory mode.(Disable JTAG)
  */
  //D21-D25
  pinMode(led4_g, OUTPUT);
  pinMode(led4_r, OUTPUT);
  pinMode(led1_g, OUTPUT);
  pinMode(led1_r, OUTPUT);
  pinMode(key1, INPUT_PULLUP);
    #endif
  int i = 0;
  Serial.begin(9600);
  FlashMemory.read();
  // Enter Configure mode if buf!= 0x00
  for(i=0;i<20;i++)
  Serial.println(FlashMemory.buf[i]);
  if(FlashMemory.buf[0] != 0x00)
  {
    clearFlash();
    Serial.println("Cleared Flash and Entering Configure Mode");
    ConfigureMode();
  }
  beginBootUp();
}
void loop() {
  delay(1000);
}
// clears flash memory to be able to enter configure mode
void clearFlash()
{
  FlashMemory.read();
  int i;
  for(i=0; i<100;i++)
  FlashMemory.buf[i]=0xFF;
  FlashMemory.update(true);
  FlashMemory.read();
  Serial.println("Flash check:");
  for(i=0;i<10;i++)
  Serial.println(FlashMemory.buf[i]);
}

WiFiClient wifiClient;
PubSubClient client(wifiClient);
char clientId[] = "rak-smart-button1";

char myssid[] = "rakbutton";      // your network SSID (name)
char mypass[] = "password";   // your network password  char channel[] = "1";
char channel[] = "1";
int keyIndex = 0;                 // your network key Index number (needed only for WEP)
int status = WL_IDLE_STATUS;
WiFiServer server(80);

// COnfiguratiuon mode where RAK acts as an AP and http server
void ConfigureMode() {
  FlashMemory.update(true);
  status = WiFi.status();
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  // attempt to start Wifi network:
  while (status != WL_CONNECTED)
  {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(myssid);
    // begin WPA/WPA2 network:
    status = WiFi.apbegin(myssid, mypass, channel);
    if (status == WL_CONNECTED) {
      break;
    }
    // wait 10 seconds for connection:
    delay(10000);
  }
  Serial.print("AP started with ssid: ");
  Serial.println(myssid);
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  server.begin();
  // you're connected now, so print out the status:
  while(1)
  serverLoop();
} // string for storing MAC Address 
// Server Init and functionality
void serverLoop() {
  led_red();
  // listen for incoming clients
  WiFiClient client = server.available();
  if (!client) {
    delay(20);
  }
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected() && !client.available()) {
      delay(1);
    }
    while(client.available())
    {
        String req = client.readStringUntil('\r');
        Serial.println(req);
        int addr_start = req.indexOf(' ');
        int addr_end = req.indexOf(' ', addr_start +1);
        req = req.substring(addr_start + 1, addr_end);
        Serial.println(req);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        String macAdd;
        if (req == "/") {
          IPAddress ip = WiFi.localIP();
          uint8_t* mac;
          mac = WiFi.macAddress(mac);
          macAdd = (char*)mac;
          String ipStr = String(ip[0]); + "." + String(ip[1]) + "." + String(ip[2]) + "." + String(ip[3]);
          String s, st;
          st = "<ul>";
          s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>Mac:";
          s += macAdd +"&";
          s += "<p>";
          s += st;
          s += "<form method='get' action='a'><label>SSID: </label><br><input name='ssid' length=32><br>";
          s += "<label>PASS:</label><br><input name='pass' length=64><br><label>Server:</label><br>";
          s += "<input name='serv' length=60><br><label>PORT:</label><br><input name='port' length=6>";
          s += "</label><br><label>TYPE:</label><br><input name='type' length=6><br>";
//        s += "<br><label>Topic for button 1:</label><br><input name='topic1' length=60>";
//        s += "<br><label>Topic for button 2:</label><br><input name='topic2' length=60>";
//        s += "<br><label>Topic for button 3:</label><br><input name='topic3' length=60>";
//        s += "<br><label>Topic for button 4:</label><br><input name='topic4' length=60>";
          s += "<br><input type='submit'></form>";
          s += "</html>\r\n\r\n";
          client.print(s);
        }
        else if (req.startsWith("/a?ssid="))
        {
          req.replace("%2F", "/");
          String ssid = req.substring(8,req.indexOf("&"));
          String pass = req.substring(req.indexOf("pass=")+5, req.indexOf('&', req.indexOf("pass=")+5));
          String server = req.substring(req.indexOf("serv=")+5, req.indexOf('&', req.indexOf("serv=")+5));
          String port = req.substring(req.indexOf("port=")+5, req.indexOf('&', req.indexOf("port=")+5));
          String type = req.substring(req.indexOf("type=")+5, req.indexOf('&', req.indexOf("type=")+5));
//          String topic1 = req.substring(req.indexOf("topic1=")+7, req.indexOf('&', req.indexOf("topic1=")+7));
//          String topic2 = req.substring(req.indexOf("topic2=")+7, req.indexOf('&', req.indexOf("topic2=")+7));
//          String topic3 = req.substring(req.indexOf("topic3=")+7, req.indexOf('&', req.indexOf("topic3=")+7));
//          String topic4 = req.substring(req.indexOf("topic4=")+7, req.indexOf('&', req.indexOf("topic4=")+7));
          Serial.print("ssid is ");
          Serial.println(ssid);
          Serial.print("pass is ");
          Serial.println(pass);
          Serial.println(server);
          Serial.println(port);
          String longstr = ssid + ";" + pass + ";" + server + ";" + port + ";" + type + ";" + macAdd;
          Serial.println(longstr);
          flasher(longstr);
          if(longstr.length())
            PowerManagement.softReset();
        }
    // give the web browser time to receive the data
    delay(1);
    client.flush();
    }
    client.stop();
    Serial.println("\nClient connected stopped");
    // close the connection:
}

// flashes string in flash memory in a predetermined encoding so that it can be retrieved properly
void flasher(String val)
{
  FlashMemory.read();
  int i;
  FlashMemory.buf[0] = 0x00;
  for(i=0;i<val.length();i++)
  {
    FlashMemory.buf[i+1] = val[i];
  }
  FlashMemory.update();
  FlashMemory.read();
  for(i=0; i<20;i++)
  {
    Serial.print(FlashMemory.buf[i]);
  }
}

// Deployment mode, device acts as WiFi client and reads flash and deploys button according to configuration
void beginBootUp()
{
  FlashMemory.read();
  int i, j;
  j = 0;
  String s[8];
  for(i=1;FlashMemory.buf[i]!=0xFF;i++)
  {
    char x = char(FlashMemory.buf[i]);
    if(FlashMemory.buf[i]==0x3B) // 0x3B represents " ; " in ascii code
    {
      j++;
      continue;
    }
    s[j] += x;
    Serial.println(s[j]);
  }
  char ssid[s[0].length()+1];
  s[0].toCharArray(ssid, s[0].length()+1);
  char pass[s[1].length()+1];
  s[1].toCharArray(pass, s[1].length()+1);
  char server[s[2].length()+1];
  s[2].toCharArray(server, s[2].length()+1);
  char port[s[3].length()+1];
  s[3].toCharArray(port, s[3].length()+1);
  char type[s[4].length()+1];
  s[4].toCharArray(type, s[4].length()+1);
  String macAdd;
  macAdd = s[5];
  char top[] = "tqb/topic";
  Serial.println(ssid);
  Serial.println(pass);
  Serial.println(port);
  Serial.println(type);
  Serial.println(macAdd);
  int status = WL_IDLE_STATUS;
  String Type(type); 
  client.setServer(server, 1883);
  led_off();
  int cnt = 0;
  while ( status != WL_CONNECTED) {

    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    Serial.println(pass);
    status = WiFi.begin(ssid, pass);
    // wait 10 seconds for connection:
    delay(10000);
    cnt++;
    if(cnt == 3)\
    {
      clearFlash();
      PowerManagement.softReset();
  
    }
  }
  Serial.println("Connected to wifi");
  printWifiStatus();
  while(true)
  {
    if (!client.connected()) {
   reconnect();
 }
 client.loop();
 led_off();

 if (digitalRead(key1) == 0) {
   delay(50);
   if (digitalRead(key1) == 0) {
   led_ctrl(LED1,BLUE);
   if(client.connected()){
    if(Type.equals("0"))
    {
      String s = "";
      s += "{\"mac\":\""; 
      s += macAdd; 
      s +="\",";
      s += "\"VM\": \"1\"}";
      client.publish(top, s.c_str());
    }

    else if(Type.equals("1"))
    {
      String s = "";
      s += "{\"mac\":\""; 
      s += macAdd; 
      s +="\",";
      s += "\"plumbing\": \"1\"}";
      client.publish("tqb/topic", s.c_str());
    }
   }
 else{
   Serial.println("client is not connected...") ;
 }
  delay(500);
   }
 }

 if (digitalRead(key2) == 0) {
   delay(50);
   if (digitalRead(key2) == 0) {
   led_ctrl(LED2,BLUE);
   Serial.println("key2");
   if(client.connected()){
    if(Type.equals("0"))
    {
      String s = "";
      s += "{\"mac\":\""; 
      s += macAdd; 
      s +="\",";
      s += "\"coffee\": \"1\"}";
      client.publish(top, s.c_str());
    }

    else if(Type.equals("1"))
    {
      String s = "";
      s += "{\"mac\":\""; 
      s += macAdd; 
      s +="\",";
      s += "\"cleaning\": \"1\"}";
      client.publish(top, s.c_str());
    } 
    }
    else{
   Serial.println("client is not connected...") ;
 }
   delay(500);
   // flash reboot sequence: keep button 2 clicked for 500 ms then button turns green
   // after it turns green, keep button 2 and 3 pressed for 5 more seconds until
   // both or one of the buttons turn red. This indicates that the flash has been cleared
   // and that the device has been reset.
   if(digitalRead(key2)==0)
   {
    led_ctrl(LED2, GREEN);
    delay(5000);
    if((digitalRead(key2)==0)&&(digitalRead(key3)==0))
    {
    digitalWrite(led2_r, 0);
    digitalWrite(led2_g, 1);
    digitalWrite(led2_b, 1);
    digitalWrite(led3_r, 0);
    digitalWrite(led3_g, 1);
    digitalWrite(led3_b, 1);
    Serial.println("Flashing!");
    delay(1000);
    clearFlash();
    PowerManagement.softReset();
   }
   }
 }
 }
 if (digitalRead(key3) == 0) {
   delay(50);
   if (digitalRead(key3) == 0) {
   led_ctrl(LED3,BLUE);
   if(client.connected()){
      if(Type.equals("0"))
    {
      String s = "";
      s += "{\"mac\":\""; 
      s += macAdd; 
      s +="\",";
      s += "\"water\": \"1\"}";
      client.publish(top, s.c_str());
    }

    else if(Type.equals("1"))
    {
      String s = "";
      s += "{\"mac\":\""; 
      s += macAdd; 
      s +="\",";
      s += "\"paper\": \"1\"}";
      client.publish(top, s.c_str());
    }
 }else{
   Serial.println("client is not connected...") ;
 }
   delay(500);
   Serial.println("key3");
   //http_get("3");
   }
 }
 if (digitalRead(key4) == 0) {
   delay(50);
   if (digitalRead(key4) == 0) {
   led_ctrl(LED4,BLUE);

   if(client.connected()){
    if(Type.equals("0"))
    {
      String s = "";
      s += "{\"mac\":\""; 
      s += macAdd; 
      s +="\",";
      s += "\"glasses\": \"1\"}";
      client.publish(top, s.c_str());
    }

    else if(Type.equals("1"))
    {
      String s = "";
      s += "{\"mac\":\""; 
      s += macAdd; 
      s +="\",";
      client.publish(top, s.c_str());
    }
 }else{
   Serial.println("client is not connected...") ;
 }
   delay(500);
   Serial.println("key4");
   //http_get("4");
   }
 }
delay(100);
  }
}
int count = 0;

// if no reconnect for 10 times the device clears flash and resets into config mode
void reconnect() {
 // Loop until we're reconnected
 while (!client.connected()) {
   Serial.print("Attempting MQTT connection...");
   // Attempt to connect
   if (client.connect(clientId)) {
     Serial.println("connected");
     // Once connected, publish an announcement..
   } else {
     Serial.print("failed, rc=");
     count++;
     if(count == 3)
     {
      clearFlash();
      PowerManagement.softReset();
     }
     Serial.print(client.state());
     Serial.println(" try again in 1 seconds");
     // Wait 5 seconds before retrying
     delay(1000);
   }
 }
}

void printWifiStatus() {
 // print the SSID of the network you're attached to:
 Serial.print("SSID: ");
 Serial.println(WiFi.SSID());
 // print your WiFi shield's IP address:
 IPAddress ip = WiFi.localIP();
 Serial.print("IP Address: ");
 Serial.println(ip);
 // print the received signal strength:
 long rssi = WiFi.RSSI();
 Serial.print("signal strength (RSSI):");
 Serial.print(rssi);
 Serial.println(" dBm");
 led_ctrl(LED1,RED);
 led_ctrl(LED2,RED);
 led_ctrl(LED3,RED);
 led_ctrl(LED4,RED);
 delay(500);
}

// LED functions
void led_off() {
  led_ctrl(LED1,OFF);
  led_ctrl(LED2,OFF);
  led_ctrl(LED3,OFF);
  led_ctrl(LED4,OFF);
}

void led_red()
{
  led_ctrl(LED1,RED);
  led_ctrl(LED2,RED);
  led_ctrl(LED3,RED);
  led_ctrl(LED4,RED);
}

void led_ctrl(uint8_t led_num, uint8_t rgb)
{
 switch (led_num) {
   case LED1:
     if (rgb == RED) {
       digitalWrite(led1_r, 0);
       digitalWrite(led1_g, 1);
       digitalWrite(led1_b, 1);
     }
     else if (rgb == GREEN) {
       digitalWrite(led1_r, 1);
       digitalWrite(led1_g, 0);
       digitalWrite(led1_b, 1);
     }
     else if (rgb == BLUE) {
       digitalWrite(led1_r, 1);
       digitalWrite(led1_g, 1);
       digitalWrite(led1_b, 0);
     }
     else if (rgb == OFF) {
       digitalWrite(led1_r, 1);
       digitalWrite(led1_g, 1);
       digitalWrite(led1_b, 1);
     }
     break;
   case LED2:
     if (rgb == RED) {
       digitalWrite(led2_r, 0);
       digitalWrite(led2_g, 1);
       digitalWrite(led2_b, 1);
     }
     else if (rgb == GREEN) {
       digitalWrite(led2_r, 1);
       digitalWrite(led2_g, 0);
       digitalWrite(led2_b, 1);
     }
     else if (rgb == BLUE) {
       digitalWrite(led2_r, 1);
       digitalWrite(led2_g, 1);
       digitalWrite(led2_b, 0);
     }
     else if (rgb == OFF) {
       digitalWrite(led2_r, 1);
       digitalWrite(led2_g, 1);
       digitalWrite(led2_b, 1);
     }
     break;
   case LED3:
     if (rgb == RED) {
       digitalWrite(led3_r, 0);
       digitalWrite(led3_g, 1);
       digitalWrite(led3_b, 1);
     }
     else if (rgb == GREEN) {
       digitalWrite(led3_r, 1);
       digitalWrite(led3_g, 0);
       digitalWrite(led3_b, 1);
     }
     else if (rgb == BLUE) {
       digitalWrite(led3_r, 1);
       digitalWrite(led3_g, 1);
       digitalWrite(led3_b, 0);
     }
     else if (rgb == OFF) {
       digitalWrite(led3_r, 1);
       digitalWrite(led3_g, 1);
       digitalWrite(led3_b, 1);
     }
     break;
   case LED4:
     if (rgb == RED) {
       digitalWrite(led4_r, 0);
       digitalWrite(led4_g, 1);
       digitalWrite(led4_b, 1);
     }
     else if (rgb == GREEN) {
       digitalWrite(led4_r, 1);
       digitalWrite(led4_g, 0);
       digitalWrite(led4_b, 1);
     }
     else if (rgb == BLUE) {
       digitalWrite(led4_r, 1);
       digitalWrite(led4_g, 1);
       digitalWrite(led4_b, 0);
     }
     else if (rgb == OFF) {
       digitalWrite(led4_r, 1);
       digitalWrite(led4_g, 1);
       digitalWrite(led4_b, 1);
     }
     break;
   default:
     break;
 }
}

