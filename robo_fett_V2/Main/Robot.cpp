#include "Robot.h"

using namespace configs;

// Constructor
Robot::Robot() {
    runState = RunState::STARTUP;
    nextEwok = 1;
    leftWheelIndex = 0;
    rightWheelIndex = 0;
    leftWheelLastTime = 0;
    rightWheelLastTime = 0;
    leftSpeed = 0;
    rightSpeed = 0;
    error = 0;
    RCServo6 = TINAH::Servo(RCSERVO6);
    RCServo7 = TINAH::Servo(RCSERVO7);
}

//starts at start
//goes into cruise_plat1
void Robot::STARTUP() {
    Serial.println("in startup");
    Funcs::sweepServo(ARM_LEFT, ARMS_DOWN_CHEWIE, ARMS_UP);
    Funcs::sweepServo(ARM_RIGHT, ARMS_DOWN_CHEWIE, ARMS_UP);
    Funcs::sweepServo(CLAW_LEFT, CLAWS_OPEN, CLAWS_CLOSED);
    Funcs::sweepServo(CLAW_RIGHT, CLAWS_OPEN, CLAWS_CLOSED);

    while(!menu.quitMenu){
        bool start = false;
        bool stopp = false;

        while(startbutton()){start = true;}
        if(start) menu.handleInput(BTN_START);

        while(stopbutton()){stopp = true;}
        if(stopp) menu.handleInput(BTN_STOP);
    }
    runState = RunState::CRUISE_PLAT1;
}

// Starts at start
//end when near first ewok 
//goes into ewok search
void Robot::CRUISE_PLAT1() {
    LCD.clear();
    LCD.setCursor(0,0);
    LCD.print("CRUISE_PLAT1");
    int startTime = millis();
    int count = 0;
    tapeFollowForDistance(150,255);
}

void Robot::EWOK_SEARCH() {
    while(true) {
        Funcs::setMotorPower(100,100);
        if(ewokDetect()) {
            
            delay(60);
            Funcs::hardStop();
            LCD.clear();LCD.home();
            LCD.setCursor(0,0); LCD.print("EWOK DETECTED");
            delay(2000);
            break;
        }
    }
    runState = RunState::EWOK_GRAB;
}

/// Grabs ewok.
/// If fails, attemps twice more, once a little in front, once behind.
void Robot::EWOK_GRAB() {
    LCD.clear();
    LCD.setCursor(0,0);
    LCD.print("EWOK_GRAB");
    // digitalWrite(configs::EWOK_IR_OUT,HIGH);
    // delay(50);
    // double with = analogRead(configs::EWOK_SENSOR);
    // digitalWrite(configs::EWOK_IR_OUT,LOW);
    // delay(50);
    // double without = analogRead(configs::EWOK_SENSOR);
    // Serial.println(with-without);
    // int count = 0;
    // while(with-without < 540) {
    //     setMotorPower(200,20);
    //     digitalWrite(configs::EWOK_IR_OUT,HIGH);
    //     delay(50);
    //     with = analogRead(configs::EWOK_SENSOR);
    //     digitalWrite(configs::EWOK_IR_OUT,LOW);
    //     delay(50);
    //     without = analogRead(configs::EWOK_SENSOR);
    //     Serial.println(with-without);
    //     count++;
    //     if(count > 10) {
    //         break;
    //     }
    // }
    // while(with-without > 540) {
    //     setMotorPower(-200,0);
    //     digitalWrite(configs::EWOK_IR_OUT,HIGH);
    //     delay(50);
    //     with = analogRead(configs::EWOK_SENSOR);
    //     digitalWrite(configs::EWOK_IR_OUT,LOW);
    //     delay(50);
    //     without = analogRead(configs::EWOK_SENSOR);
    //     Serial.println(with-without);
    //     count++;
    //     if(count > 40) {
    //         break;
    //     }
    // }
    // setMotorPower(0,0);
    
    TINAH::Servo arm;
    TINAH::Servo claw;
    int side;
    int stuffy;
    // Gets side and stuffy parameters
    switch(nextEwok) {
        case 1:
            arm = ARM_RIGHT;
            claw = CLAW_RIGHT;
            side = RIGHT;
            stuffy = EWOK;
            runState = RunState::DRAWBRIDGE;
            break;
        case 2:
            arm = ARM_RIGHT;
            claw = CLAW_RIGHT;
            side = RIGHT;
            stuffy = EWOK;
            runState = RunState::IR_WAIT;
            break;
        case 3: 
            arm = ARM_LEFT;
            claw = CLAW_LEFT;
            side = LEFT;
            stuffy = EWOK;
            runState = RunState::DUMP_PREP;        
            break;
        case 4: 
            arm = ARM_RIGHT;
            claw = CLAW_RIGHT;
            side = RIGHT;
            stuffy = EWOK;
            // TODO: Add state here
            break;
        case 5:
            arm = ARM_LEFT;
            claw = CLAW_LEFT;
            side = LEFT;
            stuffy = configs::CHEWIE;
            break;
    }
    Funcs::sweepServo(claw, CLAWS_CLOSED, CLAWS_OPEN);
    delay(3000);
    Funcs::sweepServo(arm, ARMS_UP, ARMS_DOWN_EWOK);
    delay(3000);
    //Serial.println("closing claws");
    Funcs::sweepServo(claw, CLAWS_OPEN, CLAWS_CLOSED);
    delay(3000);
    //Serial.println("raising arms");
    Funcs::sweepServo(arm, ARMS_DOWN_EWOK, ARMS_UP);
    delay(650);
    Funcs::sweepServo(claw, CLAWS_CLOSED, CLAWS_OPEN);
}

