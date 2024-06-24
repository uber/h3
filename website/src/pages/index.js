import React from "react";
import styled from "styled-components";
import Layout from "@theme/Layout";
import BrowserOnly from "@docusaurus/BrowserOnly";

const TextContainer = styled.div`
  max-width: 800px;
  padding: 64px 112px;
  width: 70%;
  font-size: 14px;

  h2 {
    font: bold 32px/48px;
    margin: 24px 0 16px;
    position: relative;
  }
  h3 {
    font: bold 16px/24px;
    margin: 16px 0 0;
    position: relative;
  }
  h3 > img {
    position: absolute;
    top: -4px;
    width: 36px;
    left: -48px;
  }
  hr {
    border: none;
    background: #e1e8f0;
    height: 1px;
    margin: 24px 0 0;
    width: 32px;
    height: 2px;
  }
  @media screen and (max-width: 768px) {
    max-width: 100%;
    width: 100%;
    padding: 48px 48px 48px 80px;
  }
`;

export default function IndexPage() {
  return (
    <Layout title="Home" description="H3">
      <BrowserOnly>
        {() => {
          const HomeExplorer = require("../components").HomeExplorer;
          return <HomeExplorer />;
        }}
      </BrowserOnly>
      <div style={{ position: "relative" }}>
        <TextContainer>
          <h2>H3 indexes points and shapes into a hexagonal grid.</h2>
          <hr className="short" />

          <p>
            H3 is a discrete global grid system for indexing geographies into a
            hexagonal grid, developed at Uber.
          </p>
          <p>
            Coordinates can be indexed to cell IDs that each represent a unique
            cell.
          </p>
          <p>
            Indexed data can be quickly joined across disparate datasets and
            aggregated at different levels of precision.
          </p>
          <p>
            H3 enables a range of algorithms and optimizations based on the
            grid, including nearest neighbors, shortest path, gradient
            smoothing, and more.
          </p>
        </TextContainer>
      </div>
    </Layout>
  );
}
