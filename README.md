Excited warfare on the seas of a Kepler IV.

The time has come to conquer Kepler IV.

Compiling and Installation
--------------------------

'Requirements'
* ODE: Open Dynamics Engine, version 0.13: http://sourceforge.net/projects/openode/
* Please follow the guidelines in http://ode-wiki.org/wiki/index.php?title=Manual:_Install_and_Use to install ODE on your Macbook.


 make
 ./waku

That's all folks.



Issues and missing stuff
------------------------

* Floor is working greatly!  Sea is good.  Sky can be improved.
* Collision detection is working:  no friction.
* Objects modelling:  Spheres work but they are hard to control the rotational movement.  Need improvement!
* Island generation: Ok from heightmaps but has some issues with height calculation.  If the island is plain works great.
*
* Walrus mechanics works but can be improved.  The MD2 model is good but can be improved.
* Manta mechanics (based on Microsoft Flight Simulator) works barely.  It is hard to control.
* UserControl can be improved.  Sensitivity must be adjusted from within the game.
