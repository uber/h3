export const MOBILE_CUTOFF_WINDOW_WIDTH = 480;

export const isMobile = (props) =>
  `@media screen and (max-width: ${MOBILE_CUTOFF_WINDOW_WIDTH}px)`;
