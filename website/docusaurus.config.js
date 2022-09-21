const math = require('remark-math');
const katex = require('rehype-katex');

/** @type {import('@docusaurus/types').DocusaurusConfig} */
module.exports = {
  title: 'H3',
  tagline: 'Hexagonal hierarchical geospatial indexing system',
  url: 'https://h3geo.org',
  baseUrl: '/',
  onBrokenLinks: 'throw',
  onBrokenMarkdownLinks: 'warn',
  favicon: 'favicon.ico',
  organizationName: 'uber', // Usually your GitHub org/user name.
  projectName: 'h3', // Usually your repo name.
  themes: ['@docusaurus/theme-live-codeblock'],
  themeConfig: {
    prism: {
      additionalLanguages: ['java'],
    },
    navbar: {
      title: 'H3',
      logo: {
        alt: 'H3 Logo',
        src: 'images/h3Logo-color.svg',
      },
      items: [
        {
          to: 'docs',
          activeBaseRegex: 'docs(/)$',
          label: 'Intro',
          position: 'left',
        },
        {
          to: 'docs/api/indexing',
          activeBasePath: 'docs/api',
          label: 'API',
          position: 'left',
        },
        {
          to: 'docs/community/bindings',
          activeBasePath: 'docs/community',
          label: 'Bindings',
          position: 'left',
        },
        {
          to: 'docs/core-library/restable',
          label: 'Resolutions',
          position: 'left',
        },
        {
          type: 'docsVersionDropdown',
          position: 'right',
          dropdownActiveClassDisabled: true,
        },
        {
          href: 'https://github.com/uber/h3',
          label: 'GitHub',
          position: 'right',
        },
      ],
    },
    footer: {
      style: 'dark',
      links: [
        {
          title: 'Docs',
          items: [
            {
              label: 'Getting Started',
              to: 'docs/',
            },
            {
              label: 'Installation',
              to: 'docs/installation',
            },
            {
              label: 'API Reference',
              to: 'docs/api/indexing',
            },
          ],
        },
        {
          title: 'Community',
          items: [
            {
              label: 'Stack Overflow',
              href: 'https://stackoverflow.com/questions/tagged/h3',
            },
            {
              label: 'Slack',
              href: 'https://join.slack.com/t/h3-core/shared_invite/zt-g6u5r1hf-W_~uVJmfeiWtMQuBGc1NNg',
            },
          ],
        },
        {
          title: 'Source',
          items: [
            {
              label: 'GitHub',
              href: 'https://github.com/uber/h3',
            },
          ],
        },
      ],
      copyright: `Copyright © ${new Date().getFullYear()} Uber Technologies, Inc. Built with Docusaurus.`,
    },
  },
  presets: [
    [
      '@docusaurus/preset-classic',
      {
        docs: {
          remarkPlugins: [math],
          rehypePlugins: [katex],
          sidebarPath: require.resolve('./sidebars.js'),
          editUrl: ({docPath}) => {
            // Per docusaurus, apply changes to the "next" version, not the current one
            return `https://github.com/uber/h3/edit/master/website/docs/${docPath}`;
          },
          lastVersion: 'current',
          versions: {
            current: {
              label: '4.x'
            },
          },
        },
        theme: {
          customCss: require.resolve('./src/css/custom.css'),
        },
      },
    ],
  ],
  stylesheets: [
      {
          href: "https://cdn.jsdelivr.net/npm/katex@0.13.11/dist/katex.min.css",
          integrity: "sha384-Um5gpz1odJg5Z4HAmzPtgZKdTBHZdw8S29IecapCSB31ligYPhHQZMIlWLYQGVoc",
          crossorigin: "anonymous",
      },
  ],
};
