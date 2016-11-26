/*
   ESP8266_SimpleAuthentication_Switch
   Modification of the SimpleAuthentication example
   user admin
   password admin
   admin:admin

   Addded code from
   https://github.com/aderhgawen/ESP8266-Webserver/blob/master/src/Esp8266_Webserver.ino
   by: Ashish Derhgawen
   Blog: http://ashishrd.blogspot.com

   Modified for Wemos D1 and the Relay Shield wich pin is on D1

   Todo
   Add wifi configuration using the code of Ashish Derhgawen
   Add the memorisation of the last Relay state

*/

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

// ESP8266 GPIO pins
//#define GPIO2 2
const int relayPin = D1;

const char* ssid = "YourSSID";
const char* password = "YourPassword";

ESP8266WebServer server(80);

//******************************************************************************************
//Check if header is present and correct
bool is_authentified() {
  Serial.println("Enter is_authentified");
  if (server.hasHeader("Cookie")) {
    Serial.print("Found cookie: ");
    String cookie = server.header("Cookie");
    Serial.println(cookie);
    if (cookie.indexOf("ESPSESSIONID=1") != -1) {
      Serial.println("Authentification Successful");
      return true;
    }
  }
  Serial.println("Authentification Failed");
  return false;
}

/******************************************************************************************

                                      handleLogin
*******************************************************************************************/
//login page, also called for disconnect
void handleLogin() {
  String msg;
  if (server.hasHeader("Cookie")) {
    Serial.print("Found cookie: ");
    String cookie = server.header("Cookie");
    Serial.println(cookie);
  }
  if (server.hasArg("DISCONNECT")) {
    Serial.println("Disconnection");
    String header = "HTTP/1.1 301 OK\r\nSet-Cookie: ESPSESSIONID=0\r\nLocation: /login\r\nCache-Control: no-cache\r\n\r\n";
    server.sendContent(header);
    return;
  }
  if (server.hasArg("USERNAME") && server.hasArg("PASSWORD")) {
    if (server.arg("USERNAME") == "admin" &&  server.arg("PASSWORD") == "admin" ) {
      String header = "HTTP/1.1 301 OK\r\nSet-Cookie: ESPSESSIONID=1\r\nLocation: /\r\nCache-Control: no-cache\r\n\r\n";
      server.sendContent(header);
      Serial.println("Log in Successful");
      return;
    }
    msg = "Wrong username/password! try again.";
    Serial.println("Log in Failed");
  }
  String content = "<html><body><form action='/login' method='POST'>To log in, please use : admin/admin<br>";
  content += "User:<input type='text' name='USERNAME' placeholder='user name'><br>";
  content += "Password:<input type='password' name='PASSWORD' placeholder='password'><br>";
  content += "<input type='submit' name='SUBMIT' value='Submit'></form>" + msg + "<br>";
  content += "You also can go <a href='/inline'>here</a></body></html>";
  server.send(200, "text/html", content);
}

/******************************************************************************************

                                      handleRoot
*******************************************************************************************/
//root page can be accessed only if authentification is ok
void handleRoot() {
  Serial.println("Enter handleRoot");
  String header;
  if (!is_authentified()) {
    String header = "HTTP/1.1 301 OK\r\nLocation: /login\r\nCache-Control: no-cache\r\n\r\n";
    server.sendContent(header);
    return;
  }
  //String content = "<html><body><H2>hello, you successfully connected to esp8266!</H2><br>";
  String content = "<html><head><title>ESP8266 Webserver</title></head>";
  content += "<body style=\"background-color:PaleGoldenRod\"><h1><center>ESP8266 Webserver</center></h1>";
  if (server.hasHeader("User-Agent")) {
    content += "the user agent used is : " + server.header("User-Agent") + "<br><br>";
  }
  content += "<ul><li><a href=\"/gpio\">Control Relay</a></li></ul>";
  content += "You can access this page until you <a href=\"/login?DISCONNECT=YES\">disconnect</a></body></html>";
  server.send(200, "text/html", content);
}

/******************************************************************************************

                                      handleNotFound
*******************************************************************************************/
//no need authentification
void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

/******************************************************************************************

                                      gpioPageHandler
*******************************************************************************************/

/* GPIO page allows you to control the GPIO pins */
void gpioPageHandler()
{
  if (!is_authentified()) {
    String header = "HTTP/1.1 301 OK\r\nLocation: /login\r\nCache-Control: no-cache\r\n\r\n";
    server.sendContent(header);
    return;
  }
  // Check if there are any GET parameters
  if (server.hasArg("gpio1"))
  {
    if (server.arg("gpio1") == "1")
    {
      digitalWrite(relayPin, HIGH);
    }
    else
    {
      digitalWrite(relayPin, LOW);
    }
  }

  String response_message = "<html><head><title>ESP8266 Webserver</title></head>";
  response_message += "<body style=\"background-color:PaleGoldenRod\"><h1><center>Control Relay</center></h1>";
  response_message += "<form method=\"get\">";

  response_message += "GPIO1:<br>";

  if (digitalRead(relayPin) == LOW)
  {
    response_message += "<input type=\"radio\" name=\"gpio1\" value=\"1\" onclick=\"submit();\">On<br>";
    response_message += "<input type=\"radio\" name=\"gpio1\" value=\"0\" onclick=\"submit();\" checked>Off<br>";
  }
  else
  {
    response_message += "<input type=\"radio\" name=\"gpio1\" value=\"1\" onclick=\"submit();\" checked>On<br>";
    response_message += "<input type=\"radio\" name=\"gpio1\" value=\"0\" onclick=\"submit();\">Off<br>";
  }
  response_message += "<li><a href=\"/\">Return</h4></li></ul>";
  response_message += "</form></body></html>";


  server.send(200, "text/html", response_message);
}

/******************************************************************************************

                                      handleNotFound
*******************************************************************************************/
/* Called if requested page is not found */
/*
  void handleNotFound()
  {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for (uint8_t i = 0; i < server.args(); i++)
  {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }

  server.send(404, "text/plain", message);
  }
*/




void setup(void) {
  Serial.begin(115200);
  //pinMode(GPIO2, OUTPUT);
  pinMode(relayPin, OUTPUT);

  /*
    // config static, DHCP of VirtualRouter fails very often.
    IPAddress ip(192, 168, 137, 140);
    IPAddress gateway(192, 168, 137, 1);
    Serial.print(F("Setting static ip to : "));
    Serial.println(ip);
    IPAddress subnet(255, 255, 255, 0);
    WiFi.config(ip, gateway, subnet);
  */

  WiFi.begin(ssid, password);
  Serial.println("/r/nESP8266_SimpleAuthentification_Switch");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());


  server.on("/", handleRoot);
  server.on("/login", handleLogin);
  server.on("/gpio", gpioPageHandler);
  server.on("/inline", []() {
    server.send(200, "text/plain", "this works without need of authentification");
  });

  server.onNotFound(handleNotFound);
  //here the list of headers to be recorded
  const char * headerkeys[] = {"User-Agent", "Cookie"} ;
  size_t headerkeyssize = sizeof(headerkeys) / sizeof(char*);
  //ask server to track these headers
  server.collectHeaders(headerkeys, headerkeyssize );
  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
}
