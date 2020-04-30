/*
 * Copyright 2016-2017 Uber Technologies, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/** @file
 * @brief functions to generate simple KML files
 */

#include "kml.h"

#include <stdio.h>

#include "h3api.h"

void kmlPtsHeader(const char* name, const char* desc) {
    printf("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    printf(
        "<kml xmlns=\"http://www.opengis.net/kml/2.2\" "
        "xmlns:gx=\"http://www.google.com/kml/ext/2.2\" "
        "xmlns:kml=\"http://www.opengis.net/kml/2.2\" "
        "xmlns:atom=\"http://www.w3.org/2005/Atom\">\n");
    printf("<Document>\n");
    printf("        <name>%s</name>\n", name);
    printf("        <description>%s</description>\n", desc);
    printf("        <Style id=\"s_circle_hl\">\n");
    printf("                <IconStyle>\n");
    printf("                        <scale>1.3</scale>\n");
    printf("                        <Icon>\n");
    printf(
        "                                "
        "<href>http://maps.google.com/mapfiles/kml/shapes/"
        "placemark_circle.png</"
        "href>\n");
    printf("                        </Icon>\n");
    printf(
        "                        <hotSpot x=\"20\" y=\"2\" xunits=\"pixels\" "
        "yunits=\"pixels\"/>\n");
    printf("                </IconStyle>\n");
    printf("                <LabelStyle>\n");
    printf("                        <color>ff0000ff</color>\n");
    printf("                        <scale>2</scale>\n");
    printf("                </LabelStyle>\n");
    printf("        </Style>\n");
    printf("        <StyleMap id=\"m_ylw-pushpin\">\n");
    printf("                <Pair>\n");
    printf("                        <key>normal</key>\n");
    printf("                        <styleUrl>#s_circle</styleUrl>\n");
    printf("                </Pair>\n");
    printf("                <Pair>\n");
    printf("                        <key>highlight</key>\n");
    printf("                        <styleUrl>#s_circle_hl</styleUrl>\n");
    printf("                </Pair>\n");
    printf("        </StyleMap>\n");
    printf("        <Style id=\"s_circle\">\n");
    printf("                <IconStyle>\n");
    printf("                        <scale>1.1</scale>\n");
    printf("                        <Icon>\n");
    printf(
        "                                "
        "<href>http://maps.google.com/mapfiles/kml/shapes/"
        "placemark_circle.png</"
        "href>\n");
    printf("                        </Icon>\n");
    printf(
        "                        <hotSpot x=\"20\" y=\"2\" xunits=\"pixels\" "
        "yunits=\"pixels\"/>\n");
    printf("                </IconStyle>\n");
    printf("                <LabelStyle>\n");
    printf("                        <color>ff000fff</color>\n");
    printf("                        <scale>2</scale>\n");
    printf("                </LabelStyle>\n");
    printf("        </Style>\n");
}

void kmlBoundaryHeader(const char* name, const char* desc) {
    printf("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    printf("<kml xmlns=\"http://earth.google.com/kml/2.1\">\n");
    printf("<Folder>\n");
    printf("   <name>%s</name>\n", name);
    printf("   <description>%s</description>\n", desc);
    printf("   <Style id=\"lineStyle1\">\n");
    printf("      <LineStyle id=\"lineStyle2\">\n");
    printf("         <color>ff000fff</color>\n");
    printf("         <width>2</width>\n");
    printf("      </LineStyle>\n");
    printf("   </Style>\n");
}

void kmlPtsFooter(void) {
    printf("</Document>\n");
    printf("</kml>\n");
}

void kmlBoundaryFooter(void) {
    printf("</Folder>\n");
    printf("</kml>\n");
}

void outputLatLonKML(const GeoCoord* g) {
    printf("            %8lf,%8lf,5.0\n", H3_EXPORT(radsToDegs)(g->lon),
           H3_EXPORT(radsToDegs)(g->lat));
}

void outputPointKML(const GeoCoord* g, const char* name) {
    printf("<Placemark>\n");
    printf("   <name>%s</name>\n", name);
    printf("   <styleUrl>#m_ylw-pushpin</styleUrl>\n");
    printf("   <Point>\n");
    printf("      <altitudeMode>relativeToGround</altitudeMode>\n");
    printf("      <coordinates>\n");
    outputLatLonKML(g);
    printf("      </coordinates>\n");
    printf("   </Point>\n");
    printf("</Placemark>\n");
}

void outputTriKML(const GeoCoord* v1, const GeoCoord* v2, const GeoCoord* v3,
                  const char* name) {
    printf("<Placemark>\n");
    printf("<name>%s</name>\n", name);
    printf("      <styleUrl>#lineStyle1</styleUrl>\n");
    printf("      <LineString>\n");
    printf("         <tessellate>1</tessellate>\n");
    printf("         <coordinates>\n");
    outputLatLonKML(v1);
    outputLatLonKML(v2);
    outputLatLonKML(v3);
    outputLatLonKML(v1);
    printf("         </coordinates>\n");
    printf("      </LineString>\n");
    printf("</Placemark>\n");
}

void outputBoundaryKML(const GeoBoundary* b, const char* name) {
    const GeoCoord* v = (const GeoCoord*)&(b->verts);
    outputPolyKML(v, b->numVerts, name);
}

void outputPolyKML(const GeoCoord geoVerts[], int numVerts, const char* name) {
    printf("<Placemark>\n");
    printf("<name>%s</name>\n", name);
    printf("      <styleUrl>#lineStyle1</styleUrl>\n");
    printf("      <LineString>\n");
    printf("         <tessellate>1</tessellate>\n");
    printf("         <coordinates>\n");

    for (int v = 0; v < numVerts; v++) outputLatLonKML(&geoVerts[v]);
    outputLatLonKML(&geoVerts[0]);

    printf("         </coordinates>\n");
    printf("      </LineString>\n");
    printf("</Placemark>\n");
}
