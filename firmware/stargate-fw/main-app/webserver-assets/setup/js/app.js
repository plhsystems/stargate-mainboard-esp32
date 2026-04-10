/* ═══════════════════════════════════════════════════════════════════════
   Stargate Ancient Control Interface — Vue.js App
   Setup page: Dial / Manual / System / Network tabs
   ═══════════════════════════════════════════════════════════════════════ */

const GLYPH_COUNT = 36;
const RING_R   = 204;   // radius in px
const RING_CX  = 240;   // container center x
const RING_CY  = 240;   // container center y
const GLYPH_H  = 19;    // half of glyph button size (38px)

let ws                = null;
let wsReconnectTimer  = null;
let wsPollTimer       = null;

/* ─── Shared reactive data ─── */
const appData = {
  is_connected: false,
  activeTab: 'dial',

  status: {
    text: 'CONNECTING...',
    cancel_request: false,
    error_text: '',
    is_error: false,
    time: null,
    ring: { is_connected: false }
  },

  /* Dial */
  selectedGlyphs: [],

  /* Manual */
  ramp_perc:             0.0,
  servo_perc:            0.5,
  selectedSoundFileID:   null,
  selectedWormholeTypeID: null,
  selectedRingAnimationID: null,

  /* Loaded data */
  galaxy_info:      { name: '', addresses: [], symbols: [], ring_animations: [], wormhole_types: [] },
  sys_info:         { infos: [] },
  sound_list:       { files: [] },
  fangate_list:     [],
  fangate_loading:  false,
  settings_entries: [],
  settings_save_status: '',
  settings_save_msg: '',

  tabs: [
    { id: 'dial',     label: 'DIAL ADDRESS',   icon: '⬡' },
    { id: 'manual',   label: 'MANUAL CONTROL', icon: '⚙' },
    { id: 'system',   label: 'SYSTEM INFO',    icon: '◈' },
    { id: 'network',  label: 'NETWORK',        icon: '◉' },
    { id: 'settings', label: 'SETTINGS',       icon: '⚛' },
  ]
};

