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

From `./website`:

Make sure your `gh-pages` branch is up to date with the upstream repo:
```
git checkout gh-pages
git pull upstream gh-pages
```

Switch back to `master` and run the build:
```
git checkout master
./scripts/build-to-gh-pages.sh
```

You're now on branch `gh-pages` again. Push changes to the live site:
```
git push upstream gh-pages:gh-pages
```