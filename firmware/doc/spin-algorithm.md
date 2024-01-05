# Spinning ring algorithm

The stepper needs to move the ring at the right location based on the current ring position and which chevron to use.

## Distance

There are two distance to calculate, because the ring can spin clockwise or counter-clockwise to reach the right glyph. 
We use the shortest path and spin the ring accordingly.
But it's not the only way to do it, we can imagine an algorithm defining a minimum spin distance just for dramatic purpose. 

--- TODO: DIAGRAM ---

## Parameters

Each Stargate type have their own different glyphs, including glyph count.

| Type | Glyph count | Spacing |
|---|---|---|
| Movie/SG1 | 39 | Equally spaced |
| Atlantis | 36 | Equally spaced |
| Universe | 36 | Not equally spaced |

## Calculation

To find how many step are necessary to move from one glyph to another :

### Movie/SG1/Atlantis

| Variable | Description |
|---|---|
| stepPerRot | How many step per full ring rotation |
| targetPos| Target absolute position in step |
| currPos | Current absolute position in step |
| distPos1 | First path distance |
| distPos2 | Second path distance |

Positive number move the gate counter-clockwise.

Glypth position is one based.

$$ targetPos={(glyphPos-1)/glyphCnt}*stepPerRot $$


-- TODO WRONG CALC --

$$ distpos1={posNext-posCurr} $$

$$ distpos2={posCurr+(stepPerRot -posNext)} $$




