/* ESP8266 Moisture Sensor
   This sketch uses an ESP8266 to read the analog signal from a moisture sensor. The data is then displayed
   using the serial console or a web browser. Based on the moisture reading, the ESP8266 will blink a RGB LED
   red, green or blue.

   Red = Dry
   Green = In between Wet and Dry
   Blue = Wet

     Viewing the data via web browser by going to the ip address. In this sketch the address is
     http://192.168.1.221

      The browser data includes a Google Chart to visually illustrate the moisture reading as a guage.

   ///////////////////////////////////////
   Arduino IDE Setup
   File:
      Preferences
        Add the following link to the "Additional Boards Manager URLs" field: 
        http://arduino.esp8266.com/stable/package_esp8266com_index.json
   Tools:
      board: NodeMCU 1.0 (ESP-12 Module)
      programmer: USBtinyISP

      
  ///////////////////////////////
*/
#include <ESP8266WiFi.h>

const char* ssid = "yourWifiSSID";
const char* password = "yourWifiPassword";

const int redPin = 4; //  ~D2
const int greenPin = 12; // ~D6
const int bluePin = 14; // ~D5


int WiFiStrength = 0;

WiFiServer server(80);

void setup() {
  Serial.begin(115200);
  delay(10);

  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);


  // color while waiting to connect
  analogWrite(redPin, 280);
  analogWrite(greenPin, 300);
  analogWrite(bluePin, 300);


  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  // Set the ip address of the webserver
  // WiFi.config(WebServerIP, Gatway, Subnet)
  // or comment out the line below and DHCP will be used to obtain an IP address
  // which will be displayed via the serial console

 // WiFi.config(IPAddress(192, 168, 1, 221), IPAddress(192, 168, 1, 1), IPAddress(255, 255, 255, 0));

  // connect to WiFi router
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");

  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.print("Use this URL to connect: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");

}

double analogValue = 0.0;
double analogVolts = 0.0;
unsigned long timeHolder = 0;


void loop() {

  WiFiStrength = WiFi.RSSI(); // get dBm from the ESP8266
  analogValue = analogRead(A0); // read the analog signal

  // convert the analog signal to voltage
  // the ESP2866 A0 reads between 0 and ~3 volts, producing a corresponding value
  // between 0 and 1024. The equation below will convert the value to a voltage value.



  analogVolts = (analogValue * 3.08) / 1024;

  // now get our chart value by converting the analog (0-1024) value to a value between 0 and 100.
  // the value of 400 was determined by using a dry moisture sensor (not in soil, just in air).
  // When dry, the moisture sensor value was approximately 400. This value might need adjustment
  // for fine tuning of the chartValue.

  int chartValue = (analogValue * 100) / 400;

  // now reverse the value so that the value goes up as moisture increases
  // the raw value goes down with wetness, we want our chart to go up with wetness
  chartValue = 100 - chartValue;

  // set a "blink" time interval in milliseconds.
  // for example, 15000 is 15 seconds. However, the blink will not always be 15 seconds due to other
  // delays set in the code.

  if (millis() - 15000 > timeHolder)
  {
    timeHolder = millis();

    // determine which color to use with the LED based on the chartValue.
    // note: PWM is used so any color combo desired can be set by changing the values sent to each pin
    // between 0 and 1024 - 0 being OFF and 1024 being full power
    ////   yellowish
    //  analogWrite(redPin, 900);
    //  analogWrite(greenPin, 1010);
    //  analogWrite(bluePin, 100);

    if (chartValue <= 25) {  // 0-25 is red "dry"

      analogWrite(redPin, 1000);
      analogWrite(greenPin, 0);
      analogWrite(bluePin, 0);

    } else if (chartValue > 25 && chartValue <= 75) // 26-75 is green
    {

      analogWrite(redPin, 0);
      analogWrite(greenPin, 1000);
      analogWrite(bluePin, 0);

    }
    else if (chartValue > 75 ) // 76-100 is blue
    {

      analogWrite(redPin, 0);
      analogWrite(greenPin, 0);
      analogWrite(bluePin, 1000);

    }

    delay(1000); // this is the duration the LED will stay ON

    analogWrite(redPin, 0);
    analogWrite(greenPin, 0);
    analogWrite(bluePin, 0);

  }

  // Serial data
  Serial.print("Analog raw: ");
  Serial.println(analogValue);
  Serial.print("Analog V: ");
  Serial.println(analogVolts);
  Serial.print("ChartValue: ");
  Serial.println(chartValue);
  Serial.print("TimeHolder: ");
  Serial.println(timeHolder);
  Serial.print("millis(): ");
  Serial.println(millis());
  Serial.print("WiFi Strength: ");
  Serial.print(WiFiStrength); Serial.println("dBm");
  Serial.println(" ");
  delay(10000); // slows amount of data sent via serial

  // check to for any web server requests. ie - browser requesting a page from the webserver
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  // Wait until the client sends some data
  Serial.println("new client");

  // Read the first line of the request
  String request = client.readStringUntil('\r');
  Serial.println(request);
  client.flush();

  // Return the response
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println(""); //  do not forget this one
  client.println("<!DOCTYPE HTML>");

  client.println("<html>");
  client.println(" <head>");
  client.println("<meta http-equiv=\"refresh\" content=\"60\">");
  client.println(" <script type=\"text/javascript\" src=\"https://www.gstatic.com/charts/loader.js\"></script>");
  client.println("  <script type=\"text/javascript\">");
  client.println("    google.charts.load('current', {'packages':['gauge']});");
  client.println("    google.charts.setOnLoadCallback(drawChart);");
  client.println("   function drawChart() {");

  client.println("      var data = google.visualization.arrayToDataTable([ ");
  client.println("        ['Label', 'Value'], ");
  client.print("        ['Moisture',  ");
  client.print(chartValue);
  client.println(" ], ");
  client.println("       ]); ");
  // setup the google chart options here
  client.println("    var options = {");
  client.println("      width: 400, height: 120,");
  client.println("      redFrom: 0, redTo: 25,");
  client.println("      yellowFrom: 25, yellowTo: 75,");
  client.println("      greenFrom: 75, greenTo: 100,");
  client.println("       minorTicks: 5");
  client.println("    };");

  client.println("   var chart = new google.visualization.Gauge(document.getElementById('chart_div'));");

  client.println("  chart.draw(data, options);");

  client.println("  setInterval(function() {");
  client.print("  data.setValue(0, 1, ");
  client.print(chartValue);
  client.println("    );");
  client.println("    chart.draw(data, options);");
  client.println("    }, 13000);");


  client.println("  }");
  client.println(" </script>");

  client.println("  </head>");
  client.println("  <body>");

  client.print("<h1 style=\"size:12px;\">ESP8266 Soil Moisture</h1>");

  // show some data on the webpage and the guage
  client.println("<table><tr><td>");

  client.print("WiFi Signal Strength: ");
  client.println(WiFiStrength);
  client.println("dBm<br>");
  client.print("Analog Raw: ");
  client.println(analogValue);
  client.print("<br>Analog Volts: ");
  client.println(analogVolts);
  client.println("<br><a href=\"/REFRESH\"\"><button>Refresh</button></a>");

  client.println("</td><td>");
  // below is the google chart html
  client.println("<div id=\"chart_div\" style=\"width: 300px; height: 120px;\"></div>");
  client.println("</td></tr></table>");

  client.println("<body>");
  client.println("</html>");
  delay(1);
  Serial.println("Client disonnected");
  Serial.println("");


}

