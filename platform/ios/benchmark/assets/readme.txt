Benchmark locations:

https://api.maptiler.com/maps/streets/?key=[key]#11/48.8664/2.3411
https://api.maptiler.com/maps/streets/?key=[key]#13/48.8356/2.3516/273.8
https://api.maptiler.com/maps/streets/?key=[key]#6/46.9599/10.6107
https://api.maptiler.com/maps/streets/?key=[key]#5/36.9400/-84.3395
https://api.maptiler.com/maps/streets/?key=[key]#9/34.0259/-117.9529
https://api.maptiler.com/maps/streets/?key=[key]#14/37.7625/-122.4202
https://api.maptiler.com/maps/streets/?key=[key]#12/37.8267/-122.2328
https://api.maptiler.com/maps/streets/?key=[key]#6/50.9262/9.2280

To download assets:

1. Style: 
https://api.maptiler.com/maps/streets/style.json?key=[key]
replace urls with assets urls
- sprites: asset://sprites/streets
- tiles: asset://tiles/tiles.json

2. Tile.json
https://api.maptiler.com/tiles/v3/tiles.json?key=[key]
- tiles: "assets://tiles/tiles/v3/{z}/{x}/{y}.pbf"

3. sprites
https://api.maptiler.com/maps/streets/sprite.json
https://api.maptiler.com/maps/streets/sprite.png
https://api.maptiler.com/maps/streets/sprite@2x.json
https://api.maptiler.com/maps/streets/sprite@2x.png

4. glyphs
verify the font list and run download.sh

4. tiles
run download.sh

