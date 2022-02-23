# GPS Exchange Format (GPX)

## About

GPX, or GPS Exchange Format, is an XML schema designed as a common GPS data format for software applications. It can be used to describe waypoints, tracks, and routes.

- Location data is expressed as decimal degrees and elevation is expressed in meters
- Time in a GPX file is are in Coordinated Universal Time (UTC) using ISO 8601 format

## Structure

The minimum properties for a GPX file are latitude and longitude for every single point. All other elements are optional.

- Track &lt;trk&gt;: Typically the raw output of, for example, a GPS recording the course of one's trip
- Route &lt;rte&gt;: The routepoints may be crossings or junctions or as distant as stopover towns, such as those making a trip project
- Waypoints &lt;wpt&gt;: Individual points

## Resources

- https://docs.fileformat.com/gis/gpx/
- https://www.topografix.com/gpx.asp
