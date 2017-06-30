# Spin Turntable with AccelStepper motor control

Based off of the Spin project from Tiffany Tseng (https://github.com/ttseng/Spin), this fork contains the following changes:
// replaced motor driver with smoother AccelStepper movements
// deleted vestigial code and moved created rotateAndPhoto method to clean up main loop
// replaced Processing debug with native Arduino serial interface
// tuned controller to work with Spin ios app as it is.
// added Easy Button button switch to circuit schematic

If you wish to build, please check out the great instructions at the original project site http://spin.media.mit.edu/

## Arduino

This folder contains the SoftModem & AccelStepper libraries needed for the spin.ino Arduino sketch.  The spin sketch should be uploaded to the Arduino Uno running the Spin app.

## CAD

If you are building a turntable and have access to a laser cutter with a bed larger than 12", build the DEFAULT turntable. If you have a smaller laser cutter that cannot cut material larger than 12", build the Modified 12-inch turntable design.  Both designs have the same exact footprint (13"), but the modified 12-inch turntable has smaller laser cut parts and has an additional set of three 3D printed feet.

Each contains three subfolders: _3D Print_, _Laser Cut_, and _Solidworks_.  Building your own Spin turntable requires laser cutting parts in 3/16" and 1/8" acrylic, which can be found in the _Laser Cut_ folder in Illustrator, Corel Draw, and DXF formats.  Additionally, you'll need to 3D print a gear (motor_gear.STL), which can be found in the _3D print_ folder.  Optionally, you can 3D print your own iOS device dock (which can be used with an iPhone 4/5/6 or iPad/iPad mini).  If you are building the modified turntable, you will also need to 3D print three feet.  

You can find all the original CAD files in the _Solidworks_ folder.

## Electronics

The Eagle files and libraries for the original Spin shield are located here, as is the schematic for the DIY shield. You can purchase the shield directly from OSH Park: https://oshpark.com/profiles/scientiffic or solder your own DIY shield