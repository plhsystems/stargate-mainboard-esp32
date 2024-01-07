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

## Dialer

The user can type any glyph one by one, the gate will spin and lock each chevrons.
If it points to an existing fan gate, it will attempt to connect to it.

Not sure yet if it should buffer the keypresses.

## Commands

- Shutdown: Shut down the wormhole or the dial process.
- Homing: Manual homing of the ring.

## Status display

Display the "CALLERID" and whether it's an incoming or outgoing wormhole.
Also display error error linked to the dial-up process.
