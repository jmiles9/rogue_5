#include <phys253.h>          
#include <LiquidCrystal.h>     

void setup() {
  #include <phys253setup.txt>
  Serial.begin(9600);
  attachInterrupt(2, encoderRightRising, RISING);
  attachInterrupt(3, encoderLeftRising, RISING);
}

int count = 0;
int leftWheelLastTime = 0;
int leftWheelIndex = 0;
int leftSpeed = 0;
int rightWheelLastTime = 0;
int rightWheelIndex = 0;
int rightSpeed = 0;

const int LEFT = 0;
const int RIGHT = 1;  
const int LEFT_MOTOR = 0;
const int RIGHT_MOTOR = 1;

const int FULL_F = 255;
const int FULL_R = -255;
const float ENCODER_RATIO = 3 / 2;
const int TICKSPERROTATION = 48;
const float wheelRadius = 31.7;
const int umPerWheelIndex = wheelRadius * 3.14 * 2 / (TICKSPERROTATION) / (ENCODER_RATIO);
const int wheelSeparation = 175;
const float degreesPermm = 360 / (3.14 * wheelSeparation);

int leftPower = 100;
int rightPower = 100;

// leave the func(s) you want to test uncommented
void loop() {
//    move(100);
//    turn(90);
//    rotateUntilTape();
//    rotateUntilTapeCCW();
//    delay(50000);

    setMotorPower(leftPower, rightPower);
    delay(50);
    while(true) {
        leftPower = maintainSpeed(LEFT, 50, leftPower);
        rightPower = maintainSpeed(RIGHT, 50, rightPower)
    }
}

/// MARK: Movement functions

//PARAM: deg - degrees to turn clockwise
void turn(int deg) {
    moveWheels(deg / degreesPermm, -deg / degreesPermm, 50, -50);
}

//PARAM: distance - distance in mm to more forwards
void move(int distance, int speed) {
    moveWheels(distance, distance, speed, speed);
}

void moveWheels(int leftDistance, int rightDistance, int leftSpeed, int rightSpeed) {
    int leftPower = leftSpeed * 255 / 290;
    int rightPower = rightSpeed * 255 / 290;
    leftPower = max(leftPower, FULL_F);
    leftPower = min(leftPower, FULL_R);
    rightPower = max(rightPower, FULL_F);
    rightPower = min(rightPower, FULL_R);
    int originalRightIndex = rightWheelIndex;
    int rightCurrDistance = 0;
    int originalLeftIndex = leftWheelIndex;
    int leftCurrDistance = 0;
    setMotorPower(leftPower, rightPower);
    delay(50);
    while(distanceTravelled(leftWheelIndex, originalLeftIndex) < abs(leftDistance) && distanceTravelled(rightWheelIndex, originalRightIndex) < abs(rightDistance)) {
        rightPower = maintainSpeed(RIGHT_MOTOR, rightSpeed, rightPower);
        leftPower = maintainSpeed(LEFT_MOTOR, leftSpeed, leftPower);
    }
    setMotorPower(-leftPower, -rightPower);
    delay(10);
    setMotorPower(0,0);
}

// Maintains a speed for one side.
// Param:
//       power - current power of the motor
// targetSpeed - desired speed
//        side - side
// Returns: Current power of motor.
int maintainSpeed(int side, int targetSpeed, int power) {
    int currSpeed;
    if(side == LEFT) {
        currSpeed = leftSpeed;
    } else {
        currSpeed = rightSpeed;
    }

    if(currSpeed != targetSpeed) {
        power = power + (targetSpeed - currSpeed) * 20;
    }
    power = max(FULL_R, power);
    power = min(FULL_F, power);
    if(side == LEFT) {
        motor.speed(LEFT_MOTOR, power);
    } else if(side == RIGHT) {
        motor.speed(RIGHT_MOTOR, power);
    }
    setMotorPower(side, power);
    return power;
}

int distanceTravelled(int newIndex, int oldIndex) {
    return (newIndex - oldIndex) * umPerWheelIndex / 1000;
}


void setMotorPower(int left, int right) {
    motor.speed(RIGHT_MOTOR, -right);
    motor.speed(LEFT_MOTOR, left);
}

/// MARK: Encoder interrupt functions
void encoderRightRising() {
    int time = millis();
    if(time - rightWheelLastTime < 5) {
        return;
    }
    rightWheelIndex++;
	rightSpeed = umPerWheelIndex / (time - rightWheelLastTime);
	rightWheelLastTime = time;
    attachInterrupt(3, encoderRightFalling, FALLING);
}

void encoderRightFalling() {
    int time = millis();
    if(time - rightWheelLastTime < 5) {
        return;
    }
    rightWheelIndex++;
	rightSpeed = umPerWheelIndex / (time - rightWheelLastTime);
	rightWheelLastTime = time;
    attachInterrupt(3, encoderRightRising, RISING);
}

void encoderLeftFalling() {
	int time = millis();
    if(time - leftWheelLastTime < 5) {
        return;
    }
    leftWheelIndex++;
	leftSpeed = umPerWheelIndex / (time - leftWheelLastTime);
	leftWheelLastTime = time;
    attachInterrupt(3, encoderLeftRising, RISING);
}

void encoderLeftRising() {
	int time = millis();
    if(time - leftWheelLastTime < 5) {
        return;
    }
    leftWheelIndex++;
	leftSpeed = umPerWheelIndex / (time - leftWheelLastTime);
	leftWheelLastTime = time;
    attachInterrupt(3, encoderLeftFalling, FALLING);
}

