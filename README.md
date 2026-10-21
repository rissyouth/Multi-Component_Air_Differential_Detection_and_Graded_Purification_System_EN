# Multi-Component Air Differential Detection and Graded Purification System
# 多组分空气差分检测与分级净化装置

> 🚧 **Status:** Patent Pending (Dual-Filed / 双报申请中)
> * **Invention Patent App No. (发明专利):** 202611542863.0[cite: 26]
> * **Utility Model App No. (实用新型):** 202621553759.7[cite: 27]
> * **Date of Filing:** Oct 1, 2026
## 🌟 项目概览 (Project Overview)
本项目针对现代工业与复杂室内环境，设计了一套具备**动态流体控制**与**靶向分级净化**的开源空气净化装置原型。系统采用前/后双路传感器进行差分闭环检测，并在流道中创新性地集成了**可调式特斯拉限速阀**。

在遭遇严重污染突发（如高浓度 CO 或 PM2.5 飙升）时，主控微处理器将基于流体连续性方程（$Q=v \cdot A$）伺服控制特斯拉阀扩大横截面积并切入挡板，强行延长污染气体在反应腔室内的接触滞留时间，以实现高效率的彻底净化与节能复位。

## ⚙️ 核心技术创新 (Core Innovations)
* **动态特斯拉阀调流 (Dynamic Tesla Valve):** 采用直线推杆电机与微型舵机，在保持总进气量不减的前提下，动态调整气流通道夹角与流阻。
* **多级靶向净化 (Targeted Purification):** 高温加热 ACO-2 催化分解一氧化碳 + 氢氧化钠改性活性炭中和吸收二氧化硫及粉尘。
* **差分闭环效能检测 (Differential Closed-loop):** 进/出风口双检测仓实时比对，闭环核算净化效率，支持突发污染的微分（斜率）极速预警。
* **智能控温与节能 (Smart Energy Saving):** 基于污染浓度的 PWM 动态加热控温机制，达标后自动缩回特斯拉阀并关闭外围机电进入深度休眠。

## 📂 仓库结构 (Repository Structure)
* `/hardware`: 硬件物料清单（BOM）与机械结构设计参考。
* `/firmware`: 基于 Arduino 架构的传感器数据采集、状态机控制与特斯拉阀机电协同源码。

## 🛠️ 技术栈 (Tech Stack)
* **硬件:** Arduino Mega 2560, I2C, UART, PWM Motor Control
* **软件:** C++ (Arduino IDE)
