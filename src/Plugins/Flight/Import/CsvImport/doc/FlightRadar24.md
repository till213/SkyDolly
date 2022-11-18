# FlightRadar 24

## CSV Format
- The CSV file is comma-separated
- The latitude and longitude values are given in one column, in quotation marks and comma-separated
  * Exmample: "33.943356,-118.42868"
- There are 7 columns according to this [example](https://gist.github.com/cgarethc/bfaecceefee6cf353d4e4fd8b374cc9e)
- There are 6 columns only in the [original documenation](https://www.flightradar24.com/blog/using-the-new-flightradar24-kml-and-csv-export-tools/): without UTC timestamp
- Altitude is in feet, speed in knots

### Altitude
Altitude data reported by ADS-B is the pressure altitude at standard pressure (1013 hPa) and not the altitude above ground level. All altitude data when the aircraft is ‘on ground’ is reported as 0.

### Example

```
Timestamp,UTC,Callsign,Position,Altitude,Speed,Direction
1635481576,2021-10-29T04:26:16Z,ANZ1005,"33.943356,-118.42868",100,220,250
```

### Regular expression (for 7 columns)
^(\d*),(\d{4}-\d{2}-\d{2})T(\d{2}:\d{2}:\d{2})Z,([\w]*|[\d]*),"([-]?\d{1,2}.\d+),([-]?\d{1,3}.\d+)",(\d+),(\d+),(\d+)$

## References
- https://cgarethc.medium.com/building-a-flight-plan-creator-with-google-cloud-functions-and-sheets-bf9d65412f0
- https://www.flightradar24.com/blog/flightradar24-data-regarding-the-crash-of-ethiopian-airlines-flight-302/
