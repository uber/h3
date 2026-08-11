"use strict";(globalThis.webpackChunkh3_website||=[]).push([[2634],{4637(e,t,n){n.d(t,{L:()=>V});var i=n(6540),o=n(4923),s=n(1474);const r=e=>"@media screen and (max-width: 480px)",l=s.Ay.div`
  height: 100%;
  ${r} {
    height: 40vh;
  }
  .tooltip,
  .deck-tooltip {
    position: absolute;
    padding: 4px 12px;
    border-radius: 4px;
    max-width: 300px;
    font-size: 12px;
    z-index: 9;
    pointer-events: none;
    white-space: nowrap;
  }
`,a=s.Ay.section`
  position: relative;
  height: 30rem;
  background: var(--ifm-color-gray-400);
  color: var(--ifm-color-gray-900);
  z-index: 0;
  ${r} {
    height: 40vh;
  }
`,d=s.Ay.div`
  position: relative;
  padding: 2rem;
  max-width: 80rem;
  width: 100%;
  height: 100%;
  margin: 0;
`,c=(0,s.Ay)(d)`
  position: absolute;
  bottom: 2rem;
  height: auto;
  max-height: 90%;
  overflow-y: scroll;
  left: 2rem;
  z-index: 0;
  width: auto;
  max-width: 20em;
  background-color: ${({colorMode:e})=>"dark"===e?"black":"white"};
  color: ${({colorMode:e})=>"dark"===e?"white":"black"};
  border-radius: 20px;
  padding: 1rem;
  font-size: 12px;
`,h=s.Ay.div`
  position: absolute;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  z-index: -1;
`;s.Ay.section`
  &:nth-child(2n + 1) {
    background: var(--ifm-color-gray-300);
  }
`;var u=n(3681),x=n(4848);function p({hex:e,setUserInput:t,onHoverCells:n}){const o=(0,i.useCallback)(()=>{t(e),n&&n([])},[e,t,n]),s=(0,i.useCallback)(()=>{n&&n([e])},[e,n]),r=(0,i.useCallback)(()=>{n&&n([])},[n]);return(0,x.jsx)("a",{onClick:o,style:{cursor:"pointer"},onMouseEnter:s,onMouseLeave:r,children:e})}function g({hexes:e,setUserInput:t,showAll:n=!0,onHoverCells:o}){const s=(0,i.useCallback)(()=>{t(e.join(", "))},[e,t]),r=(0,i.useCallback)(()=>{o&&o(e)},[e,o]),l=(0,i.useCallback)(()=>{o&&o([])},[o]);return(0,x.jsxs)(x.Fragment,{children:[e.map((e,n)=>{const i=(0,x.jsx)(p,{setUserInput:t,hex:e,onHoverCells:o},e);return 0===n?i:(0,x.jsxs)("span",{children:[", ",i]},e)}),n?(0,x.jsxs)(x.Fragment,{children:["\xa0",(0,x.jsx)("a",{onClick:s,style:{cursor:"pointer"},onMouseEnter:r,onMouseLeave:l,children:"(show all)"})]}):(0,x.jsx)(x.Fragment,{})]})}function m({setUserInput:e,showCellId:t,splitUserInput:n,showNavigation:i=!0,showDetails:s=!0,onHoverCells:r}){if(1===n.length){const l=n[0],a=function(e){return(0,o.Hn)(e)<8?{area:o.QN.km2,dist:o.QN.km}:{area:o.QN.m2,dist:o.QN.m}}(l),d=(0,o.Hn)(l),c=d/3+7,h=(0,o.l3)(l),u=(0,o.PB)(l),m=(0,o.O$)(l).join(", "),b=(0,o._k)(l).map(e=>e.toPrecision(c)).join(", "),f=0===d?"(none)":function(e){const t=(0,o.h6)(e);return[t[1]>>10&7,t[1]>>7&7,t[1]>>4&7,t[1]>>1&7,(1&t[1])<<2|t[0]>>30&3,t[0]>>27&7,t[0]>>24&7,t[0]>>21&7,t[0]>>18&7,t[0]>>15&7,t[0]>>12&7,t[0]>>9&7,t[0]>>6&7,t[0]>>3&7,7&t[0]]}(l).slice(0,d).join(""),j=(0,o.u$)(l),w=(0,o.Sq)(l,a.area).toPrecision(c),k=(0,o.Wq)(l).map(e=>(0,o.Wy)(e,a.dist)),v=(k.reduce((e,t)=>void 0!==e?e+t:t)/k.length).toPrecision(c),C=0!==d&&(0,o.s7)(l,d-1),y=15!==d&&(0,o.xZ)(l,d+1),L=(0,o.uE)(l,1).filter(e=>e!==l);return(0,x.jsxs)("p",{style:{marginBottom:"0"},children:[t?(0,x.jsxs)(x.Fragment,{children:[(0,x.jsx)("br",{}),"ID: ",(0,x.jsx)(p,{hex:l,setUserInput:e})]}):null,i?(0,x.jsxs)("details",{children:[(0,x.jsx)("summary",{children:"Relations"}),"Parent:"," ",C?(0,x.jsx)(p,{hex:C,setUserInput:e,onHoverCells:r}):(0,x.jsx)("tt",{children:"(none)"}),(0,x.jsx)("br",{}),"Children:"," ",y?(0,x.jsx)(g,{hexes:y,setUserInput:e,onHoverCells:r}):(0,x.jsx)("tt",{children:"(none)"}),(0,x.jsx)("br",{}),"Neighbors:"," ",(0,x.jsx)(g,{hexes:L,setUserInput:e,onHoverCells:r}),(0,x.jsx)("br",{})]}):(0,x.jsx)(x.Fragment,{}),s?(0,x.jsxs)("details",{children:[(0,x.jsx)("summary",{children:"Details"}),"Lat./Lng.: ",(0,x.jsx)("tt",{children:b}),(0,x.jsx)("br",{}),"Resolution: ",(0,x.jsx)("tt",{children:d}),(0,x.jsx)("br",{}),"Base cell: ",(0,x.jsx)("tt",{children:h}),(0,x.jsx)("br",{}),"Pentagon: ",(0,x.jsx)("tt",{children:`${u}`}),(0,x.jsx)("br",{}),"Icosa Face IDs: ",(0,x.jsx)("tt",{children:m}),(0,x.jsx)("br",{}),"# of Boundary Verts: ",(0,x.jsx)("tt",{children:j.length}),(0,x.jsx)("br",{}),"Cell Area: ",(0,x.jsx)("tt",{children:w})," ",a.area,(0,x.jsx)("br",{}),"Mean Edge Length: ",(0,x.jsx)("tt",{children:v})," ",a.dist,(0,x.jsx)("br",{}),"Indexing Digits: ",(0,x.jsx)("tt",{children:f})]}):(0,x.jsx)(x.Fragment,{})]})}return(0,x.jsx)("p",{style:{marginBottom:"0"},children:(0,x.jsx)(g,{hexes:n,setUserInput:e,showAll:!1,onHoverCells:r})})}var b=n(8929),f=n(1018),j=n(4437),w=n(6992),k=n(4299),v=n(3353),C=n(4586);const y=(e,t=300)=>{let n,i,o;return function(...s){n?(clearTimeout(i),i=setTimeout(()=>{Date.now()-o>=t&&(e.apply(this,s),o=Date.now())},Math.max(t-(Date.now()-o),0))):(e.apply(this,s),o=Date.now(),n=!0)}};[...Array(15).keys()].map(e=>5e3*(0,o.gb)(e,o.QN.m2));var L=n(3485);function A(e){return function(e,t,n){const i=n-t;return((e-t)%i+i)%i+t}(e,-180,180)}const I=({minLat:e,minLon:t,maxLat:n,maxLon:i})=>function(e,t){const n=[];let i=e;const o=A(t);for(;i<t;){const e=A(i),s=e<0?0:180;i=i+s-e,i>t?n.push([e,o]):n.push([e,s])}return n}(t,i).map(([t,i])=>({minLat:e,minLon:t,maxLat:n,maxLon:i}));const S=({minLat:e,minLon:t,maxLat:n,maxLon:i})=>[[t,e],[i,e],[i,n],[t,n],[t,e]],H={0:0,1:0,2:1,3:2,4:2,5:3,6:4,7:5,8:5,9:6,10:6,11:7,12:8,13:9,14:10,15:11,16:11,17:12,18:12,19:13,20:14,21:15},F=(Object.entries(H).reduce((e,[t,n])=>({...e,[n]:+t}),{}),(e,t)=>{if(t>15)return[];const n=I(e).map(S);return[...new Set(n.flatMap(e=>(0,o.r1)(e,t,!0)))]}),M=({resolutionFrozen:e,addSelectedHexes:t})=>{const[n,o]=(0,i.useState)([]),[s,r]=(0,i.useState)([]),[l,a]=(0,i.useState)([]),[d,c]=(0,i.useState)(0),[h,u]=(0,i.useState)(null);(0,i.useEffect)(()=>{u([new j.A({id:"H3HexagonLayer",extruded:!1,getHexagon:e=>e,getFillColor:[0,0,0,1],getLineColor:[0,0,0,100],getLineWidth:2,lineWidthMinPixels:2,highPrecision:!0,lineWidthUnits:"pixels",elevationScale:20,pickable:!0,stroked:!0,filled:!0,data:n,wrapLongitude:!0,onClick:e=>{e.object&&t([e.object])}}),...s.length?[new j.A({id:"H3HexagonLayer1",extruded:!1,getHexagon:e=>e,getFillColor:[0,0,0,1],getLineColor:[50,50,50,75],getLineWidth:1,lineWidthMinPixels:1,highPrecision:!0,lineWidthUnits:"pixels",elevationScale:20,pickable:!1,stroked:!0,filled:!0,data:s,wrapLongitude:!0})]:[],...l.length?[new j.A({id:"H3HexagonLayer2",extruded:!1,getHexagon:e=>e,getFillColor:[0,0,0,1],getLineColor:[100,100,100,50],getLineWidth:1,lineWidthMinPixels:1,highPrecision:!0,lineWidthUnits:"pixels",elevationScale:20,pickable:!1,stroked:!0,filled:!0,data:l,wrapLongitude:!0,getDashArray:[5,1],dashJustified:!0,dashGapPickable:!0,extensions:[new L.A({dash:!0})]})]:[]])},[t,n,s,l]);const x=(0,i.useCallback)(y(t=>{if(e)return;const n=t.zoom,i=Math.max(0,H[Math.round(n)]-1);c(i);const s=(e=>{const t=new w.A(e),{width:n,height:i}=e,o=t.unproject([0,0]),s=t.unproject([n,i]);return{minLat:s[1],minLon:o[0],maxLat:o[1],maxLon:s[0]}})(t),l=F(s,i);o(l);const d=F(s,i+1);r(d);const h=F(s,i+2);a(h)},200),[o,r,a,c,e]);return{handleResize:x,hexLayers:h,resolution:d}};var U=n(2820),P=n(5293);const W={longitude:-74.012,latitude:40.705,zoom:2.5,pitch:0,bearing:0,maxZoom:22,minZoom:0};function N(e){const{userInput:t=[],inputGeoJson:n=null,userValidHex:s=!1,initialViewState:r=W,objectOnClick:l,coordinateOnClick:a,previewCells:d}=e,c=(0,C.A)(),[h,u]=(0,i.useState)(r),[p,g]=(0,i.useState)(!1),m=(0,i.useRef)(),[y,A]=(0,i.useState)(null),{colorMode:I}=(0,P.G)();(0,i.useEffect)(()=>{const e=()=>{A(window.width)};return e(),window.addEventListener("resize",e),()=>window.removeEventListener("resize",e)},[A]),(0,i.useEffect)(()=>{if(s&&m.current){const{width:e,height:n}=m.current.deck,i=new w.A({width:e,height:n});let s=1/0,r=1/0,l=-1/0,a=-1/0;for(const d of t){const e=(0,o.u$)(d,!0);for(const t of e)t[0]<s?s=t[0]:t[0]>l&&(l=t[0]),t[1]<r?r=t[1]:t[1]>a&&(a=t[1])}if(Number.isFinite(s)&&Number.isFinite(r)&&Number.isFinite(l)&&Number.isFinite(a)&&e>1&&n>1){const{latitude:e,longitude:t,zoom:n}=i.fitBounds([[s,r],[l,a]],{padding:96});u({latitude:e,longitude:t,zoom:n,transitionInterpolator:new k.A,transitionDuration:1600})}}},[t,s,p]);const S=(0,i.useCallback)(e=>{l({hex:e})},[l]),{handleResize:H,hexLayers:F,resolution:N}=M({resolutionFrozen:!1,addSelectedHexes:S}),z=n?[new U.A({id:"userinput",data:n,getFillColor:"dark"===I?[250,250,250]:[0,0,0],getLineColor:"dark"===I?[220,220,220]:[100,100,100],getLineWidth:1,lineWidthMinPixels:1,lineWidthUnits:"pixels",pickable:!1,stroked:!0,filled:!1,getDashArray:[5,1],dashJustified:!0,dashGapPickable:!0,extensions:[new L.A({dash:!0})]})]:[],D=d?[new j.A({id:"previewhex",data:d.map(e=>({hex:e})),getHexagon:e=>e.hex,extruded:!1,filled:!1,stroked:!0,getLineColor:"dark"===I?[140,140,140]:[120,120,120],getLineWidth:2,lineWidthUnits:"pixels",lineWidthMinPixels:1,highPrecision:!0,pickable:!1,filled:!1,getDashArray:[5,5],dashJustified:!0,dashGapPickable:!0,extensions:[new L.A({dash:!0})]})]:[],J=s?[new j.A({id:"userhex",data:t.map(e=>({hex:e})),getHexagon:e=>e.hex,extruded:!1,filled:!1,stroked:!0,getLineColor:"dark"===I?[255,255,255]:[0,0,0],getLineWidth:2,lineWidthUnits:"pixels",lineWidthMinPixels:2,highPrecision:!0,pickable:!0,filled:!0,getFillColor:"dark"===I?[255,255,255,30]:[0,0,0,30]}),...D,...z]:F,R=(0,i.useCallback)(({object:e})=>{if(e&&e.hex)return{html:`<tt>${e.hex}</tt>`,style:"dark"===I?{backgroundColor:"black",color:"white"}:{backgroundColor:"white",color:"black"}}},[I]),$=(0,i.useCallback)(({isHovering:e})=>e?"pointer":"auto",[]),G=(0,i.useCallback)(({object:e,coordinate:t})=>{e&&e.hex?l&&l({hex:e.hex}):e&&"string"==typeof e?l&&l({hex:e}):a&&a({coordinate:t,resolution:N})},[l,a,N]);return(0,x.jsx)(f.A,{ref:m,layers:J,initialViewState:h,onViewStateChange:({viewState:e})=>{H(e)},views:new v.A({repeat:!0}),getTooltip:R,getCursor:$,onClick:G,onLoad:()=>g(!0),controller:{dragPan:y&&y>=480,dragRotate:!1},touchAction:"pan-y",children:(0,x.jsx)(b.T5,{reuseMaps:!0,interactive:!1,projection:"mercator",mapboxAccessToken:c.siteConfig.customFields.mapboxAccessToken,mapStyle:"dark"===I?"mapbox://styles/mapbox/dark-v11":"mapbox://styles/mapbox/light-v11"})})}const z=({setUserInput:e})=>{const[t,n]=(0,i.useState)(""),s=(0,i.useCallback)(async()=>{"geolocation"in navigator?(n("Locating..."),navigator.geolocation.getCurrentPosition(t=>{e((0,o.Sg)(t.coords.latitude,t.coords.longitude,11)),n("")},()=>{n("Error")})):n("No location services")},[e]);return(0,x.jsx)("div",{style:{position:"absolute",right:"12px",top:"12px"},children:(0,x.jsx)("button",{type:"button",onClick:s,title:"Where am I?",disabled:Boolean(t),children:t||(0,x.jsx)("img",{style:{filter:"grayscale(1)"},src:"images/icon-high-precision.svg",alt:"Where am I?",title:"Where am I?"})})})};var D=n(3587),J=n.n(D),R=n(6417),$=n.n(R);function G(e){return e?e.trim():""}function E(e){try{if(/^\d+$/.test(e)){const t=BigInt(e).toString(16);if((0,o.JS)(t))return t}}catch{}return null}function B(e){return e&&e.startsWith("0x")?e.substring(2):null}function T(e,t){"Polygon"!==e.type&&"MultiPolygon"!==e.type||(e={type:"Feature",geometry:e});const n=-1!==t;for(let i=0;i<16;i++){const o=J().featureToH3Set(e,i);if(!n&&(o.length>50||15===i)||n&&(o.length>5e3||i===t))return{splitUserInput:o,showCellId:!1,showResolutionInput:i,inputGeoJson:e}}return null}function O(e,t){if(e){const i=function(e,t){try{const n=JSON.parse(e);if(n&&n.type){const e=T(n,t);if(e)return e}}catch{}try{const n=$().parse(e);if(n&&n.type){const e=T(n,t);if(e)return e}}catch{}}(e,t);if(i)return i;let s=!1,r=null;const l=((n=e)?n=(n=(n=(n=(n=(n=(n=(n=n.trim()).replaceAll("["," ")).replaceAll("]"," ")).replaceAll('"'," ")).replaceAll("'"," ")).replaceAll(","," ")).replaceAll("{"," ")).replaceAll("}"," "):"").split(/\s/).filter(e=>""!==e),a=[];for(let e=0;e<l.length;e++){const n=G(l[e]),i=G(l[e+1]),d=E(n),c=B(n);if((0,o.JS)(n))a.push(n);else if(null!==c&&(0,o.JS)(c))a.push(c),s=!0;else if(d)a.push(d),s=!0;else if(e<l.length-1&&Number.isFinite(Number.parseFloat(n))&&Number.isFinite(Number.parseFloat(i))){const l=Number.parseFloat(n),d=Number.parseFloat(i);if(-1===t)for(let e=0;e<16;e++)a.push((0,o.Sg)(l,d,e));else a.push((0,o.Sg)(l,d,t)),s=!0;r=-1,e++}}return{splitUserInput:a,showResolutionInput:r,showCellId:s,inputGeoJson:null}}var n;return{splitUserInput:[],showCellId:!1,showResolutionInput:null,inputGeoJson:null}}function V({children:e}){const[t,n]=(0,u.ZA)("hex",""),[s,r]=(0,u.ZA)("res",-1),[d,p]=(0,i.useState)([]),{colorMode:g}=(0,P.G)(),b=(0,i.useId)(),{splitUserInput:f,showCellId:j,inputGeoJson:w,showResolutionInput:k}=(0,i.useMemo)(()=>O(t,s),[t,s]),v=(0,i.useMemo)(()=>f.map(o.JS).includes(!0),[f]),C=(0,i.useMemo)(()=>{const e=new Set(f.map(o.Hn));return 1===e.size?[...e][0]:void 0},[f]),y=(0,i.useCallback)(({hex:e})=>{const t=new Set(f);t.delete(e)||t.add(e),n([...t].join(", "))},[f,n]),L=(0,i.useCallback)(({coordinate:e,zoom:t,resolution:i})=>{if(void 0!==C){const t=new Set(f);t.add((0,o.Sg)(e[1],e[0],C)),n([...t].join(", "))}else if(0===f.length){const s=void 0!==i?i:function(e){return Math.max(Math.min(e/1.5,15),0)}(t);n(`${(0,o.Sg)(e[1],e[0],s)}`)}},[f,n,C]);return(0,x.jsxs)(x.Fragment,{children:[(0,x.jsxs)(a,{children:[(0,x.jsx)(h,{children:(0,x.jsx)(l,{children:(0,x.jsx)(N,{userInput:f,inputGeoJson:w,userValidHex:v,objectOnClick:y,coordinateOnClick:L,previewCells:d})})}),(0,x.jsxs)(c,{colorMode:g,children:[(0,x.jsx)("textarea",{value:t,onChange:e=>{n(e.target.value)},placeholder:"Click on map or enter cell IDs",style:{marginRight:"0.5rem",height:"3em",minHeight:"2em",maxHeight:"10em",width:"100%",resize:"vertical"}}),f.length?(0,x.jsx)(m,{splitUserInput:f,showCellId:j,setUserInput:n,showNavigation:!0,showDetails:!0,onHoverCells:p}):null,null!==k?(0,x.jsxs)("div",{children:[(0,x.jsx)("label",{htmlFor:b,children:"Resolution:"}),(0,x.jsx)("input",{id:b,type:"number",min:"0",max:"15",placeholder:"Auto",value:`${-1!==s?s:""}`,onChange:e=>{try{const t=parseInt(e.target.value,10);!isNaN(t)&&t>=0&&t<=15&&r(t)}catch(t){console.error(t)}},style:{marginLeft:"0.25em"}})]}):null]}),(0,x.jsx)(z,{setUserInput:n})]}),e]})}},192(e,t,n){n.r(t),n.d(t,{default:()=>a});n(6540);var i=n(1474),o=n(4042),s=n(8478),r=n(4848);const l=i.Ay.div`
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
`;function a(){return(0,r.jsxs)(o.A,{title:"Home",description:"H3",children:[(0,r.jsx)(s.A,{children:()=>{const e=n(4637).L;return(0,r.jsx)(e,{})}}),(0,r.jsx)("div",{style:{position:"relative"},children:(0,r.jsxs)(l,{children:[(0,r.jsx)("h2",{children:"H3 indexes points and shapes into a hexagonal grid."}),(0,r.jsx)("hr",{className:"short"}),(0,r.jsx)("p",{children:"H3 is a discrete global grid system for indexing geographies into a hexagonal grid, developed at Uber."}),(0,r.jsx)("p",{children:"Coordinates can be indexed to cell IDs that each represent a unique cell."}),(0,r.jsx)("p",{children:"Indexed data can be quickly joined across disparate datasets and aggregated at different levels of precision."}),(0,r.jsx)("p",{children:"H3 enables a range of algorithms and optimizations based on the grid, including nearest neighbors, shortest path, gradient smoothing, and more."})]})})]})}}}]);