const apiControlURLs =
{
    get_status:     "/api/getstatus",
    get_sysinfo:    "/api/getsysinfo",

    sound_list: "/api/sound/list",
    sound_play: "/api/sound/play",
    sound_stop: "/api/sound/stop",

    control_autohome:           "/api/control/autohome",
    control_autocalibrate:      "/api/control/autocalibrate",
    control_keypress:           "/api/control/keypress",
    control_dialaddress:        "/api/control/dialaddress",
    control_manualwormhole:     "/api/control/manualwormhole",
    control_abort:              "/api/control/abort",

    control_testramplight:  "/api/control/testramplight",
    control_testservo:      "/api/control/testservo",

    ringcontrol_poweroff:       "/api/ringcontrol/poweroff",
    ringcontrol_testanimate:    "/api/ringcontrol/testanimate",
    ringcontrol_gotofactory:    "/api/ringcontrol/gotofactory",

    settingsjson:           "/api/settingsjson",
    getfreertosdbginfo:     "/api/getfreertosdbginfo",

    getinfo_milkyway:       "/api/galaxy/getinfo/milkyway",
    getinfo_pegasus:        "/api/galaxy/getinfo/pegasus",
    getinfo_universe:       "/api/galaxy/getinfo/universe",

    getfangatelist_milkyway: "/api/getfangatelist/milkyway",
}

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
