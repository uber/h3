"use strict";(globalThis.webpackChunkh3_website=globalThis.webpackChunkh3_website||[]).push([[2634],{4637(e,t,n){n.d(t,{L:()=>J});var l=n(6540),i=n(7304),o=n(3540);const r=e=>"@media screen and (max-width: 480px)",a=o.Ay.div`
  height: 100%;
  ${r} {
    height: 40vh;
  }
  .tooltip,
  .deck-tooltip {
    position: absolute;
    padding: 4px 12px;
    background: rgba(0, 0, 0, 0.8);
    color: var(--ifm-color-white);
    max-width: 300px;
    font-size: 12px;
    z-index: 9;
    pointer-events: none;
    white-space: nowrap;
  }
`,s=o.Ay.section`
  position: relative;
  height: 30rem;
  background: var(--ifm-color-gray-400);
  color: var(--ifm-color-gray-900);
  z-index: 0;
  ${r} {
    height: 40vh;
  }
`,c=o.Ay.div`
  position: relative;
  padding: 2rem;
  max-width: 80rem;
  width: 100%;
  height: 100%;
  margin: 0;
`,u=(0,o.Ay)(c)`
  position: absolute;
  bottom: 2rem;
  height: auto;
  max-height: 90%;
  overflow-y: scroll;
  left: 2rem;
  z-index: 0;
  width: auto;
  max-width: 20em;
  background-color: white;
  border-radius: 20px;
  padding: 1rem;
`,d=o.Ay.div`
  position: absolute;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  z-index: -1;
`;o.Ay.section`
  &:nth-child(2n + 1) {
    background: var(--ifm-color-gray-300);
  }
`;var h=n(3681);function m(e){let{hex:t,setUserInput:n}=e;const i=(0,l.useCallback)((()=>{n(t)}),[t,n]);return l.createElement("a",{onClick:i,style:{cursor:"pointer"}},t)}function p(e){let{hexes:t,setUserInput:n,showAll:i=!0}=e;const o=(0,l.useCallback)((()=>{n(t.join(", "))}),[t,n]);return l.createElement(l.Fragment,null,t.map(((e,t)=>{const i=l.createElement(m,{key:e,setUserInput:n,hex:e});return 0===t?i:l.createElement("span",{key:e},", ",i)})),i?l.createElement(l.Fragment,null,"\xa0",l.createElement("a",{onClick:o,style:{cursor:"pointer"}},"(show all)")):l.createElement(l.Fragment,null))}function g(e){let{setUserInput:t,showCellId:n,splitUserInput:o,showNavigation:r=!0,showDetails:a=!0}=e;if(1===o.length){const e=o[0],s=function(e){return(0,i.getResolution)(e)<8?{area:i.UNITS.km2,dist:i.UNITS.km}:{area:i.UNITS.m2,dist:i.UNITS.m}}(e),c=(0,i.getResolution)(e),u=c/3+7,d=(0,i.getBaseCellNumber)(e),h=(0,i.isPentagon)(e),g=(0,i.getIcosahedronFaces)(e).join(", "),x=(0,i.cellToLatLng)(e).map((e=>e.toPrecision(u))).join(", "),f=0===c?"(none)":function(e){const t=(0,i.h3IndexToSplitLong)(e);return[t[1]>>10&7,t[1]>>7&7,t[1]>>4&7,t[1]>>1&7,(1&t[1])<<2|t[0]>>30&3,t[0]>>27&7,t[0]>>24&7,t[0]>>21&7,t[0]>>18&7,t[0]>>15&7,t[0]>>12&7,t[0]>>9&7,t[0]>>6&7,t[0]>>3&7,7&t[0]]}(e).slice(0,c).join(""),b=(0,i.cellToBoundary)(e),w=(0,i.cellArea)(e,s.area).toPrecision(u),E=(0,i.originToDirectedEdges)(e).map((e=>(0,i.edgeLength)(e,s.dist))),y=(E.reduce(((e,t)=>void 0!==e?e+t:t))/E.length).toPrecision(u),v=0!==c&&(0,i.cellToParent)(e,c-1),C=15!==c&&(0,i.cellToChildren)(e,c+1),k=(0,i.gridDisk)(e,1).filter((t=>t!==e));return l.createElement("p",{style:{marginBottom:"0"}},"Lat./Lng.: ",l.createElement("tt",null,x),n?l.createElement(l.Fragment,null,l.createElement("br",null),"ID: ",l.createElement(m,{hex:e,setUserInput:t})):null,r?l.createElement(l.Fragment,null,l.createElement("br",null),"Parent:"," ",v?l.createElement(m,{hex:v,setUserInput:t}):l.createElement("tt",null,"(none)"),l.createElement("br",null),"Children:"," ",C?l.createElement(p,{hexes:C,setUserInput:t}):l.createElement("tt",null,"(none)"),l.createElement("br",null),"Neighbors:"," ",l.createElement(p,{hexes:k,setUserInput:t}),l.createElement("br",null)):l.createElement(l.Fragment,null),a?l.createElement("details",null,l.createElement("summary",null,"Details"),"Resolution: ",l.createElement("tt",null,c),l.createElement("br",null),"Base cell: ",l.createElement("tt",null,d),l.createElement("br",null),"Pentagon: ",l.createElement("tt",null,`${h}`),l.createElement("br",null),"Icosa Face IDs: ",l.createElement("tt",null,g),l.createElement("br",null),"# of Boundary Verts: ",l.createElement("tt",null,b.length),l.createElement("br",null),"Cell Area: ",l.createElement("tt",null,w)," ",s.area,l.createElement("br",null),"Mean Edge Length: ",l.createElement("tt",null,y)," ",s.dist,l.createElement("br",null),"Indexing Digits: ",l.createElement("tt",null,f)):l.createElement(l.Fragment,null))}return l.createElement("p",{style:{marginBottom:"0"}},l.createElement(p,{hexes:o,setUserInput:t,showAll:!1}))}var x=n(8929),f=n(1163),b=n(2897),w=n(6992),E=n(4299),y=n(5022),v=n(4586);[...Array(15).keys()].map((e=>5e3*(0,i.getHexagonAreaAvg)(e,i.UNITS.m2)));var C=n(3485);function k(e){return function(e,t,n){const l=n-t;return((e-t)%l+l)%l+t}(e,-180,180)}const L=e=>{let{minLat:t,minLon:n,maxLat:l,maxLon:i}=e;return function(e,t){const n=[];let l=e;const i=k(t);for(;l<t;){const e=k(l),o=e<0?0:180;l=l+o-e,l>t?n.push([e,i]):n.push([e,o])}return n}(n,i).map((e=>{let[n,i]=e;return{minLat:t,minLon:n,maxLat:l,maxLon:i}}))};const I=e=>{let{minLat:t,minLon:n,maxLat:l,maxLon:i}=e;return[[n,t],[i,t],[i,l],[n,l],[n,t]]},A={0:0,1:0,2:1,3:2,4:2,5:3,6:4,7:5,8:5,9:6,10:6,11:7,12:8,13:9,14:10,15:11,16:11,17:12,18:12,19:13,20:14,21:15},S=(Object.entries(A).reduce(((e,t)=>{let[n,l]=t;return{...e,[l]:+n}}),{}),(e,t)=>{if(t>15)return[];const n=L(e).map(I);return[...new Set(n.flatMap((e=>(0,i.polygonToCells)(e,t,!0))))]}),F=e=>{let{resolutionFrozen:t,addSelectedHexes:n}=e;const[i,o]=(0,l.useState)([]),[r,a]=(0,l.useState)([]),[s,c]=(0,l.useState)([]),[u,d]=(0,l.useState)(0),[h,m]=(0,l.useState)(null);(0,l.useEffect)((()=>{m([new b.A({id:"H3HexagonLayer",extruded:!1,getHexagon:e=>e,getFillColor:[0,0,0,1],getLineColor:[0,0,0,100],getLineWidth:2,lineWidthMinPixels:2,highPrecision:!0,lineWidthUnits:"pixels",elevationScale:20,pickable:!0,stroked:!0,filled:!0,data:i,wrapLongitude:!0,onClick:e=>{e.object&&n([e.object])}}),...r.length?[new b.A({id:"H3HexagonLayer1",extruded:!1,getHexagon:e=>e,getFillColor:[0,0,0,1],getLineColor:[50,50,50,75],getLineWidth:1,lineWidthMinPixels:1,highPrecision:!0,lineWidthUnits:"pixels",elevationScale:20,pickable:!1,stroked:!0,filled:!0,data:r,wrapLongitude:!0})]:[],...s.length?[new b.A({id:"H3HexagonLayer2",extruded:!1,getHexagon:e=>e,getFillColor:[0,0,0,1],getLineColor:[100,100,100,50],getLineWidth:1,lineWidthMinPixels:1,highPrecision:!0,lineWidthUnits:"pixels",elevationScale:20,pickable:!1,stroked:!0,filled:!0,data:s,wrapLongitude:!0,getDashArray:[5,1],dashJustified:!0,dashGapPickable:!0,extensions:[new C.A({dash:!0})]})]:[]])}),[n,i,r,s]);const p=(0,l.useCallback)(function(e,t){let n,l,i;return void 0===t&&(t=300),function(){for(var o=arguments.length,r=new Array(o),a=0;a<o;a++)r[a]=arguments[a];n?(clearTimeout(l),l=setTimeout((()=>{Date.now()-i>=t&&(e.apply(this,r),i=Date.now())}),Math.max(t-(Date.now()-i),0))):(e.apply(this,r),i=Date.now(),n=!0)}}((e=>{if(t)return;const n=e.zoom,l=Math.max(0,A[Math.round(n)]-1);d(l);const i=(e=>{const t=new w.A(e),{width:n,height:l}=e,i=t.unproject([0,0]),o=t.unproject([n,l]);return{minLat:o[1],minLon:i[0],maxLat:i[1],maxLon:o[0]}})(e),r=S(i,l);o(r);const s=S(i,l+1);a(s);const u=S(i,l+2);c(u)}),200),[o,a,c,d,t]);return{handleResize:p,hexLayers:h,resolution:u}};var U=n(8551);const T={longitude:-74.012,latitude:40.705,zoom:2.5,pitch:0,bearing:0,maxZoom:22,minZoom:0},H="mapbox://styles/mapbox/light-v11";function N(e){const{userInput:t=[],inputGeoJson:n=null,userValidHex:o=!1,initialViewState:r=T,mapStyle:a=H,objectOnClick:s,coordinateOnClick:c}=e,u=(0,v.A)(),[d,h]=(0,l.useState)(r),[m,p]=(0,l.useState)(!1),g=(0,l.useRef)(),[k,L]=(0,l.useState)(null);(0,l.useEffect)((()=>{const e=()=>{L(window.width)};return e(),window.addEventListener("resize",e),()=>window.removeEventListener("resize",e)}),[L]),(0,l.useEffect)((()=>{if(o&&g.current){const{width:e,height:n}=g.current.deck,l=new w.A({width:e,height:n});let o=1/0,r=1/0,a=-1/0,s=-1/0;for(const c of t){const e=(0,i.cellToBoundary)(c,!0);for(const t of e)t[0]<o?o=t[0]:t[0]>a&&(a=t[0]),t[1]<r?r=t[1]:t[1]>s&&(s=t[1])}if(Number.isFinite(o)&&Number.isFinite(r)&&Number.isFinite(a)&&Number.isFinite(s)&&e>1&&n>1){const{latitude:e,longitude:t,zoom:n}=l.fitBounds([[o,r],[a,s]],{padding:96});h({latitude:e,longitude:t,zoom:n,transitionInterpolator:new E.A,transitionDuration:1600})}}}),[t,o,m]);const I=(0,l.useCallback)((e=>{s({hex:e})}),[s]),{handleResize:A,hexLayers:S,resolution:N}=F({resolutionFrozen:!1,addSelectedHexes:I}),P=n?[new U.A({id:"userinput",data:n,getFillColor:[0,0,0],getLineColor:[100,100,100],getLineWidth:1,lineWidthMinPixels:1,lineWidthUnits:"pixels",pickable:!1,stroked:!0,filled:!1,getDashArray:[5,1],dashJustified:!0,dashGapPickable:!0,extensions:[new C.A({dash:!0})]})]:[],z=o?[new b.A({id:"userhex",data:t.map((e=>({hex:e}))),getHexagon:e=>e.hex,extruded:!1,filled:!1,stroked:!0,getLineColor:[0,0,0],getLineWidth:2,lineWidthUnits:"pixels",lineWidthMinPixels:2,highPrecision:!0,pickable:!0,filled:!0,getFillColor:[0,0,0,30]}),...P]:S,W=(0,l.useCallback)((e=>{let{object:t}=e;if(t&&t.hex)return{html:`<tt>${t.hex}</tt>`}}),[]),j=(0,l.useCallback)((e=>{let{isHovering:t}=e;return t?"pointer":"auto"}),[]),D=(0,l.useCallback)((e=>{let{object:t,coordinate:n}=e;t&&t.hex?s&&s({hex:t.hex}):t&&t instanceof string?s&&s({hex:t}):c&&c({coordinate:n,resolution:N})}),[s,c,N]);return l.createElement(f.A,{ref:g,layers:z,initialViewState:d,onViewStateChange:e=>{let{viewState:t}=e;A(t)},views:new y.A({repeat:!0}),getTooltip:W,getCursor:j,onClick:D,onLoad:()=>p(!0),controller:{dragPan:k&&k>=480,dragRotate:!1},touchAction:"pan-y"},l.createElement(x.T5,{reuseMaps:!0,interactive:!1,projection:"mercator",mapboxAccessToken:u.siteConfig.customFields.mapboxAccessToken,mapStyle:a}))}const P=e=>{let{setUserInput:t}=e;const[n,o]=(0,l.useState)(""),r=(0,l.useCallback)((async()=>{"geolocation"in navigator?(o("Locating..."),navigator.geolocation.getCurrentPosition((e=>{t((0,i.latLngToCell)(e.coords.latitude,e.coords.longitude,11)),o("")}),(()=>{o("Error")}))):o("No location services")}),[t]);return l.createElement("div",{style:{position:"absolute",right:"12px",top:"12px"}},l.createElement("button",{type:"button",onClick:r,title:"Where am I?",disabled:Boolean(n)},n||l.createElement("img",{style:{filter:"grayscale(1)"},src:"images/icon-high-precision.svg",alt:"Where am I?",title:"Where am I?"})))};var z=n(3587),W=n.n(z),j=n(6417),D=n.n(j);function M(e){return e?e.trim():""}function R(e){try{if(/^\d+$/.test(e)){const t=BigInt(e).toString(16);if((0,i.isValidCell)(t))return t}}catch{}return null}function B(e,t){"Polygon"!==e.type&&"MultiPolygon"!==e.type||(e={type:"Feature",geometry:e});const n=-1!==t;for(let l=0;l<16;l++){const i=W().featureToH3Set(e,l);if(!n&&(i.length>50||15===l)||n&&(i.length>5e3||l===t))return{splitUserInput:i,showCellId:!1,showResolutionInput:l,inputGeoJson:e}}return null}function V(e,t){if(e){const o=function(e,t){try{const n=JSON.parse(e);if(n&&n.type){const e=B(n,t);if(e)return e}}catch{}try{const n=D().parse(e);if(n&&n.type){const e=B(n,t);if(e)return e}}catch{}}(e,t);if(o)return o;let r=!1,a=null;const s=((l=e)?l=(l=(l=(l=(l=(l=(l=(l=l.trim()).replaceAll("["," ")).replaceAll("]"," ")).replaceAll('"'," ")).replaceAll("'"," ")).replaceAll(","," ")).replaceAll("{"," ")).replaceAll("}"," "):"").split(/\s/).filter((e=>""!==e)),c=[];for(let e=0;e<s.length;e++){const l=M(s[e]),o=M(s[e+1]),u=R(l),d=(n=l)&&n.startsWith("0x")?n.substring(2):null;if((0,i.isValidCell)(l))c.push(l);else if(null!==d&&(0,i.isValidCell)(d))c.push(d),r=!0;else if(u)c.push(u),r=!0;else if(e<s.length-1&&Number.isFinite(Number.parseFloat(l))&&Number.isFinite(Number.parseFloat(o))){const n=Number.parseFloat(l),s=Number.parseFloat(o);if(-1===t)for(let e=0;e<16;e++)c.push((0,i.latLngToCell)(n,s,e));else c.push((0,i.latLngToCell)(n,s,t)),r=!0;a=-1,e++}}return{splitUserInput:c,showResolutionInput:a,showCellId:r,inputGeoJson:null}}var n,l;return{splitUserInput:[],showCellId:!1,showResolutionInput:null,inputGeoJson:null}}function J(e){let{children:t}=e;const[n,o]=(0,h.ZA)("hex",""),[r,c]=(0,h.ZA)("res",-1),{splitUserInput:m,showCellId:p,inputGeoJson:x,showResolutionInput:f}=(0,l.useMemo)((()=>V(n,r)),[n,r]),b=(0,l.useMemo)((()=>m.map(i.isValidCell).includes(!0)),[m]),w=(0,l.useMemo)((()=>{const e=new Set(m.map(i.getResolution));return 1===e.size?[...e][0]:void 0}),[m]),E=(0,l.useCallback)((e=>{let{hex:t}=e;const n=new Set(m);n.delete(t)||n.add(t),o([...n].join(", "))}),[m,o]),y=(0,l.useCallback)((e=>{let{coordinate:t,zoom:n,resolution:l}=e;if(void 0!==w){const e=new Set(m);e.add((0,i.latLngToCell)(t[1],t[0],w)),o([...e].join(", "))}else if(0===m.length){const e=void 0!==l?l:function(e){return Math.max(Math.min(e/1.5,15),0)}(n);o(`${(0,i.latLngToCell)(t[1],t[0],e)}`)}}),[m,o,w]);return l.createElement(l.Fragment,null,l.createElement(s,null,l.createElement(d,null,l.createElement(a,null,l.createElement(N,{userInput:m,inputGeoJson:x,userValidHex:b,objectOnClick:E,coordinateOnClick:y}))),l.createElement(u,null,l.createElement("textarea",{value:n,onChange:e=>{o(e.target.value)},placeholder:"Click on map or enter cell IDs",style:{marginRight:"0.5rem",height:"3em",minHeight:"2em",maxHeight:"10em",width:"100%",resize:"vertical"}}),m.length?l.createElement(g,{splitUserInput:m,showCellId:p,setUserInput:o,showNavigation:!1,showDetails:!0}):null,null!==f?l.createElement("div",null,l.createElement("input",{type:"number",min:"0",max:"15",placeholder:"Resolution",value:`${r}`,onChange:e=>{try{const t=parseInt(e.target.value,10);!isNaN(t)&&t>=0&&t<=15&&c(t)}catch(t){console.error(t)}}})):null),l.createElement(P,{setUserInput:o})),t)}},192(e,t,n){n.r(t),n.d(t,{default:()=>s});var l=n(6540),i=n(3540),o=n(2354),r=n(8478);const a=i.Ay.div`
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
`;function s(){return l.createElement(o.A,{title:"Home",description:"H3"},l.createElement(r.A,null,(()=>{const e=n(4637).L;return l.createElement(e,null)})),l.createElement("div",{style:{position:"relative"}},l.createElement(a,null,l.createElement("h2",null,"H3 indexes points and shapes into a hexagonal grid."),l.createElement("hr",{className:"short"}),l.createElement("p",null,"H3 is a discrete global grid system for indexing geographies into a hexagonal grid, developed at Uber."),l.createElement("p",null,"Coordinates can be indexed to cell IDs that each represent a unique cell."),l.createElement("p",null,"Indexed data can be quickly joined across disparate datasets and aggregated at different levels of precision."),l.createElement("p",null,"H3 enables a range of algorithms and optimizations based on the grid, including nearest neighbors, shortest path, gradient smoothing, and more."))))}}}]);