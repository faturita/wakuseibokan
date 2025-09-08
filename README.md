# 太陽系外惑星航空母艦

Excited warfare on the seas of a Kepler IV.

The time has come to conquer Kepler IV.  Humanity is now able to travel further away from our solar system, and it has now reached exoplanets.  Two companies sent surveying parties in the form of two AI carriers that aim to control the vast archipielago of Kepler IV. These AIs can be controlled remotely in real-time by space folding uplinks.

Check [Videos](https://www.youtube.com/playlist?list=PLJHMIS4ekxNSLS422Nw7D2JlJW3En1Vul)

![Manta taking off](images/screenshot1.png)

# Compiling and Installation

'Requirements'
* ODE: Open Dynamics Engine, version 0.14: https://bitbucket.org/odedevs/ode
* STK: The Synthesis Toolkit in C++ (Audio Library)
* OpenGL Version 2.1 (supported natively in Linux, Mac and Windows).

Mac OS Installation
-------------------

```bash
brew install premake
brew install libtool
```

```bash
git clone https://github.com/faturita/wakuseibokan.git
cd wakuseibokan
cp dependencies/stk.tgz ../
cd ..
tar xvzf stk.tgz
cd stk
make clean
./configure
make 
sudo make install
cd ..
git clone https://bitbucket.org/odedevs/ode.git
cd ode
rm -rf aclocal.m4 configure config.guess config.sub depcomp install-sh ltmain.sh missing
autoreconf --install --force   
./configure --disable-asserts
make
sudo make install
cd ..
cd wakuseibokan
make clean
make
```


ODE 0.14 Compilation on Mac Sierra / Apple M2 Pro Ventura
----------------------------------------------------------

<details>
<summary>For older versions, you can run the following script</summary>

* Run the following script to install automake tools for Mac

On newer OSX systems with brew installed, you can do

```bash
brew install premake
```
 
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
* Modify configure script and remove from the script "-sse .... mmx"
* Modify LIBTOOLIZE variables from 'glibtoolize' to 'libtoolize'
</details>

Install on WSL on Windows 11
----------------------------

* Install WSL2 on Windows 11
* Create a 22.04 ubuntu WSL2 box:

```bash
wsl install -d Ubuntu-22.04
```

* When the image is created a new file will appear on the windows home directory **.wslconfig**

```bash
 [wsl2]
 networkingMode=NAT
```
The NAT configuration allows both the WSL box and the host computer to see each other in a private network.  By running **ipconfig** on the host computer the IP marked as Hyper-V is the IP address on the host computer for the WSL.  On the other hand the WSL ip can be obtained from the box by running **ifconfig**.

On the other hand, configure the box as **networkingMode=mirrored** to share the same networking address space as the windows host computer.  This means that for any external computer the IP and port of any process running on the wsl2 box will be shared with the host computer.

Now, follow the same procedure for Ubuntu 22.04 the exact same procedures to compile and run the application on Ubuntu.

Ubuntu 22.04
---------------

Install the following packages
 
```bash
sudo apt-get update
sudo apt-get upgrade
sudo apt-get install libbsd-dev freeglut3-dev libasound2 libasound2-dev
sudo apt-get install git make g++ gcc libtool automake net-tools
sudo apt-get install python3-pip
 ```


```bash
git clone https://github.com/faturita/wakuseibokan.git
cd wakuseibokan
cp dependencies/stk.tgz ../
cd ..
tar xvzf stk.tgz
cd stk
make clean
./configure
make 
sudo make install
cd ..
cd wakuseibokan
cp dependencies/ode.tgz ../
cd ..
tar xvzf ode.tgz
cd ode
./configure --disable-asserts
make clean && make && sudo make install
cd ..
cd wakuseibokan
make clean
make
make testcase TC=111
```


ODE
---

You can either download it from ODE repository or copy the file in dependencies/ode.tgz back to the same directory level as this project.
Please this README or the guidelines in http://ode-wiki.org/wiki/index.php?title=Manual:_Install_and_Use to install ODE on your Macbook.

```bash
./bootstrap
./configure --disable-asserts
make
sudo make install
```

STK
---

First you need to copy the stk file from dependencies into the parent directory where you cloned wakuseiboukan.  Then you need
to compile this sound library.

```bash
cp dependencies/stk.tgz ../
cd ..
tar xvzf stk.tgz
cd stk
make clean
./configure
make 
sudo make install
```

The STK libraries are going to be copied into /usr/local/lib/.  So you need
to run the following command before executing the simulator or configure it intorc_bash or similar.

```bash
export LD_LIBRARY_PATH=/usr/local/lib/
``` 

# Compiling the game

 make

 # Running

```bash
 ./waku [-d] [-random] [-client] [-server] [-greenmode|-bluemode] [-aiplayergreen|-aiplayerblue|-aiboth] [-action|-strategy] [-loadgame savegames/file.w]

 -d: open is a window
 -random: initialize the random number generator with a seed from timestamp.
 -server: runs as a server, ready for receiving incoming connections.
 -client: runs as a client, trying to connect to a server and receiving UDP Telemetry.
 -greenmode|-bluemode: plays as green faction or blue faction.
 -aiplayergreen|aiplayerblue|aiboth: choose which side gets the AI.
 -action: action mode with a lot of islands already established.
 -strategy: starts from an initial island and conquering all the rest.
 -loadgame: load a savegame.

``` 

That's all folks.

# Playing

<details>
 <summary>How to play?</summary>

First the console can be activated with 't', like Minecraft.

Everything can be controlled.  Each vehicle has a vehicle number.  Keyboard 1-9 allows to control 9 vehicles directly from the keyboard.  Otherwise you need to activate the console  write 'control NN' and the number of the vehicle.   The list of all the entities can be accessed with '#'.  It is only possible to control vehicles from the same faction.
The keyboard entries can be configured by writing in the console 'set KEYNUMBER#NN'.

There are five degrees of freedom that can be controlled from each vehicle
* a-d: ROLL a clockwise on Z
* w-s: PITCH w noise down.
* v-b: YAW   b clockwise on Y
* z-c: PRECESION  c clockwise on Z
* r-f: THRUST    r inclease it
* h: Fire active weapon.
* j/J: Engage/Disengage autopilot.

Capital letters increase the values of the sensitivity.

The map can be activated by pressing "@".  And the main view can be restored by pressing "!".  The map can be zoomed in and out by pressing left and right mouse clicks.  While clicking and pressing shift will set a waypoint for the autopilot.  The waypoint mode can be modified from "destination" or "attack" by writing these commands in the console.

* o/O: Spawns a UGV either from any of the Carriers or from Ports or Armories.  Capital O despawns them when they are nearby.
* m/M: Spawns a UAV either from any of the Carriers or from Roadways.  Capital M despawns them when they are landed.
* c:   Spawns a UGV drone only from the Beluga Carrier.
* 'launch': Console command to launch any aircraft landed on the runways or carriers.
* 'taxi': Moves the aircraft to launching position when necessary.
* 'land': Order any nearby aircraft to land.
* 'def Island#N': Control the structure N at island 'Island'.  Numbers go from 0 to 10 (depending how old is the island).
* 'manta N': Control Manta number N
* 'walurs N': Control Walrus number N
* 'medusa N': Control Medusa Defense Airplane number N
* 'seal N': Control Seal Speeder Boat number N

 
</details>

Scenario
--------

There are several scenarios in the engine that are used for research and as test cases.

The scenario 111 is the tank battleground.  To compile everything do:

```bash
git clone https://github.com/faturita/wakuseibokan.git
cd wakuseibokan
cp dependencies/stk.tgz ../
cd ..
tar xvzf stk.tgz
cd stk
make clean
./configure
make 
sudo make install
cd ..
cd wakuseibokan
cp dependencies/ode.tgz ../
cd ..
tar xvzf ode.tgz
cd ode
./configure --disable-asserts
make clean && make && sudo make install
cd ..
cd wakuseibokan
```

and then compile the scenario itself from the same directory.

```bash
 make testcase TC=111
 export LD_LIBRARY_PATH=/usr/local/lib/
 ./testcase -mute -nointro 
```

* Add -seed NN to set a seed number for the simulation.
* Add --episodes to generate and repeat each episode.

# Description

Objective and design guidelines
-------------------------------

The purpose was to create a multiplatform simple game engine based on physics dynamic engine and a fluid simulation solver, and a testbed platform to simulate manipulators and mobile robots.

The game is a remake of an old and amazing game, which uses the engine as platform.  The design guideline for the game was to create a networked game where it is possible to control indepentendly all of the entities that are available.  You can be anyone, anywhere.

Characteristics
---------------
* Floor and Sea
    * The sea floor is a flat square that extends to the horizon, located based on camera position.  Horizon works according the fulcrum representation.
    * It is a blanket that moves along with the camera view.
    * Sea works as a texture that is shifted based on the camera position.  So it gives the appearance of movement.
    * The buoyancy model works really well.
    * Reflections and alpha are missing.
    * Waves and boat wakes are missing.  
* Sky
    * The Sky is a far-away-box with textures on the insides.
    * It is similar to the sea floor.  It is always drawn on front of the camera view, and the texture shifts depending where do you look at.
    * What you see in the sky is based on the forward direction of the camera view (Vehicle->getViewPort()).
* Sun and Lightning
    * The day lasts for 10000 ticks.  Depending on the simulation fps, it will last around 250 seconds or 4.6 minutes.  The sun raises from the east at 0 and sets at 5000.
    * Lightning implementation has been messy in this game but current implementation works and is aligned with the sun. 
* Islands
    * The engine is calculating the islands perfectly.  What you see is what you feel.
    * Islands are BMP heightmaps 60x60.  These values are hardcoded to 3600x3600 in the model.
    * Max height (255) is 60. Hardcoded.
    * Island floor, which can be less than zero, or greater than zero, need to be adjusted manually for each island. Need improvement->Some Z Fighting.
* Collision detection
    * Island heights works PERFECT for Walruses and Turtles.  Watching the bullets ricochetting on the floor is awesome.
* Models
    * MDModel: Unreal.
    * 3DSModel: it works only when you have just one object and without textures.
* Walruses
    * There are several models now.
    * They work very nicely on water as boats following the buyoncy model.
    * On land they work as Ackerman driving.
    * When they enter or leave an island, the model changes.
    * The thrust simulates and offboard engine.
* Sound
    * STK
* Manta
    * The FDM model from Flight Simulator works but it is very difficult to really control it. And there are many parameters.
    * Current implementation is a new mixed model which handle the forces from the dynamic model but keep the airplane pose based on a simplified model (not dynamic).  I will move from here which is very good for control (and automation).
* UserControl can be improved.  Sensitivity must be adjusted from within the game.

Blender
-------

* Shift+Ctrl+Alt+C -> origin to geometry
* N: dimensions: from here you can edit global positions and set the objecto to 0,0,0.  draw3DSModel assumes all objects are at the center of coordinates.
* Ctrl+J: Join selected objects.
* Shift+D Copy (Alt+D), Ctrl+M >> X,Y,Z, + ENTER (Mirror)

Sounds
------

* ffmpeg -i input.m4a output.wav
* ./playaudio hellosine.wav 44100 1.0
* https://github.com/thestk/stk/blob/master/INSTALL.md
* https://ccrma.stanford.edu/software/stk/multichannel.html

More info
---------
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
* http://lua-users.org/wiki/CppObjectBinding
* https://www-robotics.jpl.nasa.gov/how-we-do-it/facilities/the-darts-simulation-laboratory/
* MuJoCo https://github.com/deepmind/mujoco
* https://www.goodai.com/ai-in-games-open-source-spaceship-generator-released-for-space-engineers
* ARMA 3 https://en.wikipedia.org/wiki/Arma_3
* Game Networking https://pvigier.github.io/2019/09/08/beginner-guide-game-networking.html
* https://github.com/medialab-ku/openGLESbook
* https://github.com/federico-busato/Modern-CPP-Programming
* CubbyFlow Open flow dynamic engine https://github.com/CubbyFlow/CubbyFlow
* Flow Dynamics Engine https://github.com/BlainMaguire/3dfluid?tab=readme-ov-file
* https://www.nature.com/articles/s41586-024-07886-z
* https://math.hws.edu/graphicsbook/c4/s2.html
* https://github.com/2Retr0/GodotOceanWaves
* https://github.com/nasa/trick
* https://www.pydy.org/
* https://www.sciencedirect.com/science/article/pii/S0141118725001117
* https://developer.nvidia.com/isaac/sim
* https://engine-programming.github.io/
* https://github.com/gfx-rs/wgpu
* https://wiki.libsdl.org/wiki/index


