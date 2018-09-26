# Not Done yet #

This involves adding or editing a "boards.txt" file somewhere in the Arduino application or user sketch directory hierarchy.

Both the preferred location and the format of the file change from version to version of the IDE.


# IDE Version 1.6.4 (and later)
## Boards Manager
Verions 1.5.x (beta) of the IDE added support for a much broader range of non-AVR and 3rd party hardware.  This changed the directory structure in the user's sketchbook folder in ways that were incompatible with 1.0.x (in particular, adding a "platform" sub directory, so that boards.txt was in <sketchbook>/hardware/<extname>/<platformname>/)

In 1.6.4 (a production release), the IDE added a "boards manager" that permits a 3rd party extension to be automatically downloaded and installed simply by adding a URL in the preferences panel and (in yet another location and clicking an "Install" button in the Boards Manager panel.  (The actual files end up in yet a different place in the file system, but this should be transparent to the users.)  This is the preferred way to install the Optiboot boards.txt if you are running 1.6.4 or later, but it does not (currently) install source code.

# IDE version 1.0.x
In version 1.0 of the Arduino IDE, it became possible to put hardware extensions in the sketchbook folder, by putting new subdirectories in <sketchbook>/hardware/<extname>.  You could put a suitably formatted boards.txt there describing the new optiboot cpus, pointing toward .hex files located in that tree, or in other locations.


# IDE Version 00xx
Optiboot was written prior to version 1.0 of the IDE.  In those days, the only way to add additional boards was to modify the boards.txt file that included within the directory structure of the IDE itself. (<installDir>/hardware/arduino/boards.txt, with the .hex files in <installDir>/hardware/arduino/bootloaders/optiboot)  The directory structure of the Optiboot repositor reflects this old structure.

