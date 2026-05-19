#include "timer.h"
#include "Send_Recieve_data.h"
#define rs485_rate 115200
#define Max485_drive 4

// Control Variable
float Measure_speed = 0;
float Motor_speed = 0;
int8_t Motor_directionF;
int8_t Motor_directionB;
float FrontLeft_Angle = 0;
float Set_Angle = 0;

//Calculate speed
float  counter = 0;
float  rpm = 0 ;
float  rpmA = 0;
float  rpmB = 0;
float  Target_speed;
float  Set_motor_speed;
float  global_error;
int8_t motor_state;
uint16_t x;

//PID setup
float deltaT = 0.02 ;
float error = 0;
float de_dt = 0;
float e_prev = 0;
float e_integral= 0;
float Output;
float Kp = 4;
float Ki = 3;
float Kd = 0.001;

//PID_2 Setup
float error_2 = 0 ;// e(t-2)
float error_1 = 0 ;// e(t-1)
float error_0 = 0 ; // e(t)
float output = 0 ;  // Usually the current value of the actuator 

//Speed_Measure
bool Flag_50Hz = 0;
bool Flag_16kHz = 0;
uint8_t count_50Hz ;
uint8_t count_16kHz ;
int In0_chanelA_read;
int In1_chanelB_read;
float T_timer3 = 0.0000005; //(8/16 MHz)
uint16_t periodA;
uint16_t periodB;
uint16_t ICR1_lastA;
uint16_t ICR1_firstA;
uint16_t ICR1_lastB;
uint16_t ICR1_firstB;
uint16_t lastA;
uint16_t firstA;
uint16_t lastB;
uint16_t firstB;
int16_t  usable_counterA = 0;
int16_t  usable_counterB = 0;
int16_t  counterA = 0;
int16_t  counterB = 0;

//Const
float K = 255863.5394;

void setup() {
  // put your setup code here, to run once:
  //dip switch
  pinMode(A0, INPUT_PULLUP);
  pinMode(A1, INPUT_PULLUP);
  pinMode(A2, INPUT_PULLUP);
  pinMode(A3, INPUT_PULLUP);
  
  pinMode(9, OUTPUT); //motor
  pinMode(2, INPUT_PULLUP); //channelA
  pinMode(3, INPUT_PULLUP); //channelB
  pinMode(5, OUTPUT); //servo
  pinMode(6, OUTPUT); //motor in1
  pinMode(7, OUTPUT); //motor in2
  pinMode(Max485_drive, OUTPUT); //pin4
  digitalWrite(Max485_drive, LOW);
  attachInterrupt(digitalPinToInterrupt(2) , Count_pulses_A, CHANGE);
  attachInterrupt(digitalPinToInterrupt(3) , Count_pulses_B, CHANGE);
  Serial1.begin(rs485_rate);
  Serial.begin(115200);
  
  //Slave_ID
  slave_IP = PINF & B11110000;
  Modular_ID = slave_IP/16;
  Sending_order = ((Modular_ID == 2)? 11 :(Modular_ID == 3)? 12 :(Modular_ID == 4)? 13 :(Modular_ID == 5) ? 14 :(Modular_ID == 6)? 15 : 0);
  send_data.Footer[2] = Modular_ID;
  send_data.Header[2] = Sending_order; 

  //Timer
  Timer0();
  Timer1();
  Timer3();
}

void loop() 
{
  // put your main code here, to run repeatedly:
  if( Flag_50Hz)
  {    
   Flag_50Hz = 0 ;

//  Serial.println(Modular_ID);
  
  Motor_state();
  //Forward();
  Setservo_angle(Set_Angle);
  
  Cal_speed();
  PID_controller(abs(Motor_speed));
  Setmotor_speed(Set_motor_speed);  
  
  }
  
  if(FL_flag == 1)
    {
      // Serial.println("FL_flag on");
      FL_flag = 0;
      // Serial.print("FL_Speed: ");
      // Serial.println(recieve_data.FL_Speed);
      // Serial.print("FL_Angle: ");
      // Serial.println(recieve_data.FL_Angle);
      Set_Angle = (float)recieve_data.FL_Angle/10.0; 
      Motor_speed = (float)recieve_data.FL_Speed/10.0;

    }
  if(FR_flag == 1)
    {
      // Serial.println("FR_flag on");
      FR_flag = 0;
      // Serial.print("FR_Speed: ");
      // Serial.println(recieve_data.FR_Speed);
      // Serial.print("FR_Angle: ");
      // Serial.println(recieve_data.FR_Angle);
      Set_Angle = (float)recieve_data.FR_Angle/10.0; 
      Motor_speed = (float)recieve_data.FR_Speed/10.0;
    }
  if(RL_flag == 1)
    {
      // Serial.println("RL_flag on");
      RL_flag = 0;
      // Serial.print("RL_Speed: ");
      // Serial.println(recieve_data.RL_Speed);
      // Serial.print("RL_Angle: ");
      // Serial.println(recieve_data.RL_Angle);
      Set_Angle = (float)recieve_data.RL_Angle/10.0; 
      Motor_speed = (float)recieve_data.RL_Speed/10.0;
    }   
  if(RR_flag == 1)
    {
      // Serial.println("RR_flag on");
      RR_flag = 0;
      // Serial.print("RR_Speed: ");
      // Serial.println(recieve_data.RR_Speed);
      // Serial.print("RR_Angle: ");
      // Serial.println(recieve_data.RR_Angle);
      Set_Angle = (float)recieve_data.RR_Angle/10.0; 
      Motor_speed = (float)recieve_data.RR_Speed/10.0;
    }
  if(send_prepare_flag == 1)
  
    {
      send_prepare_flag = 0;

      send_data.confirm_angle = int16_t(Set_Angle*10); 
      send_data.confirm_speed = int16_t(Motor_speed*10);
      send_data.measure_speed = int16_t(Measure_speed*10);
      // Serial.println(send_data.measure_speed);
      send_data.angle_PWM_signal = int16_t(/* OCR3A*10 */ 230); 
      send_data.speed_PWM_signal = int16_t(OCR1A*10);

      send_flag = 1;
    } 
}
ISR (TIMER0_COMPA_vect)
{ 
    Sending_data();
}
ISR (TIMER1_CAPT_vect) 
{
  uint16_t input_capture = ICR1;
  //counter++;
}
ISR (TIMER1_OVF_vect) {
 TCNT1 = 24;
//  Serial.println(send_data.speed_PWM_signal);
 }
