# Plug-In Layer Architecture Design Proposal

## Motivation

This project is to add the ability to "register" additional layer types at runtime and have them integrated into the standard parameter/rendering pipeline.

## Proposed Change

For the initial implemention, the following functionality is proposed:
 
* At the platform level, be able to register a plug-in layer that is then parseable by the style parser
* The plug-in layer will be limited to simple rendering (via handing off the rendering context to the plug-in layer) and will not include the ability to define drawables/etc
* The "paint" properties will be parseable, support expressions and passed in frame by frame
* A custom set of "plugin-propeties" will also be available at the same level as the "paint" properties
* The plug-in layer will be notified about lifecycle events (creation, addition to the mapview, removal, destruction/etc) and be expected to manage it's own resources

Future features:
* Placeholder for ideas that could be implemented in the future


## API Modifications

The platform layer would provide a base class for implementing the plug-in layer.  The mapview would have a new method for registering the plug-in layer class.


## Migration Plan and Compatibility

All API changes are additive, so, no backwards compatibility issues should be present.

## Rejected Alternatives

N/A
