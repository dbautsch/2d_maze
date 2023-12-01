# Introduction

This project has been created roughly around 2010 by myself as an coding excercise. Because it was an early C++ learning stage for me, don't expect super high quality  code. Nevertheless I have decided to publish the source code and I plan to modernize it, migrate to more modern C++ standard.

# Game details

The game is a 2D board where you move your "player" using left/right/up/down arrows to find an exit from maze.

# Known bugs

There is one known bug, which is in the maze generator. It does not generate alternate paths, thus there is only one correct path rendered making the whole maze game stupid simple.
The plan is to rewrite maze generator from scratch.

# Technical details

The game uses very simple WinBGIm library from https://winbgim.codecutter.org/ including several compatibility fixes made by myself (more to come).

Originally game was written in CodeBlocks and built with MinGW 4.4 and C++03 standard (kind of, mixed with C).
