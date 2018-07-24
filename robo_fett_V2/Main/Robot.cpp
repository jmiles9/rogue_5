#include "Robot.h"
#include "Menu.h"

using namespace configs;

class Robot : public Funcs {

public:
	
	// Constructor
	ROBOT::Robot() {
		state = STARTUP;
        nextEwok = 1;
		leftWheelIndex = 0;
        rightWheelIndex = 0;
		leftWheelLastTime = 0;
        rightWheelLastTime = 0;
        leftSpeed = 0;
        rightSpeed = 0;
		cruiseIndex = 0;
	}

	void STARTUP() { 
		while(!menu.quitMenu){
			bool start = false;
   			bool stopp = false;
			   
    		while(startbutton()){start = true;}
    		if(start) menu.handleInput(BTN_START);

    		while(stopbutton()){stopp = true;}
    		if(stopp) menu.handleInput(BTN_STOP);
		}
		state = CRUISE;
	}

	/// Tape follows until reaching the first gap.
	void CRUISE_PLAT1() {
		while() {
			int temp = tapeFollow(PROPORTIONAL1,DERIVITIVE1,TAPEFOLLOW_GAIN1,TAPEFOLLOW_SPEED1);
			if(temp == ON_EDGE) {
				hardStop();
				state = DRAWBRIDGE;
				break;
			}
		}
	}

	/// Lowers drawbridge then backs up.
	//backing up will probably not use tape since inaccurate, maybe encoders?
	void DRAWBRIDGE() {
		lowerBridge();
		tapeFollowForDistance(-15);
	}

	// TODO: write
	// look for extrema in value?
	void EWOK_SEARCH() {
		while() {
			tapeFollow(16, 10, 10, PUT_SOME_STUFF_HERE);
			if(ewokDetect()) {
				setMotorPower(0,0);
				state = EWOK_GRAB;
				break;
			}
		}
	}

	/// Grabs ewok. 
	/// If fails, attemps twice more, once a little in front, once behind.
	void EWOK_GRAB() {
		int side;
		int stuffy;
		// Gets side and stuffy parameters
		if(nextEwok == 3) {
			side = LEFT;
			stuffy = EWOK;
		} else if(nextEwok == 5) {
			side = LEFT;
			stuffy = CHEWIE;
		} else {
			side = RIGHT;
			stuffy = EWOK;
		}

		// If don't get ewok, try again on either side
		int count = 1;
		while(count <= 3) {
			if(pickUp(side, stuffy)) {
				break;
			}
			if count == 1 {
            	tapeFollowForDistance(10);
			} else {
				tapeFollowForDistance(-10);
			}
			count++;
		}
		// Update ewok count
		nextEwok++;
	}

	/// Follows tape until IR signals are strong.
	void CRUISE_IR() {
		while(record10KIRBeacon() < IR_THRESHOLD && record1KIRBeacon < IR_THRESHOLD) {
			tapeFollow(PROPORTIONAL1, DERIVITIVE1, TAPEFOLLOW_GAIN1, TAPEFOLLOW_SPEED1);
		}
		stop();
	}
	// TODO: Write
	void IR_WAIT() {

	}
	//TODO: Write
	void CRUISE_PLAT2() {

	}

	// TODO: Write
	void DUMP_PREP() {

	}
	void DUMP_EWOKS() {
		dumpBasket();
	}
	// TODO: Write
	void ZIP_HOOK() {
		//need to find zipline, hook up
		//assume is started from already being on tape directly under line
		//likely need to do something other than turning
		turn(15);
		extendZipline();
		turn(-15);
		contractZipline();
	}
	//TODO: WRite
	void ZIP_UP() {
		zipUp();
		//zipUp function checks for contact switch and has a time limit 
	}
	// TODO: Write
	void ZIP_UNHOOK() {
		extendZipline();
		turn(10);
		contractZipline();
		turn(-10);
	}
	// TODO: Write
	void EWOK_4() {
		//may need to reverse a bit first
		EWOK_SEARCH();
		//may need to have something in here about edge detecting also, or should add to ewok search
	
	}
	// TODO: Write
	void BRIDGE_FOLLOW() {
		
		turn(90);

		// this assumes when they are on tape they read HI = true
		//follows side of bridge until sees tape indicating zipline
        while(!(digitalRead(TAPE_QRD_LEFT) && digitalRead(TAPE_QRD_RIGHT))){
			bridgeFollow();
		}

	}
	// TODO: WRite
	void CHEWIE() {
		//will end on tape so can follow tape for a little bit longer
		EWOK_SEARCH();
	}
	// TODO: Write
	void ZIP_DOWN() {
		turn(190);
		//turn around 180 degrees so goes down with sensor going outwards
		extendZipline();
		turn(-10);
		contractZipline();
		zipUp();
	}

#include "Robot.h"
#include "Menu.h"

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
}

void Robot::STARTUP() { 
	// while(!menu.quitMenu){
	// 	bool start = false;
	// 	bool stopp = false;
			
	// 	while(startbutton()){start = true;}
	// 	if(start) menu.handleInput(BTN_START);

	// 	while(stopbutton()){stopp = true;}
	// 	if(stopp) menu.handleInput(BTN_STOP);
	// }
	runState = RunState::CRUISE_PLAT1;
}

