#pragma once

#include "ATmega2560-HW.h"
#include <AS5048A.h>

#define M_PI 3.14159265359

// 0 to +2*Pi
const boolean ZERO_TO_2PI PROGMEM = false;
// -Pi to +Pi
const boolean PLUS_MINUS_PI PROGMEM = true;

class MagneticEncoder
{
	boolean angleMode;
	float initialAngles;

	double lastTime;
	float lastPos;
	
	AS5048A *Encoder;
	sensor_msgs::JointState msg;
	ros::Publisher pub;

	float read2angle(uint16_t);
	float normalize(float);

public:
	//MagneticEncoder();
	MagneticEncoder(uint8_t);
	MagneticEncoder(uint8_t, boolean);

	void PublishAngle(const boolean);
};

///////////////////////////////////////////////////////////////
/*
MagneticEncoder::MagneticEncoder()
	: MagneticEncoder::MagneticEncoder(CS1, PLUS_MINUS_PI) {}
*/
MagneticEncoder::MagneticEncoder(uint8_t digitalPin)
	: MagneticEncoder::MagneticEncoder(digitalPin, PLUS_MINUS_PI) {}

MagneticEncoder::MagneticEncoder(uint8_t digitalPin, boolean mode) 
	: Encoder(new AS5048A(digitalPin)), 
	  angleMode(mode), lastTime(0.0), lastPos(0.0),
	  pub("/encoder", &msg) {

	Encoder->init();

	initialAngles = 
		MagneticEncoder::read2angle( Encoder->getRawRotation() );

	nh.advertise(pub);
}

///////////////////////////////////////////////////////////////

float MagneticEncoder::read2angle(uint16_t registerOutput) {

	return registerOutput * ((float)2*M_PI / 16383);
}

float MagneticEncoder::normalize(float angle) {

	if (angleMode == PLUS_MINUS_PI)	angle += M_PI;

	angle = fmod(angle, 2*PI);
	
	if (angle < 0) angle += 2*PI;

	if (angleMode == PLUS_MINUS_PI)	angle -= M_PI;

	return angle;
}

void MagneticEncoder::PublishAngle(const boolean encoder_name) {
		
	float currentAngle = 
			MagneticEncoder::read2angle(Encoder->getRawRotation());

	if (encoder_name == LEFT) {
		
		msg.header.frame_id = "left_wheel_frame";
	}
	else {

		msg.header.frame_id = "right_wheel_frame";
	}

	msg.header.stamp = nh.now();
	double dT 		 = msg.header.stamp.toSec() - lastTime;

	msg.position[0]	 = normalize( currentAngle - initialAngles);
	msg.velocity[0]	 = (msg.position[0] - lastPos) / dT;

	// setting the length
	msg.name_length 	= 0;	// Empty because of an error (use frame_id instead)
	msg.position_length = 1;
	msg.velocity_length = 1;
	msg.effort_length 	= 0;

	pub.publish(&msg);

	lastTime = msg.header.stamp.toSec();
	lastPos = msg.position[0];
}