# H3 Documentation Website

This is the source for the [H3 documentation website](https://h3geo.org/).

The source of the pages in the documentation is in the [docs](./docs) directory.

## Building Locally

Building the documentation requires Node.js. To build and view the H3 documentation,
run the following from `./website`:

```
yarn
yarn start
```

You will then be able to open http://localhost:3000 in your browser.

## Updating the H3 Website

Make sure your `gh-pages` branch has its remote set to the upstream repo, then

```
make publish-website
```
