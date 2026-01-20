// Stargate Universe - Dial Device Interface with Vue.js

const TOTAL_GLYPHS = 36;
const MAX_ADDRESS_LENGTH = 9;
const MIN_SAVE_LENGTH = 8;

// Check if Vue is available
if (typeof Vue !== 'undefined') {
    // Vue.js Application
    new Vue({
        el: '#app',
        data: {
            selectedAddress: [],
            dialingInProgress: false,
            statusMessage: 'READY TO DIAL',
            statusType: '',
            destinationText: 'UNKNOWN',
            glyphs: [],
            totalGlyphs: TOTAL_GLYPHS,
            maxAddressLength: MAX_ADDRESS_LENGTH,

            // Address Book
            showAddressBook: false,
            addressBook: [
                { id: 1, name: 'Earth', address: [1, 5, 12, 18, 23, 28, 33, 36, 2], length: 9 },
                { id: 2, name: 'Destiny', address: [3, 9, 15, 21, 27, 30, 34, 7], length: 8 },
                { id: 3, name: 'Icarus Base', address: [2, 8, 14, 20, 26, 31, 35, 10, 4], length: 9 },
                { id: 4, name: 'Langara', address: [6, 11, 17, 22, 25, 32, 36, 13], length: 8 },
                { id: 5, name: 'New Lantea', address: [4, 10, 16, 19, 24, 29, 33, 8, 1], length: 9 },
            ]
        },
        computed: {
            progress() {
                return (this.selectedAddress.length / this.maxAddressLength) * 100;
            },
            progressText() {
                return `${this.selectedAddress.length} / ${this.maxAddressLength}`;
            },
            canDial() {
                return this.selectedAddress.length >= 6 && !this.dialingInProgress;
            },
            canClear() {
                return this.selectedAddress.length > 0 && !this.dialingInProgress;
            },
            canAbort() {
                return this.dialingInProgress;
            }
        },
        mounted() {
            console.log('Stargate Universe Dial Device - Vue.js Initialized');
            this.initializeGlyphs();
            this.setupKeyboardListeners();
            this.addBackgroundEffects();

            // Check for API URL if available
            if (typeof API_URL !== 'undefined') {
                console.log('API URL available:', API_URL);
            }

            console.log(`Address book contains ${this.addressBook.length} destinations`);
        },
        methods: {
            initializeGlyphs() {
                const angleStep = 360 / this.totalGlyphs;
                const radius = 240;

                for (let i = 0; i < this.totalGlyphs; i++) {
                    const glyphNumber = i + 1;
                    const angle = (i * angleStep) - 90;
                    const angleRad = (angle * Math.PI) / 180;
                    const x = radius * Math.cos(angleRad);
                    const y = radius * Math.sin(angleRad);

                    this.glyphs.push({
                        number: glyphNumber,
                        x: x,
                        y: y,
                        rotation: angle + 90,
                        selected: false,
                        image: `glyphs/${String(glyphNumber).padStart(3, '0')}.svg`
                    });
                }
            },

            selectGlyph(glyph) {
                if (this.dialingInProgress) {
                    this.setMessage('DIALING IN PROGRESS', 'error');
                    return;
                }

                if (this.selectedAddress.length >= this.maxAddressLength) {
                    this.setMessage('ADDRESS COMPLETE', 'error');
                    return;
                }

                if (this.selectedAddress.includes(glyph.number)) {
                    this.setMessage('GLYPH ALREADY SELECTED', 'error');
                    return;
                }

                this.selectedAddress.push(glyph.number);
                glyph.selected = true;

                this.updateDestination();
                this.setMessage(`GLYPH ${glyph.number} LOCKED`, 'success');

                console.log('Address:', this.selectedAddress);
            },

            async startDialing() {
                if (!this.canDial) {
                    if (this.selectedAddress.length < 6) {
                        this.setMessage('MINIMUM 6 GLYPHS REQUIRED', 'error');
                    }
                    return;
                }

                this.dialingInProgress = true;
                this.setMessage('INITIATING DIALING SEQUENCE', 'success');

                // Add dialing animation
                const glyphRing = this.$refs.glyphRing;
                if (glyphRing) {
                    glyphRing.classList.add('dialing');
                }

                // Simulate dialing each glyph
                for (let i = 0; i < this.selectedAddress.length; i++) {
                    await new Promise(resolve => setTimeout(resolve, 800));
                    const glyphNumber = this.selectedAddress[i];
                    this.setMessage(`ENCODING GLYPH ${i + 1}: ${glyphNumber}`, 'success');
                }

                await new Promise(resolve => setTimeout(resolve, 1000));

                // Remove dialing animation
                if (glyphRing) {
                    glyphRing.classList.remove('dialing');
                }

                // Complete dialing
                if (this.selectedAddress.length === this.maxAddressLength) {
                    this.setMessage('WORMHOLE ESTABLISHED', 'success');
                    this.destinationText = 'CONNECTED';

                    // If API is available, send dial request
                    if (typeof API_URL !== 'undefined') {
                        this.sendDialRequest();
                    }
                } else {
                    this.setMessage('ADDRESS LOCKED', 'success');
                    this.destinationText = 'PARTIAL LOCK';
                }

                await new Promise(resolve => setTimeout(resolve, 2000));

                // Auto-clear after successful dial
                this.clearAddress();
                this.dialingInProgress = false;
                this.setMessage('READY TO DIAL', '');
            },

            clearAddress() {
                if (this.dialingInProgress) {
                    return;
                }

                this.selectedAddress = [];
                this.glyphs.forEach(glyph => glyph.selected = false);
                this.updateDestination();
                this.setMessage('ADDRESS CLEARED', 'success');

                console.log('Address cleared');
            },

            abortDialing() {
                if (!this.dialingInProgress) {
                    return;
                }

                this.dialingInProgress = false;
                const glyphRing = this.$refs.glyphRing;
                if (glyphRing) {
                    glyphRing.classList.remove('dialing');
                }

                this.setMessage('DIALING ABORTED', 'error');

                setTimeout(() => {
                    this.clearAddress();
                    this.setMessage('READY TO DIAL', '');
                }, 1500);
            },

            removeLastGlyph() {
                if (this.dialingInProgress || this.selectedAddress.length === 0) {
                    return;
                }

                const lastGlyph = this.selectedAddress.pop();
                const glyph = this.glyphs.find(g => g.number === lastGlyph);
                if (glyph) {
                    glyph.selected = false;
                }

                this.updateDestination();
                this.setMessage(`GLYPH ${lastGlyph} REMOVED`, '');
            },

            updateDestination() {
                if (this.selectedAddress.length === 0) {
                    this.destinationText = 'UNKNOWN';
                } else if (this.selectedAddress.length < this.maxAddressLength) {
                    this.destinationText = 'INCOMPLETE';
                } else {
                    this.destinationText = 'READY';
                }
            },

            setMessage(text, type = '') {
                this.statusMessage = text;
                this.statusType = type;
            },

            getLockedGlyphImage(position) {
                if (position <= this.selectedAddress.length) {
                    const glyphNumber = this.selectedAddress[position - 1];
                    return `glyphs/${String(glyphNumber).padStart(3, '0')}.svg`;
                }
                return null;
            },

            // Address Book Methods
            openAddressBook() {
                this.showAddressBook = true;
            },

            closeAddressBook() {
                this.showAddressBook = false;
            },

            loadAddress(address) {
                if (this.dialingInProgress) {
                    this.setMessage('CANNOT LOAD DURING DIALING', 'error');
                    return;
                }

                this.clearAddress();

                // Load the address
                address.address.forEach(glyphNumber => {
                    const glyph = this.glyphs.find(g => g.number === glyphNumber);
                    if (glyph) {
                        this.selectedAddress.push(glyphNumber);
                        glyph.selected = true;
                    }
                });

                this.updateDestination();
                this.setMessage(`LOADED: ${address.name}`, 'success');
                this.closeAddressBook();

                console.log('Address loaded:', address);
            },

            formatAddress(address) {
                return address.map(g => String(g).padStart(2, '0')).join('-');
            },

            getGlyphImagePath(glyphNumber) {
                return `glyphs/${String(glyphNumber).padStart(3, '0')}.svg`;
            },

            setupKeyboardListeners() {
                document.addEventListener('keydown', (e) => {
                    // Don't handle keys if a modal is open or typing in input
                    if (e.target.tagName === 'INPUT') {
                        return;
                    }

                    switch (e.key.toLowerCase()) {
                        case 'enter':
                            if (this.canDial) {
                                this.startDialing();
                            }
                            break;
                        case 'escape':
                            if (this.showAddressBook) {
                                this.closeAddressBook();
                            } else if (this.dialingInProgress) {
                                this.abortDialing();
                            } else if (this.selectedAddress.length > 0) {
                                this.clearAddress();
                            }
                            break;
                        case 'backspace':
                            if (!this.showAddressBook) {
                                this.removeLastGlyph();
                            }
                            break;
                        case 'b':
                            if (!this.showAddressBook) {
                                this.openAddressBook();
                            }
                            break;
                    }
                });

                console.log('Keyboard controls active');
                console.log('  ENTER - Start dialing');
                console.log('  BACKSPACE - Remove last glyph');
                console.log('  B - Open address book');
                console.log('  ESCAPE - Clear address / Close address book / Abort dialing');
            },

            addBackgroundEffects() {
                setInterval(() => {
                    if (Math.random() > 0.95 && !this.dialingInProgress) {
                        const unselectedGlyphs = this.glyphs.filter(g => !g.selected);
                        if (unselectedGlyphs.length > 0) {
                            const randomGlyph = unselectedGlyphs[Math.floor(Math.random() * unselectedGlyphs.length)];
                            const element = document.querySelector(`[data-glyph="${randomGlyph.number}"] .glyph-button`);
                            if (element) {
                                element.style.borderColor = 'rgba(0, 212, 255, 0.6)';
                                setTimeout(() => {
                                    element.style.borderColor = '';
                                }, 300);
                            }
                        }
                    }
                }, 500);
            },

            async sendDialRequest() {
                try {
                    const response = await fetch(`${API_URL}/dial`, {
                        method: 'POST',
                        headers: {
                            'Content-Type': 'application/json'
                        },
                        body: JSON.stringify({
                            address: this.selectedAddress,
                            galaxy: 'universe'
                        })
                    });

                    if (response.ok) {
                        console.log('Dial request sent successfully');
                    } else {
                        console.error('Dial request failed:', response.status);
                    }
                } catch (error) {
                    console.error('Error sending dial request:', error);
                }
            }
        },
        template: `
            <div class="device-container">
                <div class="device-frame">
                    <!-- Top Display Panel -->
                    <div class="display-panel">
                        <!-- Status Indicators -->
                        <div class="status-bar">
                            <div class="status-indicator" :class="{active: !dialingInProgress}"></div>
                            <div class="status-text">REMOTE DIALING DEVICE</div>
                            <div class="status-indicator" :class="{active: dialingInProgress}"></div>
                        </div>

                        <!-- Locked Glyphs Display -->
                        <div class="locked-glyphs">
                            <div v-for="position in 9" :key="position"
                                 class="locked-glyph-slot"
                                 :class="{filled: position <= selectedAddress.length}"
                                 :data-position="position">
                                <img v-if="getLockedGlyphImage(position)"
                                     :src="getLockedGlyphImage(position)"
                                     :alt="'Glyph ' + position">
                                <div v-else class="glyph-placeholder"></div>
                            </div>
                        </div>

                        <!-- Circular Glyph Selector -->
                        <div class="glyph-circle">
                            <div class="circle-track"></div>

                            <!-- Center Info Display -->
                            <div class="center-display">
                                <div class="destination-info">
                                    <div class="info-label">DESTINATION</div>
                                    <div class="info-value">{{ destinationText }}</div>
                                </div>
                                <div class="dial-progress">
                                    <div class="progress-label">ADDRESS</div>
                                    <div class="progress-bar">
                                        <div class="progress-fill" :style="{width: progress + '%'}"></div>
                                    </div>
                                    <div class="progress-text">{{ progressText }}</div>
                                </div>
                            </div>

                            <!-- Glyphs arranged in circle -->
                            <div class="glyph-ring" ref="glyphRing">
                                <div v-for="glyph in glyphs"
                                     :key="glyph.number"
                                     class="glyph-item"
                                     :class="{selected: glyph.selected}"
                                     :data-glyph="glyph.number"
                                     :style="{transform: 'translate(' + glyph.x + 'px, ' + glyph.y + 'px) rotate(' + glyph.rotation + 'deg)'}"
                                     @click="selectGlyph(glyph)">
                                    <div class="glyph-button">
                                        <img :src="glyph.image" :alt="'Glyph ' + glyph.number">
                                    </div>
                                </div>
                            </div>
                        </div>
                    </div>

                    <!-- Control Buttons -->
                    <div class="control-panel">
                        <button class="control-btn dial-btn"
                                :disabled="!canDial"
                                @click="startDialing">
                            <span class="btn-icon">⚡</span>
                            <span class="btn-label">DIAL</span>
                        </button>
                        <button class="control-btn book-btn"
                                @click="openAddressBook">
                            <span class="btn-icon">📖</span>
                            <span class="btn-label">BOOK</span>
                        </button>
                        <button class="control-btn clear-btn"
                                :disabled="!canClear"
                                @click="clearAddress">
                            <span class="btn-icon">✕</span>
                            <span class="btn-label">CLEAR</span>
                        </button>
                        <button class="control-btn abort-btn"
                                :disabled="!canAbort"
                                @click="abortDialing">
                            <span class="btn-icon">◼</span>
                            <span class="btn-label">ABORT</span>
                        </button>
                    </div>

                    <!-- Status Message -->
                    <div class="status-message" :class="statusType">
                        <div class="message-text">{{ statusMessage }}</div>
                    </div>
                </div>

                <!-- Address Book Modal -->
                <div v-if="showAddressBook" class="modal-overlay" @click.self="closeAddressBook">
                    <div class="modal-content">
                        <div class="modal-header">
                            <h2>ADDRESS BOOK</h2>
                            <button class="modal-close" @click="closeAddressBook">✕</button>
                        </div>
                        <div class="modal-body">
                            <div v-if="addressBook.length > 0" class="address-hint">
                                Click on an address to load it
                            </div>
                            <div class="address-list">
                                <div v-if="addressBook.length === 0" class="empty-state">
                                    No addresses available
                                </div>
                                <div v-else>
                                    <div v-for="address in addressBook"
                                         :key="address.id"
                                         class="address-item"
                                         @click="loadAddress(address)">
                                        <div class="address-header">
                                            <h3>{{ address.name }}</h3>
                                            <span class="address-length">{{ address.length }} glyphs</span>
                                        </div>
                                        <div class="address-glyphs">
                                            <img v-for="(glyphNumber, index) in address.address"
                                                 :key="index"
                                                 :src="getGlyphImagePath(glyphNumber)"
                                                 :alt="'Glyph ' + glyphNumber"
                                                 class="address-glyph-icon">
                                        </div>
                                    </div>
                                </div>
                            </div>
                        </div>
                    </div>
                </div>
            </div>
        `
    });

    console.log('Stargate Universe Dial Device - Vue.js version loaded');
} else {
    console.error('Vue.js not found. Please ensure vue.min.js is loaded.');
}
