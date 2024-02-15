function SendAction(actionURL, data)
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

let currentData =
{
	is_connected: false,

  ramp_perc: 0.0,
};

var currentApp = new Vue({
  el: '#app',
  data: currentData
})

async function timerHandler() {

  // Get system informations
  await fetch('/api/getstatus')
      .then((response) => {
          if (!response.ok) {
              throw new Error('Unable to get status');
          }
          return response.json();
      })
      .then((data) =>
      {
        console.log("data: ", data);
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

window.addEventListener(
  "load",
  (event) => {
    console.log("page is fully loaded");
    setTimeout(timerHandler, 500);
});
