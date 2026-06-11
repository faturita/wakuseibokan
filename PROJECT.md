Wakuseibokan:

## Project context

- Read global CLAUDE.md: all those rules apply.
- Read README.md: There you will find compiling instructions.
- If you run ./waku or ./testcase or ./test with "-d" just a small screen appears and it is faster.
- The game is an engine/game based on old Carrier Command.
- There are two carriers BLUE and GREEN
- Every unit in the game can be controlled.
- Multiplayer game (One engine all the other clients)
- Everything can be automated as well.


## Coding standards
- Each time you add a new feature or a bug is solved, add a new test case in testbox.
- If the test is more complex you can add a new src/tests/testcase_NNN.cpp (compiled with make testcase TC=NNN)
- When you solve an issue, create a new test with a number that needs to be solved.  You can use the test to set up the issue and try to pass the test.  This way works super good.
- Do not commit anything at all, I will commit everything at the end.
- Produce logs to see the OpenGL outputs when you are dealing with opengl issues like things that look bad.









## Scenario TC=113

The scenario TC=113 of this game (testcase_113.cpp) consist of two tanks that fight against each other.  Each tank is controlled by a python script that runs on the scripts/ directoy.  The engine is in C++ and send telemetry information to each tank on the UDP ports 460n where n is the tank number and receives UDP commands in the port 450n.

## BACKLOG

- Ubuntu 24.xx startup crash (ThreeMaxLoader): fixed on 2026-06-11 by zeroing the ~2MB obj_type before the chunk reader fills it (the polygon-index freads only write 2 of the 4 bytes of each int) and by passing obj_type by const reference instead of by value. Covered by testbox test 100. STILL PENDING: verify on a real Ubuntu 24.xx machine.
- ThreeMaxLoader.cpp: the two int draw3DSModel(...) functions never return a value (pre-existing -Wreturn-type warnings, callers ignore the result). Left untouched.

- Issue #112 (sprintf removal): one sprintf remains at src/units/Vehicle.cpp:962 — file is off-limits per user instruction. src/mainworkingstd.cpp and src/maincarrierbackup.cpp also have one each but are not compiled by the Makefile (dead backup files).
- Issue #113 (input sanitation), remaining surfaces not yet hardened:
  - src/propertystore.cpp:154-157 — unbounded strcpy/strcat into fixed sAux buffer while serializing config values read from .ini files.
  - src/ThreeMaxLoader.cpp:779 — strcpy of caller-provided filename into a fixed buffer (3DS model loader).
  - src/imageloader.cpp — BMP heightmap parsing trusts header fields (not audited yet).
  - recvfrom in src/tests/testcase_111.cpp, testcase_117.cpp, testcase_131.cpp do not validate datagram size (test-only code, low priority).
- src/carrier.cpp:1858 — loadgame() return value ignored for the -load command line option; a corrupted savegame is reported but the game continues with a partially initialized world.
