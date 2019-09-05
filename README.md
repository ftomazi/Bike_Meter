# Bike_Meter
Complete ciclocomputer to use in bike home trainer speed range power cadence time etc...

Resume:

I needed meassuare a lot os infos about my bike home training, but is very expensive a complete home trainer.
I a bought a basic home trainer and development this app.

How this sistem work.

I use an ESP8266 built in wifi connection and driver to e-paper 2.9 inch color waveshare, infos are showed in real time (rate update minimum is 16s) to biker while ride in your home, also send a complete infos via wifi conection to some server in cloud.

Steps to work

turn on via usb, the lcd show title and stay waiting until 60s to wifi connect. if not connect it start works and can init ride.
I read turns in bike to meassure speed and range,  the power is meassures at all cicles, and speed is calculate a medium in some turns (can change is value)

was created a table of powers in each speed, affering power consuption for this home trainer (can ajust by var).

well, after done you can do a good traines in our home, and can see yor improvements in your trainer.

hardware used:
- board ESP8266 waveshares whith driver e-paper 2.9b
- two sensor hall (cadence / speed)
- wires
- simple home-trainer

features
- meassure speed
- meassure distance
- meassure time
- meassure cadence
- calc power by speed
- show all data in e-paper display
- update each 20s
- stop calc power when stop ride
- update display only when speed change 2+ or 2- speed
- ajust for error in read interrups (wrong calcs and infinite speeds)
- write all data each 3s in serial port
- connect local wifi to send json data
- try connect or run without wifi (show in display with wifi or without)
- send all data to cloud to generate tcx file and import to strava app
