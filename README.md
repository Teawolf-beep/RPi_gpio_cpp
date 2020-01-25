# RPi_gpio_cpp
A C++ GPIO library for newer Raspberry Pi models (supports direct register access and kernel interrupts)

## Introduction

There are a lot of %GPIO libraries for the Raspberry Pi out there. However I was not able to find a decent library, which allows me to read, set and clear pins via direct register access (should be the fastest possible way) and poll on file descriptor to wait for an interrupt to happen.

This library combines both features. 