// The config page (without the already stored name / addess pairs)
module.exports = [
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
        type: 'section',
        items: [
          {
            type: 'heading',
            defaultValue: 'Address 1',
          },
          {
            type: 'input',
            messageKey: 'namedAddressName_0',
            label: 'Name',
            description: 'Use a name that makes sense to you, such as \'home\', \'my college\' or \'mom\'.',
            attributes: {
              placeholder: 'e.g. home',
            },
          },
          {
            type: 'input',
            messageKey: 'namedAddressAddress_0',
            label: 'Address',
            description: 'The address should be as specific as possible. Something like \'Schleusenufer Kreuzberg, 10997 Berlin\'.',
            attributes: {
              placeholder: 'e.g. Schleusenufer',
            },
          },
        ]
      },


      // Address #2
      {
        type: 'section',
        items: [
          {
            type: 'heading',
            defaultValue: 'Address 2',
          },
          {
            type: 'input',
            messageKey: 'namedAddressName_1',
            label: 'Name',
            attributes: {
              placeholder: 'e.g. home',
            },
          },
          {
            type: 'input',
            messageKey: 'namedAddressAddress_1',
            label: 'Address',
            attributes: {
              placeholder: 'e.g. Schleusenufer',
            },
          },
        ]
      },


      // Address #3
      {
        type: 'section',
        items: [
          {
            type: 'heading',
            defaultValue: 'Address 3',
          },
          {
            type: 'input',
            messageKey: 'namedAddressName_2',
            label: 'Name',
            attributes: {
              placeholder: 'e.g. home',
            },
          },
          {
            type: 'input',
            messageKey: 'namedAddressAddress_2',
            label: 'Address',
            attributes: {
              placeholder: 'e.g. Schleusenufer',
            },
          },
        ]
      },


      // Address #4
      {
        type: 'section',
        items: [
          {
            type: 'heading',
            defaultValue: 'Address 4',
          },
          {
            type: 'input',
            messageKey: 'namedAddressName_3',
            label: 'Name',
            attributes: {
              placeholder: 'e.g. home',
            },
          },
          {
            type: 'input',
            messageKey: 'namedAddressAddress_3',
            label: 'Address',
            attributes: {
              placeholder: 'e.g. Schleusenufer',
            },
          },
        ]
      },


      // Address #4
      {
        type: 'section',
        items: [
          {
            type: 'heading',
            defaultValue: 'Address 5',
          },
          {
            type: 'input',
            messageKey: 'namedAddressName_4',
            label: 'Name',
            attributes: {
              placeholder: 'e.g. home',
            },
          },
          {
            type: 'input',
            messageKey: 'namedAddressAddress_4',
            label: 'Address',
            attributes: {
              placeholder: 'e.g. Schleusenufer',
            },
          },
        ]
      },
    ],
  },

  // The submit / store settings button
  {
    type: 'submit',
    defaultValue: 'Save preferences',
  },
];
