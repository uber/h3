# H3 Documentation Website

This is the source for the [H3 documentation website](https://h3geo.org/).

The source of the pages in the documentation is in the [docs](./docs) directory.

The website is built using [Docusaurus](https://docusaurus.io/).

## Building Locally

Building the documentation requires Node.js. To build and view the H3 documentation,
run the following from `./website`:

```
yarn
yarn start
```

You will then be able to open http://localhost:3000 in your browser.

You can also test a production-ready build with:

```
yarn build
```

## Updating the H3 Website

[Website deployment](https://docusaurus.io/docs/deployment#deploy) happens
automatically via GitHub Actions whenever changes are landed to
the `master` branch. 
