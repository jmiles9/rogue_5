#include "configs.h"
#include "Funcs.h"

using namespace configs;

int tf_power;
int deg = 0;
int cumError = 0;

// Used in tapeFollow
void Funcs::setMotorPower(int left, int right) {
    motor.speed(RIGHT_MOTOR, -right);
    motor.speed(LEFT_MOTOR, left);
}


/**
 * tape following - reads sensor values, sets motor speeds using pd
 * param: kp   = proportional constant
 *        kd   = derivative constant
 *        ki   = integral constant
 *        gain = gain for pd
 */
void Funcs::tapeFollow(int kp, int kd, int ki, int gain, int power) {
    tf_power = power;
    bool farLeftOnTape = (analogRead(TAPE_QRD_FAR_LEFT) < TAPE_QRD_THRESHOLD) ? OFF_TAPE : ON_TAPE;
    bool midLeftOnTape = (analogRead(TAPE_QRD_MID_LEFT) < TAPE_QRD_THRESHOLD) ? OFF_TAPE : ON_TAPE;
    bool midRightOnTape = (analogRead(TAPE_QRD_MID_RIGHT) < TAPE_QRD_THRESHOLD) ? OFF_TAPE : ON_TAPE;
    bool farRightOnTape = (analogRead(TAPE_QRD_FAR_RIGHT) < TAPE_QRD_THRESHOLD) ? OFF_TAPE : ON_TAPE;

    //setting error values
    int lasterr = error; //saving the old error value

    // If one or two adjacent qrds are on tape, behaves as expected - gives error to center robot.
    // If three qrds are on tape, gives error to center robot if qrds are adjacent
    // If four qrds are on tape, or any combination of non-adjacent qrds are on tape,
    // this is unexpected, treat as anomaly, maintains last error.
    if(farLeftOnTape) {
        if(midLeftOnTape) {
            if(midRightOnTape) {
                if(farRightOnTape) {
                    error = lasterr;
                } else {
                    error = LARGE_LEFT_ERROR;
                }
            } else if(farRightOnTape) {
                error = lasterr;
            } else {
                error = MED_LEFT_ERROR;
            }
        } else if(midRightOnTape || farRightOnTape) {
            error = lasterr;
        } else {
            error = LARGE_LEFT_ERROR;
        }
    } else if(midLeftOnTape) {
        if(midRightOnTape) {
            if(farRightOnTape) {
                error = LARGE_RIGHT_ERROR;
            } else {
                error = CENTERED_ERROR;
                if(abs(deg) < 300) {
                  cumError = 0;
                 }
            }
        } else if(farRightOnTape) {
            error = lasterr;
        } else {
            error = SMALL_LEFT_ERROR;
            if(abs(deg) < 300) {
                cumError = 0;
            }
        }
    } else if(midRightOnTape) {
        if(farRightOnTape) {
            error = MED_RIGHT_ERROR;
        } else {
            error = SMALL_RIGHT_ERROR;
            if(abs(deg) < 300) {
                cumError = 0;
            }
        }
    } else if(farRightOnTape) {
        error = LARGE_RIGHT_ERROR;
    } else {
        error = lasterr;
    }
    cumError += error;
    cumError = min(100,cumError);
    cumError = max(-100,cumError);
    deg = (kp*error + kd*(error - lasterr) + ki * cumError)*gain;
    steer(deg);
}


// Used in tapefollow
void Funcs::steer(int error) {
    if(error > 0) {
        if(error > tf_power) error = tf_power;
        setMotorPower(tf_power, tf_power - error);
    } 
    else if(error < 0) {
        if(-error > tf_power) error = -tf_power;
        setMotorPower(tf_power + error, tf_power);
    } 
    else setMotorPower(tf_power, tf_power);
}

// Stops hard
void Funcs::hardStop() {
    Serial.println("STOP");
    setMotorPower(FULL_R, FULL_R);
    delay(10);
    setMotorPower(0,0);
}

// Param - distance in mm
void Funcs::tapeFollowForDistance(int distance) {
    leftWheelIndex = 0;
    rightWheelIndex = 0;
    int originalLeftIndex = leftWheelIndex;
    int originalRightIndex = rightWheelIndex;
    setMotorPower(230,230);
    while(distanceTravelled(leftWheelIndex, originalLeftIndex) < distance && distanceTravelled(rightWheelIndex, originalRightIndex) < distance) {
        tapeFollow(TF_KP1, TF_KD1, TF_KI1, TF_GAIN1, 230);
        Serial.println(distanceTravelled(leftWheelIndex, originalLeftIndex) < distance);
    }
    hardStop();
}

