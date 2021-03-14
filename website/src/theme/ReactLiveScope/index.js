/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

const h3 = require("h3-js");

// Add react-live imports you need here
const ReactLiveScope = {
  h3,
  ...h3,
};

export default ReactLiveScope;
