/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

import * as React from "react";
import { LiveProvider, LiveEditor, LiveError, LiveContext } from "react-live";
import clsx from "clsx";
import Translate from "@docusaurus/Translate";
import useDocusaurusContext from "@docusaurus/useDocusaurusContext";
import usePrismTheme from "@theme/hooks/usePrismTheme";
import styles from "./styles.module.css";

function Header({ children }) {
  return <div className={clsx(styles.playgroundHeader)}>{children}</div>;
}

// Modified version to wrap:
// https://github.com/FormidableLabs/react-live/blob/4ef370647d4e52f8f87148b70c6429237de508cb/src/utils/transpile/errorBoundary.js
// In order to fix issues with null rendering. Note that this just swallows errors
// that would have crashed the above.
class ErrorBoundary2 extends React.Component {
  componentDidCatch() {
    // Don't do anything
  }

  render() {
    return this.props.children;
  }
};

function ResultWithHeader() {
  return (
    <>
      <Header>
        <Translate
          id="theme.Playground.result"
          description="The result label of the live codeblocks"
        >
          Result
        </Translate>
      </Header>
      <div className={styles.playgroundPreview}>
        {/* This div replaces LivePreview */}
        <LiveContext.Consumer>
          {({ element: Element }) =>
            Element ? (
              <pre>
                <ErrorBoundary2>
                  <Element />
                </ErrorBoundary2>
              </pre>
            ) : null
          }
        </LiveContext.Consumer>
        <LiveError />
      </div>
    </>
  );
}

function EditorWithHeader() {
  return (
    <>
      <Header>
        <Translate
          id="theme.Playground.liveEditor"
          description="The live editor label of the live codeblocks"
        >
          Live Editor
        </Translate>
      </Header>
      <LiveEditor className={styles.playgroundEditor} />
    </>
  );
}

export default function Playground({ children, transformCode, ...props }) {
  const {
    isClient,
    siteConfig: {
      themeConfig: {
        liveCodeBlock: { playgroundPosition },
      },
    },
  } = useDocusaurusContext();
  const prismTheme = usePrismTheme();

  return (
    <div className={styles.playgroundContainer}>
      <LiveProvider
        key={isClient}
        code={isClient ? children.replace(/\n$/, "") : ""}
        transformCode={
          transformCode || ((code) => `JSON.stringify(${code}());`)
        }
        theme={prismTheme}
        {...props}
      >
        {playgroundPosition === "top" ? (
          <>
            <ResultWithHeader />
            <EditorWithHeader />
          </>
        ) : (
          <>
            <EditorWithHeader />
            <ResultWithHeader />
          </>
        )}
      </LiveProvider>
    </div>
  );
}
