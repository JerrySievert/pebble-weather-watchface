var forecastIOKey = "YOUR_API_KEY";

// make it easy to convert an icon to a number for the pebble side
var icons = [
  'clear-day',
  'clear-night',
  'cloudy',
  'fog',
  'partly-cloudy-day',
  'partly-cloudy-night',
  'rain',
  'sleet',
  'snow',
  'wind',
  'error'
];


function getAndShowWeather ( ) {
  navigator.geolocation.getCurrentPosition(function (position) {
    // position.coords.latitude, position.coords.longitude
    getCurrentWeather(position.coords.longitude, position.coords.latitude);
  });

  setTimeout(getAndShowWeather, 300000);
}

function getCurrentWeather (lon, lat) {
  var req = new XMLHttpRequest();
    req.open('GET',"https://api.forecast.io/forecast/" + forecastIOKey + "/" + lat + "," + lon, true);
    req.onload = function(e) {
      if (req.readyState == 4 && req.status == 200) {
        if(req.status == 200) {
          var response = JSON.parse(req.responseText);

          var send = { };
          if (response.currently) {
            var icon = icons.indexOf(response.currently.icon);

            // if the icon isn't found, default to error
            if (icon === -1) {
              icon = 10;
            }

            send.icon = icon;
            send.temperature_f = Number(response.currently.temperature).toFixed(0);
            send.temperature_c = Number(FtoC(response.currently.temperature)).toFixed(0);
          }

          Pebble.sendAppMessage(send);
        } else {
          console.log("Error");
        }
      }
    };

    req.send(null);
}

function FtoC (f) {
  return (f - 32) * 5 / 9;
}

Pebble.addEventListener("ready",
  function(e) {
    setTimeout(getAndShowWeather, 2000);
  }
);
