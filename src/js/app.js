/**
* SETUP OVERVIEW
*
* About the send messages
* –––
* (1) Overview data is send (distance, time needed)
* (2) Step icon data is send as a 20 char string (each char encodes one icon!)
* (3) Step data array is send (each step string, max 20 entrys) -> THE LAST SEND INDEX IS THE LENGTH!
* (4) Success value is send (terminates the transmittion; it's true/false
*   value determines whether the transmition was successfull or not (E.g.
*   if false is send as the success first, no route was found)
*
* About the success codes:
* –––
* 0 = Success; 1 = Route not found; 2 = Too many steps; 3 = No current position found; ...
* The code is send in the way of: code + messageNumber * messagePadding
*
* About the recived message:
* –––
* Field: SEARCH
* Contents: {selected_type}{address} (e.g. 0Brockhofweg 9)
*   The first character is always the type selected, the rest
*   is the written address.
* {selected_type} vals: 0 = Car; 1 = Bike; 2 = Train; 3 = Walk;
*
* About the step icon data string
* –––
* Chars map to the following icons: type: 'a', forward: 'b', right: 'c', left: 'd', uRight: 'e', uLeft: 'f', attr: 'g', final: 'h'
*
* About the config of 'named addresses'
* –––
* The config stores an array of these name / address pairs, every time a search request hits the phone the
* recived search string is matched against the names of all entrys and replaced with the addess is a match is found.
*/


// Data keys
var keys = require('message_keys');
var maxStepCount = 40;
var maxStepStringLength = 128;
var currentMessageNumber = 0;
var messagePadding = 10;

// Location services
var locationService = require('./location.js');
// Configuration
var config = require('./config.js');


// App Message functions
function sendSuccess(code, messageNumber) {
  // Build message
  var key = keys.SUCCESS;
  var dict = {};
  dict[key] = code + (messageNumber * messagePadding);

  // Send message to pebble
  Pebble.sendAppMessage(dict, function() {
    // Success!
    console.log('Transmission completed:', code, messageNumber);
  }, function() {
    // Error
    console.log('Transmission failed at [SUCCESS MESSAGE]');
  });
}

function sendStepItem(stepList, index, messageNumber) {
  // Build message
  var key = keys.INSTRUCTION_LIST;
  var dict = {};
  dict[key] = stepList[index].substr(0, maxStepStringLength);

  // Send message to pebble
  Pebble.sendAppMessage(dict, function() {
    // Success, send next item
    index ++;
    if (index < stepList.length && index < maxStepCount) {
      // Recursive callbacks, hell yeah!
      sendStepItem(stepList, index, messageNumber);
    } else {
      // We are finished
      sendSuccess(0, messageNumber);
    }
  }, function() {
    // Error
    console.log('Transmission failed at index '.concat(index));
  });
}

function sendRoute(success, distance, time, stepList, stepIconsString, messageNumber) {
  // Send message to pebble if a route was found
  if (success && maxStepCount >= stepList.length) {
    // Build message
    var keyDistance = keys.DISTANCE;
    var keyTime = keys.TIME;
    var keyIcons = keys.INSTRUCTION_ICONS;
    var dict = {};
    dict[keyDistance] = +distance;
    dict[keyTime] = +time;
    dict[keyIcons] = ''.concat(stepIconsString);

    // Transmit
    Pebble.sendAppMessage(dict, function() {
      // Success!
      sendStepItem(stepList, 0, messageNumber);
    }, function() {
      // Error
      console.log('Transmission failed at [OVERVIEW]');
    });
  } else if (maxStepCount < stepList.length) {
    // Too many steps error
    sendSuccess(2, messageNumber);
  } else {
    // Send error message (route not found)
    sendSuccess(1, messageNumber);
  }
}

// Fetch a route from the here api and send it to the pebble
function fetchAndSendRoute(routeType, searchText, messageNumber) {
  // Log the recived data
  console.log('Route type:', routeType);
  console.log('Search text:', searchText);
  // Convert the search text using the named addresses
  config.getNamedAddresses().forEach(function(namedAddress) {
    if (namedAddress.name.toLowerCase() == searchText.toLowerCase()) {
      searchText = namedAddress.address;
      console.log('Search text was named address:', searchText);
    }
  });
  // Load a route from here api. Data format: { distance, time, stepList[string], stepIconsString }
  locationService.createRoute(routeType, searchText, function(success, data) {
    console.log('Will send:', success, data.distance, data.time, data.stepList.length, data.stepIconsString, messageNumber);
    sendRoute(success, data.distance, data.time, data.stepList, data.stepIconsString, messageNumber);
  });
}

// Accept data from the pebble watch
Pebble.addEventListener('appmessage', function(e) {
  // Get the dictionary from the message
  var dict = e.payload;

  // Does the SEARCH field exist?
  if (dict['SEARCH']) {
    // Get the current message number and increment the message number
    var messageNumber = currentMessageNumber;
    currentMessageNumber ++;
    console.log('Message number send:', messageNumber);
    fetchAndSendRoute(dict['SEARCH'].substr(0, 1), dict['SEARCH'].substr(1), messageNumber);
  }
});
