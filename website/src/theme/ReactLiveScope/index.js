// It's not clear why this dance is necessary, but without it
// the build fails due to "document" references in h3-js. If we
// import the UMD bundle for h3-js instead, the build fails on
// "Buffer" references.

const window = require("global/window");
window.document ||= require("global/document");
const h3 = require("h3-js/dist/browser/h3-js.js");

// Add react-live imports you need here
const ReactLiveScope = {
  h3,
  ...h3,
};

export default ReactLiveScope;
