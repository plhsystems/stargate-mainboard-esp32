let currentData =
{
  // Default values, mostly to shut-up vuejs
  status: { text: "", cancel_request: false, error_text: "", is_error: false, time: null, ring: { is_connected: false } },
	is_connected: false,

  ramp_perc: 0.0,
  servo_perc: 0.0,

  galaxy_info: { name: "", addresses: [], symbols: [], ring_animations: [], wormhole_types: [] },
  sys_info: { infos: [] },
  sound_list: { files: [] },

  fangate_list: [],

  selectedSoundFileID: null,
  selectedRingAnimationID: null,
  selectedWormholeTypeID: null
};

// WebSocket connection
let ws = null;
let wsReconnectTimeout = null;
let wsPollInterval = null;

var currentApp = new Vue({
  el: '#app',
  data: currentData,
  mounted: function () {
    this.$nextTick(function () {
      console.log("page is fully loaded");
      // Start WebSocket connection instead of polling
      connectWebSocket();

      // Get system information
      console.log("loading system info");
      getData(apiControlURLs.get_sysinfo,
      {
          success: data => {
            currentApp.sys_info = data;
          }
      });

      // Code that will run only after the
      // entire view has been rendered
      console.log("loading universe infos");
      getData(apiControlURLs.getinfo_universe,
      {
          success: data => {
            currentData.galaxy_info = data;
          },
          fail: ex => {
            currentData.galaxy_info = null;
          }
      });

      // Fan gate list
      console.log("loading fan gates list");
      getData(apiControlURLs.getfangatelist_milkyway,
      {
          success: data => {
            console.log("fangate_list", data);
            currentApp.fangate_list = data.sort( (a, b) => 
            {
              if (a.status < b.status) {
                return 1;
              }
              else if (a.status > b.status) {
                return -1;
              }
              return a.name.localeCompare(b.name);
            });
          }
      });

      // Sound list
      console.log("loading sound list");
      getData(apiControlURLs.sound_list,
      {
          success: data => {
            currentApp.sound_list = data;
          }
      });
    })
  }
})

function connectWebSocket() {
  // Clear any existing reconnect timeout
  if (wsReconnectTimeout) {
    clearTimeout(wsReconnectTimeout);
    wsReconnectTimeout = null;
  }

  // Clear any existing poll interval
  if (wsPollInterval) {
    clearInterval(wsPollInterval);
    wsPollInterval = null;
  }

  // Determine WebSocket URL (ws:// or wss:// based on current protocol)
  const protocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
  const wsUrl = `${protocol}//${window.location.host}/ws`;

  console.log('Connecting to WebSocket:', wsUrl);

  try {
    ws = new WebSocket(wsUrl);

    ws.onopen = function(event) {
      console.log('WebSocket connected');
      currentData.is_connected = true;

      // Start polling for status updates over WebSocket
      // Send first request immediately
      if (ws && ws.readyState === WebSocket.OPEN) {
        ws.send('get_status');
      }

      // Then poll every 250ms (4 Hz)
      wsPollInterval = setInterval(function() {
        if (ws && ws.readyState === WebSocket.OPEN) {
          ws.send('get_status');
        }
      }, 250);
    };

    ws.onmessage = function(event) {
      try {
        const data = JSON.parse(event.data);
        // Update status with received data
        currentData.status = data.status;
        currentData.is_connected = true;
      } catch (ex) {
        console.error('Failed to parse WebSocket message:', ex);
      }
    };

    ws.onerror = function(error) {
      console.error('WebSocket error:', error);
      currentData.is_connected = false;
    };

    ws.onclose = function(event) {
      console.log('WebSocket closed, reconnecting in 5 seconds...');
      currentData.is_connected = false;
      ws = null;

      // Stop polling
      if (wsPollInterval) {
        clearInterval(wsPollInterval);
        wsPollInterval = null;
      }

      // Attempt to reconnect after 5 seconds
      wsReconnectTimeout = setTimeout(connectWebSocket, 5000);
    };
  } catch (ex) {
    console.error('Failed to create WebSocket:', ex);
    currentData.is_connected = false;
    // Retry connection after 5 seconds
    wsReconnectTimeout = setTimeout(connectWebSocket, 5000);
  }
}
