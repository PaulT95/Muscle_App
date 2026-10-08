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

## Installation & usage 

The app is distributed as pre-compiled binaries for Windows and macOS that you can download [here](https://github.com/PaulT95/Muscle_App/releases), so no complex build setup is required—just download and run. (Linux users can easily build it by importing the repository into CLion).

The app opens with exemplary data, and further there is an example .csv file in case you want to first write down numbers in a separated file. However, you can directly insert the angle/length you tested directly into the app. It needs angle/length, rest value and peak value (it doesn't matter whether positive or negative), it calculates automatically the peak to peak in the last column (i.e., active torque/force generated).
The usage should be quite straightforward, where based on the points you insert, you can adjust the fitting (grade) and it returns in live the R^2 and RMSE.

You can then adjust in the bottom left panel the condition: match by a given angle or return the angle/length corresponding to a given percentage of normalized force/torque.

Have fun using it!

## Example

<img width="800" height="450" alt="example" src="https://github.com/user-attachments/assets/87370fd7-8e26-4296-b6f9-344d13a6f9c7" />

