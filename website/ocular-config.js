const DOCS = require('../docs/table-of-contents.json');

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
  DOCS,

  THEME_OVERRIDES: [
    //  {key: 'primaryFontFamily', value: 'serif'}
  ],

  PROJECT_TYPE: 'github',
  PROJECT_NAME: 'H3',
  PROJECT_ORG: 'uber',
  PROJECT_URL: 'https://github.com/uber/H3',
  PROJECT_DESC: 'Hexagonal hierarchical geospatial indexing system.',
  PATH_PREFIX: '/h3/',

  FOOTER_LOGO: '',

  DOC_PAGE_URL: 'src/templates/doc-page-markdown.jsx',

  HOME_PATH: '/',
  HOME_HEADING: 'Hexagonal hierarchical geospatial indexing system.',
  HOME_RIGHT: null,
  HOME_BULLETS: [
    {
      text: 'Partition data into a hexagonal grid',
      desc: 'H3 has an easy API for indexing coordinates into a hexagonal, global grid.',
      img: 'images/high-precision.svg'
    },
    {
      text: 'Hierarchical grid system',
      desc: 'Easy, bitwise truncation to coarser, approximate cells, along with area compression/decompression algorithms.',
      img: 'images/icon-layers.svg'
    },
    {
      text: 'Fully global',
      desc: 'Along with twelve pentagons, the entire world is addressable in H3, down to square meter resolution.',
      img: 'images/icon-chart.svg'
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
