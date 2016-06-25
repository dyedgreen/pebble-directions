// Api keys
var hereAppId = 'O9qHlTv2c773rK1cC7Iw';
var hereAppCode = 'vjQQRbJewPVjPhPuNZOclQ';


// Make a http request and return the recived json to the callback (callback params: success / json)
function makeJsonHttpGetRequest(url, callback, logResponseText) {
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
    // Debugging helper stuff
    if (logResponseText) {
      console.log(this.responseText);
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
    { enableHighAccuracy: true, timeout: 10000, maximumAge: 30000 }
  );
}

// Load a geolocation from here and return the found lon/lat to the callback (callback params: success / lat / lon)
function loadLocationForSearch(searchText, currentLat, currentLon, callback) {
  // Create the url
  var url = 'https://geocoder.api.here.com/6.2/geocode.json?app_id=';
    url = url.concat(hereAppId);
    url = url.concat('&app_code=').concat(hereAppCode);
    url = url.concat('&gen=9'); /*don't break on here api update*/
    url = url.concat('&searchtext=').concat(searchText.split(' ').join('+'));
    url = url.concat('&prox=').concat(currentLat).concat(',').concat(currentLon).concat(',150000'); /* favour results within 150 km range */
  // Perform an request
  makeJsonHttpGetRequest(url, function(success, res) {
    if (success) {
      // Success (will fail if expected fields are not available in response)
      try {
        var foundLat = res.Response.View[0].Result[0].Location.NavigationPosition[0].Latitude;
        var foundLon = res.Response.View[0].Result[0].Location.NavigationPosition[0].Longitude;
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
function loadRouteData(routeType, fromLat, fromLon, toLat, toLon, callback) {
  // Transport modes, mapped to pebble ids
  var modes = ['car', 'bicycle', 'publicTransport', 'pedestrian'];
  // Pebble direction icons, mapped to pebble ids (type = show nav type icon, attr = show attribution icon)
  var icons = {
    type: 'a',
    forward: 'b',
    right: 'c',
    left: 'd',
    uRight: 'e',
    uLeft: 'f',
    attr: 'g',
  };
  // Maps the direction retrived from here api to icon name
  var directionMap = {
    forward: 'forward',
    bearRight: 'right', lightRight: 'right', right: 'right', hardRight: 'right',
    bearLeft: 'left', lightLeft: 'left', left: 'left', hardLeft: 'left',
    uTurnRight: 'uRight',
    uTurnLeft: 'uLeft',
  };

  // Create the url
  var url = 'https://route.api.here.com/routing/7.2/calculateroute.json?app_id=';
    url = url.concat(hereAppId);
    url = url.concat('&app_code=').concat(hereAppCode);
    url = url.concat('&gen=9'); /*don't break on here api update*/
    url = url.concat('&waypoint0=geo!').concat(fromLat).concat(',').concat(fromLon);
    url = url.concat('&waypoint1=geo!').concat(toLat).concat(',').concat(toLon);
    url = url.concat('&mode=fastest;').concat(modes[routeType]); /* selectes the transit method, e.g. teleportation / car / ... */
    // Format the response
    url = url.concat('&routeattributes=none,summary,legs&routelegattributes=none,maneuvers&maneuverattributes=none,direction&instructionformat=text');
  // Perform the request
  makeJsonHttpGetRequest(url, function(success, res) {
    if (success) {
      // Success (will fail if expected fields are not available in response)
      try {
        // Our route data will go here. Format: { distance, time, stepList[string], stepIconsString[int] }
        var routeData = {};
        // Get the summary
        routeData.distance = res.response.route[0].summary.distance; /* in meters */
        routeData.time = Math.ceil(res.response.route[0].summary.travelTime / 60); /* in minutes */
        // Get the steps
        routeData.stepList = [];
        routeData.stepIconsString = '';
        res.response.route[0].leg[0].maneuver.forEach(function(step, index) {
          // Add the text
          routeData.stepList[index] = step.instruction;
          // Add the icon
          if (step.hasOwnProperty('direction')) {
            // Display the specified icon
            if (directionMap.hasOwnProperty(step.direction)) {
              routeData.stepIconsString = routeData.stepIconsString.concat(icons[directionMap[step.direction]]);
            } else {
              // Display the travel type icon
              routeData.stepIconsString = routeData.stepIconsString.concat(icons['type']);
            }
          } else {
            // Display the travel type icon
            routeData.stepIconsString = routeData.stepIconsString.concat(icons['type']);
          }
        });
        // Test for attribution
        if (res.response.hasOwnProperty('sourceAttribution')) {
          // Add attribution text as last list entry, removing all html markup from it
          routeData.stepList.push(res.response.sourceAttribution.attribution.replace(/<.+?>/g, ''));
          routeData.stepIconsString = routeData.stepIconsString.concat(icons['attr']);
        }
        // We are done
        callback(true, routeData);
      } catch (e) {
        routeErrorCallback(callback);
      }
    } else {
      // Error
      routeErrorCallback(callback);
    }
  });
}

// Performs all the steps neccessary to return a complete route (the callback takes: success / route data)
function createRoute(routeType, searchText, callback) {
  // Load the current location
  loadCurrentLocation(function(successCurrentLocation, fromLat, fromLon) {
    console.log('current found:', fromLat, fromLon);
    if (successCurrentLocation) {
      // Geocode the search term
      loadLocationForSearch(searchText, fromLat, fromLon, function(successSearchLocation, toLat, toLon) {
        console.log('search found:', toLat, toLon);
        if (successSearchLocation) {
          // Load a route and pass it the callback
          loadRouteData(routeType, fromLat, fromLon, toLat, toLon, callback);
        } else {
          routeErrorCallback(callback);
        }
      });
    } else {
      routeErrorCallback(callback);
    }
  });
}
// Helper function to define createRoute error callback all in one place
function routeErrorCallback(callback) {
  callback(false, { distance: 0, time: 0, stepList: [], stepIconsString: '' });
}


// Exports
module.exports.createRoute = createRoute;
