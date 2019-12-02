# dem
quick and dirty dem visualization, started with repo "multiPick" and went from there..data is from:

* Canada geospatial extraction tool: https://maps.canada.ca/czs/index-en.html
## dem data
![alt text](dem.png)

## downhill operator
![alt text](downhill.png)

ubuntu:
  make
  ./dem

## patches
Labelled patches (distinct colours) correspond to basins of attraction (for water)

## spill map (todo)

Todo for watershed delineation: calculate spill map (function from set of patches, to set of patches; has an associated "spill level"):

Spill map identifies, for an input patch, a lower-altitude patch such that, water from the input patch will spill into, provided a certain altitude of water (the spill level) is spilled onto that patch.

Note: In the case that the patch is locally minimal, the spill map may refer to a patch that's not adjacent. 

Calculating the spill map involves marching upwards, a minimal amount, until the water spills over into a lower-altitude patch.

Altitude of a patch is defined as the minimal altitude for that patch 