//starts after picking up first ewok
//ends when near second ewok
//goes into ewok_search
void Robot::DRAWBRIDGE() {

    runState = RunState::EWOK_SEARCH;
}

// starts right after first ewok is picked up
//ends after right IR is detected
//enters cruise_plat_2
void Robot::IR_WAIT() {

    while (record10KIRBeacon() < record1KIRBeacon()) {
    }
    runState = RunState::CRUISE_PLAT2;
}

//starts right after 10khz has been detected
//ends when ready to detect third ewok
//enters ewok_search
void Robot::CRUISE_PLAT2() {

    tapeFollowForDistance(PLAT2_CRUISE,255);
    runState = RunState::EWOK_SEARCH;

}

//starts after 3rd ewok is grabbed
//ends when aligned and has front right at wall
//goes into dump_ewoks
void Robot::DUMP_PREP() {
    turn(-TURN_90);
    //can either TapeFollowForDistance or put contact sensor on front? 
    tapeFollowForDistance(DUMP_PREP_DIST,255);
    runState = RunState::DUMP_EWOKS;

}

//starts after robot is aligned and has front right at wall
//ends after robots are dumped
//goes into find_zip_Plat2
void Robot::DUMP_EWOKS() {
    dumpBasket();
    runState = RunState::ZIP_HOOK;
}

//ALL STATES ABOVE THIS READY TO RUN

//starts after dump is finished
//ends when located under zipline on the 2nd platform
//enters zip_hook
void Robot::FIND_ZIP_PLAT2(){
    //need to move around
}

// starts when ewoks are dumped
//ends when attached to zipline and ready to go 
void Robot::ZIP_HOOK() {
    //need to find zipline, hook up
    //assume is started from already being on tape directly under line
    //likely need to do something other than turning
    turn(ZIPLINE_ATTACH_ROTATION);
    extendZipline();
    turn(-ZIPLINE_ATTACH_ROTATION);
    contractZipline();

    if (nextEwok ==  4){
        //going up to 3rd platform
    }else{
        //going down to safe zone :')
    }


}
//TODO: WRite
void Robot::ZIP_UP() {
    zipUp();
    //zipUp function checks for contact switch and has a time limit
}
// TODO: Write
void Robot::ZIP_UNHOOK() {
    extendZipline();
    turn(ZIPLINE_ATTACH_ROTATION);
    contractZipline();
    turn(-ZIPLINE_ATTACH_ROTATION);
}
// TODO: Write
void Robot::EWOK_4() {
    //may need to reverse a bit first
    EWOK_SEARCH();
    //may need to have something in here about edge detecting also, or should add to ewok search

}
// TODO: Write
void Robot::BRIDGE_FOLLOW() {

    turn(90);

    // this assumes when they are on tape they read HI = true
    //follows side of bridge until sees tape indicating zipline
    while(!(digitalRead(BRIDGE_QRD_LEFT) && digitalRead(BRIDGE_QRD_RIGHT))){
        bridgeFollow(TF_KP1,TF_KD1,TF_GAIN1);
    }

}
// TODO: WRite
void Robot::CHEWIE() {
    //will end on tape so can follow tape for a little bit longer
    EWOK_SEARCH();
}
// TODO: Write
void Robot::ZIP_DOWN() {
    turn(190);
    //turn around 180 degrees so goes down with sensor going outwards
    extendZipline();
    turn(-10);
    contractZipline();
    zipUp();
}
