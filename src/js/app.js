// Data keys
var keys = require('message_keys');

// Common functions
function createData(success, distance, time, steps, stepsDistances) {
  var dict = {};
  dict[keys.SUCCESS] = success;
  dict[keys.DISTANCE] = distance;
  dict[keys.TIME] = time;
  dict[keys.STEPS] = steps;
  dict[keys.STEPS_DISTANCES] = stepsDistances;
  return dict;
}

// Some dummy data (let's make the c app work first ;)
Pebble.addEventListener('appmessage', function(e) {
  // Get the dictionary from the message
  var dict = e.payload;
  console.log(dict.SEARCH);

  var message = createData(
    true,
    140,
    13,
    ["Do sometoing", "Step 3", "the very last step"],
    [11, 0, 5321]
  );
  Pebble.sendAppMessage(message, function() {
    console.log('Message sent successfully: ' + JSON.stringify(message));
  }, function(e) {
    console.log('Message failed: ' + JSON.stringify(message));
  });
});
