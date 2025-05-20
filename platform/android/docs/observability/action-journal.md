# Action Journal

{{ activity_source_note("ObserverActivity.kt") }}

The Action Journal provides functionality for persistent logging of top level map events.

<!-- NOTE: keep this text in sync with platform/ios/MapLibre.docc/ActionJournalExample.md -->

Its primary use case is to assist in debugging problematic sessions and crashes by offering additional insight into the actions performed by the map at the time of failure. Data is stored in human readable format, which is useful for analyzing individual cases, but can also be easily translated and aggregated into a database, allowing for efficient analysis of multiple cases and helping to identify recurring patterns (Google BigQuery, AWS Glue + S3 + Athena, etc).

We are always interested in improving observability, so if you have a special use case, feel free to [open an issue or pull request](https://github.com/maplibre/maplibre-native) to extend the types of observability methods.

## Enabling the Action Journal

You can enable the action journal either through XML:

```xml
--8<-- "MapLibreAndroidTestApp/src/main/res/layout/activity_map_events.xml:MapView"
```

Or by passing the corresponding options with `MapLibreMapOptions` to `MapView`. For more information see [Configuration](../configuration.md).

## Logging implementation details

The logging is implemented using rolling files with a size based policy:

- A new file is created when the current log size exceeds `MapLibreMapOptions.actionJournalLogFileSize`.
- When the maximum number of files exceeds `MapLibreMapOptions.actionJournalLogFileCount`:
    - The oldest one is deleted.
    - The remaining files are renamed sequentially to maintain the naming convention `action_journal.0.log` through `action_journal.{logFileCount - 1}.log`.
- Each file contains one event per line.
- All files are stored in an umbrella `action_journal` directory at `MapLibreMapOptions.actionJournalPath`.

See also: `MapLibreMap`, `MapLibreMapOptions`.

## Event format

Events are stored as JSON objects with the following format:

| Field | Type | Required | Description |
| :---- | :--: | :------: | :---------- |
| name | string | true | event name |
| time | string | true | event time ([ISO 8601](https://en.wikipedia.org/wiki/ISO_8601) with milliseconds) |
| styleName | string | false | currently loaded style name |
| styleURL | string | false | currently loaded style URL |
| clientName | string | false | |
| clientVersion | string | false | |
| event | object | false | event specific data - consists of encoded values of the parameters passed to their `MLNMapViewDelegate` counterparts


```
{
    "name" : "onTileAction",
    "time" : "2025-04-17T13:13:13.974Z",
    "styleName" : "Streets",
    "styleURL" : "maptiler://maps/streets",
    "clientName" : "App",
    "clientVersion" : "1.0",
    "event" : {
        "action" : "RequestedFromNetwork",
        "tileX" : 0,
        "tileY" : 0,
        "tileZ" : 0,
        "overscaledZ" : 0,
        "sourceID" : "openmaptiles"
    }
}
```

## Usage

```kotlin
--8<-- "MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/events/ObserverActivity.kt:printActionJournal"
```

## Alternative

The implementation is kept close to the core events to minimize additional locking and avoid platform-specific conversions and calls. As a result customization options and extensibility is limited.

For greater flexibility, consider using the `MapView` event interface (see also `MapChangeReceiver`). It provides hooks for most Action Journal events and allows for more customizable querying and storage of map data. However, this comes at the cost of added complexity. See [Observe Map Events](./observe-map-events.md) to learn about the map events that you can listen for, which mirror the events available in the action journal.
