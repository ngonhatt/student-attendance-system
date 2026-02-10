# ⚙️ STM32 Bare-metal Projects

## 📌 Overview
This repository contains bare-metal firmware projects developed for STM32
microcontrollers without using HAL or STM32CubeIDE.
The focus is on low-level register programming and understanding
microcontroller hardware behavior.

---

## 🔧 Hardware
- STM32F411
- DHT11 Temperature & Humidity Sensor
- LCD 16x2 (I2C)

---

## ⚙️ Implemented Features
- Clock configuration using registers
- GPIO configuration (input/output)
- I2C communication for LCD
- DHT11 temperature and humidity reading
- Displaying sensor data on LCD

---

## 🛠️ Software & Tools
- C Programming
- arm-none-eabi-gcc
- Makefile
- MSYS2
- STM32 Reference Manual & Datasheet

---

## 🧠 Project Structure
```text
stm32-baremetal/
├── src/
├── inc/
├── Makefile
└── README.md
