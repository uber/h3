# H3 Documentation Website

This is the source for the [H3 documentation website](https://uber.github.io/h3/).

The source of the pages in the documentation is in the [docs](../docs) directory.

## Building Locally

Building the documentation requires Node.js. To build and view the H3 documentation,
run the following from `./website`:

```
yarn
yarn run build
```

You will then be able to open `public/index.html` in your browser. Note that images
will not render because they expect a `/h3/` path prefix.

## Updating the H3 Website

Make sure your `gh-pages` branch has its remote set to the upstream repo, then

```
make publish-website
```
