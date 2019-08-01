
module.exports = {
  // Adjusts amount of debug information from ocular-gatsby
  logLevel: 4,

  DOC_FOLDER: `${__dirname}/../docs/`,
  ROOT_FOLDER: `${__dirname}/../`,
  DIR_NAME: `${__dirname}`,

  EXAMPLES: [
    // {
    //   title: 'my example',
    //   path: 'examples/my-example/',
    //   image: 'images/my-example.jpg',
    //   componentUrl: '../examples/app.js'
    // }
  ],
  DOCS: [
    // your table of contents go there
  ],

  THEME_OVERRIDES: [
    //  {key: 'primaryFontFamily', value: 'serif'}
  ],

  PROJECT_TYPE: 'github',
  PROJECT_NAME: 'H3',
  PROJECT_ORG: 'uber',
  PROJECT_URL: 'https://github.com/uber/H3',
  PROJECT_DESC: 'DESCRPTION GOES HERE',
  PATH_PREFIX: '',

  FOOTER_LOGO: '',

  HOME_PATH: '/',
  HOME_HEADING: 'DESCRPTION GOES HERE',
  HOME_RIGHT: null,
  HOME_BULLETS: [
    {
      text: 'Designed for React',
      desc: 'Seamless integration.',
      img: 'images/icon-react.svg'
    },
    {
      text: 'Totally ready for production',
      img: 'images/icon-layers.svg'
    }
  ],

  PROJECTS: [],
    // 'Project name': 'http://project.url',
  ADDITIONAL_LINKS: [
    // {name: 'link label', href: 'http://link.url'}
  ],

  GA_TRACKING: null,

  // For showing star counts and contributors.
  // Should be like btoa('YourUsername:YourKey') and should be readonly.
  GITHUB_KEY: null
};
