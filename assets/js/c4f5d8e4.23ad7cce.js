"use strict";(globalThis.webpackChunkh3_website||=[]).push([[2634],{8682(e,t,n){n.d(t,{L:()=>q});var i=n(6540),s=n(4923),o=n(1474);const r=e=>"@media screen and (max-width: 480px)",l=o.Ay.div`
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
`,a=o.Ay.section`
  position: relative;
  height: 30rem;
  background: var(--ifm-color-gray-400);
  color: var(--ifm-color-gray-900);
  z-index: 0;
  ${r} {
    height: 40vh;
  }
`,d=o.Ay.div`
  position: relative;
  padding: 2rem;
  max-width: 80rem;
  width: 100%;
  height: 100%;
  margin: 0;
`,c=(0,o.Ay)(d)`
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
`,h=o.Ay.div`
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
`;var u=n(3681),x=n(4848);function p(e){return(0,s.Hn)(e)<8?{area:s.QN.km2,dist:s.QN.km}:{area:s.QN.m2,dist:s.QN.m}}function g({hex:e,setUserInput:t,onHoverCells:n}){const s=(0,i.useCallback)(()=>{t(e),n&&n([])},[e,t,n]),o=(0,i.useCallback)(()=>{n&&n([e])},[e,n]),r=(0,i.useCallback)(()=>{n&&n([])},[n]);return(0,x.jsx)("a",{onClick:s,style:{cursor:"pointer"},onMouseEnter:o,onMouseLeave:r,children:e})}function m({hexes:e,setUserInput:t,showAll:n=!0,onHoverCells:s}){const o=(0,i.useCallback)(()=>{t(e.join(", "))},[e,t]),r=(0,i.useCallback)(()=>{s&&s(e)},[e,s]),l=(0,i.useCallback)(()=>{s&&s([])},[s]);return(0,x.jsxs)(x.Fragment,{children:[e.map((e,n)=>{const i=(0,x.jsx)(g,{setUserInput:t,hex:e,onHoverCells:s},e);return 0===n?i:(0,x.jsxs)("span",{children:[", ",i]},e)}),n?(0,x.jsxs)(x.Fragment,{children:["\xa0",(0,x.jsx)("a",{onClick:o,style:{cursor:"pointer"},onMouseEnter:r,onMouseLeave:l,children:"(show all)"})]}):(0,x.jsx)(x.Fragment,{})]})}function b({edge:e,showDetails:t=!0}){const n=(0,s.OQ)(e),i=(0,s.gw)(e),o=(0,s.Hn)(n),r=p(n),l=o/3+7,a=(0,s.Wy)(e,r.dist).toPrecision(l);return(0,x.jsxs)("p",{style:{marginBottom:"0"},children:[(0,x.jsx)("br",{}),"Directed edge: ",(0,x.jsx)("tt",{children:e}),t?(0,x.jsxs)("details",{children:[(0,x.jsx)("summary",{children:"Details"}),"Resolution: ",(0,x.jsx)("tt",{children:o}),(0,x.jsx)("br",{}),"Origin: ",(0,x.jsx)("tt",{children:n}),(0,x.jsx)("br",{}),"Destination: ",(0,x.jsx)("tt",{children:i}),(0,x.jsx)("br",{}),"Edge Length: ",(0,x.jsx)("tt",{children:a})," ",r.dist]}):(0,x.jsx)(x.Fragment,{})]})}function j({setUserInput:e,showCellId:t,splitUserInput:n,showNavigation:i=!0,showDetails:o=!0,onHoverCells:r}){if(1===n.length){const l=n[0],a=p(l),d=(0,s.Hn)(l),c=d/3+7,h=(0,s.l3)(l),u=(0,s.PB)(l),b=(0,s.O$)(l).join(", "),j=(0,s._k)(l).map(e=>e.toPrecision(c)).join(", "),f=0===d?"(none)":function(e){const t=(0,s.h6)(e);return[t[1]>>10&7,t[1]>>7&7,t[1]>>4&7,t[1]>>1&7,(1&t[1])<<2|t[0]>>30&3,t[0]>>27&7,t[0]>>24&7,t[0]>>21&7,t[0]>>18&7,t[0]>>15&7,t[0]>>12&7,t[0]>>9&7,t[0]>>6&7,t[0]>>3&7,7&t[0]]}(l).slice(0,d).join(""),w=(0,s.u$)(l),k=(0,s.Sq)(l,a.area).toPrecision(c),v=(0,s.Wq)(l).map(e=>(0,s.Wy)(e,a.dist)),C=(v.reduce((e,t)=>void 0!==e?e+t:t)/v.length).toPrecision(c),y=0!==d&&(0,s.s7)(l,d-1),L=15!==d&&(0,s.xZ)(l,d+1),I=(0,s.uE)(l,1).filter(e=>e!==l);return(0,x.jsxs)("p",{style:{marginBottom:"0"},children:[t?(0,x.jsxs)(x.Fragment,{children:[(0,x.jsx)("br",{}),"ID: ",(0,x.jsx)(g,{hex:l,setUserInput:e})]}):null,i?(0,x.jsxs)("details",{children:[(0,x.jsx)("summary",{children:"Relations"}),"Parent:"," ",y?(0,x.jsx)(g,{hex:y,setUserInput:e,onHoverCells:r}):(0,x.jsx)("tt",{children:"(none)"}),(0,x.jsx)("br",{}),"Children:"," ",L?(0,x.jsx)(m,{hexes:L,setUserInput:e,onHoverCells:r}):(0,x.jsx)("tt",{children:"(none)"}),(0,x.jsx)("br",{}),"Neighbors:"," ",(0,x.jsx)(m,{hexes:I,setUserInput:e,onHoverCells:r}),(0,x.jsx)("br",{})]}):(0,x.jsx)(x.Fragment,{}),o?(0,x.jsxs)("details",{children:[(0,x.jsx)("summary",{children:"Details"}),"Lat./Lng.: ",(0,x.jsx)("tt",{children:j}),(0,x.jsx)("br",{}),"Resolution: ",(0,x.jsx)("tt",{children:d}),(0,x.jsx)("br",{}),"Base cell: ",(0,x.jsx)("tt",{children:h}),(0,x.jsx)("br",{}),"Pentagon: ",(0,x.jsx)("tt",{children:`${u}`}),(0,x.jsx)("br",{}),"Icosa Face IDs: ",(0,x.jsx)("tt",{children:b}),(0,x.jsx)("br",{}),"# of Boundary Verts: ",(0,x.jsx)("tt",{children:w.length}),(0,x.jsx)("br",{}),"Cell Area: ",(0,x.jsx)("tt",{children:k})," ",a.area,(0,x.jsx)("br",{}),"Mean Edge Length: ",(0,x.jsx)("tt",{children:C})," ",a.dist,(0,x.jsx)("br",{}),"Indexing Digits: ",(0,x.jsx)("tt",{children:f})]}):(0,x.jsx)(x.Fragment,{})]})}return(0,x.jsx)("p",{style:{marginBottom:"0"},children:(0,x.jsx)(m,{hexes:n,setUserInput:e,showAll:!1,onHoverCells:r})})}var f=n(8929),w=n(1018),k=n(4437),v=n(6992),C=n(4299),y=n(3353),L=n(4586);const I=(e,t=300)=>{let n,i,s;return function(...o){n?(clearTimeout(i),i=setTimeout(()=>{Date.now()-s>=t&&(e.apply(this,o),s=Date.now())},Math.max(t-(Date.now()-s),0))):(e.apply(this,o),s=Date.now(),n=!0)}};[...Array(15).keys()].map(e=>5e3*(0,s.gb)(e,s.QN.m2));var A=n(3485);function H(e){return function(e,t,n){const i=n-t;return((e-t)%i+i)%i+t}(e,-180,180)}const S=({minLat:e,minLon:t,maxLat:n,maxLon:i})=>function(e,t){const n=[];let i=e;const s=H(t);for(;i<t;){const e=H(i),o=e<0?0:180;i=i+o-e,i>t?n.push([e,s]):n.push([e,o])}return n}(t,i).map(([t,i])=>({minLat:e,minLon:t,maxLat:n,maxLon:i}));const F=({minLat:e,minLon:t,maxLat:n,maxLon:i})=>[[t,e],[i,e],[i,n],[t,n],[t,e]],M={0:0,1:0,2:1,3:2,4:2,5:3,6:4,7:5,8:5,9:6,10:6,11:7,12:8,13:9,14:10,15:11,16:11,17:12,18:12,19:13,20:14,21:15},P=(Object.entries(M).reduce((e,[t,n])=>({...e,[n]:+t}),{}),(e,t)=>{if(t>15)return[];const n=S(e).map(F);return[...new Set(n.flatMap(e=>(0,s.r1)(e,t,!0)))]}),U=({resolutionFrozen:e,addSelectedHexes:t})=>{const[n,s]=(0,i.useState)([]),[o,r]=(0,i.useState)([]),[l,a]=(0,i.useState)([]),[d,c]=(0,i.useState)(0),[h,u]=(0,i.useState)(null);(0,i.useEffect)(()=>{u([new k.A({id:"H3HexagonLayer",extruded:!1,getHexagon:e=>e,getFillColor:[0,0,0,1],getLineColor:[0,0,0,100],getLineWidth:2,lineWidthMinPixels:2,highPrecision:!0,lineWidthUnits:"pixels",elevationScale:20,pickable:!0,stroked:!0,filled:!0,data:n,wrapLongitude:!0,onClick:e=>{e.object&&t([e.object])}}),...o.length?[new k.A({id:"H3HexagonLayer1",extruded:!1,getHexagon:e=>e,getFillColor:[0,0,0,1],getLineColor:[50,50,50,75],getLineWidth:1,lineWidthMinPixels:1,highPrecision:!0,lineWidthUnits:"pixels",elevationScale:20,pickable:!1,stroked:!0,filled:!0,data:o,wrapLongitude:!0})]:[],...l.length?[new k.A({id:"H3HexagonLayer2",extruded:!1,getHexagon:e=>e,getFillColor:[0,0,0,1],getLineColor:[100,100,100,50],getLineWidth:1,lineWidthMinPixels:1,highPrecision:!0,lineWidthUnits:"pixels",elevationScale:20,pickable:!1,stroked:!0,filled:!0,data:l,wrapLongitude:!0,getDashArray:[5,1],dashJustified:!0,dashGapPickable:!0,extensions:[new A.A({dash:!0})]})]:[]])},[t,n,o,l]);const x=(0,i.useCallback)(I(t=>{if(e)return;const n=t.zoom,i=Math.max(0,M[Math.round(n)]-1);c(i);const o=(e=>{const t=new v.A(e),{width:n,height:i}=e,s=t.unproject([0,0]),o=t.unproject([n,i]);return{minLat:o[1],minLon:s[0],maxLat:s[1],maxLon:o[0]}})(t),l=P(o,i);s(l);const d=P(o,i+1);r(d);const h=P(o,i+2);a(h)},200),[s,r,a,c,e]);return{handleResize:x,hexLayers:h,resolution:d}};var W=n(2820),N=n(2114),z=n(5293);const D={longitude:-74.012,latitude:40.705,zoom:2.5,pitch:0,bearing:0,maxZoom:22,minZoom:0};function J(e){const{userInput:t=[],userEdges:n=[],inputGeoJson:o=null,userValidInput:r=!1,initialViewState:l=D,objectOnClick:a,coordinateOnClick:d,previewCells:c}=e,h=(0,L.A)(),[u,p]=(0,i.useState)(l),[g,m]=(0,i.useState)(!1),b=(0,i.useRef)(),[j,I]=(0,i.useState)(null),{colorMode:H}=(0,z.G)();(0,i.useEffect)(()=>{const e=()=>{I(window.width)};return e(),window.addEventListener("resize",e),()=>window.removeEventListener("resize",e)},[I]),(0,i.useEffect)(()=>{if(r&&b.current){const{width:e,height:i}=b.current.deck,o=new v.A({width:e,height:i});let r=1/0,l=1/0,a=-1/0,d=-1/0;const c=[...t.map(e=>(0,s.u$)(e,!0)),...n.map(e=>(0,s.HQ)(e,!0))];for(const t of c)for(const e of t)e[0]<r?r=e[0]:e[0]>a&&(a=e[0]),e[1]<l?l=e[1]:e[1]>d&&(d=e[1]);if(Number.isFinite(r)&&Number.isFinite(l)&&Number.isFinite(a)&&Number.isFinite(d)&&e>1&&i>1){const{latitude:e,longitude:t,zoom:n}=o.fitBounds([[r,l],[a,d]],{padding:96});p({latitude:e,longitude:t,zoom:n,transitionInterpolator:new C.A,transitionDuration:1600})}}},[t,n,r,g]);const S=(0,i.useCallback)(e=>{a({hex:e})},[a]),{handleResize:F,hexLayers:M,resolution:P}=U({resolutionFrozen:!1,addSelectedHexes:S}),J=o?[new W.A({id:"userinput",data:o,getFillColor:"dark"===H?[250,250,250]:[0,0,0],getLineColor:"dark"===H?[220,220,220]:[100,100,100],getLineWidth:1,lineWidthMinPixels:1,lineWidthUnits:"pixels",pickable:!1,stroked:!0,filled:!1,getDashArray:[5,1],dashJustified:!0,dashGapPickable:!0,extensions:[new A.A({dash:!0})]})]:[],E=c?[new k.A({id:"previewhex",data:c.map(e=>({hex:e})),getHexagon:e=>e.hex,extruded:!1,filled:!1,stroked:!0,getLineColor:"dark"===H?[140,140,140]:[120,120,120],getLineWidth:2,lineWidthUnits:"pixels",lineWidthMinPixels:1,highPrecision:!0,pickable:!1,filled:!1,getDashArray:[5,5],dashJustified:!0,dashGapPickable:!0,extensions:[new A.A({dash:!0})]})]:[],R=n.length?[new N.A({id:"useredge",data:n.map(e=>({hex:e})),getPath:e=>(0,s.HQ)(e.hex,!0),getColor:"dark"===H?[255,255,255]:[0,0,0],getWidth:3,widthUnits:"pixels",widthMinPixels:3,pickable:!0})]:[],$=r?[new k.A({id:"userhex",data:t.map(e=>({hex:e})),getHexagon:e=>e.hex,extruded:!1,filled:!1,stroked:!0,getLineColor:"dark"===H?[255,255,255]:[0,0,0],getLineWidth:2,lineWidthUnits:"pixels",lineWidthMinPixels:2,highPrecision:!0,pickable:!0,filled:!0,getFillColor:"dark"===H?[255,255,255,30]:[0,0,0,30]}),...R,...E,...J]:M,G=(0,i.useCallback)(({object:e})=>{if(e&&e.hex)return{html:`<tt>${e.hex}</tt>`,style:"dark"===H?{backgroundColor:"black",color:"white"}:{backgroundColor:"white",color:"black"}}},[H]),Q=(0,i.useCallback)(({isHovering:e})=>e?"pointer":"auto",[]),B=(0,i.useCallback)(({object:e,coordinate:t})=>{e&&e.hex?a&&a({hex:e.hex}):e&&"string"==typeof e?a&&a({hex:e}):d&&d({coordinate:t,resolution:P})},[a,d,P]);return(0,x.jsx)(w.A,{ref:b,layers:$,initialViewState:u,onViewStateChange:({viewState:e})=>{F(e)},views:new y.A({repeat:!0}),getTooltip:G,getCursor:Q,onClick:B,onLoad:()=>m(!0),controller:{dragPan:j&&j>=480,dragRotate:!1},touchAction:"pan-y",children:(0,x.jsx)(f.T5,{reuseMaps:!0,interactive:!1,projection:"mercator",mapboxAccessToken:h.siteConfig.customFields.mapboxAccessToken,mapStyle:"dark"===H?"mapbox://styles/mapbox/dark-v11":"mapbox://styles/mapbox/light-v11"})})}const E=({setUserInput:e})=>{const[t,n]=(0,i.useState)(""),o=(0,i.useCallback)(async()=>{"geolocation"in navigator?(n("Locating..."),navigator.geolocation.getCurrentPosition(t=>{e((0,s.Sg)(t.coords.latitude,t.coords.longitude,11)),n("")},()=>{n("Error")})):n("No location services")},[e]);return(0,x.jsx)("div",{style:{position:"absolute",right:"12px",top:"12px"},children:(0,x.jsx)("button",{type:"button",onClick:o,title:"Where am I?",disabled:Boolean(t),children:t||(0,x.jsx)("img",{style:{filter:"grayscale(1)"},src:"images/icon-high-precision.svg",alt:"Where am I?",title:"Where am I?"})})})};var R=n(3587),$=n.n(R),G=n(6417),Q=n.n(G);function B(e){return e?e.trim():""}function O(e){try{if(/^\d+$/.test(e)){const t=BigInt(e).toString(16);if((0,s.JS)(t))return t}}catch{}return null}function T(e){return e&&e.startsWith("0x")?e.substring(2):null}function V(e,t){"Polygon"!==e.type&&"MultiPolygon"!==e.type||(e={type:"Feature",geometry:e});const n=-1!==t;for(let i=0;i<16;i++){const s=$().featureToH3Set(e,i);if(!n&&(s.length>50||15===i)||n&&(s.length>5e3||i===t))return{splitUserInput:s,showCellId:!1,showResolutionInput:i,inputGeoJson:e}}return null}function Z(e,t){if(e){const i=function(e,t){try{const n=JSON.parse(e);if(n&&n.type){const e=V(n,t);if(e)return e}}catch{}try{const n=Q().parse(e);if(n&&n.type){const e=V(n,t);if(e)return e}}catch{}}(e,t);if(i)return i;let o=!1,r=null;const l=((n=e)?n=(n=(n=(n=(n=(n=(n=(n=n.trim()).replaceAll("["," ")).replaceAll("]"," ")).replaceAll('"'," ")).replaceAll("'"," ")).replaceAll(","," ")).replaceAll("{"," ")).replaceAll("}"," "):"").split(/\s/).filter(e=>""!==e),a=[];for(let e=0;e<l.length;e++){const n=B(l[e]),i=B(l[e+1]),d=O(n),c=T(n);if((0,s.JS)(n))a.push(n);else if(null!==c&&(0,s.JS)(c))a.push(c),o=!0;else if(d)a.push(d),o=!0;else if((0,s.Qn)(n))a.push(n);else if(null!==c&&(0,s.Qn)(c))a.push(c),o=!0;else if(e<l.length-1&&Number.isFinite(Number.parseFloat(n))&&Number.isFinite(Number.parseFloat(i))){const l=Number.parseFloat(n),d=Number.parseFloat(i);if(-1===t)for(let e=0;e<16;e++)a.push((0,s.Sg)(l,d,e));else a.push((0,s.Sg)(l,d,t)),o=!0;r=-1,e++}}return{splitUserInput:a,showResolutionInput:r,showCellId:o,inputGeoJson:null}}var n;return{splitUserInput:[],showCellId:!1,showResolutionInput:null,inputGeoJson:null}}function q({children:e}){const[t,n]=(0,u.ZA)("hex",""),[o,r]=(0,u.ZA)("res",-1),[d,p]=(0,i.useState)([]),{colorMode:g}=(0,z.G)(),m=(0,i.useId)(),{splitUserInput:f,showCellId:w,inputGeoJson:k,showResolutionInput:v}=(0,i.useMemo)(()=>Z(t,o),[t,o]),C=(0,i.useMemo)(()=>f.filter(s.JS),[f]),y=(0,i.useMemo)(()=>f.filter(s.Qn),[f]),L=C.length>0||y.length>0,I=(0,i.useMemo)(()=>{const e=new Set(C.map(s.Hn));return 1===e.size?[...e][0]:void 0},[C]),A=(0,i.useCallback)(({hex:e})=>{const t=new Set(f);t.delete(e)||t.add(e),n([...t].join(", "))},[f,n]),H=(0,i.useCallback)(({coordinate:e,zoom:t,resolution:i})=>{if(void 0!==I){const t=new Set(f);t.add((0,s.Sg)(e[1],e[0],I)),n([...t].join(", "))}else if(0===f.length){const o=void 0!==i?i:function(e){return Math.max(Math.min(e/1.5,15),0)}(t);n(`${(0,s.Sg)(e[1],e[0],o)}`)}},[f,n,I]);return(0,x.jsxs)(x.Fragment,{children:[(0,x.jsxs)(a,{children:[(0,x.jsx)(h,{children:(0,x.jsx)(l,{children:(0,x.jsx)(J,{userInput:C,userEdges:y,inputGeoJson:k,userValidInput:L,objectOnClick:A,coordinateOnClick:H,previewCells:d})})}),(0,x.jsxs)(c,{colorMode:g,children:[(0,x.jsx)("textarea",{value:t,onChange:e=>{n(e.target.value)},placeholder:"Click on map or enter cell IDs",style:{marginRight:"0.5rem",height:"3em",minHeight:"2em",maxHeight:"10em",width:"100%",resize:"vertical"}}),C.length?(0,x.jsx)(j,{splitUserInput:C,showCellId:w,setUserInput:n,showNavigation:!0,showDetails:!0,onHoverCells:p}):null,1===y.length?(0,x.jsx)(b,{edge:y[0]}):null,null!==v?(0,x.jsxs)("div",{children:[(0,x.jsx)("label",{htmlFor:m,children:"Resolution:"}),(0,x.jsx)("input",{id:m,type:"number",min:"0",max:"15",placeholder:"Auto",value:`${-1!==o?o:""}`,onChange:e=>{try{const t=parseInt(e.target.value,10);!isNaN(t)&&t>=0&&t<=15&&r(t)}catch(t){console.error(t)}},style:{marginLeft:"0.25em"}})]}):null]}),(0,x.jsx)(E,{setUserInput:n})]}),e]})}},192(e,t,n){n.r(t),n.d(t,{default:()=>a});n(6540);var i=n(1474),s=n(4042),o=n(8478),r=n(4848);const l=i.Ay.div`
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
`;function a(){return(0,r.jsxs)(s.A,{title:"Home",description:"H3",children:[(0,r.jsx)(o.A,{children:()=>{const e=n(8682).L;return(0,r.jsx)(e,{})}}),(0,r.jsx)("div",{style:{position:"relative"},children:(0,r.jsxs)(l,{children:[(0,r.jsx)("h2",{children:"H3 indexes points and shapes into a hexagonal grid."}),(0,r.jsx)("hr",{className:"short"}),(0,r.jsx)("p",{children:"H3 is a discrete global grid system for indexing geographies into a hexagonal grid, developed at Uber."}),(0,r.jsx)("p",{children:"Coordinates can be indexed to cell IDs that each represent a unique cell."}),(0,r.jsx)("p",{children:"Indexed data can be quickly joined across disparate datasets and aggregated at different levels of precision."}),(0,r.jsx)("p",{children:"H3 enables a range of algorithms and optimizations based on the grid, including nearest neighbors, shortest path, gradient smoothing, and more."})]})})]})}}}]);