ISR (TIMER3_OVF_vect) {
     Flag_50Hz =1;
     lastA  =  ICR1_lastA ;
     firstA =  ICR1_firstA;
     lastB  =  ICR1_lastB;
     firstB =  ICR1_firstB;
     usable_counterA = counterA;
     usable_counterB = counterB;
     counterA = 0;
     counterB = 0; 
}
void Motor_state(){
  //motor_state = Motor_directionF + Motor_directionB;
  if(Motor_speed > 0)
  {
    Forward();
  }
  else if(Motor_speed < 0)
  {
    Backward();
  }
  else if(Motor_speed == 0)
  {
    Stop();
  }
}
void Setservo_angle(int16_t Servo_angle){
  OCR3A = mapFloat(Servo_angle, -450, 450, 2000, 4000);
}

void Setmotor_speed(uint16_t Motor_speed ){
  OCR1A = mapFloat(Motor_speed, 0, 400, 24, 820); //16.8/1023*12 + voltage drop
}
void Cal_speed(){
  periodA = ((lastA > firstA) ? (lastA - firstA) : (40000 - firstA + lastA));
  rpmA = (float)(((abs(usable_counterA)-1)*K)/(periodA));
  periodB = ((lastB > firstB) ? (lastB - firstB) : (40000 - firstB + lastB));
  rpmB = (float)(((abs(usable_counterB)-1)*K)/(periodB));
  rpm = (rpmA+rpmB)/2.0;
  Measure_speed = rpm;
  // Serial.println(Measure_speed);
  if(Measure_speed >= 400) Measure_speed = 400;
  else if(Measure_speed <= 20) Measure_speed = 0;
}
void Count_pulses_A(){
  if (counterA) ICR1_lastA = TCNT3;
  else ICR1_firstA = TCNT3;
  In0_chanelA_read = PIND & B00000011;                                
  counterA += ((In0_chanelA_read != 0) && (In0_chanelA_read != 3) ? (-1) : (+1)); 
}
void Count_pulses_B(){
  if (counterB) ICR1_lastB = TCNT3;
  else ICR1_firstB = TCNT3;
  In1_chanelB_read = PIND & B00000011;                                
  counterB += ((In1_chanelB_read != 0) && (In1_chanelB_read != 3) ? (-1) : (+1));
}
void PID_controller(int Tar_speed){  
  error = (Tar_speed - Measure_speed) ;
  e_integral += error*deltaT;
  e_integral  = (e_integral > 400)?400:(e_integral < -400)?-400:e_integral;
  de_dt = (error - e_prev)/deltaT;

  Output =  Kp*error + Ki*e_integral + Kd*de_dt;
  Output = (Output > 400)? 400 :(Output < 20) ? 0 : Output ;
  Set_motor_speed = Output;
  global_error = error;
  e_prev = error;
}
void PID_controller_2(int setpoint){ 
float A0 = Kp + Ki*deltaT + Kd/deltaT;
float A1 = -Kp - 2*Kd/deltaT;
float A2 = Kd/deltaT;
 
  error_2 = error_1;
  error_1 = error_0;
  error_0 = (setpoint - Measure_speed);
  Output = Output + A0*error_0 + A1*error_1 + A2*error_2;
  Output = (Output > 400)? 400 :(Output < 20) ? 0 : Output;
     
  Set_motor_speed = Output;
}

void Forward()
{
 PORTE &= ~(1<<PE6);
 PORTD |= (1<<PD7);
}

void Backward()
{
 PORTD &= ~(1<<PD7);
 PORTE |= (1<<PE6);
}

void Stop()
{
 PORTE &= ~(1<<PE6);
 PORTD &= ~(1<<PD7);
}
float mapFloat(float x, float x_min, float x_max, float y_min, float y_max){
  float y;
  y = y_min + (x - x_min)/(x_max - x_min)*(y_max - y_min);
  return y;
}
