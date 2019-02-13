#include "PCA9685.h"
#include <SDL2/SDL.h>

class Car
{
 private:
    PCA9685 *driver;
    SDL_Joystick *joy;
 public:
    // Parameterized constructor for the car class
    // Inputs:
    // driver - pointer to PCA9685 driver
    // joy - pointer to SDL_Joystick
    Car(PCA9685 *driver, SDL_Joystick *joy);

    // Read value from joystick
    int readJoystick();

    // Set the steering angle on the steering servo
    void setSteeringAngle(int value);

    // Set the car's throttle
    void setCarThrottle(int value);

	// Convienence range method	
	int to_range(int val, int val_min, int val_max, int new_min, int new_max);

};
