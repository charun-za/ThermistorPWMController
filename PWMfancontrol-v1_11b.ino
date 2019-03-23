/*
SQUIRE's (charun-za) MODDED temperature sensor and PWM fan speed controller (https://github.com/Aristocles/ThermistorPWMController)
Added a few more data points (15 in total) for smoother control. As my in case its controling 12x60mm 20 000 RPM server fans in
2U rack mount server. Original server fan controller is too loud for home use and you cannot edit it.

This code is protected under the GNU GENERAL PUBLIC LICENSE.
See https://github.com/Aristocles/TempController for more info.

Original AUTHOR:
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
// The fan is never off.
// If temp goes higher than 47c (78% duty cycle) is applied.
// If temp drops below 34c, the fan switches to low duty cycle (27%),  If the temp rises above 37 "mid_0" speed begins
// If temp drops below 38c, the fan drops back to "low_4" speed. If the temp rises above 42 "hi_0:" speed begins
// If temp drops below 42, the fan drops back to "mid_4" speed.
// Customise the Temp and Duty Cycle in % (255 max pwm) to get duty cycle in % use this equation; 
// PWM/255*100 EG; 80/255*100=31.37% duty cycle

//Temperature for each state(15) 0-47c
int low_0_temp = 0; //state 1
int low_1_temp = 34; //state 2
int low_2_temp = 35; //state 3
int low_3_temp = 36; //state 4
int low_4_temp = 37; //state 5
int mid_0_temp = 38; //state 6
int mid_1_temp = 39; //state 7
int mid_2_temp = 40; //state 8
int mid_3_temp = 41; //state 9
int mid_4_temp = 42; //state 10
int hi_0_temp = 43; //state 11
int hi_1_temp = 44; //state 12
int hi_2_temp = 45; //state 13
int hi_3_temp = 46; //state 14
int hi_4_temp = 47; //state 15
// Fan power is a PWM signal between 0 and 200(max255), for 15 states.
int slo_0_speed = 70; //0-34c state 1
int slo_1_speed = 71; //34-35c state 2
int slo_2_speed = 72; //35-36c state 3
int slo_3_speed = 73; //36-37c state 4
int slo_4_speed = 74; //37-38c state 5
int fst_0_speed = 75; //38-39c state 6
int fst_1_speed = 76; //39-40c state 7
int fst_2_speed = 78; //40-41c state 8
int fst_3_speed = 80; //41-42c state 9
int fst_4_speed = 82; //42-43c state 10
int max_0_speed = 84; //43-44c state 11
int max_1_speed = 84; //44-45c state 12
int max_2_speed = 92; //45-46c state 13
int max_3_speed = 100; //46-47c state 14
int max_4_speed = 200; //47c+ state 15
//

int dly = 1000; // Delay in ms to wait between polling temp sensor
bool testing = false; // Setting to true sends cycling false temperatures. For testing only.
//
// END CONFIGURATION (do not edit anything below this line)

int fanSpeed = 1;
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
  Serial.print("("); Serial.print(temp); Serial.print("°C) ");
  executeAction(fanSpeed); // Sets the fans to appropriate action
  delay(dly);
}

//
// FUNCTIONS:
//
double Thermister(int RawADC) {
  if (testing) {
    if (temp >= (hi_4_temp + 9)) incrementing = false;
    if (temp <= 0) incrementing = true;
    if (incrementing) temp++;
    else temp--;
    return (temp % (hi_4_temp + 10)); // executed during testing
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
      if (temp < low_0_temp) return 0;
    case 2:
      if (temp < low_1_temp) return 1;
    case 3:
      if (temp < low_2_temp) return 2;      
    case 4:
      if (temp < low_3_temp) return 3; 
    case 5:
      if (temp < low_4_temp) return 4;                           
    case 6:
      if (temp < mid_0_temp) return 5;
    case 7:
      if (temp < mid_1_temp) return 6;
    case 8:
      if (temp < mid_2_temp) return 7;
    case 9:
      if (temp < mid_3_temp) return 8;
    case 10:
      if (temp < mid_4_temp) return 9;                        
    case 11:
      if (temp < hi_0_temp) return 10;
    case 12:
      if (temp < hi_1_temp) return 11;
    case 13:
      if (temp < hi_2_temp) return 12;
    case 14:
      if (temp < hi_3_temp) return 13;  
    case 15:
      if (temp < hi_4_temp) return 14;     
  }
  if (temp > hi_4_temp) return 13;
  if ((temp > hi_3_temp) && (fanSpeed == 1)) return 12; // the fanSpeed part ensures that the change only occurs when temp going up, not down
  if ((temp > hi_2_temp) && (fanSpeed == 0)) return 11;
  if ((temp > hi_1_temp) && (fanSpeed == 0)) return 10;
  if ((temp > hi_0_temp) && (fanSpeed == 0)) return 9;
  if ((temp > mid_4_temp) && (fanSpeed == 0)) return 8;
  if ((temp > mid_3_temp) && (fanSpeed == 0)) return 7;
  if ((temp > mid_2_temp) && (fanSpeed == 0)) return 6;
  if ((temp > mid_1_temp) && (fanSpeed == 0)) return 5;
  if ((temp > mid_0_temp) && (fanSpeed == 0)) return 4;
  if ((temp > low_4_temp) && (fanSpeed == 0)) return 3;
  if ((temp > low_3_temp) && (fanSpeed == 0)) return 2;
  if ((temp > low_2_temp) && (fanSpeed == 0)) return 1;
  if ((temp > low_1_temp) && (fanSpeed == 0)) return 0;
  if ((temp > low_0_temp) && (fanSpeed == 0)) return 0;
    
  return fanSpeed;
}

void executeAction(int fanSpeed) {
  Serial.print("Fan speed set to: "); Serial.println(fanSpeed);
  int rpm;
  switch (fanSpeed) {
    case 1:
      rpm = slo_0_speed;
      break;
    case 2:
      rpm = slo_1_speed;
      break;
    case 3:
      rpm = slo_2_speed;
      break;
    case 4:
      rpm = slo_3_speed;
      break;
     case 5:
      rpm = slo_4_speed;
      break;    
      case 6:
      rpm = fst_0_speed;
      break;     
     case 7:
      rpm = fst_1_speed;
      break;      
     case 8:
      rpm = fst_2_speed;
      break;      
     case 9:
      rpm = fst_3_speed;
      break;      
     case 10:
      rpm = fst_4_speed;
      break;      
     case 11:
      rpm = max_0_speed;
      break;      
     case 12:
      rpm = max_1_speed;
      break;      
      case 13:
      rpm = max_2_speed;
      break;      
      case 14:
      rpm = max_3_speed;
      break;      
     case 15:
      rpm = max_4_speed;
      break;      
     
      default:
      rpm = 0;
  }
  analogWrite(FAN1, rpm);
  analogWrite(FAN2, rpm);
  analogWrite(FAN3, rpm); 
}
