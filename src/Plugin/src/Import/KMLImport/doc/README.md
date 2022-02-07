# KML (Keyhole Markup Language) File Format

## Introduction

The Keyhole Markup Language (KML) is an XML based file format which can e.g. be viewed with Google Earth.

## Structure

Various XML elements may store coordinates (latitude/longitude/altitude), as children of the &lt;Placemark&gt; element (which on its turn may be grouped in a &lt;Folder&gt;

- &lt;gx:Track&gt;
  * Stores both timestamp &lt;when&gt; and coordinates &lt;gx:coord&gt;, in "parallel arrays"; the later including altitude according to &lt;altitudeMode&gt;
  * Example: KML exported from flightaware.com
- &lt;Point&gt;
  * Stores &lt;coordinates&gt; (including altitude &lt;altitudeMode&gt;)
  * The sibling &lt;TimeStamp&gt; contains the timestamp
  * Example: KML exported from flightradar24.com
- &lt;LineString&gt; (may be child of &lt;MultiGeometry&gt; or directly a &lt;Placemark&gt;)
  * Connects two or more coordinates
  * No associated timestamps
  * Example: KML exported from flightradar24.com

## Resources

- KML documentation by Google: https://developers.google.com/kml/documentation/