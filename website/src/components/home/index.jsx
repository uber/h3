import React, { useState } from 'react';
import useDocusaurusContext from '@docusaurus/useDocusaurusContext';
import {Banner, BannerContainer, HeroExampleContainer, ProjectName, GetStartedLink} from './styled';
import BrowserOnly from '@docusaurus/BrowserOnly';

export default function renderPage({HeroExample, children}) {
  const {siteConfig} = useDocusaurusContext();

  const [hex, setHex] = useState("");

  // Note: The Layout "wrapper" component adds header and footer etc
  return (
    <>
      <Banner>
        <BrowserOnly>
          {() => <HeroExampleContainer>{HeroExample && <HeroExample
            hex={hex}
          />}</HeroExampleContainer>}
        </BrowserOnly>
        <BannerContainer>
          {/* <ProjectName>{siteConfig.title}</ProjectName>
          <p>{siteConfig.tagline}</p>
          <GetStartedLink href="./docs/get-started/getting-started">GET STARTED</GetStartedLink> */}
          <input type="text" value={hex} onChange={(e) => {setHex(e.target.value);}}/>
        </BannerContainer>
      </Banner>
      {children}
    </>
  );
}
