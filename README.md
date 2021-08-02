# Arduino-RPM-meter
Arduino RPM meter based on interrupts 

Ive been working on making a DC treadmill motor run in a PID loop for a lathe application.  Why?  Well because its there I guess and I like to recycle.   The tradmill motor I have is a permanent magnet type, and has an optical speed sensor, using a slotted disk (36 holes) mounted on the motor shaft adn a slotted optocoupler.

Since the treadmill circuit has a 16F690 PIC and the electronics to drive the opto, I didnt have to design any of the electronics other than the pulse counter. 

I did some searching and it seems there are 2 types of RPM meters.  One type measures the time interval between the occurence of a pulse and the other counts the number of pulses occurring in a defined period.  Apparently for slow RPM, its best to measure the time, but for fast RPM its best to count pulses. Intuitively that seems to be right as the pulses are much closer together with fast RPM, so CPU clock speed becomes an issue.  As it takes clock cycles to process the timing measurement the error grows the faster the pulses arrive. GreatScottLabs did a nice time based RPM meter  https://www.instructables.com/DIY-Tachometer-RPM-Meter/ and I used that as a start point for mine.

I tried 2 approaches:
1 - In a vain attempt to improve accuracy, I thought Id let the Arduino count pulses in 0.1s, then repeat that 10 times, calc an average of the pulses occurring in 0.1s convert it to RPM and publish that.  It had all sorts of problems, and eventually I realised that if I count pulses in 0.1s then those get scaled by the conversion factors and even a small error in count is magnified hugely.  There are also timing issues between the main loop execution time and the 0.1s period as the loop executes faster than the timer which further complicated the problem.

2 - The second aproach works much better.  Count the number of pulses occurring in 1s, convert that to RPM and display that. That works nicely but to improve it, I wanted to start counting pulses only when the timer was started, and stop counting pulses immediately on TOV (timer overflow) interrupt occurring.  So I start the timer and the conter in the main loop, then in the timer service routine, stop the counter, read the counter, set a variable that says dats is available to publish.  This allows the timer to start, allow counting, stop at the rigt time, and then I have whatever time needed to display the data, and when ready can restart the timer to repeat the readings.

Graphically it can be shown as in the image "RPM pulse counter workings"

The code assumes that the pulses are coming in on INT0 on PD2 on the Arduino, and timer 1 is used to count the time.  Pulses are counted on the rising edge.

The Schematic for the pulse counter electronicvs is borrowed from the treadmill circuit.  The infrared reflective sensor used by GreatScott also works but the counter factor must be adjusted.  Count factor for 36 pulses per revolution at 1s count intervals is pulses*60/36.  For a reflective counter it would be pulses * 60 I guess? 

![RPM pulse counter workings](https://user-images.githubusercontent.com/78244043/127774834-80ffe884-591f-4f94-9c57-251e17349553.png)