/* ─── Vue instance ─── */
const app = new Vue({
  el: '#app',
  data: appData,

  computed: {
    /* 36 glyph descriptors */
    glyphs() {
      return Array.from({ length: GLYPH_COUNT }, (_, i) => ({
        id: i + 1,
        src: this.glyphSrc(i + 1)
      }));
    },

    /* Always 9 slots; null means empty */
    addrSlots() {
      return Array.from({ length: 9 }, (_, i) =>
        i < this.selectedGlyphs.length ? this.selectedGlyphs[i] : null
      );
    },

    canDial() {
      const n = this.selectedGlyphs.length;
      return n >= 6 && n <= 9;
    },

    isDialing() {
      const t = (this.status.text || '').toLowerCase();
      return t.includes('dial') || t.includes('wormhole');
    },

    stateClass() {
      if (!this.is_connected)      return 's-idle';
      if (this.status.is_error)    return 's-error';
      const t = this.status.text || '';
      if (t === 'Idle' || t === '') return 's-idle';
      return 's-active';
    }
  },

  mounted() {
    this.$nextTick(() => {
      connectWS();
      this.loadAll();
      this.updateRingScale();
      window.addEventListener('resize', this.updateRingScale);
    });
  },

  methods: {
    /* ── Ring scale to fill available space ── */
    updateRingScale() {
      this.$nextTick(() => {
        const container = this.$el ? this.$el.querySelector('.ring-container') : null;
        if (!container) return;
        const avail = Math.min(container.clientHeight, container.clientWidth);
        if (0 >= avail) return;
        const scale = Math.min(2.0, Math.max(0.4, avail / 480));
        document.documentElement.style.setProperty('--rs', scale);
      });
    },

    /* ── Glyph ring position ── */
    gpos(index) {
      const angle = (index / GLYPH_COUNT) * 2 * Math.PI - Math.PI / 2;
      return {
        left: (RING_CX + RING_R * Math.cos(angle) - GLYPH_H) + 'px',
        top:  (RING_CY + RING_R * Math.sin(angle) - GLYPH_H) + 'px'
      };
    },

    glyphSrc(id) {
      return `/galaxies/universe/glyphs/${String(id).padStart(3, '0')}.svg`;
    },

    fanGlyphSrc(id) {
      return `/galaxies/milkyway/glyphs/${String(id).padStart(3, '0')}.svg`;
    },

    parseFanAddr(addr) {
      if (Array.isArray(addr)) return addr.filter(n => n > 0);
      if (!addr) return [];
      try { const p = JSON.parse(addr); if (Array.isArray(p)) return p.filter(n => n > 0); } catch(e) {}
      return String(addr).replace(/[\[\]]/g, '').split(/[\s,\-]+/).map(Number).filter(n => n > 0);
    },

    /* ── Dial actions ── */
    pickGlyph(id) {
      if (this.selectedGlyphs.length >= 9) return;
      this.selectedGlyphs.push(id);
    },
    removeAt(i) {
      this.selectedGlyphs.splice(i, 1);
    },
    clearAddr() {
      this.selectedGlyphs = [];
    },
    loadAddr(address) {
      this.selectedGlyphs = address.slice(0, 9);
    },
    dial() {
      if (!this.canDial || !this.is_connected) return;
      sendAction(apiControlURLs.control_dialaddress, { addr: this.selectedGlyphs });
    },
    abort() {
      sendAction(apiControlURLs.control_abort, {});
    },

    /* ── Manual actions ── */
    doRingFactory() {
      if (confirm('Factory reset the ring controller?')) {
        sendAction(apiControlURLs.ringcontrol_gotofactory, {});
      }
    },

    /* ── System tab ── */
    refreshSys() {
      getData(apiControlURLs.get_sysinfo, {
        success: d => { appData.sys_info = d; }
      });
    },

    /* ── Fan gate list ── */
    refreshFanGates() {
      appData.fangate_loading = true;
      getData(apiControlURLs.getfangatelist_milkyway, {
        success: d => {
          const list = Array.isArray(d) ? d : [];
          appData.fangate_list = list.sort(
            (a, b) => b.status - a.status || a.name.localeCompare(b.name)
          );
          appData.fangate_loading = false;
        },
        fail: () => { appData.fangate_loading = false; }
      });
    },

    /* ── Settings save ── */
    saveSettings() {
      const payload = JSON.stringify({ entries: this.settings_entries });
      fetch(apiControlURLs.settingsjson, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: payload
      }).then(r => {
        if (r.ok) {
          appData.settings_save_status = 'OK';
          appData.settings_save_msg    = 'SAVED';
        } else {
          appData.settings_save_status = 'ERR';
          appData.settings_save_msg    = 'ERROR ' + r.status;
        }
      }).catch(() => {
        appData.settings_save_status = 'ERR';
        appData.settings_save_msg    = 'REQUEST FAILED';
      });
    },

    /* ── Initial data load ── */
    loadAll() {
      getData(apiControlURLs.getinfo_universe, {
        success: d => { appData.galaxy_info = d; },
        fail:    () => {}
      });
      getData(apiControlURLs.get_sysinfo, {
        success: d => { appData.sys_info = d; }
      });
      getData(apiControlURLs.sound_list, {
        success: d => { appData.sound_list = d; }
      });
      this.refreshFanGates();
      getData(apiControlURLs.settingsjson, {
        success: d => { appData.settings_entries = d.entries; }
      });
    }
  }
});

/* ═══════════════════════════ WebSocket ════════════════════════════════ */
function connectWS() {
  if (wsReconnectTimer) { clearTimeout(wsReconnectTimer);  wsReconnectTimer = null; }
  if (wsPollTimer)      { clearInterval(wsPollTimer);       wsPollTimer      = null; }

  const proto = location.protocol === 'https:' ? 'wss:' : 'ws:';
  try {
    ws = new WebSocket(`${proto}//${location.host}/ws`);

    ws.onopen = () => {
      appData.is_connected = true;
      ws.send('get_status');
      wsPollTimer = setInterval(() => {
        if (ws && ws.readyState === WebSocket.OPEN) ws.send('get_status');
      }, 250);
    };

    ws.onmessage = ev => {
      try {
        const d = JSON.parse(ev.data);
        appData.status       = d.status;
        appData.is_connected = true;
      } catch(e) {}
    };

    ws.onerror  = ()  => { appData.is_connected = false; };

    ws.onclose  = ()  => {
      appData.is_connected = false;
      ws = null;
      if (wsPollTimer) { clearInterval(wsPollTimer); wsPollTimer = null; }
      wsReconnectTimer = setTimeout(connectWS, 5000);
    };
  } catch(e) {
    appData.is_connected = false;
    wsReconnectTimer = setTimeout(connectWS, 5000);
  }
}