/// Tape follows until reaching the first gap.
void Robot::CRUISE_PLAT1() {
	while(true) {
		int temp = Funcs::tapeFollow(TF_KP1,TF_KD1,TF_GAIN1,Speed::SPEED);
		if(temp == ON_EDGE) {
			hardStop();
			runState = RunState::DRAWBRIDGE;
			break;
		}
	}
}

/// Lowers drawbridge then backs up.
//backing up will probably not use tape since inaccurate, maybe encoders?
void Robot::DRAWBRIDGE() {
	lowerBridge();
	tapeFollowForDistance(-15);
}

// TODO: write
// look for extrema in value?
void Robot::EWOK_SEARCH() {
	while(true) {
		tapeFollow(16, 10, 10, SPEED);
		if(ewokDetect()) {
			setMotorPower(0,0);
			runState = RunState::EWOK_GRAB;
			break;
		}
	}
}

/// Grabs ewok. 
/// If fails, attemps twice more, once a little in front, once behind.
void Robot::EWOK_GRAB() {
	int side;
	int stuffy;
	// Gets side and stuffy parameters
	if(nextEwok == 3) {
		side = LEFT;
		stuffy = configs::EWOK;
	} else if(nextEwok == 5) {
		side = LEFT;
		stuffy = configs::CHEWIE;
	} else {
		side = RIGHT;
		stuffy = configs::EWOK;
	}

	// If don't get ewok, try again on either side
	int count = 1;
	while(count <= 3) {
		if(pickUp(side, stuffy)) {
			break;
		}
		if(count == 1) {
			tapeFollowForDistance(10);
		} else {
			tapeFollowForDistance(-10);
		}
		count++;
	}
	// Update ewok count
	nextEwok++;
}

/// Follows tape until IR signals are strong.
void Robot::CRUISE_IR() {
	while(record10KIRBeacon() < IR_THRESHOLD && record1KIRBeacon() < IR_THRESHOLD) {
		tapeFollow(TF_KP1, TF_KD1, TF_GAIN1, SPEED);
	}
	hardStop();
}
// TODO: Write
void Robot::IR_WAIT() {

}

<<<<<<< HEAD
    }    void DUMP_EWOKS() {
        dumpBasket();
    }
    // TODO: Write
    void ZIP_HOOK() {
        //need to find zipline, hook up
        //assume is started from already being on tape directly under line
        //likely need to do something other than turning
        turn(15);
        extendZipline();
        turn(-15);
        contractZipline();
    }
    //TODO: WRite
    void ZIP_UP() {
        zipUp();
        //zipUp function checks for contact switch and has a time limit
    }
    // TODO: Write
    void ZIP_UNHOOK() {
        extendZipline();
        turn(10);
        contractZipline();
        turn(-10);
    }
    // TODO: Write
    void EWOK_4() {
        //may need to reverse a bit first
        EWOK_SEARCH();
        //may need to have something in here about edge detecting also, or should add to ewok search

    }
    // TODO: Write
    void BRIDGE_FOLLOW() {

        turn(90);

        // this assumes when they are on tape they read HI = true
        //follows side of bridge until sees tape indicating zipline
        while(!(digitalRead(TAPE_QRD_LEFT) && digitalRead(TAPE_QRD_RIGHT))){
            bridgeFollow();
        }

    }
    // TODO: WRite
    void CHEWIE() {
        //will end on tape so can follow tape for a little bit longer
        EWOK_SEARCH();
    }
    // TODO: Write
    void ZIP_DOWN() {
        turn(190);
        //turn around 180 degrees so goes down with sensor going outwards
        extendZipline();
        turn(-10);
        contractZipline();
        zipUp();
    }
	void cruisePlat2_1() {
=======
// TODO: Write
void Robot::DUMP_PREP() {

}
void Robot::DUMP_EWOKS() {
	dumpBasket();
}
// TODO: Write
void Robot::ZIP_HOOK() {

}
//TODO: WRite
void Robot::ZIP_UP() {

}
// TODO: Write
void Robot::ZIP_UNHOOK() {

}
// TODO: Write
void Robot::EWOK_4() {

}
// TODO: Write
void Robot::BRIDGE_FOLLOW() {

}
// TODO: WRite
void Robot::CHEWIE() {

}
// TODO: Write
void Robot::ZIP_DOWN() {

}

void Robot::CRUISE_PLAT2() {

	//just dropped bridge

	uint16_t initialDist = (leftWheelIndex + rightWheelIndex) * cmPerWheelIndex / 2 ;

	while (((leftWheelIndex + rightWheelIndex) * cmPerWheelIndex / 2 - initialDist) < 69) {
		tapeFollow(TF_KP1,TF_KD1,TF_GAIN1,Speed::SPEED);
	}

	runState = RunState::EWOK_4;
}

void Robot::IRHANDLE() {

	//move until IR signal is strong?

	while (record10KIRBeacon() < record1KIRBeacon()) {
	}

	runState = RunState::CRUISE_PLAT1;

}
