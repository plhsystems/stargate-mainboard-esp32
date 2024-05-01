function sendAction(actionURL, data)
{
  console.log("SendAction", actionURL, data);

  var xhr = new XMLHttpRequest();
  xhr.open("POST", actionURL, true);
  if (data) {
    xhr.setRequestHeader('Content-Type', 'application/json');
    xhr.send(JSON.stringify(data));
  }
  else {
    xhr.setRequestHeader('Content-Type', 'application/json');
    xhr.send(JSON.stringify({}));
  }
}

function getData(actionURL, cb)
{
  fetch(actionURL)
    .then(response => {
      if (!response.ok) {
        throw new Error('Network response was not ok');
      }
      return response.json();
    })
    .then(data => {
      // Handle the JSON data here
      if (cb && cb.success) {
        cb.success(data);
      }
    })
    .catch(error => {
      // Handle any errors here
      console.log('Fetch error:', error);
      if (cb && cb.fail) {
        cb.fail(error);
      }
    });
}

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

var currentApp = new Vue({
  el: '#app',
  data: currentData,
  mounted: function () {
    this.$nextTick(function () {
      console.log("page is fully loaded");
      setTimeout(timerHandler, 500);

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

async function timerHandler() {

  // Get system informations
  await fetch(apiControlURLs.get_status)
      .then((response) => {
          if (!response.ok) {
              throw new Error('Unable to get status');
          }
          return response.json();
      })
      .then((data) =>
      {
        currentData.status = data;
        currentData.is_connected = true;
        setTimeout(timerHandler, 500);
      })
      .catch((ex) =>
      {
          setTimeout(timerHandler, 5000);
          currentData.is_connected = false;
          console.error('getstatus', ex);
      });
}
