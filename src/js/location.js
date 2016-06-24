// Api keys
var hereAppId = 'O9qHlTv2c773rK1cC7Iw';
var hereAppCode = 'vjQQRbJewPVjPhPuNZOclQ';


// Make a http request and return the recived json to the callback (callback params: success / json)
function makeJsonHttpGetRequest(url, callback) {
  // Create a request
  var request = new XMLHttpRequest();
  // Add a two minute timeout
  request.timeout = 120000;
  // Set up the requests callbacks
  request.onload = function() {
    // Send data to callback / error if json can not be parsed
    try {
      var data = JSON.parse(this.responseText);
      callback(true, data);
    } catch (e) {
      callback(false, null);
    }
  };
  request.onerror = function() {
    callback(false, null);
  };
  request.ontimeout = function() {
    callback(false, null);
  };
  // Fire the request
  request.open('GET', url);
  request.send();
}

// Return the current position to the callback (callback params: success / lat / lon)
function loadCurrentLocation(callback) {
  navigator.geolocation.getCurrentPosition(
    // Success
    function(pos) {
      callback(true, pos.coords.latitude, pos.coords.longitude);
    },
    // Error
    function() {
      callback(false, 0, 0);
    },
    // Options
    { enableHighAccuracy: false, timeout: 10000, maximumAge: 30000 }
  );
}

// Load a geolocation from here and return the found lon/lat to the callback (callback params: success / lat / lon)
function loadLocationForSearch(searchText, currentLat, currentLon, callback) {
  // Create the url
  var url = 'https://geocoder.api.here.com/6.2/geocode.json?app_id=';
    url = url.concat(hereAppId);
    url = url.concat('&app_code=').concat(hereAppCode);
    url = url.concat('&gen=9');
    url = url.concat('&searchtext=').concat(searchText.split(' ').join('+'));
    url = url.concat('&prox=').concat(currentLat).concat(',').concat(currentLon).concat(',150000'); /* favour results within 150 km range */
  // Perform an request
  makeJsonHttpGetRequest(url, function(success, res) {
    if (success) {
      // Success (will fail if expected fields are not available in response)
      try {
        var foundLat = res.Response.View[0].Result[0].Location.NavigationPosition.Latitude;
        var foundLon = res.Response.View[0].Result[0].Location.NavigationPosition.Longitude;
        callback(true, foundLat, foundLon);
      } catch (e) {
        callback(false, 0, 0);
      }
    } else {
      // Error
      callback(false, 0, 0);
    }
  });
}

// Load a route from here and return the pebble-app conform data to the callback (callback params: success / data)
function loadRouteData(mode, fromLat, fromLon, toLat, toLon, callback) {
  var modes = ['car', 'bike', 'train', 'walk']; //FIXME map to corret here maps keywords!
}


// Exports
module.exports.createRoute = function() {}; // FIXME
