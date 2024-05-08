let currentData =
{
    settings_entries: []
};

var currentApp = new Vue({
  el: '#app',
  data: currentData,
  mounted: function () {
    this.$nextTick(function () {
      // Get system information
      console.log("loading system info");
      getData(apiControlURLs.settingsjson,
      {
          success: data => {
            currentApp.settings_entries = data.entries;
          }
      });
    })
  }
})
