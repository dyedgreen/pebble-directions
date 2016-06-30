// The config page (without the already stored name / addess pairs)
module.exports = [
  // Navigation settings
  {
    type: 'section',
    items: [
      // Description
      {
        type: 'heading',
        defaultValue: 'Navigation settings',
      },
      // Enable / disable real time navigation
      {
        type: 'toggle',
        messageKey: 'navigationAutoEnable',
        label: 'Enable automatic navigation',
        defaultValue: true,
        description: 'Automatic navigation tells you when to take a turn in real time based on the GPS of your phone. This feature does not require an active internet connection, once the route has been loaded. Disable this feature if you want to reduce battery usage on your phone.',
      },
    ],
  },

  // The named addessses section
  {
    type: 'section',
    items: [
      // Description
      {
        type: 'heading',
        defaultValue: 'Favorite addresses',
      },
      {
        type: 'text',
        defaultValue: 'Favorite addresses allow you to quickly get to your favorite locations. For instance you could store your home address named \'home\'. Every time you search for a specified name, the app will automatically search for the address associated with it. You can add up to five of these addresses.<br style="display:block">This data is keep on your phone and never send to any external service.',
      },


      // Address #1
      {
        type: 'input',
        messageKey: 'namedAddressName_0',
        label: 'Name #1',
        description: 'Use a name that makes sense to you, such as \'home\', \'my college\' or \'mom\'.',
        attributes: {
          placeholder: 'e.g. home',
        },
      },
      {
        type: 'input',
        messageKey: 'namedAddressAddress_0',
        label: 'Address #1',
        description: 'The address should be as specific as possible. Something like \'Schleusenufer Kreuzberg, 10997 Berlin\'.',
        attributes: {
          placeholder: 'e.g. Schleusenufer',
        },
      },


      // Address #2
      {
        type: 'input',
        messageKey: 'namedAddressName_1',
        label: 'Name #2',
        attributes: {
          placeholder: 'e.g. home',
        },
      },
      {
        type: 'input',
        messageKey: 'namedAddressAddress_1',
        label: 'Address #2',
        attributes: {
          placeholder: 'e.g. Schleusenufer',
        },
      },


      // Address #3
      {
        type: 'input',
        messageKey: 'namedAddressName_2',
        label: 'Name #3',
        attributes: {
          placeholder: 'e.g. home',
        },
      },
      {
        type: 'input',
        messageKey: 'namedAddressAddress_2',
        label: 'Address #3',
        attributes: {
          placeholder: 'e.g. Schleusenufer',
        },
      },


      // Address #4
      {
        type: 'input',
        messageKey: 'namedAddressName_3',
        label: 'Name #4',
        attributes: {
          placeholder: 'e.g. home',
        },
      },
      {
        type: 'input',
        messageKey: 'namedAddressAddress_3',
        label: 'Address #4',
        attributes: {
          placeholder: 'e.g. Schleusenufer',
        },
      },


      // Address #4
      {
        type: 'input',
        messageKey: 'namedAddressName_4',
        label: 'Name #5',
        attributes: {
          placeholder: 'e.g. home',
        },
      },
      {
        type: 'input',
        messageKey: 'namedAddressAddress_4',
        label: 'Address #5',
        attributes: {
          placeholder: 'e.g. Schleusenufer',
        },
      },
    ],
  },

  // Support / feedback stuff
  {
    type: 'section',
    items: [
      // Description
      {
        type: 'heading',
        defaultValue: 'Feedback',
      },
      {
        type: 'text',
        defaultValue: 'To submit your feedback, please use this form: <a href="http://goo.gl/forms/KeN3q0oVqg9h4rWf1">FEEDBACK FORM</a> (save your preferences, before clicking the link).',
      },
    ],
  },

  // Api attribution stuff
  {
    type: 'section',
    items: [
      // Description
      {
        type: 'heading',
        defaultValue: 'Navigation data',
      },
      {
        type: 'text',
        defaultValue: 'The data is provided by the HERE API (https://developer.here.com).',
      },
    ],
  },

  // The submit / store settings button
  {
    type: 'submit',
    defaultValue: 'Save preferences',
  },
];
