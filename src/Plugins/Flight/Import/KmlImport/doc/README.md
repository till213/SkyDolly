# KML (Keyhole Markup Language) File Format

## About

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
  
## Altitude

Both FlightAware and FlightRadar24 (respectively transponders and ADS-B) report pressure altitudes at standard pressure (29.92 In-Hg or 1013.25 hPa). FlightRadar24 also reports all altitudes to be 0 at ground level. Also, ADS-B can only encode the altitude to the nearest 25 feet.

FlightAware reports "absolute" (KML: "above mean sea level") altitude values, which may lead "into the ground" (below runways) or be "above the runway" (depending on the actual pressure measured at the time).

FlightRadar24 on the other hand reports 0 altitude values while the aircraft is on the ground (otherwise pressure altitude, as discussed above). Interestingly while the KML Placemarks have an "absolute" altitude mode (putting them "under the ground as well") the actual track data has (properly) a "clampToGround" KML altitude mode, aligning the visible track with runways and taxiways.

## Resources

- KML documentation by Google: https://developers.google.com/kml/documentation/
- FlightAware altitude discussion: https://www.pilotsofamerica.com/community/threads/flightaware-altitude-reporting.129363/
