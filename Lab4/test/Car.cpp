//NEED SUDO TO RUN EXECUTABLE
//WE ARE USING BUS 1 For the PCA
#include <fstream>
#include <iostream>
#include "Car.h"
#include "PCA9685.h"
#include <SDL2/SDL.h>


//LOWERED VALUES FOR DEBUGGING
#define STEERING_CHANNEL 0
#define ESC_CHANNEL 1
#define THROTTLE_MIN_FORWARD 319
//#define THROTTLE_MAX_FORWARD 365
#define THROTTLE_MAX_FORWARD 345
#define THROTTLE_MIN_REVERSE 290
//#define THROTTLE_MAX_REVERSE 238
#define THROTTLE_MAX_REVERSE 260
#define THROTTLE_NEUTRAL 307
//#define STEERING_MAX_RIGHT 355
#define STEERING_MAX_RIGHT 355
//#define STEERING_MAX_LEFT 205
#define STEERING_MAX_LEFT 205
#define STEERING_NEUTRAL 284
#define SHORT_MAX 32767
#define SHORT_MIN -32768
#define DEAD_ZONE 1000	// Applied in the range [SHORT_MIN, SHORT_MAX]

int Car::to_range(int val, int val_min, int val_max, int new_min, int new_max)
{
	// Tested and works
	double n = (val - val_min) * 1.0 / (val_max - val_min);
	n *= (new_max - new_min);
	n += new_min;
	//std::cout << "to range return value: " << n << std::endl;
	return (int) n;
}

Car::Car(PCA9685 *driver, SDL_Joystick *joy)
{
	this->driver = driver;
  	this->joy = joy;
}

int Car::readJoystick()
{
// fixed code to look more like repo
	SDL_Delay(10);	// In ms
	bool new_steering_event = false;
	bool new_motor_event = false;
	int steering = 0;
	int motor = 0;
	int quit = 0;
	SDL_Event event;
	while (SDL_PollEvent(&event)){
  		switch(event.type) {
			case SDL_JOYAXISMOTION:
				if (event.jaxis.axis == 0) {	// STEERING EVENT
					new_steering_event = true;
					steering = event.jaxis.value;
				}
				if (event.jaxis.axis == 5) {	// MOTOR EVENT
					new_motor_event = true;
					motor = -1 * event.jaxis.value;
				}
				break;
			case SDL_QUIT:
				std::cout << "quitting\n";
				quit = 0;
				break;
			default:	 // Do nothing about other events
				break;
		}
	}
	if (new_steering_event) {
		setSteeringAngle(steering);
		new_steering_event = false;
	}
	if (new_motor_event) {
		setCarThrottle(motor);
		new_motor_event = false;
	}


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
	//std::cout << "PCA Value for steering: " << pca_value << std::endl;
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
	//std::cout << "PCA Value for throttle: " << pca_value << std::endl;
	if (pca_value < THROTTLE_MAX_REVERSE || pca_value > THROTTLE_MAX_FORWARD) {
		std::cerr << "INVALID PCA VALUE: " << pca_value << std::endl;
		return;
	}

	driver->setPWM(ESC_CHANNEL, 0, pca_value);
}

int main(int argc, char** argv) {
	// Initialize SDL, this makes the joystick available
	SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");

	if(SDL_Init(SDL_INIT_JOYSTICK) < 0) {
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
	
 	printf("PCA9685 Device Address: 0x%02X\n",pca->kI2CAddress) ;
	std::cout << "BUS NUMBER: " << (int) pca->kI2CBus << std::endl;
	
	std::cout << "Setting PCA PWM to 0\n" << std::flush;
	pca->setAllPWM(0, 0);

	pca->reset();
	
	std::cout << "Setting PCA frequency\n" << std::flush;
	pca->setPWMFrequency(50);
	//std::cout << "after setPWMfrequency" << std::endl;

	std::cout << "Setting PCA THROTTLE\n" << std::flush;
	pca->setPWM(ESC_CHANNEL, 0, THROTTLE_NEUTRAL);

	std::cout << "Setting PCA STEERING\n" << std::flush;
	pca->setPWM(STEERING_CHANNEL, 0, STEERING_NEUTRAL);

	// Initialize car object
	Car* car = new Car(pca, joy);
	
	// Main loop to read input from controller and sent output to car
	std::cerr << "STARTING MAIN LOOP" << std::endl;
	SDL_Event event;
	//int error = SDL_PollEvent(&event);
	//std::cout << "error: " << error << std::endl;
	int quit = 0;

	//NEEDED TWO WHILE LOOPS, INNER ONE JUST FOR POOLING
	while(!car->readJoystick());

	return 0;
}

