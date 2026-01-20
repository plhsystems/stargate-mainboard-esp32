// Stargate Universe - Dial Device Interface

const TOTAL_GLYPHS = 36;
const MAX_ADDRESS_LENGTH = 9;

// State management
let selectedAddress = [];
let isDialing = false;
let dialingInProgress = false;

// Address Book
const addressBook = [
    { id: 1, name: 'Earth', address: [1, 5, 12, 18, 23, 28, 33, 36, 2], length: 9 },
    { id: 2, name: 'Destiny', address: [3, 9, 15, 21, 27, 30, 34, 7], length: 8 },
    { id: 3, name: 'Icarus Base', address: [2, 8, 14, 20, 26, 31, 35, 10, 4], length: 9 },
    { id: 4, name: 'Langara', address: [6, 11, 17, 22, 25, 32, 36, 13], length: 8 },
    { id: 5, name: 'New Lantea', address: [4, 10, 16, 19, 24, 29, 33, 8, 1], length: 9 },
];

// DOM elements
const glyphRing = document.querySelector('.glyph-ring');
const lockedGlyphsContainer = document.querySelector('.locked-glyphs');
const progressFill = document.querySelector('.progress-fill');
const progressText = document.querySelector('.progress-text');
const destinationValue = document.querySelector('.info-value');
const messageText = document.querySelector('.message-text');
const statusMessage = document.querySelector('.status-message');
const dialBtn = document.getElementById('dialBtn');
const clearBtn = document.getElementById('clearBtn');
const abortBtn = document.getElementById('abortBtn');
const bookBtn = document.getElementById('bookBtn');
const addressBookModal = document.getElementById('addressBookModal');
const closeBookBtn = document.getElementById('closeBookBtn');
const addressList = document.getElementById('addressList');

// Initialize the interface
function init() {
    console.log('Stargate Universe Dial Device - Initializing');
    createGlyphRing();
    updateUI();
    setupEventListeners();
    console.log(`Address book contains ${addressBook.length} destinations`);
    console.log('Initialization complete');
}

// Create circular glyph arrangement
function createGlyphRing() {
    const angleStep = 360 / TOTAL_GLYPHS;
    const radius = 240; // pixels from center

    for (let i = 0; i < TOTAL_GLYPHS; i++) {
        const glyphNumber = i + 1;
        const angle = (i * angleStep) - 90; // Start at top
        const angleRad = (angle * Math.PI) / 180;

        // Calculate position
        const x = radius * Math.cos(angleRad);
        const y = radius * Math.sin(angleRad);

        // Create glyph item
        const glyphItem = document.createElement('div');
        glyphItem.className = 'glyph-item';
        glyphItem.dataset.glyph = glyphNumber;
        glyphItem.style.transform = `translate(${x}px, ${y}px) rotate(${angle + 90}deg)`;

        // Create glyph button
        const glyphButton = document.createElement('div');
        glyphButton.className = 'glyph-button';

        // Create glyph image
        const glyphImg = document.createElement('img');
        const paddedNum = String(glyphNumber).padStart(3, '0');
        glyphImg.src = `glyphs/${paddedNum}.svg`;
        glyphImg.alt = `Glyph ${glyphNumber}`;

        glyphButton.appendChild(glyphImg);
        glyphItem.appendChild(glyphButton);
        glyphRing.appendChild(glyphItem);

        // Add click event
        glyphItem.addEventListener('click', () => selectGlyph(glyphNumber));
    }
}

// Select a glyph
function selectGlyph(glyphNumber) {
    if (dialingInProgress) {
        setMessage('DIALING IN PROGRESS', 'error');
        return;
    }

    if (selectedAddress.length >= MAX_ADDRESS_LENGTH) {
        setMessage('ADDRESS COMPLETE', 'error');
        return;
    }

    // Check if glyph is already selected
    if (selectedAddress.includes(glyphNumber)) {
        setMessage('GLYPH ALREADY SELECTED', 'error');
        return;
    }

    // Add to address
    selectedAddress.push(glyphNumber);

    // Visual feedback
    const glyphItem = document.querySelector(`.glyph-item[data-glyph="${glyphNumber}"]`);
    glyphItem.classList.add('selected');

    // Play selection animation
    animateGlyphSelection(glyphItem);

    // Update locked glyphs display
    updateLockedGlyphs();

    // Update UI
    updateUI();

    setMessage(`GLYPH ${glyphNumber} LOCKED`, 'success');

    console.log('Address:', selectedAddress);
}

// Animate glyph selection
function animateGlyphSelection(glyphItem) {
    const glyphButton = glyphItem.querySelector('.glyph-button');
    glyphButton.style.animation = 'none';
    setTimeout(() => {
        glyphButton.style.animation = '';
    }, 10);
}

