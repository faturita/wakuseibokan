Excited warfare on the seas of a Kepler IV.

The time has come to conquer Kepler IV.   Check [video](https://www.youtube.com/watch?v=_LJj1x4orbU). 

![Manta taking off](screenshot1.png)

Compiling and Installation
--------------------------

'Requirements'
* ODE: Open Dynamics Engine, version 0.14: https://bitbucket.org/odedevs/ode
* Please this README or the guidelines in http://ode-wiki.org/wiki/index.php?title=Manual:_Install_and_Use to install ODE on your Macbook.


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

Next steps
----------
* Resolve issues: the open architecture based on ODE generates too many glitches in the game.  In such a way, the game has too many degrees of freedom that needs to be constrained.


Issues and missing stuff
------------------------
* Floor is working great.
    * However, the floor is squared so when you fly high you see that the horizon changes weirdly.  Need improvement.
    * Sea is good.  
    * Sky must be improved.
* Island generation works very good.
    * The engine is calculating the islands perfectly.
    * Islands are BMP heightmaps 60x60.  These values are hardcoded to 600x600 in the model.
    * Max height (255) is 60. Hardcoded.
    * Island floor, which can be less than zero, or greater than zero, need to be adjusted manually for each island. Need improvement.
* Collision detection has been improved.
    * Island heights works PERFECT for Walrus.  It is amazing how well they work.
* 3DS Engine has been incorporated so I can now use 3DS models!!!!
* Walrus mechanics works but can be improved, and the texture of the Walrus is broken again.
    * Walrus control, although it works, can be improved also.
* New library for sound-support.
    * I am using TEMPORARILY system calls and afplay.
* Manta
    * The FDM model from Flight Simulator works but it is very difficult to really control it. And there are many parameters.
    * I implemented a new mixed model which handle the forces form the dynamic model but keep the airplane pose based on a simplified model (not dynamic).  I will move from here which is very good for control (and automation).
* UserControl can be improved.  Sensitivity must be adjusted from within the game.

Blender
-------

* Shift+Ctrl+Alt+C -> origin to geometry
* N: dimensions

Sounds
------

* ffmpeg -i input.m4a output.wav
* ./playaudio hellosine.wav 44100 1.0
* https://github.com/thestk/stk/blob/master/INSTALL.md

References
----------
* Normalize error: https://github.com/resibots/robdyn/issues/3
* ODE http://ode.org/wiki/index.php?title=Main_Page
* ODE http://www.ode.org/ode-0.5-userguide.html
* ODE http://opende.sourceforge.net/docs/index.html
* ODE Manual http://ode.org/wiki/index.php?title=Manual
* OpenGL Textures: https://learnopengl.com/Getting-started/Textures
* OpenGL Demura Blog: http://demura.net/english
* Langevin Functions: https://journals.sagepub.com/doi/full/10.1177/1081286518811876
* https://ccrma.stanford.edu/software/stk/download.html
* https://pleiades.ucsc.edu/hyades/OpenGL_on_OS_X
* https://www.opengl.org/archives/resources/features/KilgardTechniques/oglpitfall/
