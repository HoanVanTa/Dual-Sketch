
# ESP32 Dual Sketch Demo (OTA Partitions)

This project demonstrates how to store **two independent Arduino sketches on a single ESP32** using the ESP32 OTA partition scheme.

Instead of using OTA for remote firmware updates, this project shows how OTA partitions can be used to keep two separate applications in flash memory and switch between them under software control.

## Features

* Two independent ESP32 applications (Sketch A and Sketch B)
* ESP32 OTA partition layout
* Software-controlled switching between applications
* Shared data using NVS (Non-Volatile Storage)
* Explanation of the bootloader, partition table, and OTA Data partition
* Simple and well-commented source code for learning purposes

## What You'll Learn

* How ESP32 flash memory is organized
* The purpose of the Bootloader
* How the Partition Table defines flash memory
* The role of the OTA Data partition
* How the bootloader decides which application to start
* How two applications can exchange persistent data using NVS
* Flash memory and partition size limitations

## Repository Structure

* **SketchA/** — First application
* **SketchB/** — Second application
* **partitions.csv** — Custom partition table

This project is intended as an educational example for anyone interested in ESP32 internals, OTA, embedded systems, and firmware architecture.
