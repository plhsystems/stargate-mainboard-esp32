# Dial algorithm

# Ring moving

The stepper needs to move the ring at the right location based on the current ring position and which chevron to use.

## Distance

Every symbol are part of a ring, it need to calculate the minimum distance between two symbols.

Example, on a 39 symbols gate, the symbol 1 and 39 are not 38 symbols appart but are one only one symbol appart.

## Parameters

Each Stargate type have their own different symbols, including symbol count.

| Type | Symbol count | Spacing |
|---|---|---|
| Movie/SG1 | 39 | Equally spaced |
| Atlantis | 36 | Equally spaced |
| Universe | 36 | Not equally spaced |

## Calculation

To find how many step are necessary to move from one symbol to another :

### Movie/SG1/Atlantis

| Variable | Description |
|---|---|
| stepPerRot | How many step per full ring rotation |
| targetPos| Target absolute position in step |
| currPos | Current absolute position in step |
| relDistPos | Shortest distance between two symbols |
| symbolNum | Symbol number [1-39] |
| symbolCnt | Symbol count (39 or 36) |
| stepPerSymbolWidth | Step count to point to another symbol |

Positive number move the gate counter-clockwise.

Assumptions:
- Symbol number is one based.
- TargetPos 0 means the center of the symbol 1 is under the master chevron.

![](./assets/stargate-position-diagram.png)

```C
const int32_t stepPerRot = 7000; // Abritrary value
const int32_t symbolCnt = 39;
const int32_t stepPerSymbolWidth = stepPerRot / symbolCnt;

const uint8_t symbols[] = { 12, 37, 24, 1, 32, 10, 1 };

posCurr = 0;
MoveStepperAbs(posCurr); //Move to 0.

for(int i = 0; i < sizeof(symbols)/sizeof(symbols[0]); i++)
{
    const uint8_t symbolNum = symbols[i];

    // Absolute position on the ring.
    // Notice, the algorithm is different for the universe gate. Symbols aren't equally spaced.
    const int32_t targetPos=((symbolNum-1) * stepPerSymbolWidth);

    const int32_t relDistPos = MISCFA_CircleDiff(posCurr, targetPos, stepPerRot);

    // Move the stepper by 'relDistPos' position
    MoveStepperRel(relDistPos);

    lastPosition = targetPos;
}

double MISCFA_CircleDiff(double a, double b, double rotation)
{
    const double halfRotation = rotation / 2;
    return fmod((a - b + rotation + halfRotation), rotation) - halfRotation;
}
```
