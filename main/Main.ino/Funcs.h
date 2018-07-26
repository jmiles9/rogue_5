//Functions for use in robot.
#include <phys253.h> //this is for the uints
#include "configs.h"

#ifndef FUNCS_H
#define FUNCS_H

class Funcs {
    public:
        uint16_t leftWheelIndex;
        uint16_t rightWheelIndex;
        uint32_t leftWheelLastTime;
        uint32_t rightWheelLastTime;
        uint8_t leftSpeed;
        uint8_t rightSpeed;
        uint8_t nextEwok;
        uint8_t error;

        void setMotorPower(int left, int right);
        void steer(int deg);
        void hardStop();
        bool tapeFollow(int kp, int kd, int gain, configs::Speed speed);
        void tapeFollowForDistance(int distance);
        bool pickUp(int side, int stuffy);
        double record1KIRBeacon();
        double record10KIRBeacon();
        void lowerBridge();
        bool checkBeacon();
        bool ewokDetect();
        void turn(int deg);
        void move(int distance);
        void moveWheels(int leftDistance, int rightDistance, int speed);
        int speedToPower(int speed);
        void dumpBasket();
        void extendZipline();
        void contractZipline();
        void zipUp();
        void moveZipline(int distance);
        void zipppp();
        void findEdge();
        void bridgeFollow(int kp, int kd, int gain);
        bool isOnEdge();
        double distanceTravelled(int newIndex, int oldIndex);
        void tapeFollowToEdge();
};

#endif