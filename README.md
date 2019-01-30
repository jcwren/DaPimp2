# Da Pimp 2
Modified version of Mikey Sklar's [Da Pimp 2](http://mikeysklar.blogspot.com/p/da-pimp-battery-desulfator.html)
firmware that compiles and runs under AVR-GCC 8.2.

The original source seems to be borked under the 8.2 and 7.x compilers. The
display just flashes `00.0`, as if the `switch()` statement is returning the
wrong value, and the `delay_ms()` function appears to be about 50 times
slower than it should be.

## Changes
* Added a display self-test at power-up that runs 000..999 through the
display. 
* All source is in a single file
* Added the HiLetGo USBASP-clone as a make target
* Cleaned up code formatting to be more consistent
* Replaced `delay_ms()` assembly with a hard-coded `for()` loop

## Notes
* [3D printable enclosure](https://www.thingiverse.com/thing:786407) by tromano32 on Thingiverse
* D2 on the [schematic](https://drive.google.com/file/d/0Bx5Als-UeiZbN0F5RlJUdXhsWXM/view) is drawn backwards (PCB is correct)
* U1 shows no ground connection on the shematic
* No fuse on A/C side of rectifier (SCARY!)
