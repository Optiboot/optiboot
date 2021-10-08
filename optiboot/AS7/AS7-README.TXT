Building Optiboot with Atmel Studio (or Microchip Studio.)

This is made based on Atmel Studio version 7 (AS7)

There is one "Solution" containing multiple projects, one project for each
of the common Microchip Development boards that might "want" to use
Optiboot.  A big advantage of using AS7 with one of the development boards
is that it allows for debugging using the AS7 debugging features (the
development boards have built-in debug support as well.)

Building:

To use AS7 to build one of the Optiboot versions, open the "Optiboot.atsln"
file from Studio (or double-click on it in Explorer.)

"Build Solution" will compile all of the projects.  To build a single
project, select the desired project/board in the "Solution Explorer" and
then use "Build xxx" from the Build menu.  To debug, right-click on the
project and select "Set as StartUp Project" before using "Start Debugging
and Break" from the DEBUG menu.  You may need to change he Debug "Tool"
setting in the Project Properties as well (my experience is that if you have
an actual hardware tool plugged in, AS will automatically offer it for
selection.)


How things work:

All of the AS projects are built using an "external makefile", so they're
not "real" AS projects in the sense that you could change toolchain settings
and have them take effect.  In theory, this means that you can add
additional projects to support any of the other targets supported by the
Optiboot Makefiles.  In reality, there is an unknown problem with recursive
makes, so that you have to (for example) build the curiosity4809 board as
  make -f Makefile.mega0 optiboot_atmega4809.hex UARTTX=B0 LED=F5 ...
instead of
  makef  Makefile.mega0 curiosity4809

The Project should be set for the correct Device, the "Build" Panel should
point to the .../bootloaders/optiboot/Makefile (or Makefile.mega0 for mega-0
anx xTiny chips.)

The "Build Commandline" should contain the makefile target, and also
PACKS="$(DEVICE_STARTUP_ROOT)" if the chip is dependent on a Pack being
installed.  The "Build Events" should copy the .elf file produced by the
makefile:
   copy optiboot_attiny817.elf $(MSBuildProjectName).elf

The current Projects are all "flattened" somewhat from the directory
structure that AS would have produced, but that requires manual editing and
isn't necessary.

For the sake of clarity, not all of the files actually used in an optiboot
build are shown as "Solution Items."
