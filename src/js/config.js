// Functions to handle storing / retriving of the values

// Store the named addresses data
function storeNamedAddresses(configDict) {
  // This array will hold the raw named addesses
  var namedAddressesNamesRaw = [];
  var namedAddressesAddressesRaw = [];
  // Loop the stored recived config elements
  for (var key in configDict) {
    if (!configDict.hasOwnProperty(key)) continue;

    if (/^namedAddressName_\d+$/.test(key)) {
      // This is a name
      var index = +(key.replace('namedAddressName_', ''));
      namedAddressesNamesRaw[index] = configDict[key].value;
    } else if (/^namedAddressAddress_\d+$/.test(key)) {
      // This is an address
      var index = +(key.replace('namedAddressAddress_', ''));
      namedAddressesAddressesRaw[index] = configDict[key].value;
    }
  }

  // Loop the named addresses and store the ones that have a value
  var namedAddresses = [];
  namedAddressesNamesRaw.forEach(function(name, index) {
    var address = namedAddressesAddressesRaw[index];
    // Validate the inputs
    if (typeof name === 'string' && typeof address === 'string') {
      // Remove any leading / ending spaces
      name = name.replace(/^\s+/, '').replace(/\s+$/, '');
      address = address.replace(/^\s+/, '').replace(/\s+$/, '');
      // Test if the fields are set
      if (name && address) {
        namedAddresses.push({ name: name, address: address });
      }
    }
  });

  // Store the named addresses
  try {
    localStorage.setItem('namedAddresses', JSON.stringify(namedAddresses));
  } catch (e) {}
}

// Get the named addresses data (exposed)
function getNamedAddresses() {
  // Load the stored data
  data = localStorage.getItem('namedAddresses');
  try {
    data = JSON.parse(data);
  } catch(e) {
    return [];
  }

  // Test if the data is an array as expected
  if (data instanceof Array) {
    var namedAddresses = [];
    // Validate the data points format
    data.forEach(function(element) {
      if (
        element instanceof Object &&
        element.hasOwnProperty('name') &&
        element.hasOwnProperty('address') &&
        typeof element.name === 'string' &&
        typeof element.address === 'string'
      ) {
        namedAddresses.push({ name: element.name, address: element.address });
      }
    });
    // Return the data
    return namedAddresses;
  }

  // Empty fallback
  return [];
}


// Clay things
var Clay = require('pebble-clay');
var clayConfig = require('./clayConfig.js');
var clay = new Clay(clayConfig, null, { autoHandleEvents: false });

// Add the config event listeners
Pebble.addEventListener('showConfiguration', function() {
  // Present the config
  Pebble.openURL(clay.generateUrl());
});

// Handle the config finish
Pebble.addEventListener('webviewclosed', function(e) {
  // Catch errors
  if (e && !e.response) {
    return;
  }

  // Store the addresses returned by the config page
  var configDict = clay.getSettings(e.response, false);
  storeNamedAddresses(configDict);
});


// Exports for use in the app.js
module.exports.getNamedAddresses = getNamedAddresses;