/// Parameters:
/// side and stuffy are defined in config now.
/// returns true if pickup was successful.
void Funcs::pickUp(int side, int stuffy) {
    TINAH::Servo arm;
    TINAH::Servo claw;

    int clawClose;
    int clawOpen;
    int clawRelease;
    int armUp;
    int armDown;
    int armRest;

    if(side == LEFT) {
        arm = ARM_LEFT;
        claw = CLAW_LEFT;
        clawClose = CLAW_CLOSED_LEFT;
        clawOpen = CLAW_OPEN_LEFT;
        clawRelease = CLAW_RELEASED_LEFT;
        armUp = ARM_UP_LEFT;
        if(stuffy == EWOK) {
            armDown = ARM_DOWN_EWOK_LEFT;
        } else {
            armDown = ARM_DOWN_CHEWIE_LEFT;
        }
        armRest = ARM_REST_LEFT;
    } else {
        arm = ARM_RIGHT;
        claw = CLAW_RIGHT;
        clawClose = CLAW_CLOSED_RIGHT;
        clawOpen = CLAW_OPEN_RIGHT;
        armUp = ARM_UP_RIGHT;
        clawRelease = CLAW_RELEASED_RIGHT;
        if(stuffy == EWOK) {
            armDown = ARM_DOWN_EWOK_RIGHT;
        } else {
            armDown = ARM_DOWN_CHEWIE_RIGHT;
        }
        armRest = ARM_REST_LEFT;
    }
    sweepServo(arm,armUp,armDown);
    delay(750);
    sweepServo(claw, clawOpen, clawClose);
    delay(600);
    sweepServo(arm, armDown, armUp);
    delay(750);
    sweepServo(claw, clawClose, clawRelease);
    delay(750);
    sweepServo(claw, clawOpen, clawClose);
    sweepServo(arm,armUp,armRest);
}

double Funcs::record1KIRBeacon() {
    return analogRead(IR_1KHZ);
}

double Funcs::record10KIRBeacon() {
    return analogRead(IR_10KHZ);
}

void Funcs::lowerBridge() {
    BASKET.write(BASKET_DROPBRIDGE);
    delay(1000);
    // may not need to close
    BASKET.write(BASKET_REST);
}

bool Funcs::checkBeacon() {
    return record10KIRBeacon() > record1KIRBeacon();
}

//PARAM: deg - degrees to turn clockwise
void Funcs::turn(int deg) {
    if(deg < 0) {
        moveWheels(-deg / degreesPermm, deg / degreesPermm, -200, 200);

    } else {
        moveWheels(deg / degreesPermm, -deg / degreesPermm, 200, -200);

    }
}

//PARAM: distance - distance in mm to more forwards
void Funcs::move(int distance, int speed) {
    int32_t originalLeft = leftWheelIndex;
    int32_t originalRight = rightWheelIndex;
    int32_t leftPower = speed / float(700) * 255;
    int32_t rightPower = speed / float(600) * 255;
    setMotorPower(leftPower, rightPower);
    Serial.println(leftPower);
    delay(100);
    while(distanceTravelled(leftWheelIndex, originalLeft) < abs(distance) && distanceTravelled(rightWheelIndex, originalRight) < abs(distance)) {
        Serial.print("Left: "); Serial.print(distanceTravelled(leftWheelIndex,originalLeft));
        Serial.print("     Right: "); Serial.println(distanceTravelled(rightWheelIndex,originalRight));
    }
    setMotorPower(0,0);
}

void Funcs::moveWheels(int leftDistance, int rightDistance, int32_t lSpeed, int32_t rSpeed) {
    int32_t leftPower = lSpeed * 255 / 550;
    int32_t rightPower = rSpeed * 255 / 500;
    int originalRightIndex = rightWheelIndex;
    int originalLeftIndex = leftWheelIndex;
    setMotorPower(leftPower, rightPower);
    delay(200);
    while(distanceTravelled(leftWheelIndex, originalLeftIndex) < abs(leftDistance) && distanceTravelled(rightWheelIndex, originalRightIndex) < abs(rightDistance)) {
        rightPower = maintainSpeed(RIGHT, abs(rSpeed), rightPower);
        leftPower = maintainSpeed(LEFT, abs(lSpeed), leftPower);
    }
    setMotorPower(-leftPower, -rightPower);
    delay(10);
    setMotorPower(0,0);
}


