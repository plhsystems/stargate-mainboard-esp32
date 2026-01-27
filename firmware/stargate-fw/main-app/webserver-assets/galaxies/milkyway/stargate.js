// Stargate Control Interface - Animation Script

// Update binary display with random values
function updateBinaryDisplay() {
    const binaryRows = document.querySelectorAll('.binary-row');
    binaryRows.forEach(row => {
        const binary = Array.from({length: 5}, () => Math.random() > 0.5 ? '1' : '0').join(' ');
        row.textContent = binary;
    });
}

// Fluctuate energy meter
function fluctuateEnergy() {
    const meterFill = document.querySelector('.meter-fill');
    const randomHeight = 85 + Math.random() * 15; // 85-100%
    meterFill.style.height = randomHeight + '%';
}

// Cycle through glyphs (39 total)
const totalGlyphs = 39;
let currentGlyphIndex = 1;

function cycleGlyphs() {
    const glyphDisplays = document.querySelectorAll('.right-panel .glyph-display');
    glyphDisplays.forEach((display, index) => {
        const glyphNumber = ((currentGlyphIndex + index) % totalGlyphs) + 1;
        const paddedNum = String(glyphNumber).padStart(3, '0');
        const img = display.querySelector('img');
        if (img) {
            img.src = `glyphs/${paddedNum}.svg`;
            img.alt = `Glyph ${glyphNumber}`;
        }
        display.setAttribute('data-glyph', glyphNumber);
    });
    currentGlyphIndex = (currentGlyphIndex % totalGlyphs) + 1;
}

// Randomly activate/deactivate chevrons
function randomChevronActivation() {
    const chevrons = document.querySelectorAll('.chevron-light');
    const chevronItems = document.querySelectorAll('.chevron-item');

    // Occasionally toggle random chevrons
    if (Math.random() > 0.7) {
        const randomIndex = Math.floor(Math.random() * chevrons.length);
        chevrons[randomIndex].classList.toggle('active');
        if (randomIndex < chevronItems.length) {
            chevronItems[randomIndex].classList.toggle('active');
        }
    }
}

// Update status glyphs
function updateStatusGlyphs() {
    const statusGlyphs = document.querySelectorAll('.status-glyph:not(.locked)');
    // Occasionally change one unlocked glyph
    if (Math.random() > 0.8 && statusGlyphs.length > 0) {
        const randomIndex = Math.floor(Math.random() * statusGlyphs.length);
        const randomGlyphNumber = Math.floor(Math.random() * totalGlyphs) + 1;
        const paddedNum = String(randomGlyphNumber).padStart(3, '0');
        const img = statusGlyphs[randomIndex].querySelector('img');
        if (img) {
            img.src = `glyphs/${paddedNum}.svg`;
            img.alt = `Glyph ${randomGlyphNumber}`;
        }
        statusGlyphs[randomIndex].setAttribute('data-glyph', randomGlyphNumber);
    }
}

// Simulate chevron locking sequence
function simulateChevronLock() {
    const statusGlyphs = document.querySelectorAll('.status-glyph');
    // Occasionally lock/unlock a glyph
    if (Math.random() > 0.92) {
        const randomIndex = Math.floor(Math.random() * Math.min(7, statusGlyphs.length));
        statusGlyphs[randomIndex].classList.toggle('locked');
    }
}

// Update authorization code
function updateAuthCode() {
    const authDigits = document.querySelectorAll('.auth-digits span:not(.separator)');
    // Occasionally change a few digits
    if (Math.random() > 0.85) {
        const randomIndex = Math.floor(Math.random() * authDigits.length);
        const randomDigit = Math.floor(Math.random() * 10);
        authDigits[randomIndex].textContent = randomDigit;
    }
}

// Simulate gate engagement sequence
let engagementPhase = 0;
function simulateEngagement() {
    const engagedText = document.querySelector('.engaged-text');
    const phases = ['DIALING', 'ENCODING', 'ENGAGED', 'WORMHOLE STABLE'];

    if (Math.random() > 0.95) { // Rarely change phase
        engagementPhase = (engagementPhase + 1) % phases.length;
        engagedText.textContent = phases[engagementPhase];
    }
}

// Add random scan lines effect
function addScanLines() {
    const container = document.querySelector('.interface-container');
    const scanLine = document.createElement('div');
    scanLine.className = 'scan-line';
    scanLine.style.cssText = `
        position: absolute;
        top: ${Math.random() * 100}%;
        left: 0;
        right: 0;
        height: 2px;
        background: linear-gradient(90deg, transparent, rgba(0, 217, 255, 0.5), transparent);
        animation: scanLineMove 3s linear;
        pointer-events: none;
        z-index: 1000;
    `;

    container.appendChild(scanLine);

    setTimeout(() => {
        scanLine.remove();
    }, 3000);
}

// Add scan line animation to CSS dynamically
const style = document.createElement('style');
style.textContent = `
    @keyframes scanLineMove {
        from { transform: translateY(0); opacity: 0; }
        10% { opacity: 1; }
        90% { opacity: 1; }
        to { transform: translateY(100vh); opacity: 0; }
    }
`;
document.head.appendChild(style);

// Initialize animations
setInterval(updateBinaryDisplay, 2000);
setInterval(fluctuateEnergy, 1500);
setInterval(cycleGlyphs, 5000);
setInterval(randomChevronActivation, 4000);
setInterval(updateStatusGlyphs, 3000);
setInterval(simulateChevronLock, 4500);
setInterval(updateAuthCode, 2500);
setInterval(simulateEngagement, 10000);
setInterval(addScanLines, 5000);

// Add keyboard interaction
document.addEventListener('keydown', (e) => {
    switch(e.key) {
        case ' ': // Space - cycle engagement phases
            engagementPhase = (engagementPhase + 1) % 4;
            const phases = ['DIALING', 'ENCODING', 'ENGAGED', 'WORMHOLE STABLE'];
            document.querySelector('.engaged-text').textContent = phases[engagementPhase];
            break;
        case 'c': // C - activate all chevrons
            document.querySelectorAll('.chevron-light').forEach(c => c.classList.add('active'));
            document.querySelectorAll('.chevron-item').forEach(c => c.classList.add('active'));
            break;
        case 'r': // R - reset chevrons
            document.querySelectorAll('.chevron-light').forEach((c, i) => {
                if (i % 2 === 0) c.classList.add('active');
                else c.classList.remove('active');
            });
            break;
    }
});

// Add flicker effect to text occasionally
setInterval(() => {
    if (Math.random() > 0.9) {
        const engagedText = document.querySelector('.engaged-text');
        engagedText.style.opacity = '0.3';
        setTimeout(() => {
            engagedText.style.opacity = '1';
        }, 100);
    }
}, 1000);

console.log('Stargate Control Interface Active');
console.log('Controls:');
console.log('  SPACE - Cycle engagement phases');
console.log('  C     - Activate all chevrons');
console.log('  R     - Reset chevrons');
