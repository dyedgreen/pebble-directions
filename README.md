# Pebble Directions

## _[!! now maintained for the rebble store here !!](https://github.com/Kuchenm0nster/pebble-directions)_

![Pebble Appstore Banner](./banner.png)

A simple directions app for the Pebble smartwatch.
Users can select their method of transport and then speak their destination using Rebbles voice input apis.
Then the app uses the Here directions api to
get a list of directions starting at the users current location.

Sadly public transport is not working in certain locations.
> This coverage currently includes, but is not limited to, the following: Paris, France (STIF); Berlin, Germany (VBB); full coverage Germany (Deutsche Bahn); and Helsinki, Finland (HSL).

You can find it in the [Rebble Appstore](https://apps.rebble.io/en_US/application/576ee8e6ba2fe5a0c10000b9).

## Building

In addition to the Pebble SDK you need to include your own API-keys for Here and mapQuest.
Therefore you need to include the file `src/js/apiKeys.js` with the following content:
```
module.exports.hereApiKey = 'YOUR HERE API KEY';
module.exports.mapQuestKey = 'YOUR MAPQUEST API KEY';
```
