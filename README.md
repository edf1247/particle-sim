## README

Particle Simulation in C

## Summary

I implemented a basic particle simulation in C, using raylib. The physics simulation is done using verlet velocity integration, with the following formulas:
- $x(t + \Delta{t}) = x(t) + v(t)\Delta(t) + \frac{1}{2}a(t)\Delta(t)^2$
- $v(t + \Delta{t}) = v(t) + \frac{(a(t) + a(t + \Delta_t))\Delta(t)}{2}$
- In this sim, acceleration remained constant for each particle, so the new velocity calculation can be simplified to just $v(t) + (a(t))\Delta(t)$

To have more interesting behavior, on initialization each particle is assigned a random acceleration.
