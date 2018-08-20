# Distance functions

## h3Distance

```
int h3Distance(H3Index origin, H3Index h3);
```

Returns the distance in grid cells between the two indexes.

Returns a negative number if finding the distance failed. Finding the distance can fail because the two
indexes are not comparable (different resolutions), too far apart, or are separated by pentagonal
distortion.
