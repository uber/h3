"use strict";(self.webpackChunkh3_website=self.webpackChunkh3_website||[]).push([[1301],{3905:(e,t,a)=>{a.d(t,{Zo:()=>c,kt:()=>d});var l=a(7294);function n(e,t,a){return t in e?Object.defineProperty(e,t,{value:a,enumerable:!0,configurable:!0,writable:!0}):e[t]=a,e}function r(e,t){var a=Object.keys(e);if(Object.getOwnPropertySymbols){var l=Object.getOwnPropertySymbols(e);t&&(l=l.filter((function(t){return Object.getOwnPropertyDescriptor(e,t).enumerable}))),a.push.apply(a,l)}return a}function o(e){for(var t=1;t<arguments.length;t++){var a=null!=arguments[t]?arguments[t]:{};t%2?r(Object(a),!0).forEach((function(t){n(e,t,a[t])})):Object.getOwnPropertyDescriptors?Object.defineProperties(e,Object.getOwnPropertyDescriptors(a)):r(Object(a)).forEach((function(t){Object.defineProperty(e,t,Object.getOwnPropertyDescriptor(a,t))}))}return e}function i(e,t){if(null==e)return{};var a,l,n=function(e,t){if(null==e)return{};var a,l,n={},r=Object.keys(e);for(l=0;l<r.length;l++)a=r[l],t.indexOf(a)>=0||(n[a]=e[a]);return n}(e,t);if(Object.getOwnPropertySymbols){var r=Object.getOwnPropertySymbols(e);for(l=0;l<r.length;l++)a=r[l],t.indexOf(a)>=0||Object.prototype.propertyIsEnumerable.call(e,a)&&(n[a]=e[a])}return n}var u=l.createContext({}),s=function(e){var t=l.useContext(u),a=t;return e&&(a="function"==typeof e?e(t):o(o({},t),e)),a},c=function(e){var t=s(e.components);return l.createElement(u.Provider,{value:t},e.children)},p={inlineCode:"code",wrapper:function(e){var t=e.children;return l.createElement(l.Fragment,{},t)}},v=l.forwardRef((function(e,t){var a=e.components,n=e.mdxType,r=e.originalType,u=e.parentName,c=i(e,["components","mdxType","originalType","parentName"]),v=s(a),d=n,m=v["".concat(u,".").concat(d)]||v[d]||p[d]||r;return a?l.createElement(m,o(o({ref:t},c),{},{components:a})):l.createElement(m,o({ref:t},c))}));function d(e,t){var a=arguments,n=t&&t.mdxType;if("string"==typeof e||n){var r=a.length,o=new Array(r);o[0]=v;var i={};for(var u in t)hasOwnProperty.call(t,u)&&(i[u]=t[u]);i.originalType=e,i.mdxType="string"==typeof e?e:n,o[1]=i;for(var s=2;s<r;s++)o[s]=a[s];return l.createElement.apply(null,o)}return l.createElement.apply(null,a)}v.displayName="MDXCreateElement"},5162:(e,t,a)=>{a.d(t,{Z:()=>o});var l=a(7294),n=a(6010);const r="tabItem_Ymn6";function o(e){let{children:t,hidden:a,className:o}=e;return l.createElement("div",{role:"tabpanel",className:(0,n.Z)(r,o),hidden:a},t)}},5488:(e,t,a)=>{a.d(t,{Z:()=>d});var l=a(7462),n=a(7294),r=a(6010),o=a(2389),i=a(7392),u=a(7094),s=a(2466);const c="tabList__CuJ",p="tabItem_LNqP";function v(e){var t,a;const{lazy:o,block:v,defaultValue:d,values:m,groupId:g,className:x}=e,f=n.Children.map(e.children,(e=>{if((0,n.isValidElement)(e)&&"value"in e.props)return e;throw new Error("Docusaurus error: Bad <Tabs> child <"+("string"==typeof e.type?e.type:e.type.name)+'>: all children of the <Tabs> component should be <TabItem>, and every <TabItem> should have a unique "value" prop.')})),b=null!=m?m:f.map((e=>{let{props:{value:t,label:a,attributes:l}}=e;return{value:t,label:a,attributes:l}})),h=(0,i.l)(b,((e,t)=>e.value===t.value));if(h.length>0)throw new Error('Docusaurus error: Duplicate values "'+h.map((e=>e.value)).join(", ")+'" found in <Tabs>. Every value needs to be unique.');const k=null===d?d:null!=(t=null!=d?d:null==(a=f.find((e=>e.props.default)))?void 0:a.props.value)?t:f[0].props.value;if(null!==k&&!b.some((e=>e.value===k)))throw new Error('Docusaurus error: The <Tabs> has a defaultValue "'+k+'" but none of its children has the corresponding value. Available values are: '+b.map((e=>e.value)).join(", ")+". If you intend to show no default tab, use defaultValue={null} instead.");const{tabGroupChoices:y,setTabGroupChoices:T}=(0,u.U)(),[N,j]=(0,n.useState)(k),V=[],{blockElementScrollPositionUntilNextRender:I}=(0,s.o5)();if(null!=g){const e=y[g];null!=e&&e!==N&&b.some((t=>t.value===e))&&j(e)}const w=e=>{const t=e.currentTarget,a=V.indexOf(t),l=b[a].value;l!==N&&(I(t),j(l),null!=g&&T(g,String(l)))},L=e=>{var t;let a=null;switch(e.key){case"ArrowRight":{var l;const t=V.indexOf(e.currentTarget)+1;a=null!=(l=V[t])?l:V[0];break}case"ArrowLeft":{var n;const t=V.indexOf(e.currentTarget)-1;a=null!=(n=V[t])?n:V[V.length-1];break}}null==(t=a)||t.focus()};return n.createElement("div",{className:(0,r.Z)("tabs-container",c)},n.createElement("ul",{role:"tablist","aria-orientation":"horizontal",className:(0,r.Z)("tabs",{"tabs--block":v},x)},b.map((e=>{let{value:t,label:a,attributes:o}=e;return n.createElement("li",(0,l.Z)({role:"tab",tabIndex:N===t?0:-1,"aria-selected":N===t,key:t,ref:e=>V.push(e),onKeyDown:L,onFocus:w,onClick:w},o,{className:(0,r.Z)("tabs__item",p,null==o?void 0:o.className,{"tabs__item--active":N===t})}),null!=a?a:t)}))),o?(0,n.cloneElement)(f.filter((e=>e.props.value===N))[0],{className:"margin-top--md"}):n.createElement("div",{className:"margin-top--md"},f.map(((e,t)=>(0,n.cloneElement)(e,{key:t,hidden:e.props.value!==N})))))}function d(e){const t=(0,o.Z)();return n.createElement(v,(0,l.Z)({key:String(t)},e))}},9805:(e,t,a)=>{a.r(t),a.d(t,{assets:()=>c,contentTitle:()=>u,default:()=>d,frontMatter:()=>i,metadata:()=>s,toc:()=>p});var l=a(7462),n=(a(7294),a(3905)),r=a(5488),o=a(5162);const i={id:"vertex",title:"Vertex functions",sidebar_label:"Vertexes",slug:"/api/vertex"},u=void 0,s={unversionedId:"api/vertex",id:"api/vertex",title:"Vertex functions",description:"Vertex mode allows encoding the topological vertexes of H3 cells.",source:"@site/docs/api/vertex.mdx",sourceDirName:"api",slug:"/api/vertex",permalink:"/docs/api/vertex",draft:!1,editUrl:"https://github.com/uber/h3/edit/master/website/docs/api/vertex.mdx",tags:[],version:"current",frontMatter:{id:"vertex",title:"Vertex functions",sidebar_label:"Vertexes",slug:"/api/vertex"},sidebar:"someSidebar",previous:{title:"Directed edges",permalink:"/docs/api/uniedge"},next:{title:"Miscellaneous",permalink:"/docs/api/misc"}},c={},p=[{value:"cellToVertex",id:"celltovertex",level:2},{value:"cellToVertexes",id:"celltovertexes",level:2},{value:"vertexToLatLng",id:"vertextolatlng",level:2},{value:"isValidVertex",id:"isvalidvertex",level:2}],v={toc:p};function d(e){let{components:t,...a}=e;return(0,n.kt)("wrapper",(0,l.Z)({},v,a,{components:t,mdxType:"MDXLayout"}),(0,n.kt)("p",null,"Vertex mode allows encoding the topological vertexes of H3 cells."),(0,n.kt)("h2",{id:"celltovertex"},"cellToVertex"),(0,n.kt)(r.Z,{groupId:"language",defaultValue:"c",values:[{label:"C",value:"c"},{label:"Python",value:"python"},{label:"Java",value:"java"},{label:"JavaScript (Live)",value:"javascript"}],mdxType:"Tabs"},(0,n.kt)(o.Z,{value:"c",mdxType:"TabItem"},(0,n.kt)("pre",null,(0,n.kt)("code",{parentName:"pre",className:"language-c"},"H3Error cellToVertex(H3Index origin, int vertexNum, H3Index *out);\n"))),(0,n.kt)(o.Z,{value:"python",mdxType:"TabItem"},(0,n.kt)("pre",null,(0,n.kt)("code",{parentName:"pre",className:"language-py"},"h3.cell_to_vertex(origin, vertex_num)\n"))),(0,n.kt)(o.Z,{value:"java",mdxType:"TabItem"},(0,n.kt)("pre",null,(0,n.kt)("code",{parentName:"pre",className:"language-java"},"Long cellToVertex(long origin, int vertexNum);\nString cellToVertex(String origin, int vertexNum);\n"))),(0,n.kt)(o.Z,{value:"javascript",mdxType:"TabItem"},(0,n.kt)("pre",null,(0,n.kt)("code",{parentName:"pre",className:"language-js"},"h3.cellToVertex(origin, vertexNum)\n")),(0,n.kt)("pre",null,(0,n.kt)("code",{parentName:"pre",className:"language-js",metastring:"live",live:!0},"function example() {\n  const h = '85283473fffffff';\n  const vertexNum = 2;\n  return h3.cellToVertex(h, vertexNum);\n}\n")))),(0,n.kt)("p",null,"Returns the index for the specified cell vertex. Valid vertex numbers are between 0 and 5 (inclusive)\nfor hexagonal cells, and 0 and 4 (inclusive) for pentagonal cells."),(0,n.kt)("h2",{id:"celltovertexes"},"cellToVertexes"),(0,n.kt)(r.Z,{groupId:"language",defaultValue:"c",values:[{label:"C",value:"c"},{label:"Python",value:"python"},{label:"Java",value:"java"},{label:"JavaScript (Live)",value:"javascript"}],mdxType:"Tabs"},(0,n.kt)(o.Z,{value:"c",mdxType:"TabItem"},(0,n.kt)("pre",null,(0,n.kt)("code",{parentName:"pre",className:"language-c"},"H3Error cellToVertexes(H3Index origin, H3Index *out);\n"))),(0,n.kt)(o.Z,{value:"python",mdxType:"TabItem"},(0,n.kt)("pre",null,(0,n.kt)("code",{parentName:"pre",className:"language-py"},"h3.cell_to_vertexes(origin)\n"))),(0,n.kt)(o.Z,{value:"java",mdxType:"TabItem"},(0,n.kt)("pre",null,(0,n.kt)("code",{parentName:"pre",className:"language-java"},"List<Long> cellToVertexes(long origin);\nList<String> cellToVertexes(String origin);\n"))),(0,n.kt)(o.Z,{value:"javascript",mdxType:"TabItem"},(0,n.kt)("pre",null,(0,n.kt)("code",{parentName:"pre",className:"language-js"},"h3.cellToVertexes(origin)\n")),(0,n.kt)("pre",null,(0,n.kt)("code",{parentName:"pre",className:"language-js",metastring:"live",live:!0},"function example() {\n  const h = '85283473fffffff';\n  return h3.cellToVertexes(h);\n}\n")))),(0,n.kt)("p",null,"Returns the indexes for all vertexes of the given cell index."),(0,n.kt)("p",null,"The length of the ",(0,n.kt)("inlineCode",{parentName:"p"},"out")," array must be 6. If the given cell index represents a pentagon, one member of the\narray will be set to ",(0,n.kt)("inlineCode",{parentName:"p"},"0"),"."),(0,n.kt)("h2",{id:"vertextolatlng"},"vertexToLatLng"),(0,n.kt)(r.Z,{groupId:"language",defaultValue:"c",values:[{label:"C",value:"c"},{label:"Python",value:"python"},{label:"Java",value:"java"},{label:"JavaScript (Live)",value:"javascript"}],mdxType:"Tabs"},(0,n.kt)(o.Z,{value:"c",mdxType:"TabItem"},(0,n.kt)("pre",null,(0,n.kt)("code",{parentName:"pre",className:"language-c"},"H3Error vertexToLatLng(H3Index vertex, LatLng *point);\n"))),(0,n.kt)(o.Z,{value:"python",mdxType:"TabItem"},(0,n.kt)("pre",null,(0,n.kt)("code",{parentName:"pre",className:"language-py"},"h3.vertex_to_latlng(vertex)\n"))),(0,n.kt)(o.Z,{value:"java",mdxType:"TabItem"},(0,n.kt)("pre",null,(0,n.kt)("code",{parentName:"pre",className:"language-java"},"LatLng vertexToLatLng(long vertex);\nLatLng vertexToLatLng(String vertex);\n"))),(0,n.kt)(o.Z,{value:"javascript",mdxType:"TabItem"},(0,n.kt)("pre",null,(0,n.kt)("code",{parentName:"pre",className:"language-js"},"h3.vertexToLatLng(vertex)\n")),(0,n.kt)("pre",null,(0,n.kt)("code",{parentName:"pre",className:"language-js",metastring:"live",live:!0},"function example() {\n  const h = '255283463fffffff';\n  return h3.vertexToLatLng(h);\n}\n")))),(0,n.kt)("p",null,"Returns the latitude and longitude coordinates of the given vertex."),(0,n.kt)("h2",{id:"isvalidvertex"},"isValidVertex"),(0,n.kt)(r.Z,{groupId:"language",defaultValue:"c",values:[{label:"C",value:"c"},{label:"Python",value:"python"},{label:"Java",value:"java"},{label:"JavaScript (Live)",value:"javascript"}],mdxType:"Tabs"},(0,n.kt)(o.Z,{value:"c",mdxType:"TabItem"},(0,n.kt)("pre",null,(0,n.kt)("code",{parentName:"pre",className:"language-c"},"int isValidVertex(H3Index vertex);\n"))),(0,n.kt)(o.Z,{value:"python",mdxType:"TabItem"},(0,n.kt)("pre",null,(0,n.kt)("code",{parentName:"pre",className:"language-py"},"h3.is_valid_vertex(vertex)\n"))),(0,n.kt)(o.Z,{value:"java",mdxType:"TabItem"},(0,n.kt)("pre",null,(0,n.kt)("code",{parentName:"pre",className:"language-java"},"boolean isValidVertex(long vertex);\nboolean isValidVertex(String vertex);\n"))),(0,n.kt)(o.Z,{value:"javascript",mdxType:"TabItem"},(0,n.kt)("pre",null,(0,n.kt)("code",{parentName:"pre",className:"language-js"},"h3.isValidVertex(vertex)\n")),(0,n.kt)("pre",null,(0,n.kt)("code",{parentName:"pre",className:"language-js",metastring:"live",live:!0},"function example() {\n  const h = '255283463fffffff';\n  return h3.isValidVertex(h);\n}\n")))),(0,n.kt)("p",null,"Returns 1 if the given index represents a valid H3 vertex."))}d.isMDXComponent=!0}}]);