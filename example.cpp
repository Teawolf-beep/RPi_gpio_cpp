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
        // The readPin() method will return true, if the passed pin is high, else it will return low
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