// Update locked glyphs display
function updateLockedGlyphs() {
    const slots = document.querySelectorAll('.locked-glyph-slot');

    selectedAddress.forEach((glyphNumber, index) => {
        if (index < slots.length) {
            const slot = slots[index];
            slot.classList.add('filled');

            // Check if image already exists
            let img = slot.querySelector('img');
            if (!img) {
                img = document.createElement('img');
                slot.appendChild(img);
            }

            const paddedNum = String(glyphNumber).padStart(3, '0');
            img.src = `glyphs/${paddedNum}.svg`;
            img.alt = `Glyph ${glyphNumber}`;
        }
    });
}

// Update UI state
function updateUI() {
    // Update progress bar
    const progress = (selectedAddress.length / MAX_ADDRESS_LENGTH) * 100;
    progressFill.style.width = `${progress}%`;
    progressText.textContent = `${selectedAddress.length} / ${MAX_ADDRESS_LENGTH}`;

    // Update destination
    if (selectedAddress.length === 0) {
        destinationValue.textContent = 'UNKNOWN';
    } else if (selectedAddress.length < MAX_ADDRESS_LENGTH) {
        destinationValue.textContent = 'INCOMPLETE';
    } else {
        destinationValue.textContent = 'READY';
    }

    // Update button states
    dialBtn.disabled = selectedAddress.length < 6 || dialingInProgress;
    clearBtn.disabled = selectedAddress.length === 0 || dialingInProgress;
    abortBtn.disabled = !dialingInProgress;
}

// Clear address
function clearAddress() {
    if (dialingInProgress) {
        return;
    }

    selectedAddress = [];

    // Clear visual selections
    document.querySelectorAll('.glyph-item.selected').forEach(item => {
        item.classList.remove('selected');
    });

    // Clear locked glyphs
    const slots = document.querySelectorAll('.locked-glyph-slot');
    slots.forEach(slot => {
        slot.classList.remove('filled');
        const img = slot.querySelector('img');
        if (img) {
            img.remove();
        }
    });

    updateUI();
    setMessage('ADDRESS CLEARED', 'success');

    console.log('Address cleared');
}

// Start dialing sequence
async function startDialing() {
    if (selectedAddress.length < 6) {
        setMessage('MINIMUM 6 GLYPHS REQUIRED', 'error');
        return;
    }

    if (dialingInProgress) {
        return;
    }

    dialingInProgress = true;
    updateUI();

    setMessage('INITIATING DIALING SEQUENCE', 'success');

    // Animate ring rotation
    glyphRing.classList.add('dialing');

    // Simulate dialing each glyph
    for (let i = 0; i < selectedAddress.length; i++) {
        await new Promise(resolve => setTimeout(resolve, 800));

        const glyphNumber = selectedAddress[i];
        setMessage(`ENCODING GLYPH ${i + 1}: ${glyphNumber}`, 'success');

        // Highlight current glyph
        const glyphItem = document.querySelector(`.glyph-item[data-glyph="${glyphNumber}"]`);
        glyphItem.style.animation = 'glyphPulse 0.5s ease-in-out 3';
    }

    await new Promise(resolve => setTimeout(resolve, 1000));

    // Remove dialing animation
    glyphRing.classList.remove('dialing');

    // Complete dialing
    if (selectedAddress.length === MAX_ADDRESS_LENGTH) {
        setMessage('WORMHOLE ESTABLISHED', 'success');
        destinationValue.textContent = 'CONNECTED';
    } else {
        setMessage('ADDRESS LOCKED', 'success');
        destinationValue.textContent = 'PARTIAL LOCK';
    }

    await new Promise(resolve => setTimeout(resolve, 2000));

    // Auto-clear after successful dial
    clearAddress();
    dialingInProgress = false;
    updateUI();
    setMessage('READY TO DIAL', '');
}

// Abort dialing
function abortDialing() {
    if (!dialingInProgress) {
        return;
    }

    dialingInProgress = false;
    glyphRing.classList.remove('dialing');

    setMessage('DIALING ABORTED', 'error');

    setTimeout(() => {
        clearAddress();
        setMessage('READY TO DIAL', '');
    }, 1500);
}

// Set status message
function setMessage(text, type = '') {
    messageText.textContent = text;
    statusMessage.className = 'status-message';

    if (type === 'error') {
        statusMessage.classList.add('error');
    } else if (type === 'success') {
        statusMessage.classList.add('success');
    }
}

// Address Book Functions
function openAddressBook() {
    populateAddressList();
    addressBookModal.style.display = 'flex';
}

function closeAddressBook() {
    addressBookModal.style.display = 'none';
}

