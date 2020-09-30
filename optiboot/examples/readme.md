## "Examples."

In general, **test_xxx** is a test program for checking some optiboot functionality, while **demo_xxx** is an example of how a user might use an optiboot feature.

----
----

### chaucer*

"big" sketches for testing behavior as sketch sizes get closer to the flash capacity of chips.  Especially, "bigger than the old bootloader" and "more than 64k".

----

### demo_reset
Shows out to figure out the reset cause from an application.  Since the bootloaders need to fiddle with the normal registers that provide this information, there need to be some extra effort to tell what the original reset cause was.

----

### demo_dospm
Demonstration of an application writing to PROGMEM data arrays using the **do_spm()** or **do_nvmctrl()** features of v8+ optiboot and optiboot_x.

This demo uses the "higher level" macros from Marek Wodzinsk's "optiboot.h", and so works with both older and new AVRs.

----

### test_nvmctrl
Try out the nvmctrl features of the Mega0 and xTiny chips with optiboot.x
This is rather bare-bones and requires knowledge of the nvctrl "peripheral"

