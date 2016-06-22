/**
* SETUP OVERVIEW
*
* About the send messages
* –––
* (1) Overview data is send (distance, time needed)
* (2) Step data array is send (each step string, max 20 entrys) -> THE LAST SEND INDEX IS THE LENGTH!
* (3) Success value is send (terminates the transmittion; it's true/false
*   value determines whether the transmition was successfull or not (E.g.
*   if false is send as the success first, no route was found)
*
* About the success codes:
* –––
* 0 = Success; 1 = Route not found; 2 = Too many steps; ...
*
* About the recived message:
* –––
* Field: SEARCH
* Contents: {selected_type}{address} (e.g. 0Brockhofweg 9)
*   The first character is always the type selected, the rest
*   is the written address.
* {selected_type} vals: 0 = Car; 1 = Bike; 2 = Train; 3 = Walk;
*/


// Data keys
var keys = require('message_keys');
var maxStepCount = 20;


// App Message functions
function sendSuccess(code) {
  // Build message
  var key = keys.SUCCESS;
  var dict = {};
  dict[key] = +code;

  // Send message to pebble
  Pebble.sendAppMessage(dict, function() {
    // Success!
    console.log('Transmission completed');
  }, function() {
    // Error
    console.log('Transmission failed at [SUCCESS MESSAGE]');
  });
}

function sendStepItem(stepList, index) {
  // Build message
  var key = keys.INSTRUCTIONS + index;
  var dict = {};
  dict[key] = stepList[index];

  // Send message to pebble
  Pebble.sendAppMessage(dict, function() {
    // Success, send next item
    index ++;
    if (index < stepList.length && index < maxStepCount) {
      // Recursive callbacks, hell yeah!
      sendStepItem(stepList, index);
    } else {
      // We are finished
      sendSuccess(0);
    }
  }, function() {
    // Error
    console.log('Transmission failed at index '.concat(index));
  });
}

function sendRoute(success, distance, time, stepList) {
  // Send message to pebble if a route was found
  if (success) {
    // Build message
    var keyDistance = keys.DISTANCE;
    var keyTime = keys.TIME;
    var dict = {};
    dict[keyDistance] = +distance;
    dict[keyTime] = +time;

    // Transmit
    Pebble.sendAppMessage(dict, function() {
      // Success!
      sendStepItem(stepList, 0);
    }, function() {
      // Error
      console.log('Transmission failed at [OVERVIEW]');
    });
  } else {
    // Send error message (route not found)
    sendSuccess(1);
  }
}

// Api data functions FIXME: Acutally use api
function fetchAndSendRoute(routeType, destination) {
  // TODO: Add api data here (+ use geolocation etc)
  console.log(routeType);
  console.log(destination);
  /* dummy data: */
  setTimeout(function() {
    // Some dummy loading time
    sendRoute(true, 560, 16, ['This is the first step', 'This is the second step', 'This is the third step', 'This is the final step']);
  }, 10000);
}

// Accept data from the pebble watch
Pebble.addEventListener('appmessage', function(e) {
  // Get the dictionary from the message
  var dict = e.payload;

  // Does the SEARCH field exist?
  if (dict['SEARCH']) {
    fetchAndSendRoute(dict['SEARCH'].substr(0, 1), dict['SEARCH'].substr(1));
  }
});

// Ready event
Pebble.addEventListener('ready', function() {
  // Tell the watch that the js part is ready
  var key = keys.READY;
  var dict = {};
  dict[key] = true;
  Pebble.sendAppMessage(dict);

  console.log('Js part is ready!');
});