function populateAddressList() {
    addressList.innerHTML = '';

    addressBook.forEach(address => {
        const addressItem = document.createElement('div');
        addressItem.className = 'address-item';
        addressItem.onclick = () => loadAddress(address);

        // Create address header
        const addressHeader = document.createElement('div');
        addressHeader.className = 'address-header';
        addressHeader.innerHTML = `
            <h3>${address.name}</h3>
            <span class="address-length">${address.length} glyphs</span>
        `;

        // Create glyphs container
        const glyphsContainer = document.createElement('div');
        glyphsContainer.className = 'address-glyphs';

        // Add glyph images
        address.address.forEach(glyphNumber => {
            const glyphImg = document.createElement('img');
            const paddedNum = String(glyphNumber).padStart(3, '0');
            glyphImg.src = `glyphs/${paddedNum}.svg`;
            glyphImg.alt = `Glyph ${glyphNumber}`;
            glyphImg.className = 'address-glyph-icon';
            glyphsContainer.appendChild(glyphImg);
        });

        addressItem.appendChild(addressHeader);
        addressItem.appendChild(glyphsContainer);
        addressList.appendChild(addressItem);
    });
}

function loadAddress(address) {
    if (dialingInProgress) {
        setMessage('CANNOT LOAD DURING DIALING', 'error');
        return;
    }

    // Clear current address
    clearAddress();

    // Load the new address
    address.address.forEach(glyphNumber => {
        selectedAddress.push(glyphNumber);
        const glyphItem = document.querySelector(`.glyph-item[data-glyph="${glyphNumber}"]`);
        if (glyphItem) {
            glyphItem.classList.add('selected');
        }
    });

    // Update locked glyphs
    updateLockedGlyphs();
    updateUI();

    setMessage(`LOADED: ${address.name}`, 'success');
    closeAddressBook();

    console.log('Address loaded:', address);
}

function formatAddress(address) {
    return address.map(g => String(g).padStart(2, '0')).join('-');
}

// Setup event listeners
function setupEventListeners() {
    dialBtn.addEventListener('click', startDialing);
    clearBtn.addEventListener('click', clearAddress);
    abortBtn.addEventListener('click', abortDialing);
    bookBtn.addEventListener('click', openAddressBook);
    closeBookBtn.addEventListener('click', closeAddressBook);

    // Close modal when clicking outside
    addressBookModal.addEventListener('click', (e) => {
        if (e.target === addressBookModal) {
            closeAddressBook();
        }
    });

    // Keyboard shortcuts
    document.addEventListener('keydown', (e) => {
        switch (e.key.toLowerCase()) {
            case 'enter':
                if (!dialBtn.disabled) {
                    startDialing();
                }
                break;
            case 'escape':
                if (addressBookModal.style.display === 'flex') {
                    closeAddressBook();
                } else if (dialingInProgress) {
                    abortDialing();
                } else if (selectedAddress.length > 0) {
                    clearAddress();
                }
                break;
            case 'backspace':
                if (!dialingInProgress && selectedAddress.length > 0 && addressBookModal.style.display !== 'flex') {
                    // Remove last glyph
                    const lastGlyph = selectedAddress.pop();
                    const glyphItem = document.querySelector(`.glyph-item[data-glyph="${lastGlyph}"]`);
                    glyphItem.classList.remove('selected');

                    // Update locked glyphs
                    const slots = document.querySelectorAll('.locked-glyph-slot');
                    const lastSlot = slots[selectedAddress.length];
                    lastSlot.classList.remove('filled');
                    const img = lastSlot.querySelector('img');
                    if (img) {
                        img.remove();
                    }

                    updateUI();
                    setMessage(`GLYPH ${lastGlyph} REMOVED`, '');
                }
                break;
            case 'b':
                if (addressBookModal.style.display !== 'flex') {
                    openAddressBook();
                }
                break;
        }
    });

    console.log('Event listeners setup complete');
    console.log('Controls:');
    console.log('  Click glyphs to select address');
    console.log('  ENTER - Start dialing');
    console.log('  BACKSPACE - Remove last glyph');
    console.log('  B - Open address book');
    console.log('  ESCAPE - Close address book / Clear address / Abort dialing');
}

// Add subtle background animation
function addBackgroundEffects() {
    setInterval(() => {
        if (Math.random() > 0.95 && !dialingInProgress) {
            const glyphItems = document.querySelectorAll('.glyph-item:not(.selected)');
            const randomItem = glyphItems[Math.floor(Math.random() * glyphItems.length)];
            if (randomItem) {
                const glyphButton = randomItem.querySelector('.glyph-button');
                glyphButton.style.borderColor = 'rgba(0, 212, 255, 0.6)';
                setTimeout(() => {
                    glyphButton.style.borderColor = '';
                }, 300);
            }
        }
    }, 500);
}

// Initialize when DOM is ready
if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', () => {
        init();
        addBackgroundEffects();
    });
} else {
    init();
    addBackgroundEffects();
}

console.log('Stargate Universe Dial Device - Script loaded');
console.log('Total glyphs available:', TOTAL_GLYPHS);
console.log('Maximum address length:', MAX_ADDRESS_LENGTH);
