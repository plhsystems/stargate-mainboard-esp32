# Homing algorithm

The homing algorithm should run every time the gate power up. 
It's also possible to manually run it. 

## About hall sensor

Also hall sensor have an hysterisis, meaning the position to activate it and deactivate it are not the same. 
There could be several degrees of hysterisis, depending on how it is mechanically assembled or what sensor is used. 

--TODO diagram about hysterisis 

## About light sensor

The ring contains an hissed hole with one led on one side and a sensor on the other side of the hole.
The hysterisis is likely very low. But the algorithm remains the same as for the Hall sensor.

--TODO diagram about hysterisis 

# Homing procedure

If the ring is already in a position were the home sensor is ON (magnet near the sensor), it doesn't mean the homing is done correctly. 
The stepper while unpowered could move the ring slightly out of alignment.

But the good part is that it makes the homing significantly faster, it doesn't need the ring to do an entire rotation.

- Spin the ring until the home sensor is on. Consider it's the temporary 0 position. 
- Continue moving in the same direction util it go off.
- Move in the other direction until the home sensor go home.
- Check how many step were made between two home sensor activation and divide the number by two.
- Move the gate by the number of step the previous part of the algorithm returned.


