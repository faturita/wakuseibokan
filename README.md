# 太陽系外惑星航空母艦

Excited warfare on the seas of a Kepler IV.

The time has come to conquer Kepler IV.  Humanity is now able to travel further away from our solar system, and it has now reached exoplanets.  Two companies sent surveying parties in the form of two AI carriers that aim to control the vast archipielago of Kepler IV. These AIs can be controlled remotely in real-time by space folding uplinks.


Check [video](https://www.youtube.com/watch?v=_LJj1x4orbU). 

![Manta taking off](images/screenshot1.png)

Compiling and Installation
--------------------------

'Requirements'
* ODE: Open Dynamics Engine, version 0.14: https://bitbucket.org/odedevs/ode
* Please this README or the guidelines in http://ode-wiki.org/wiki/index.php?title=Manual:_Install_and_Use to install ODE on your Macbook.

# ODE
```bash
./bootstrap
./configure --disable-asserts
make
make install
```

# Ubuntu Packages

 libbsd-dev  freeglut3-dev

# Compiling

 make
 ./waku

That's all folks.


ODE 0.14 Compilation on Mac Sierra
----------------------------------

* Run the following script to install automake tools for Mac

```bash
#!/bin/sh

##
# Install autoconf, automake and libtool smoothly on Mac OS X.
# Newer versions of these libraries are available and may work better on OS X
#
# This script is originally from http://jsdelfino.blogspot.com.au/2012/08/autoconf-and-automake-on-mac-os-x.html
#

export build=~/devtools # or wherever you'd like to build
mkdir -p $build

##
# Autoconf
# http://ftpmirror.gnu.org/autoconf

cd $build
curl -OL http://ftpmirror.gnu.org/autoconf/autoconf-2.69.tar.gz
tar xzf autoconf-2.69.tar.gz
cd autoconf-2.69
./configure --prefix=/usr/local
make
sudo make install
export PATH=$PATH:/usr/local/bin

##
# Automake
# http://ftpmirror.gnu.org/automake

cd $build
curl -OL http://ftpmirror.gnu.org/automake/automake-1.15.tar.gz
tar xzf automake-1.15.tar.gz
cd automake-1.15
./configure --prefix=/usr/local
make
sudo make install

##
# Libtool
# http://ftpmirror.gnu.org/libtool

cd $build
curl -OL http://ftpmirror.gnu.org/libtool/libtool-2.4.6.tar.gz
tar xzf libtool-2.4.6.tar.gz
cd libtool-2.4.6
./configure --prefix=/usr/local
make
sudo make install

echo "Installation complete."
```

* Now download the ode-0.14.tar tarball.
* Modify LIBTOOLIZE variables from 'glibtoolize' to 'libtoolize'
* ./bootstrap
* ./configure
* make clean && make && sudo make install


Characteristics
---------------
* Floor
    * The floor is squared.  Horizon works according the fulcrum representation.
    * Sea is good.  Works as a texture that is shifted based on the camera position.
    * Reflections are missing.
    * Waves and boat wakes are missing.  
* Sky
    * The Sky is a far-away-box with textures on the insides.
    * It works well, but the representation relies on the camera view and there are issues with how they look like from certain viewports.
* Islands
    * The engine is calculating the islands perfectly.  What you see is what you feel.
    * Islands are BMP heightmaps 60x60.  These values are hardcoded to 3600x3600 in the model.
    * Max height (255) is 60. Hardcoded.
    * Island floor, which can be less than zero, or greater than zero, need to be adjusted manually for each island. Need improvement->Some Z Fighting.
* Collision detection
    * Island heights works PERFECT for Walruses. 
* Models
    * MDModel: Unreal.
    * 3DSModel: it works only when you have just one object and without textures.
* Walruses
    * Two models.  They work very nicely on water.  The thrust simulates and offboard engine.
* Sound
    * Using TEMPORARILY system calls and afplay.
    * STK
* Manta
    * The FDM model from Flight Simulator works but it is very difficult to really control it. And there are many parameters.
    * Current implementation is a new mixed model which handle the forces form the dynamic model but keep the airplane pose based on a simplified model (not dynamic).  I will move from here which is very good for control (and automation).
* UserControl can be improved.  Sensitivity must be adjusted from within the game.

Blender
-------

* Shift+Ctrl+Alt+C -> origin to geometry
* N: dimensions: from here you can edit global positions and set the objecto to 0,0,0.  draw3DSModel assumes all objects are at the center of coordinates.
* Ctrl+J: Join selected objects.

Sounds
------

* ffmpeg -i input.m4a output.wav
* ./playaudio hellosine.wav 44100 1.0
* https://github.com/thestk/stk/blob/master/INSTALL.md

References
----------
* OpenGL Tutorial: http://www.opengl-tutorial.org/
* Normalize error: https://github.com/resibots/robdyn/issues/3
* ODE http://ode.org/wiki/index.php?title=Main_Page
* ODE http://www.ode.org/ode-0.5-userguide.html
* ODE http://opende.sourceforge.net/docs/index.html
* ODE Manual http://ode.org/wiki/index.php?title=Manual
* Rigid Body Dynamics http://www.chrishecker.com/Rigid_Body_Dynamics
* Rigid Body Dynamics Sample https://www.myphysicslab.com/engine2D/collision-en.html
* OpenGL Textures: https://learnopengl.com/Getting-started/Textures
* OpenGL Demura Blog: http://demura.net/english
* Langevin Functions: https://journals.sagepub.com/doi/full/10.1177/1081286518811876
* STK https://ccrma.stanford.edu/software/stk/download.html
* OpenGL on OSX https://pleiades.ucsc.edu/hyades/OpenGL_on_OS_X
* OpenGL common pitfalls https://www.opengl.org/archives/resources/features/KilgardTechniques/oglpitfall/
* gloLookAt Issue https://forums.khronos.org/showthread.php/4991-The-Solution-for-gluLookAt()-Function!!!!
* Old and excellent OpenGL video tutorial www.videotutorialsrock.com
* Learning modern 3d graphics programming https://paroj.github.io/gltut/
* https://learnopengl.com/Advanced-OpenGL
* https://www.salome-platform.org/
* https://github.com/jacobaustin123/Titan
* https://libigl.github.io/
* https://twitter.com/page_eco/status/1372175858174062594?s=20
* Core Techniques and Algorithms in Game Programming, Daniel Sanchez Crespo Dalmau, 2003
* https://rosettacode.org/wiki/K-d_tree
* Project Chrono: https://projectchrono.org/
* Rust programming language https://doc.rust-lang.org/book/ch00-00-introduction.html
* Vulkan Tutorial https://kylemayes.github.io/vulkanalia/introduction.html
* https://github.com/avikola/fire-and-smoke
* https://community.khronos.org/t/efficient-smoke/67120/6
* http://www.inf.ufsc.br/~aldo.vw/grafica/apostilas/openGL/lesson19/lesson19.html
