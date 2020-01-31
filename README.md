## Introduction

There are a lot of GPIO libraries for the Raspberry Pi out there. However I was not able to find a decent library, which allows me to read, set and clear pins via direct register access (should be the fastest possible way) and set up file descriptors to wait for an interrupt to happen.

This library combines both options. Regular GPIO access is realized by writing or reading from the registers directly and interrupts are set up via sysfs. 

It is tested with c++11 and c++14 on newer Raspberry Pi versions (3, 3B+ and 4) all of them running a recent Raspbian. In older Raspbian version you will probably not have the opportunity to access the gpio memory without root rights (i.e. /dev/gpiomem does not exist). 

## Usage

First install git and clone this repository. 

```
git clone https://github.com/Teawolf-beep/RPi_gpio_cpp.git
```

Now you are ready to go. The whole code is documented with doxygen, for a starting point open the "index.html" file inside the "html" folder with any browser. You should be able to check out each method as well as constructor and destructor.

The constructor tries to access and map the gpio memory of the Raspberry Pi. In case anything goes wrong, a GPIOException is thrown. It expects as argument the path to the gpio file, where you can choose between the regular and a non root versions (check GPIO.h for possible options). 

Check "example.cpp" (or see below) for a simple example on how to use an input, an output and an interrupt. For further possibilities check the documentation.

### Example (example.cpp)
```cpp
/****************************************************************
 * An example on how to use the GPIO library.
 * This example does not handle any interrupts thrown by the GPIO constructor!
 ****************************************************************/

// System includes
#include <poll.h>
#include <cstring>
#include <unistd.h>

// Project includes
#include "GPIO.h"

// BCM pin numbers used in this example
#define INPUT 23
#define OUTPUT 24
#define INTERRUPT 25

int main(int argc, char** argv)
{
    // Call the constructor with the file path defined in GPIO.h ("/dev/gpiomem")
    GPIO gpio = GPIO(GPIO_USER_FILENAME);
    struct pollfd pfd;
    char buffer[8];
    int fd;

    // Make the input pin an input. Returns true, if everything is ok. Otherwise an error occured
    if (!gpio.makeInput(INPUT)) {
        printf("Pin %d could not be made an input!\n", INPUT);
        return -1;
    }

    // Make the output pin an output. Returns true, if everything is ok. Otherwise an error occurred
    if (!gpio.makeOutput(OUTPUT)) {
        printf("Pin %d could not be made an output!\n", OUTPUT);
        return -1;
    }

    // Set up the interrupt pin with the use of an internal pull down resistor and rising edge activation
    fd = gpio.setSingleKernelDriver(INTERRUPT, PullState::DOWN, Edge::RISING);
    // Less than zero, if an error occurred. Otherwise the return value is the file descriptor we have to monitor
    if (fd < 0) {
        printf("Could not set up an interrupt for pin %d\n", INTERRUPT);
        return -1;
    }

    // Assign the file descriptor to the pollfd struct
    pfd.fd = fd;
    // Assign the input event of interest to the pollfd struct
    pfd.events = POLLPRI;

    // Wait for an interrupt to happen without time limit (third parameter). This thread will consume no unnecessary CPU power while inside poll()
    if (poll(&pfd, 1, -1) == -1) {
        printf("Error while poll(): %s\n", strerror(errno));
        return -1;
    }
    // True if an interrupt happened
    else if (pfd.revents & POLLPRI) {
        // Consume the interrupt (we are not really interested in the return value of read)
        lseek(pfd.fd, 0, SEEK_SET);
        read(pfd.fd, &buffer, sizeof(buffer));

        // Perform here any action you want to do, when the interrupt has happened!
        // The readPin() method will return true, if the passed pin is high, else it will return false
        printf("Interrupt occurred on pin %d! The value of the input pin is %s!\n", INTERRUPT, (gpio.readPin(INPUT)) ? "high" : "low");

        // Toggle the output pin
        if (gpio.readPin(OUTPUT)) {
            gpio.clearPin(OUTPUT);
        }
        else {
            gpio.setPin(OUTPUT);
        }
    }
    return 0;
}
```

## Further reading

No warranty on anything you use the library for. I do not think that it works for all SoC's used in Raspberry Pis and hope nobody crashes its system while using it. Well, that's all...have fun!

This library combines different things I found on the interent. Handling with sysfs is described [here](https://elinux.org/RPi_GPIO_Code_Samples#sysfs) and the register addresses etc do I have from [here](https://www.raspberrypi.org/app/uploads/2012/02/BCM2835-ARM-Peripherals.pdf)

