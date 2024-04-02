#include <AFMotor.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ThreeWire.h>
#include <RtcDS1302.h>
#include <Servo.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);

ThreeWire myWire(10, 6, 5);          // DAT, CLK, RST
RtcDS1302<ThreeWire> Rtc(myWire);    // RTC Object

// Motor Driver
AF_DCMotor motor2(2);
AF_DCMotor motor3(3);

Servo servoMotor;

int yellowLed = 1; // Heating light
int Relay = 13;    // Relay for Light Switching

// Data wire is plugged into digital pin 2 on the Arduino
#define ONE_WIRE_BUS 2

// Setup a oneWire instance to communicate with any OneWire device
OneWire oneWire(ONE_WIRE_BUS);

// Pass oneWire reference to DallasTemperature library
DallasTemperature sensors(&oneWire);

void setup()
{
  lcd.init();
  lcd.backlight();
  lcd.clear();

  lcd.begin(16, 2);
  lcd.setCursor(1, 1);
  lcd.print("STARTING");
  delay(500);
  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("TEMP. READING ");
  delay(500);
  lcd.clear();
  lcd.print(" AUTO SENSING ");
  delay(1000);
  lcd.clear();
  lcd.print(" PARAMETERS SET");
  delay(1000);
  lcd.clear();

  // RTC
  Rtc.Begin();
  pinMode(Relay, OUTPUT); // Set the relay pin as output

  // Motor Driver
  motor2.setSpeed(0); // Set initial speed to 0
  motor3.setSpeed(0);

  // Heating light
  pinMode(yellowLed, OUTPUT);

  // Servo Motor attach to pin 9
  servoMotor.attach(9);

  sensors.begin(); // Start up the temperature sensor library
  Serial.begin(9600); // Start the serial communication
}

void operateServo() {
  RtcDateTime now = Rtc.GetDateTime(); // Get current date and time

  if (now.Hour() == 20 && now.Minute() == 32 && now.Second() == 10)
  {
    Serial.println(" Moving servo to 90 degrees ");
    for (int pos = 0; pos <= 90; pos += 1)
    {
      servoMotor.write(pos); // Assuming 90 degrees is the desired position
      delay(15); // Introduce a delay
    }
  }
  else if (now.Hour() == 20  && now.Minute() == 23 && now.Second() == 10)
  {
    Serial.println(" Moving servo to 0 degrees ");
    for (int pos = 90; pos >= 0; pos -= 1)
    {
      servoMotor.write(pos);
      delay(15); // Introduce a delay
    }
  }
}

void loop()
{
  RtcDateTime now = Rtc.GetDateTime(); // Get current date and time

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Date: ");
  lcd.print(now.Day());
  lcd.print("/");
  lcd.print(now.Month());
  lcd.print("/");
  lcd.print(now.Year());

  lcd.setCursor(0, 1);
  lcd.print("Time: ");
  lcd.print(now.Hour());
  lcd.print(":");
  lcd.print(now.Minute());
  lcd.print(":");
  lcd.print(now.Second());

  delay(2000);
  lcd.clear();

  if (now.Hour() == 16 && now.Minute() == 30)
  {
    digitalWrite(Relay, LOW); // TURN OFF LIGHT RELAY
    Serial.println("LIGHT ON");
  }
  else if (now.Hour() == 16 && now.Minute() == 40)
  {
    digitalWrite(Relay, HIGH); // TURN ON LIGHT RELAY
    Serial.println("LIGHT OFF");
  }
  else if (now.Hour() == 17 && now.Minute() == 0)
  {
    digitalWrite(Relay, LOW); // TURN OFF LIGHT RELAY
    Serial.println("LIGHT ON");
  }
  else if (now.Hour() == 17 && now.Minute() == 10)
  {
    digitalWrite(Relay, HIGH); // TURN ON LIGHT RELAY
    Serial.println("LIGHT OFF");
  }

  operateServo(); // Call the servo operation function

  // Temperature sensing code
  sensors.requestTemperatures(); // Send the command to get temperatures

  // Print the temperature in Celsius
  Serial.print("Temperature: ");
  Serial.print(sensors.getTempCByIndex(0));
  Serial.print((char)176); // shows degrees character
  Serial.print("C  |  ");

  // Print the temperature in Fahrenheit
  Serial.print((sensors.getTempCByIndex(0) * 9.0) / 5.0 + 32.0);
  Serial.print((char)176); // shows degrees character
  Serial.println("F");

  int Temp = sensors.getTempCByIndex(0) * 100; // Convert to integer for compatibility with existing code
  temperature(Temp);
  delay(500);
}

// Temperature-related code
void temperature(int Temp)
{
  float c = Temp / 100.0; // Convert back to float temperature value

  lcd.setCursor(0, 0);
  lcd.print("T=");
  lcd.setCursor(3, 0);
  lcd.print(c);
  lcd.setCursor(8, 0);
  lcd.print("C");
  lcd.setCursor(10, 0);
  lcd.print((c * 9.0 / 5.0) + 32.0); // Print Fahrenheit directly from Celsius
  lcd.setCursor(15, 0);
  lcd.print("F");

  lcd.setCursor(0, 1);

  motor2.setSpeed(200);
  motor3.setSpeed(200);

  if (c < 18)
  {
    motor2.run(FORWARD); // Fan ON
    lcd.print("Heat:ON");
    lcd.setCursor(7, 1);
    lcd.print("FAN:ON");
    digitalWrite(yellowLed, HIGH); // Heating light ON
  }
  else if (c >= 18 && c < 25)
  {
    motor2.run(FORWARD);
    lcd.print("Fan:ON");
    digitalWrite(yellowLed, HIGH); // Heating light ON
  }
  else if (c >= 25 && c < 30)
  {
    motor2.run(FORWARD);
    motor3.run(RELEASE);
    lcd.print("FAN2:ON");
    lcd.setCursor(7, 1);
    lcd.print("HEAT:OFF");
    digitalWrite(yellowLed, LOW); // Heating light OFF
  }
  else if (c >= 30 && c < 35)
  {
    motor2.run(FORWARD);
    motor3.run(FORWARD);
    lcd.print("FAN3:ON");
    lcd.setCursor(7, 1);
    lcd.print("HEAT:OFF");
    digitalWrite(yellowLed, LOW); // Heating light OFF
  } else if (c >= 35 && c < 45) {
    motor2.run(FORWARD);
    motor3.run(FORWARD);
    lcd.print("FAN4:ON");
    lcd.setCursor(7, 1);
    lcd.print("HEAT:OFF");
    digitalWrite(yellowLed, LOW ); // Heating light OFF
  }
}
