# Miscellaneous things

## LED linearity

LED lumen output is somewhat linear according to the pwm duty cycle. 
However, human eyes perception isn't linear (I don't know about the Asguard). 

If we want to do a fade-in / fade-out using a for loop increasing linearly the duty cycle, it will works poorly. 

Recommended formula to give a perceived linear effect to human eyes (sorry Thor).

| Variable | Description |
|---|---|
| input |Input between [0-1]
| output |Output between [0-1]

$$ out={(10^{input}-1) \over 9} $$
