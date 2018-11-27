# Fulgurous #

This is an experiment with C++14/17 to create an API which makes working with Modern OpenGL much more easy and painless.

Fast iteration is the goal, not performance.

This is an attempt to boil down a modern renderer to the absolute minimum, whith the following objects

* Context
* Shader Program
* Framebuffer
* Geometry
* Draw Call (shader + geometry)

Inspired by Randy Gaul's cutegl  (in turn inspired by ThatGameCompany's internal proptyping code)