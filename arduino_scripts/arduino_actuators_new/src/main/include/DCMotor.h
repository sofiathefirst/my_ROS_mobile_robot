#pragma once

#include "MagneticEncoder.h"

//////////////////////////////////////////////////////////////////

class DCMotor {

  MagneticEncoder *encoder;

  int INL; 
  int INH;
  boolean name;

  void initPins();
  void Stop();
  void CW(INT_PWM);
  void CCW(INT_PWM);
  INT_PWM protectOutput(INT_PWM);

  void motorCb(const robot_msgs::Motor&);

  ros::Subscriber<robot_msgs::Motor, DCMotor> sub;

public:

  //DCMotor();
  //DCMotor(int,int);
  //DCMotor(MagneticEncoder*);
  //DCMotor(int,int,MagneticEncoder*);
  DCMotor(int,int,MagneticEncoder*,boolean);

  void PublishAngle();
};

//////////////////////////////////////////////////////////////////
/*
DCMotor::DCMotor()
  : DCMotor::DCMotor(new MagneticEncoder(CS1)) {}

DCMotor::DCMotor(int INL, int INH) 
  : DCMotor::DCMotor(INL, INH, (MagneticEncoder*)NULL) {}

DCMotor::DCMotor(MagneticEncoder* encoder) 
  : DCMotor::DCMotor(IN1, IN2, encoder) {}

DCMotor::DCMotor(int INL, int INH, MagneticEncoder* encoder)
  : DCMotor::DCMotor(INL, INH, encoder, "left") {}  
*/
DCMotor::DCMotor(int INL, int INH, MagneticEncoder* encoder, boolean name)
  : INL(INL), INH(INH), encoder(encoder), name(name),
    sub("/dc_motor", &DCMotor::motorCb, this) 
    {
      DCMotor::initPins();
      nh.subscribe(sub);
    }

//////////////////////////////////////////////////////////////////

void DCMotor::initPins() {

  pinMode(this->INH, OUTPUT);
  pinMode(this->INL, OUTPUT);
  DCMotor::Stop();
}

void DCMotor::Stop() {
  // Motor don't stop
  analogWrite (INL, LOW); 
  analogWrite (INH, LOW);
}

void DCMotor::CW(INT_PWM val) {
  // Motor turns forward or CW
  analogWrite(INL, LOW);
  analogWrite(INH, protectOutput(val));
}

void DCMotor::CCW(INT_PWM val) {
  // Motor turns in the inverse direction or CCW
  analogWrite (INL, protectOutput(val));
  analogWrite (INH, LOW);
}

INT_PWM DCMotor::protectOutput(INT_PWM val) {

  // For security reasons
  val > MAX_VALUE? val = MAX_VALUE : val;

  return val;
}

void DCMotor::PublishAngle() {

  encoder->PublishAngle(name);
}

//////////////////////////////////////////////////////////////////

void DCMotor::motorCb(const robot_msgs::Motor& msg) {

  /**
   * robot_msgs::Motor
   *
   * bool name (LEFT: false, RIGHT: true)
   * float32 data
   *
   */

//  INT_PWM right_map = 
//    map(static_cast<INT_PWM>(u_r), 0, boundRight, 0, MAX_VALUE);
//  INT_PWM left_map = 
//    map(static_cast<INT_PWM>(u_l), 0, boundLeft, 0, MAX_VALUE);

  INT_PWM value = static_cast<INT_PWM>(msg.data);

  if (strcmp(msg.name, "left") == 0 && (name == LEFT)) 
  {
    // left
    msg.data? DCMotor::CW(value) : DCMotor::CCW(value); 
    
  } 
  else if (strcmp(msg.name, "right") && (name == RIGHT))
  {
    // right
    msg.data? DCMotor::CCW(value) : DCMotor::CW(value);
  }
}