//// Maintains a speed for one side.
//// Param:
//// power       - current power of the motor
//// targetSpeed - desired speed
//// side        - side
//// Returns: Current power of motor.
int Funcs::maintainSpeed(int side, int targetSpeed, int power) {
    int currSpeed;
    if(side == LEFT) {
        if(millis() - leftWheelLastTime > 500) {
            Serial.println(leftWheelLastTime);
            Serial.println("LEFT:");
            Serial.println(millis());
          LCD.clear(); LCD.setCursor(0,0); LCD.print("ERROR");
          leftSpeed = 0;
        } 
        currSpeed = leftSpeed;
    } else {
        if(millis() - rightWheelLastTime > 500) {
          LCD.clear(); LCD.setCursor(0,0); LCD.print("ERROR");
          rightSpeed = 0;
            Serial.println(rightWheelLastTime);
            Serial.println("RIGHT:");
            Serial.println(millis());
        } 
        currSpeed = rightSpeed;
    }

 
    if(currSpeed != targetSpeed) {
        if(power < 0) {
          if(targetSpeed - currSpeed < 0) {
              Serial.print("TARGET: p-"); Serial.print(targetSpeed);
              Serial.print("  CURR: +"); Serial.println(currSpeed);
            power++;
          } else {
            power--;
              Serial.print("TARGET: p-"); Serial.print(targetSpeed);
              Serial.print("  CURR: -"); Serial.println(currSpeed);
          }
        } else {
          if(targetSpeed - currSpeed < 0) {
            power--;
                          Serial.print("TARGET: p+"); Serial.print(targetSpeed);
              Serial.print("  CURR: -"); Serial.println(currSpeed);
          } else {
            power++;
                          Serial.print("TARGET: p+"); Serial.print(targetSpeed);
              Serial.print("  CURR: +"); Serial.println(currSpeed);
          }
        }
    }
    power = max(FULL_R, power);
    power = min(FULL_F, power);
    if(side == LEFT) {
        motor.speed(LEFT_MOTOR, power);
    } else if(side == RIGHT) {
        motor.speed(RIGHT_MOTOR, -power);
    }
    return power;
}

void Funcs::rotateUntilTape() {
    setMotorPower(100,-100);
    while(true) {
        if(analogRead(TAPE_QRD_MID_LEFT > TAPE_QRD_THRESHOLD) ? OFF_TAPE : ON_TAPE == ON_TAPE 
        || analogRead(TAPE_QRD_MID_RIGHT > TAPE_QRD_THRESHOLD) ? OFF_TAPE : ON_TAPE == ON_TAPE) {
            setMotorPower(0,0);
            break;
        }
    }
}

void Funcs::rotateUntilTapeCCW() {
    setMotorPower(-100,100);
    while(true) {
        if(analogRead(TAPE_QRD_MID_LEFT > TAPE_QRD_THRESHOLD) ? OFF_TAPE : ON_TAPE == ON_TAPE 
        || analogRead(TAPE_QRD_MID_RIGHT > TAPE_QRD_THRESHOLD) ? OFF_TAPE : ON_TAPE == ON_TAPE) {
            setMotorPower(0,0);
            break;
        }
    }
}

int Funcs::speedToPower(int speed) {
    int power = speed * 20;
    power = min(FULL_F, power);
    power = max(FULL_R, power);
    return power;
}

void Funcs::dumpBasket() {
    BASKET.write(BASKET_DUMP);
    delay(500);
    BASKET.write(BASKET_REST);
}

void Funcs::extendZipline() {
    motor.speed(ZIP_ARM_MOTOR, ZIP_ARM_EXTENDING);
    long startTime = millis();
    while(!digitalRead(ZIP_SWITCH_EXTENDED)) {
        // Give the arm 8 seconds to extend
        if(millis() > startTime + 8000) {
            break;
        }
    }
    motor.stop(ZIP_ARM_MOTOR);
}

