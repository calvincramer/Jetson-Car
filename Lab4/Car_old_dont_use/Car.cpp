#include <fstream>
#include <iostream>
#include "Car.h"
#include "PCA9685.h"
#include <SDL2/SDL.h>

#define STEERING_CHANNEL 0
#define ESC_CHANNEL 1
#define THROTTLE_MIN_FORWARD 319
//#define THROTTLE_MAX_FORWARD 365
#define THROTTLE_MAX_FORWARD 345
#define THROTTLE_MIN_REVERSE 290
#define THROTTLE_MAX_REVERSE 238
#define THROTTLE_NEUTRAL 307
#define STEERING_MAX_RIGHT 355
#define STEERING_MAX_LEFT 205
#define STEERING_NEUTRAL 284
#define SHORT_MAX 32767
#define SHORT_MIN -32768
#define DEAD_ZONE 1000

int Car::to_range(int val, int val_min, int val_max, int new_min, int new_max)
{
	double n = (val - val_min) * 1.0 / (val_min + val_max);
	n *= new_min + new_max;
	n += new_min;
	return (int) n;
}

Car::Car(PCA9685 *driver, SDL_Joystick *joy)
{
	this->driver = driver;
  	this->joy = joy;
}

int Car::readJoystick(SDL_Event event)
{
	int quit = 1;
  	switch(event.type) {
		case SDL_JOYAXISMOTION:
			if (event.jaxis.axis == 0) {
				setSteeringAngle(event.jaxis.value);
			}
			else if(event.jaxis.axis == 4) {
				setCarThrottle(event.jaxis.value);
			}
			break;
		case SDL_QUIT:
			std::cout << "quitting\n";
			quit = 0;
			break;
		//default:
		//	std::cout << "Cannot handle event: " << std::endl;
		//	break;
	}
	//std::cout << "HELLO: " << quit << std::endl;
	return quit;
}

void Car::setSteeringAngle(int value)
{
	if (!driver)
		return;

	// Dead zone
	if (abs(value) < DEAD_ZONE)
		value = 0;

	int pca_value = to_range(value, SHORT_MIN, SHORT_MAX, STEERING_MAX_LEFT, STEERING_MAX_RIGHT);
	
	// Check pca value
	std::cout << "PCA Value for steering: " << pca_value << std::endl;
	if (pca_value < STEERING_MAX_LEFT || pca_value > STEERING_MAX_RIGHT) {
		std::cerr << "INVALID PCA VALUE: " << pca_value << std::endl;
		return;
	}

	driver->setPWM(STEERING_CHANNEL, 0, pca_value);
}

void Car::setCarThrottle(int value)
{
	if (!driver)
		return;
	
	// Dead zone
	if (abs(value) < DEAD_ZONE)
		value = 0;

	int pca_value = to_range(value, SHORT_MIN, SHORT_MAX, THROTTLE_MAX_REVERSE, THROTTLE_MAX_FORWARD);
	
	// Check pca value
	std::cout << "PCA Value for throttle: " << pca_value << std::endl;
	if (pca_value < THROTTLE_MAX_REVERSE || pca_value > THROTTLE_MAX_FORWARD) {
		std::cerr << "INVALID PCA VALUE: " << pca_value << std::endl;
		return;
	}

	driver->setPWM(STEERING_CHANNEL, 0, pca_value);
}

int main(int argc, char** argv) {
	// Initialize SDL, this makes the joystick available
	
	// Error
	//SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");

	if(SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO 
              | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC) < 0) {
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		return 1;
    }
	
	// Showing available joysticks:
	std::cout << "Number of joysticks available: " << SDL_NumJoysticks() << std::endl;

	// Initialize the joystick controller
	SDL_Joystick* joy;
	int joys_num = 0;
	while (joys_num <= 100000) {
		joy = SDL_JoystickOpen(joys_num);
		if (joy) break;
		joys_num++;
	}
	if (!joy) {
		std::cerr << "COULD NOT FIND ANY JOYSTICKS" << std::endl;
		return 1;
	}
	std::cout << "USING JOYSTICK NUMBER " << joys_num << std::endl;

	// PCA init  	
	PCA9685* pca = new PCA9685();
	int err = pca->openPCA9685();
	if (err < 0) {
		std::cout << "no PCA9685\n";
		return 1;
	}
	std::cout << "Setting PCA frequency\n" << std::flush;
 	pca->reset();
	return -1;
	pca->setPWMFrequency(50);

	std::cout << "Setting PCA PWM to 0\n" << std::flush;
	pca->setAllPWM(0, 0);

	std::cout << "Setting PCA STEERING\n" << std::flush;
	pca->setPWM(STEERING_CHANNEL, 0, STEERING_NEUTRAL);

	

	std::cout << "Setting PCA THROTTLE\n" << std::flush;
	pca->setPWM(ESC_CHANNEL, 0, THROTTLE_NEUTRAL);

	// Initialize car object
	Car* car = new Car(pca, joy);
	
	// Main loop to read input from controller and sent output to car
	std::cerr << "STARTING MAIN LOOP" << std::endl;
	SDL_Event event;
	while(SDL_PollEvent(&event)) {
		SDL_Delay(10);
    		car->readJoystick(event);
  	}
	return 0;
}

