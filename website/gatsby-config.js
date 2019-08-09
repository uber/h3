const {getGatsbyConfig} = require('ocular-gatsby/api');

const config = require('./ocular-config');

const configured = getGatsbyConfig(config);

configured.plugins.push(
  {
    resolve: `gatsby-transformer-remark`,
    options: {
      plugins: [
        {
          resolve: `gatsby-remark-katex`,
          options: {
          }
        }
      ],
    },
  }
);

module.exports = configured;
