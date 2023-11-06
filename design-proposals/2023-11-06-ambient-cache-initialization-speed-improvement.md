# Design Proposal : Ambient cache initialization speed improvement

## Motivation

We would like to improve the speed of ambient cache initialization when there is a lot of downloaded offline regions.

The problem is described in this issue : https://github.com/maplibre/maplibre-native/issues/1815

## Proposed Change

We propose to split the ambient cache from the offline database cache.
In terms of database structure, that means create two new tables. Let's say ambient_tiles and ambient_resources.

### When we need to store resource for ambient cache:

We store it into the ambient_resources table.

### When we need to store tile for ambient cache:

We store it into the ambient_tiles table.

### When we need to store resource for offline data:

We store it into the resources table with the associated region inside the region_resources table.
We delete the same resource from the ambient cache (*).

### When we need to store tile for offline data:

We store it into the tile table with the associated region inside the region_tiles table.
We delete the same tile from the ambient cache (*).

(*) Even if something wrong happens and the delete instruction cannot be done for a specified resource/tile, that only means the ambient cache will waste some size.
But with the continuous refresh of the ambient cache, this waste will be erased after some time.

By doing this, the initialization of the ambient cache will take no time, because we'll doing the request on a table that is limited to the Max Ambient Cache Size anyway.

## API Modifications

No public API modifications

## Migration Plan and Compatibility

- Database structure migration : We would implement the database structure changes (creating two new tables) in database upgrade methods
- We would clear the ambient cache at first upgrade to the new database version => Some slowness may occur only on the first initialization after the upgrade

## Rejected Alternatives

2 possible alternatives to solve this issue:

- Store the current ambient cache size inside the database, and update it each time his cache size is modified. This way, the initialization simply request the computed cache size from the last execution.
- Store a flag inside the tiles and resources table that tells if the resource/tile is ambient or not. That way we simplify the request by deleting the need to make a left join. With the cost that for each download/cleaning, the flag must be re-set correctly.

The main problem we have with these 2 solutions are in term of database corruption. If something wrong happens between the initial modification and the computation modification, you'll have a database with incoherent information stored. That could leads to issues difficult to overcome.
