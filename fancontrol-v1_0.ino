/*
Eddy's temperature sensor and PWM fan speed controller.
This project was made to be placed inside the Riverbed x55 chasis.
See: https://makeitbreakitfixit.com/2016/06/20/hacking-a-riverbed-steelhead-cx755/

This code is protected under the GNU GENERAL PUBLIC LICENSE.
See https://github.com/Aristocles/TempController for more info.

www.makeitbreakitfixit.com
June 2016

This program reads the input from a thermistor (temperature sensor) and provides a PWM
signal to 3 fans to either switch them off or spin them at various speeds accordingly.
*/

//
// START CONFIGURATION (do not edit anything above this line)
//
unsigned int serial_baud = 9600; // Serial baud speed. Must match the baud set in Linux script file
// There are three temperature thresholds with corresponding fan speeds. All temps in celsius.
// You can configure the temperature variables below, but the explanation below uses the default values.
// Between 0 and 34 degrees the fan is off.
// Once temp reaches 35 degrees, LOW_FAN speed begins
// If temp drops below 30, the fan switches off again. If the temp rises above 55 MID_FAN speed begins
// If temp drops below 50, the fan drops back to LOW_FAN speed. If the temp rises above 65 HI_FAN speed begins
// If temp drops below 60, the fan drops back to MID_FAN speed.
//
// 0 ........................... 30 .. 35 ..............50 .. 55 ....... 60 .. 65 ...........
// |                              |    |                 |     |          |     |
// Off   
int low_min_temp = 40;
int low_max_temp = 55;
int mid_min_temp = 55;
int mid_max_temp = 65;
int hi_min_temp = 65;
int hi_max_temp = 75;
// Fan power is a PWM signal between 0 and 255.
int slo_fan_speed = 50;
int fst_fan_speed = 100;
int max_fan_speed = 250;
//
int dly = 1000; // Delay in ms to wait between polling temp sensor
bool testing = false; // Setting to true sends cycling false temperatures. For testing only.
//
// END CONFIGURATION (do not edit anything below this line)

int fanSpeed = 0;
int temp = 1;
bool incrementing = true;
#define THERMISTOR A0 // Analog pin for temp sensor
#define FAN1 3 // All 3 fans are controlled at the same time (ie. not controlled separetely)
#define FAN2 6
#define FAN3 9

void setup() {
  Serial.begin(serial_baud); // Serial sync speed
  pinMode(FAN1, OUTPUT); // PWM output pins
  pinMode(FAN2, OUTPUT);
  pinMode(FAN3, OUTPUT);
  delay(500);
}

void loop() {
  double temp = Thermister(analogRead(THERMISTOR));  // Read temp sensor
  fanSpeed = actionCheck(temp, fanSpeed); // Checks temp and returns one of 4 actions for fans. 0 = off, 1 = slow, 2 = fast, 3 = max
  Serial.print("["); Serial.print(temp); Serial.print("deg] ");
  executeAction(fanSpeed); // Sets the fans to appropriate action
  delay(dly);
}

//
// FUNCTIONS:
//
double Thermister(int RawADC) {
  if (testing) {
    if (temp >= (hi_max_temp + 9)) incrementing = false;
    if (temp <= 0) incrementing = true;
    if (incrementing) temp++;
    else temp--;
    return (temp % (hi_max_temp + 10)); // executed during testing
  }
  double temp = log(((10240000/RawADC) - 10000));
  // See http://en.wikipedia.org/wiki/Thermistor for explanation of formula
  temp = 1 / (0.001129148 + (0.000234125 * temp) + (0.0000000876741 * temp * temp * temp));
  temp = temp - 273.15;           // Convert Kelvin to Celcius
  return temp;
}

int actionCheck(int temp, int fanSpeed) {
  switch (fanSpeed) {
    case 1:
      if (temp < low_min_temp) return 0;
    case 2:
      if (temp < mid_min_temp) return 1;
    case 3:
      if (temp < hi_min_temp) return 2;
  }
  if (temp > hi_max_temp) return 3;
  if ((temp > mid_max_temp) && (fanSpeed == 1)) return 2; // the fanSpeed part ensures that the change only occurs when temp going up, not down
  if ((temp > low_max_temp) && (fanSpeed == 0)) return 1;
  return fanSpeed;
}

void executeAction(int fanSpeed) {
  Serial.print("Fan speed set to: "); Serial.println(fanSpeed);
  int rpm;
  switch (fanSpeed) {
    case 1:
      rpm = slo_fan_speed;
      break;
    case 2:
      rpm = fst_fan_speed;
      break;
    case 3:
      rpm = max_fan_speed;
      break;
    default:
      rpm = 0;
  }
  analogWrite(FAN1, rpm);
  analogWrite(FAN2, rpm);
  analogWrite(FAN3, rpm); 
}
