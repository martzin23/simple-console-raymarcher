# Simple console raymarcher

## Description

Simple software ray-marching renderer that prints an image into the console in an ASCII-character style. This is my first program that can render a 3D scene. I made it for fun and to learn ray marching. Created in November 2023.

## Usage

1. Compile the code
2. Lower the text size in the terminal (so that the result fits on the screen)
3. Run it

- Configuring the scene is done by editing `struct bodyData bodies[] = { ... }`, which contains a list of bodies and their properties. 
- Additionally, you can edit macro values to change some parameters (like resolution, fov, etc.).
- Only works for windows, because it uses `windows.h` to color the text.

## Screenshots

![output_2](/screenshots/output_2.png)

![output_4](/screenshots/output_4.png)
