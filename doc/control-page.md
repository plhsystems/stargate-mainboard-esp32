# Control page

Friendly user interface to allows the user to dial the gate.

## Address book

The address book contains all hardcoded addresses and fan gates addresses.
We may use an API to download the address book from the "build the stargate" website.

| Address | Description |
|---|---|
| Hardcoded addresses | Many addresses are hardcoded |
| Fan Gate X | Any fan gate on the subspace network, with its online status |
| Blackhole| Change the wormhole animation |
| Destiny | 9 chevrons address, may require a ZPM |

### Universe Gate Interface

For the Universe gate type, the address book displays glyph images instead of numeric representations:
- Each address entry shows the actual glyph symbols from `glyphs/001.svg` through `glyphs/036.svg`
- Glyphs are displayed in sequence using the `.address-glyph-icon` CSS class
- Visual representation matches the gate ring, making address selection more intuitive
- Address entries include gate name, number of glyphs, and the visual glyph sequence

**Implementation**:
- JavaScript: `dial.js` - `populateAddressList()` function creates `<img>` elements for each glyph
- CSS: `dial.css` - `.address-glyph-icon` class styles the glyph images (width: 40px, height: auto)
- Vue.js: `js/app.js` - Vue component uses `getGlyphImagePath()` method for reactive rendering
- Each glyph is zero-padded to 3 digits (e.g., `001.svg`, `012.svg`, `036.svg`)

## Dialer

The user can type any symbol one by one, the gate will spin and lock each chevrons.
If it points to an existing fan gate, it will attempt to connect to it.

Not sure yet if it should buffer the keypresses.

## Commands

- Shutdown: Shut down the wormhole or the dial process.
- Homing: Manual homing of the ring.

## Status display

Display the "CALLERID" and whether it's an incoming or outgoing wormhole.
Also display error error linked to the dial-up process.
