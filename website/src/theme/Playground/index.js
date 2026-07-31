import React from "react";
import PlaygroundProvider from "@theme/Playground/Provider";
import PlaygroundContainer from "@theme/Playground/Container";
import PlaygroundLayout from "@theme/Playground/Layout";
export default function Playground({
  children,
  transformCode,
  position,
  ...props
}) {
  /*
       H3 specific changes:
        1) JSON.stringify of the code results, with indentation
        2) `|| 'undefined'` is to work around https://github.com/facebook/docusaurus/issues/8009
        */
  const transformCodeDefaulted =
    transformCode ||
    ((code) =>
      `function example(){return JSON.stringify(${code}(), null, 2) || 'undefined';}`);
  return (
    <PlaygroundContainer>
      <PlaygroundProvider
        code={children}
        {...props}
        transformCode={transformCodeDefaulted}
      >
        <PlaygroundLayout position={position} />
      </PlaygroundProvider>
    </PlaygroundContainer>
  );
}
