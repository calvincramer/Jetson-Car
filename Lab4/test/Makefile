# Don't print the commands unless explicitely requested with `make V=1`
ifneq ($(V),1)
	Q = @
	V = 0
endif

# Current directory
CUR_PWD := $(shell pwd)

Car.x:
	@echo "Making Car.x"
	$(Q)g++ -o Car.x Car.cpp PCA9685.cpp -lSDL2


clean:
	@echo "CLEAN	$(CUR_PWD)"
	$(Q)rm -rf *.x
