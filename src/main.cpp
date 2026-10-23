#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <Servo.h>

const char *ssid = "EpsilonNET";
const char *password = "zaq123456";

ESP8266WebServer server(80);

Servo servo;

unsigned long last_time_s0 = 0;
unsigned long last_time_s90 = 0;
unsigned long last_time_s180 = 0;
unsigned long last_time_connecting = 0;
unsigned long start_attempt_time = 0;

enum State
{
  CONNECTING,
  SUCCESSFULLY,
  FAILED
};

State cur_state = CONNECTING;

void s0()
{
  server.send(200, "text/plain", "Server rotate on 0 degrees");
  servo.write(0);
}

void s90()
{
  server.send(200, "text/plain", "Server rotate on 90 degrees");
  servo.write(90);
}

void s180()
{
  server.send(200, "text/plain", "Server rotate on 180 degrees");
  servo.write(180);
}

void setup()
{
  WiFi.begin(ssid, password);
  Serial.begin(115200);
  servo.attach(D7);
  servo.write(0);
  server.begin();
}

void loop()
{
  unsigned long cur_time = millis();

  switch (cur_state)
  {
  case CONNECTING:
  {
    if (cur_time - last_time_connecting >= 500)
    {
      last_time_connecting = cur_time;
      Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.println("");
      Serial.println("WiFi connected successfully!");
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());

      // Реєструємо маршрути сервера один раз після успішного підключення
      server.on("/s0", HTTP_GET, s0);
      server.on("/s90", HTTP_GET, s90);
      server.on("/s180", HTTP_GET, s180);

      server.begin();
      Serial.println("HTTP server started");

      cur_state = SUCCESSFULLY;
    }
    // Якщо минуло більше 30 секунд — переходимо до стану помилки
    else if (cur_time - start_attempt_time >= 30000)
    {
      Serial.println("");
      Serial.println("Connection FAILED! Timeout reached.");
      cur_state = FAILED;
      start_attempt_time = cur_time;
    }
    break;
  }
  case SUCCESSFULLY:
  {
    server.handleClient();

    // Опціонально: перевірка на втрату зв'язку під час роботи
    if (WiFi.status() != WL_CONNECTED)
    {
      Serial.println("WiFi connection lost! Reconnecting...");
      WiFi.reconnect();
      cur_state = CONNECTING;
      start_attempt_time = cur_time;
    }

    break;
  }
  case FAILED:
  {
    if (cur_time - start_attempt_time >= 5000)
    {
      start_attempt_time = cur_time;
      Serial.println("Restarting ESP8266...");
      ESP.restart();
    }
    break;
  }
  default:
    break;
  }
}
