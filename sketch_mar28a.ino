#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define DHTPIN 2
#define DHTTYPE DHT11
#define PIR_PIN 3
#define BUZZER 8
#define SLEEP_TIMEOUT 60000
#define ALERT_COOLDOWN 300000

DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);

unsigned long lastMotion = 0;
unsigned long lastAlert = -ALERT_COOLDOWN;
bool displayOn = false;
int lastLightState = -1;

void beep(int times, int duration) {
  for (int i = 0; i < times; i++) {
    digitalWrite(BUZZER, LOW);
    delay(duration);
    digitalWrite(BUZZER, HIGH);
    delay(duration);
  }
}

void longBeep(int duration) {
  digitalWrite(BUZZER, LOW);
  delay(duration);
  digitalWrite(BUZZER, HIGH);
}

void checkPIR() {
  if (digitalRead(PIR_PIN) == HIGH) lastMotion = millis();
}

void screenDelay(int ms) {
  unsigned long s = millis();
  while (millis() - s < ms) checkPIR();
}

void scrollMessage(String msg) {
  Serial.println("ALERT: " + msg);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("!! Alert !!");
  String padded = "                " + msg + "                ";
  for (int i = 0; i < (int)padded.length() - 15; i++) {
    lcd.setCursor(0, 1);
    lcd.print(padded.substring(i, i + 16));
    delay(300);
    checkPIR();
  }
}

void scrollInfo(String msg) {
  Serial.println("INFO: " + msg);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("-- Info --");
  String padded = "                " + msg + "                ";
  for (int i = 0; i < (int)padded.length() - 15; i++) {
    lcd.setCursor(0, 1);
    lcd.print(padded.substring(i, i + 16));
    delay(300);
    checkPIR();
  }
}

String getAirCategory(int val) {
  if (val < 300) return "Good";
  else if (val < 600) return "Moderate";
  else return "Poor";
}

int getRoomScore(float t, float h, int airVal) {
  int score = 10;
  if (airVal > 600) score -= 4;
  else if (airVal > 300) score -= 2;
  if (t > 32 && h > 60) score -= 3;
  else if (t > 32) score -= 2;
  else if (h > 60) score -= 1;
  if (score < 0) score = 0;
  return score;
}

void setup() {
  Serial.begin(9600);
  dht.begin();
  lcd.init();
  lcd.backlight();
  pinMode(PIR_PIN, INPUT);
  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, HIGH);

  lcd.setCursor(0, 0);
  lcd.print("ARIA starting...");
  lcd.setCursor(0, 1);
  lcd.print("Calibrating...");
  Serial.println("ARIA starting...");
  Serial.println("Calibrating sensors...");
  Serial.println("Alert limits: Temp 32C | Humidity 60% | Air Quality 600");
  beep(2, 100);
  delay(3000);

  float h = dht.readHumidity();
  float t = dht.readTemperature();
  int airQuality = analogRead(A0);
  Serial.print("Boot readings — Temp: "); Serial.print(t);
  Serial.print("C  Humidity: "); Serial.print(h);
  Serial.print("%  Air Quality: "); Serial.println(airQuality);
  if (t > 32) Serial.println("WARNING: Room already above temp limit at boot");
  if (h > 60) Serial.println("WARNING: Room already above humidity limit at boot");
  if (airQuality > 600) Serial.println("WARNING: Air quality already poor at boot");

  lcd.noBacklight();
  lcd.clear();
}

void loop() {
  checkPIR();

  if (digitalRead(PIR_PIN) == HIGH) {
    if (!displayOn) {
      lcd.backlight();
      displayOn = true;
      beep(1, 100);
      Serial.println("INFO: Motion detected - display on");
    }
  }

  if (displayOn && (millis() - lastMotion > SLEEP_TIMEOUT)) {
    lcd.noBacklight();
    lcd.clear();
    displayOn = false;
    Serial.println("INFO: No motion - display off");
    return;
  }

  if (!displayOn) return;

  float h = dht.readHumidity();
  float t = dht.readTemperature();
  int airQuality = analogRead(A0);
  int light = analogRead(A1);

  Serial.print("Temp: "); Serial.print(t); Serial.print("C  ");
  Serial.print("Humidity: "); Serial.print(h); Serial.print("%  ");
  Serial.print("Air Quality: "); Serial.print(airQuality); Serial.print("  ");
  Serial.print("Light: "); Serial.println(light);

  int currentLightState = (light < 500) ? 1 : 0;
  if (lastLightState != -1 && currentLightState != lastLightState) {
    if (currentLightState == 1) {
      beep(2, 80);
      scrollInfo("Lights turned on");
    } else {
      longBeep(500);
      scrollInfo("Lights turned off");
    }
  }
  lastLightState = currentLightState;

  if (millis() - lastAlert > ALERT_COOLDOWN) {
    if (airQuality > 600) {
      beep(3, 300);
      scrollMessage("Open your door - Poor air");
      lastAlert = millis();
    } else if (t > 32 && h > 60) {
      beep(2, 150);
      scrollMessage("Too hot and humid");
      lastAlert = millis();
    } else if (t > 32) {
      beep(1, 200);
      scrollMessage("Room is too hot");
      lastAlert = millis();
    } else if (h > 60) {
      beep(1, 200);
      scrollMessage("Room is too humid");
      lastAlert = millis();
    }
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("T:"); lcd.print(t, 1); lcd.print("C H:"); lcd.print(h, 0); lcd.print("%");
  lcd.setCursor(0, 1);
  if (t > 32 && h > 60) lcd.print("Too hot&humid");
  else if (t > 32) lcd.print("Too hot");
  else if (h > 60) lcd.print("Too humid");
  else lcd.print("Comfortable");
  screenDelay(3000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Air Quality:");
  lcd.setCursor(0, 1);
  lcd.print(getAirCategory(airQuality));
  lcd.print(" ("); lcd.print(airQuality); lcd.print(")");
  screenDelay(3000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Light: "); lcd.print(light);
  lcd.setCursor(0, 1);
  lcd.print("Brightness level");
  screenDelay(3000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Room Score:");
  lcd.setCursor(0, 1);
  lcd.print(getRoomScore(t, h, airQuality));
  lcd.print("/10  ");
  lcd.print(getAirCategory(airQuality));
  screenDelay(3000);
}