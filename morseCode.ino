/*
This is a program developed by Nandan Shukla. This code can be used in a non-commercial capacity by anyone. To use this code in a non-commercial capacity, 
the author has to be given credit inside the code and in the README or the source code file. This code is a multi-threaded software designed for ESP32 which will keep repeating a specific morse code
until it is changed by sending the new phase over UART or resetting.
*/

#define DOT 100
#define DASH 300
#define SPACE 700
#define END 1000
#define LED 2

String morseCharset[] = { ".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..", ".---", "-.-", ".-..", "--", "-.", "---", ".--.", "--.-", ".-.", "...", "-", "..-", "...-", ".--", "-..-", "-.--", "--..", ".----", "..---", "...--", "....-", ".....", "-....", "--...", "---..", "----.", "-----", "..--..", "-.-.--", ".-.-.-", "--..--", "-.-.-.", "---...", ".-.-.", "-....-", "-..-.", "-...-" };
String netCharset = "abcdefghijklmnopqrstuvwxyz1234567890?!.,;:+-/=";
String inputBuffer = "";
String morse = "";

TaskHandle_t blinkTaskHandle = NULL;

void setup() {
  // put your setup code here, to run once:
  pinMode(LED, OUTPUT);
  Serial.begin(115200);
  xTaskCreate(blinkTask, "Blink LED", 1000, NULL, 1, &blinkTaskHandle);
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

void blinkTask(void *parameters) {
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

void loop() {
  // put your main code here, to run repeatedly:
  while (Serial.available() > 0) {
    char input = Serial.read();

    // Check for newline or carriage return (end of command)
    if (input == '\n' || input == '\r') {
      if (inputBuffer.length() > 0) {
        morse = translator(inputBuffer);
        Serial.println(translator(inputBuffer));
        inputBuffer = "";  // Clear buffer after processing
      }
    } else {
      inputBuffer += input;  // Add char to buffer
    }
  }
}
