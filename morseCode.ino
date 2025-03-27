/*
This is a program developed by Nandan Shukla. This code can be used in a non-commercial capacity by anyone. To use this code in a non-commercial capacity, 
the author has to be given credit inside the code and in the README or the source code file. This code is a multi-threaded software designed for ESP32 which will keep repeating a specific morse code
until it is changed by sending the new phase over UART or resetting.
*/

#include <WiFi.h>

#define DOT 100
#define DASH 300
#define SPACE 700
#define END 1000
#define LED 5

// WiFi credentials
const char *ssid = "Airtel_Dicot";
const char *password = "Dicot@240122";

const int ledPin = 5;
float blinkFreq = 1.0; // Hz
bool blinkEnabled = true;

String morseCharset[] = { ".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..", ".---", "-.-", ".-..", "--", "-.", "---", ".--.", "--.-", ".-.", "...", "-", "..-", "...-", ".--", "-..-", "-.--", "--..", ".----", "..---", "...--", "....-", ".....", "-....", "--...", "---..", "----.", "-----", "..--..", "-.-.--", ".-.-.-", "--..--", "-.-.-.", "---...", ".-.-.", "-....-", "-..-.", "-...-" };
String netCharset = "abcdefghijklmnopqrstuvwxyz1234567890?!.,;:+-/=";
String inputBuffer = "";
String morse = "-----  ";

WiFiServer server(80);
TaskHandle_t networkTaskHandle = NULL;
TaskHandle_t blinkTaskHandle = NULL;

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);

  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected. IP: " + WiFi.localIP().toString());

  server.begin();

  xTaskCreate(networkTask, "HTTP Server", 12288, NULL, 2, &networkTaskHandle);
  xTaskCreate(blinkTask, "Blink LED", 2048, NULL, 1, &blinkTaskHandle);
}

void networkTask(void *parameter) {
  while (true) {
    WiFiClient client = server.available();
    if (client) {
      Serial.println("New Client connected.");
      String req = "";
      unsigned long timeout = millis() + 2000;

      while (client.connected() && millis() < timeout) {
        while (client.available()) {
          char c = client.read();
          req += c;
          if (req.endsWith("\r\n\r\n")) break;
        }
        if (req.endsWith("\r\n\r\n")) break;
        vTaskDelay(1);
      }

      Serial.println("=== REQUEST START ===");
      Serial.println(req);
      Serial.println("=== REQUEST END ===");

      // Check for frequency set request
      if (req.indexOf("GET /freq?val=") >= 0) {
        int start = req.indexOf("GET /freq?val=") + 14;
        int end = req.indexOf(" ", start);
        String freqString = urlDecode(req.substring(start, end));
        morse = translator(freqString);
        Serial.println(morse);
      }

      sendHTML(client);

      client.stop();
      Serial.println("Client disconnected.");
    }
    vTaskDelay(5);
  }
}

String translator(String text) {
  text.toLowerCase();
  String morseCodet = "";
  for (int i = 0; i < text.length(); i++) {
    int location = netCharset.indexOf(text[i]);
    if (location >= 0) {
      morseCodet += morseCharset[location];
      morseCodet += " ";
    } else {
      if (text[i] == ' ') {
        morseCodet += "  ";
      }
    }
  }
  return morseCodet;
}

String urlDecode(String input) {
  String decoded = "";
  char temp[] = "0x00";
  unsigned int len = input.length();
  for (int i = 0; i < len; i++) {
    if (input[i] == '%') {
      if (i + 2 < len) {
        temp[2] = input[i + 1];
        temp[3] = input[i + 2];
        decoded += (char) strtol(temp, NULL, 16);
        i += 2;
      }
    } else if (input[i] == '+') {
      decoded += ' ';
    } else {
      decoded += input[i];
    }
  }
  return decoded;
}

void blinkTask(void *parameter) {
  while (true) {
    if (morse.length() > 0) {
      for (int i = 0; i < morse.length(); i++) {
        int currentDelay = END;

        if (morse[i] == '.') {
          currentDelay = DOT;
        } else if (morse[i] == '-') {
          currentDelay = DASH;
        } else if (morse[i] == ' ') {
          currentDelay = SPACE;
        }

        if(currentDelay != END && currentDelay!=SPACE){
          digitalWrite(LED, HIGH);
          vTaskDelay(currentDelay / portTICK_PERIOD_MS);
          digitalWrite(LED, LOW);
          vTaskDelay(currentDelay / portTICK_PERIOD_MS);
        }
        else
        {
          vTaskDelay(currentDelay / portTICK_PERIOD_MS);
        }
      }
    }
    else {
      vTaskDelay(100 / portTICK_PERIOD_MS);  // Avoid tight loop
    }
  }
}

void sendHTML(WiFiClient &client) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html");
  client.println("Connection: close");
  client.println();
  client.println("<!DOCTYPE html><html lang='en'><head><meta charset='UTF-8'>");
  client.println("<meta name='viewport' content='width=device-width, initial-scale=1.0'>");
  client.println("<title>Morse Indication</title>");
  client.println("<style>");
  client.println("body {");
  client.println("  font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;");
  client.println("  background: linear-gradient(135deg, #0f2027, #203a43, #2c5364);");
  client.println("  color: white;");
  client.println("  display: flex;");
  client.println("  flex-direction: column;");
  client.println("  align-items: center;");
  client.println("  justify-content: center;");
  client.println("  height: 100vh;");
  client.println("  margin: 0;");
  client.println("}");
  client.println(".container {");
  client.println("  background: rgba(255, 255, 255, 0.1);");
  client.println("  padding: 40px;");
  client.println("  border-radius: 12px;");
  client.println("  box-shadow: 0 8px 32px rgba(0,0,0,0.3);");
  client.println("  backdrop-filter: blur(10px);");
  client.println("  text-align: center;");
  client.println("}");
  client.println("input[type=text] {");
  client.println("  padding: 12px;");
  client.println("  width: 200px;");
  client.println("  border-radius: 6px;");
  client.println("  border: none;");
  client.println("  margin-right: 10px;");
  client.println("  font-size: 16px;");
  client.println("}");
  client.println("button {");
  client.println("  padding: 12px 20px;");
  client.println("  border: none;");
  client.println("  border-radius: 6px;");
  client.println("  background-color: #00c6ff;");
  client.println("  color: #fff;");
  client.println("  font-size: 16px;");
  client.println("  cursor: pointer;");
  client.println("  transition: background 0.3s ease;");
  client.println("}");
  client.println("button:hover { background-color: #0072ff; }");
  client.println("</style>");
  client.println("</head><body>");
  client.println("<div class='container'>");
  client.println("<h2 style='margin-bottom: 20px;'>Morse LED Controller</h2>");
  client.println("<input id='freqInput' type='text' placeholder='Enter message' />");
  client.println("<button onclick='setFreq()'>Send</button>");
  client.println("</div>");
  client.println("<script>");
  client.println("function setFreq() {");
  client.println("  let val = document.getElementById('freqInput').value;");
  client.println("  if(val) { fetch(`/freq?val=${encodeURIComponent(val)}`); }");
  client.println("}");
  client.println("</script>");
  client.println("</body></html>");
}


void loop() {
  // Nothing here – everything is in tasks
}
