# Muscle App

[![License: CC BY-NC 4.0](https://img.shields.io/badge/License-CC%20BY--NC%204.0-lightgrey.svg)](https://creativecommons.org/licenses/by-nc/4.0/)
[![Language: C++](https://img.shields.io/badge/Language-C%2B%2B17-blue.svg)](https://en.cppreference.com/)
[![GUI: Dear ImGui](https://img.shields.io/badge/GUI-Dear%20ImGui-orange.svg)](https://github.com/ocornut/imgui)

This is a simple application designed for the scientific analysis and construction of the torque-angle / force-length relationship.
The primary use was to define quickly conditions in an empirical way (e.g. stretch or shortening the muscle from/to an angle/length corresponding to 70% of peak force).

---

## ⚖️ License & Terms of Use

This project is licensed under the **Creative Commons Attribution-NonCommercial 4.0 International (CC BY-NC 4.0)** license.

*   **Non-Commercial Use:** You are free to copy, redistribute, remix, transform, and build upon the material for non-commercial purposes, provided you give appropriate credit/attribution to the author.
*   **Commercial Use:** Any commercial utilization, integration, or deployment of this software requires prior written permission and licensing from the author.

---

## Usage

The app is avaiable both for Windows and MacOS (intel). You can also build for linux, by simply importing this repo into CLion and build.
The app opens with exemplary data, and further there is an example .csv file in case you want to first write down numbers in a separated file.
The usage should be quite straightfoward, where base on the point, you can adjust the fitting (grade) and it returns in live the R^2 and RMSE.

You can then adjust in the bottom left panel the condition: match by a given angle or return the angle/length corresponding to a given percentage of normalized force/torque.

Have fun using it!