void Funcs::contractZipline() {
    motor.speed(ZIP_ARM_MOTOR, ZIP_ARM_CONTRACTING);
    long startTime = millis();
    while(!digitalRead(ZIP_SWITCH_CLOSED)) {
        // Give the arm 8 seconds to contract
        if(millis() > startTime + 8000) {
            break;
        }
    }
    motor.stop(ZIP_ARM_MOTOR);
}

void Funcs::zipUp() {
    motor.speed(ZIP_WHEEL_MOTOR, ZIPPING_UP);
    double startTime = millis();
    while(!digitalRead(ZIPPED_UP_SWITCH)) {
        // Give 10 seconds to make it up zipline
        if(millis() > startTime + 10000) {
            break;
        }
    }
    motor.stop(ZIP_WHEEL_MOTOR);
}

void Funcs::findEdge() {
}

//TODO: write this. Should pretty much be tapefollow, different sensors.
void Funcs::bridgeFollow(int kp, int kd, int gain) {

    //should be true if they are OFF the bridge
    bool rightOffBridge = digitalRead(BRIDGE_QRD_RIGHT);
    bool leftOffBridge = digitalRead(BRIDGE_QRD_LEFT);

    int lastError = error;
    //use same static error that tapefollow uses, shouldn't be issue as don't tapefollow again

    if (!(leftOffBridge) && rightOffBridge){
        error = 0;
        //is good
    }
    if (leftOffBridge && rightOffBridge){
        //need to turn left
        error = -1;
    }
    if (!(leftOffBridge) && (!(rightOffBridge))){
        //need to turn right
        error = 1;
    }
    
    steer((kp*error + kd*(error - lastError))*gain) ;
}

bool Funcs::edgeDetect() {
  return analogRead(TAPE_QRD_FAR_LEFT) > EDGE_QRD_THRESHOLD
        && analogRead(TAPE_QRD_MID_LEFT) > EDGE_QRD_THRESHOLD 
        && analogRead(TAPE_QRD_MID_RIGHT) > EDGE_QRD_THRESHOLD 
        && analogRead(TAPE_QRD_FAR_RIGHT) > EDGE_QRD_THRESHOLD;
}

void Funcs::tapeFollowToEdge(int speed){
    while(true) {
        tapeFollow(TF_KP1, TF_KD1, TF_KI1, TF_GAIN1, speed);
        if(edgeDetect()) {
            hardStop();
        }
    }
}

// mm
int Funcs::distanceTravelled(int newIndex, int oldIndex) {
    return float((newIndex - oldIndex)) / float(1000.0) * umPerWheelIndex * 2.7 / 2 / 1.4 / 1.08 / 1.08 / 1.05;
}

bool Funcs::ewokDetectRight() {
    int count = 0;
    while(checkEwokSensor(RIGHT) > EWOK_THRESH) {
        count++;
        if(count >= 2) {
            Serial.println("RETURNING TRUE");
            return true;
        }
    }
    return false;
}

bool Funcs::ewokDetectLeft() {
    digitalWrite(EWOK_IR_OUT_LEFT,HIGH);
    delay(20);
    double with = analogRead(EWOK_SENSOR_LEFT);
    digitalWrite(EWOK_IR_OUT_LEFT,LOW);
    delay(20);
    double without = analogRead(EWOK_SENSOR_LEFT);
    int count = 0;
    while(abs(with-without > EWOK_THRESH)) {
        Serial.println(with-without-EWOK_THRESH);
        Serial.println(count);
        count++;
        if(count >= 2) {
            Serial.println("RETURNING TRUE");
            return true;
        }
        digitalWrite(EWOK_IR_OUT_LEFT,HIGH);
        delay(10);
        with = analogRead(EWOK_SENSOR_LEFT);
        digitalWrite(EWOK_IR_OUT_LEFT,LOW);
        delay(10);
        without = analogRead(EWOK_SENSOR_LEFT);
        Serial.println(with-without);
        }
    return false;
}

//function to move servo, works more consistently than servo.write
void Funcs::sweepServo(TINAH::Servo servo, int startAngle, int endAngle) {
    int dA = endAngle > startAngle ? 3 : -3;
    int currAngle = startAngle;
    while(abs(endAngle-currAngle) > 10) {
        servo.write(currAngle + dA);
        currAngle += dA;
    }
    servo.write(endAngle);
    servo.write(endAngle);
}

