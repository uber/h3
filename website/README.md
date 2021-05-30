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

To clean any generated files, you can run:

```
yarn clear
rm -rf node_modules
```

## Updating the H3 Website

Deployment is done via [Docusaurus](https://docusaurus.io/docs/deployment#deploy). You will need
to set `GIT_USER` to your Github username, and then run:

```
USE_SSH=true yarn deploy
```
