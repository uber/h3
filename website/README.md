# H3 Documentation Website

This is the source for the [H3 documentation website](https://uber.github.io/h3/).

The source of the pages in the documentation is in the [docs](../docs) directory.

## Building Locally

Building the documentation requires Node.js. To build and view the H3 documentation, 
run the following from `./website`:

```
npm install
npm run build
npm start
```

## Updating the H3 Website

Make sure your `gh-pages` branch has its remote set to the upstream repo, then

```
make publish-website
```