//rotate until middle qrds are on tape
void Funcs::rotateUntilTape(int direction) {
    if(direction == CLOCKWISE) {
        setMotorPower(100,-100);
    } else {
        setMotorPower(-100,100);
    }
    while(true) {
        if(digitalRead(TAPE_QRD_MID_RIGHT) || digitalRead(TAPE_QRD_MID_LEFT)) {
            //stop
            if(direction == CLOCKWISE) {
                setMotorPower(-100,100);
            } else {
                setMotorPower(100,-100);
            }
            setMotorPower(0,0);
            break;
        }
    }
}

bool Funcs::digitalReadMultiplex(int port){
  int bin[3] = {0,0,0};
  int i = 0;
  int num = port;
  while (num > 0)
   {
       bin[i] = num % 2;
       num /= 2;
        i++;
   }

  digitalWrite(MULTI_CHOOSEA, bin[0]);
  digitalWrite(MULTI_CHOOSEB, bin[1]);
  digitalWrite(MULTI_CHOOSEC, bin[2]);

  return digitalRead(MULTIPLEX_IN);
  
}

// finds tape, turns right, turns left, big right, big left
void Funcs::findTape() {
    //first check left slightly
    int time = millis();
    setMotorPower(100,-100);
    while(millis() - time < 250) {
        if( analogRead(TAPE_QRD_MID_RIGHT) > TAPE_QRD_THRESHOLD || analogRead(TAPE_QRD_MID_LEFT) > TAPE_QRD_THRESHOLD ) {
            setMotorPower(-100,100);
            setMotorPower(0,0);
            return;
        }
    }
    //then check right slightly
    time = millis();
    setMotorPower(-100,100); {
        while(millis() - time < 500) {
        if( analogRead(TAPE_QRD_MID_RIGHT) > TAPE_QRD_THRESHOLD || analogRead(TAPE_QRD_MID_LEFT) > TAPE_QRD_THRESHOLD ) {
            setMotorPower(100,-100);
            setMotorPower(0,0);
            return;
        }
        }
    }
    //then check left more
    time = millis();
    setMotorPower(100,-100);
    while(millis() - time < 750) {
        if( analogRead(TAPE_QRD_MID_RIGHT) > TAPE_QRD_THRESHOLD|| analogRead(TAPE_QRD_MID_LEFT) > TAPE_QRD_THRESHOLD ) {
            setMotorPower(-100,100);
            setMotorPower(0,0);
            return;
        }
    }
    //then check right more
    time = millis();
    setMotorPower(-100,100); {
        while(millis() - time < 1500) {
        if( analogRead(TAPE_QRD_MID_RIGHT) > TAPE_QRD_THRESHOLD|| analogRead(TAPE_QRD_MID_LEFT) > TAPE_QRD_THRESHOLD ) {
            setMotorPower(100,-100);
            setMotorPower(0,0);
            return;
        }
        }
    }
}
    int Funcs::checkEwokSensor(int side) {
        int sensor;
        int digOut;
        if(side == LEFT) {
            sensor = EWOK_SENSOR_LEFT;
            digOut = EWOK_IR_OUT_LEFT;
        } else {
            sensor = EWOK_SENSOR_RIGHT;
            digOut = EWOK_IR_OUT_RIGHT;
        }
        digitalWrite(digOut,HIGH);
        delay(10);
        double with = analogRead(sensor);
        digitalWrite(digOut,LOW);
        delay(10);
        double without = analogRead(sensor);
        return with-without;
    }

void Funcs::centerOffEdge() {
    while(analogRead(TAPE_QRD_FAR_LEFT) > EDGE_QRD_THRESHOLD || analogRead(TAPE_QRD_FAR_RIGHT) > EDGE_QRD_THRESHOLD) {
        while(analogRead(TAPE_QRD_FAR_LEFT) > EDGE_QRD_THRESHOLD) {
            setMotorPower(-120,98);
        }
        while(analogRead(TAPE_QRD_FAR_RIGHT) > EDGE_QRD_THRESHOLD) {
            setMotorPower(98,-120);
        }
    }
    setMotorPower(0,0);
    setMotorPower(-85,-90);
    delay(350);
    setMotorPower(0,0);
}