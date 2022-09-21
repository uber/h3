import React from 'react';
import clsx from 'clsx';
import useIsBrowser from '@docusaurus/useIsBrowser';
import {LiveProvider, LiveEditor, LiveError, LivePreview} from 'react-live';
import Translate from '@docusaurus/Translate';
import useDocusaurusContext from '@docusaurus/useDocusaurusContext';
import BrowserOnly from '@docusaurus/BrowserOnly';
import {usePrismTheme} from '@docusaurus/theme-common';
import styles from './styles.module.css';

function Header({children}) {
  return <div className={clsx(styles.playgroundHeader)}>{children}</div>;
}
function LivePreviewLoader() {
  // Is it worth improving/translating?
  // eslint-disable-next-line @docusaurus/no-untranslated-text
  return <div>Loading...</div>;
}
function ResultWithHeader() {
  return (
    <>
      <Header>
        <Translate
          id="theme.Playground.result"
          description="The result label of the live codeblocks">
          Result
        </Translate>
      </Header>
      {/* https://github.com/facebook/docusaurus/issues/5747 */}
      <div className={styles.playgroundPreview}>
        <BrowserOnly fallback={<LivePreviewLoader />}>
          {() => (
            <>
              {/* H3 specific change: use the code component here for styling reasons */}
              <LivePreview Component='code' />
              <LiveError />
            </>
          )}
        </BrowserOnly>
      </div>
    </>
  );
}
function ThemedLiveEditor() {
  const isBrowser = useIsBrowser();
  return (
    <LiveEditor
      // We force remount the editor on hydration,
      // otherwise dark prism theme is not applied
      key={String(isBrowser)}
      className={styles.playgroundEditor}
    />
  );
}
function EditorWithHeader() {
  return (
    <>
      <Header>
        <Translate
          id="theme.Playground.liveEditor"
          description="The live editor label of the live codeblocks">
          Live Editor
        </Translate>
      </Header>
      <ThemedLiveEditor />
    </>
  );
}
export default function Playground({children, transformCode, ...props}) {
  const {
    siteConfig: {themeConfig},
  } = useDocusaurusContext();
  const {
    liveCodeBlock: {playgroundPosition},
  } = themeConfig;
  const prismTheme = usePrismTheme();
  const noInline = props.metastring?.includes('noInline') ?? false;
  return (
    <div className={styles.playgroundContainer}>
      {/* @ts-expect-error: type incompatibility with refs */}
      <LiveProvider
        code={children.replace(/\n$/, '')}
        noInline={noInline}
        /*
        H3 specific changes:
        1) JSON.stringify of the code results, with indentation
        2) `|| 'undefined'` is to work around https://github.com/facebook/docusaurus/issues/8009
        */
        transformCode={transformCode || ((code) => `JSON.stringify(${code}(), null, 2) || 'undefined';`)}
        theme={prismTheme}
        {...props}>
        {playgroundPosition === 'top' ? (
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
