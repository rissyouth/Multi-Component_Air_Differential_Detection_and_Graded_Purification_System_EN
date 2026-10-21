# Multi-Component_Air_Differential_Detection_and_Graded_Purification_System_EN
# Multi-Component Air Adaptive Differential Detection and Graded Purification System

[English](./README.md) | [简体中文](./README_ZH.md)
> 🚧 **Status:** Patent Pending (Dual-Filed) | **Author:** Yiming Wu (rissyouth)
> * **Invention Patent App No.:** 202611542863.0
> * **Utility Model App No.:** 202621553759.7
> * **Date of Filing:** Oct 1, 2026

## 🌟 Project Overview
Targeting complex industrial and indoor environments, this project presents an open-source air purification prototype featuring **dynamic fluid control** and **targeted graded purification**. The system employs a dual-sensor array (front/rear) for differential closed-loop detection and innovatively integrates an **adjustable Tesla valve** in the fluid channel. 

In the event of severe sudden pollution (e.g., soaring CO or PM2.5), the MCU utilizes the fluid continuity equation ($Q=v \cdot A$) to servo-control the Tesla valve, expanding the cross-sectional area and engaging the baffle. This forces an extension of the residence time of polluted gases in the reaction chambers, achieving highly efficient, thorough purification and intelligent energy-saving resets.

## ⚙️ Core Innovations
* **Dynamic Tesla Valve Flow Control:** Utilizes a linear actuator and a micro servo to dynamically adjust the airflow channel angle and flow resistance without reducing the total intake volume.
* **Multi-Stage Targeted Purification:** High-temperature ACO-2 catalytic decomposition for CO + NaOH-modified activated carbon for neutralizing SO2 and trapping PM2.5.
* **Differential Closed-Loop Efficacy Detection:** Real-time comparison between inlet/outlet detection chambers to calculate purification efficiency in a closed loop, supporting ultra-fast differential (slope) early warnings for sudden pollution.
* **Smart Temperature Control & Energy Saving:** PWM dynamic heating mechanism based on pollution concentration. Automatically retracts the Tesla valve and shuts down peripheral electromechanical modules to enter deep sleep mode once standards are met.

## 📂 Repository Structure
* `/hardware`: Bill of Materials (BOM) and mechanical structure design references.
* `/firmware`: Arduino-based source code for sensor data acquisition, state machine control, and Tesla valve electromechanical synergy.

## 🛠️ Tech Stack
* **Hardware:** Arduino Mega 2560, I2C, UART, PWM Motor Control
* **Software:** C++ (Arduino IDE)
* **Mechanical:** Fusion 360 / Blender (3D Modeling)
