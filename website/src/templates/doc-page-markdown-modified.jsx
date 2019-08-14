// Copied from ocular-gatsby
import React from 'react';

import {graphql} from 'gatsby';

import "katex/dist/katex.min.css"

// Query for the markdown doc by slug
// (Note: We could just search the allMarkdown from WebsiteConfig ourselves)
export const query = graphql`
  query DocBySlugH3($slug: String!) {
    docBySlug: markdownRemark(fields: {slug: {eq: $slug}}) {
      html
      timeToRead
      excerpt
      frontmatter {
        title
        cover
        category
        tags
      }
    }
  }
`;

function replaceLinks(props) {
  const {html} = props.data.docBySlug;
  const {relativeLinks} = props.pageContext;

  return html.replace(/href="([^"]+)"/g, (link, href) => {
    // don't rewrite external links, don't rewrite links to anchors
    if (href.startsWith('http') || href.startsWith('#')) {
      // TODO - we could style them differently though
      return link;
    }
    const hrefWithoutLeadingSlash = href.startsWith('/') ? href.slice(1) : href;
    // replace links to:
    // - known physical files, either relative to this file or relative to root
    // - known routes, either relative to the route of this page or to the home page
    // by a link to their corresponding route, expresed relative to the home page
    if (relativeLinks[hrefWithoutLeadingSlash] === undefined) {
      throw new Error(`Unresolved href ${hrefWithoutLeadingSlash}`);
    }
    return `href="${relativeLinks[hrefWithoutLeadingSlash]}"`;
  });
}

export default class DocTemplate extends React.Component {
  constructor(props) {
    super(props);
    this.state = {html: replaceLinks(props)};
  }

  render() {
    const {html} = this.state;
    return (
      <div>
        <div
          className="markdown-body"
          dangerouslySetInnerHTML={{__html: html}}
        />
      </div>
    );
  }
}
