// Api keys
var hereApiKey = require('./apiKeys.js').hereApiKey;
var mapQuestKey = require('./apiKeys.js').mapQuestKey;


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
function loadCurrentLocation(callback, retry) {
  navigator.geolocation.getCurrentPosition(
    // Success
    function(pos) {
      try {
        //callback(true, 52.5, 13.4); /* THIS IS FOR DEMO / SCREENSHOTS IN THE EMULATOR */
        if (pos.coords.accuracy <= 100 || retry !== true) {
          // Accuracy is good, use this location OR we should not retry!
          callback(true, pos.coords.latitude, pos.coords.longitude);
        } else if (retry === true) {
          // Try once more
          console.log('Will try to reload current location, accuracy:', pos.coords.accuracy);
          loadCurrentLocation(callback, false);
        }
      } catch (e) {
        // No location permission
        callback(false, 0, 0);
      }
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
  var url = 'https://geocoder.ls.hereapi.com/6.2/geocode.json?apiKey=';
    url = url.concat(hereApiKey);
    url = url.concat('&gen=9'); /*don't break on here api update*/
    url = url.concat('&searchtext=').concat(encodeURI(searchText.split(' ').join('+')));
    url = url.concat('&prox=').concat(currentLat).concat(',').concat(currentLon).concat(',50000'); /* favour results within 50 km range */
  // Perform an request
  makeJsonHttpGetRequest(url, function(success, res) {
    if (success) {
      // Success (will fail if expected fields are not available in response)
      try {
        var foundLat = res.Response.View[0].Result[0].Location.NavigationPosition[0].Latitude;
        var foundLon = res.Response.View[0].Result[0].Location.NavigationPosition[0].Longitude;
        //callback(true, 52.5, 13.45); /* THIS IS FOR DEMO / SCREENSHOTS IN THE EMULATOR (add 'Schleusenufer' on watch as address string) */
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
    final: 'h',
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
  var url = 'https://route.ls.hereapi.com/routing/7.2/calculateroute.json?apiKey=';
    url = url.concat(hereApiKey);
    url = url.concat('&gen=9'); /*don't break on here api update*/
    url = url.concat('&waypoint0=geo!').concat(fromLat).concat(',').concat(fromLon);
    url = url.concat('&waypoint1=geo!').concat(toLat).concat(',').concat(toLon);
    url = url.concat('&mode=fastest;').concat(modes[routeType]); /* selectes the transit method, e.g. teleportation / car / ... */
    if (modes[routeType] == 'publicTransport') {
      // Special url params for public transport routes
      url = url.concat('&departure=now&combineChange=true');
    }
    // Format the response
    url = url.concat('&routeattributes=none,summary,legs&routelegattributes=none,maneuvers&maneuverattributes=none,direction,position&instructionformat=text');
    // Log the final url (for rare use)
    //console.log(url);
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
        routeData.stepPositionList = [];
        routeData.stepIconsString = '';
        res.response.route[0].leg[0].maneuver.forEach(function(step, index) {
          // Add the text
          routeData.stepList[index] = step.instruction;
          // Add the position
          routeData.stepPositionList[index] = {
            lat: step.position.latitude,
            lon: step.position.longitude,
          };
          // Add the icon
          if (res.response.route[0].leg[0].maneuver.length == index + 1) {
            // This is the last step, add the finished icon
            routeData.stepIconsString = routeData.stepIconsString.concat(icons['final']);
          } else if (step.hasOwnProperty('direction')) {
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

// Load a route from the MapQuest PublicTransit API
function loadPublicTransitRouteData(fromLat, fromLon, toLat, toLon, callback) {
  // Pebble direction icons, mapped to pebble ids (type = show nav type icon, attr = show attribution icon)
  var icons = {
    type: 'a',
    forward: 'b',
    right: 'c',
    left: 'd',
    uRight: 'e',
    uLeft: 'f',
    attr: 'g',
    final: 'h',
  };
  // Maps the direction retrived from here api to icon name (-1 == final)
  var directionMap = [
    'forward',
    'right',
    'right',
    'right',
    'uRight',
    'left',
    'left',
    'left',
    'uRight',
    'uLeft',
    'right',
    'left',
    'right',
    'left',
    'right',
    'left',
    'right',
    'left',
    'forward',
    'type',
    'type',
    'type',
    'type'
  ];

  // Create the url
  var url = 'http://www.mapquestapi.com/directions/v2/route?key=';
    url = url.concat(mapQuestKey);
    url = url.concat('&from=').concat(fromLat).concat(',').concat(fromLon);
    url = url.concat('&to=').concat(toLat).concat(',').concat(toLon);
    url = url.concat('&routeType=multimodal&maxWalkingDistance=500&timeType=1')
    // Format the response
    url = url.concat('&unit=k&doReverseGeocode=false&narrativeType=text&locale=en_US&outFormat=json');
    // Log the final url (for rare use)
    //console.log(url);
  // Perform the request
  makeJsonHttpGetRequest(url, function(success, res) {
    if (success) {
      // Success (will fail if expected fields are not available in response)
      try {
        // Our route data will go here. Format: { distance, time, stepList[string], stepIconsString[int] }
        var routeData = {};
        // Get the summary
        routeData.distance = Math.ceil(res.route.distance * 1000); /* in meters */
        routeData.time = Math.ceil(res.route.time / 60); /* in minutes */
        // Get the steps
        routeData.stepList = [];
        routeData.stepPositionList = [];
        routeData.stepIconsString = '';
        res.route.legs[0].maneuvers.forEach(function(step, index) {
          // Add the text
          routeData.stepList[index] = step.narrative;
          // Add the position
          routeData.stepPositionList[index] = {
            lat: step.startPoint.lat,
            lon: step.startPoint.lng,
          };
          // Add the icon
          if (res.route.legs[0].maneuvers.length == index + 1) {
            // This is the last step, add the finished icon
            routeData.stepIconsString = routeData.stepIconsString.concat(icons['final']);
          } else if (step.hasOwnProperty('turnType')) {
            // Display the specified icon
            if (directionMap.length > step.turnType) {
              routeData.stepIconsString = routeData.stepIconsString.concat(icons[directionMap[step.turnType]]);
            } else {
              // Display the travel type icon
              routeData.stepIconsString = routeData.stepIconsString.concat(icons['type']);
            }
          } else {
            // Display the travel type icon
            routeData.stepIconsString = routeData.stepIconsString.concat(icons['type']);
          }
        });
        // We are done
        callback(true, routeData);
      } catch (e) {
        console.log(e);
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
          // Load a route and pass it the callback (uses different api for public transit)
          if (routeType != 2) {
            loadRouteData(routeType, fromLat, fromLon, toLat, toLon, callback);
          } else {
            loadPublicTransitRouteData(fromLat, fromLon, toLat, toLon, callback);
          }
        } else {
          routeErrorCallback(callback);
        }
      });
    } else {
      routeErrorCallback(callback);
    }
  }, true);
}
// Helper function to define createRoute error callback all in one place
function routeErrorCallback(callback) {
  callback(false, { distance: 0, time: 0, stepList: [], stepPositionList: [], stepIconsString: '' });
}

// Calculates the distance of two sets of coordinates (in meters)
function getApproxDistance(fromLat, fromLon, toLat, toLon) {
  var p = 0.017453292519943295;    // Math.PI / 180
  var c = Math.cos;
  var a = 0.5 - c((fromLat - toLat) * p)/2 + c(toLat * p) * c(fromLat * p) * (1 - c((fromLon - toLon) * p))/2;

  return 12742000 * Math.asin(Math.sqrt(a)); // 2 * R; R = 6371000 m
}

// Determine the current waypoint index, based on a list of waypoint coords [{lat,lon},...], the current position, and the current index
function getCurrentStepIndex(steps, lat, lon, accuracy, currentIndex) {
  // Test if the accuracy is good enought
  if (accuracy > 50) return currentIndex === 'number' ? currentIndex : 0;
  // Determine the current step
  try {
    // Determine the max distance
    var maxDistance = 30 + accuracy;
    // Loop through the steps and find the next one, that is close enought
    var foundIndex = currentIndex;
    steps.forEach(function(step, index) {
      // Test all upcoming waypoints
      if (index > currentIndex) {
        if (getApproxDistance(lat, lon, step.lat, step.lon) <= maxDistance) {
          // Move on to this waypoint
          foundIndex = index;
        }
      }
    });
    // Return the step found
    return foundIndex;
  } catch (e) {}

  // In case of an error, return 0 or the current index if it is a number
  return typeof currentIndex === 'number' ? currentIndex : 0;
}


// Exports
module.exports.createRoute = createRoute;
module.exports.getCurrentStepIndex = getCurrentStepIndex;
