#import <Foundation/Foundation.h>

#ifndef NS_ENUM
#define NS_ENUM(_type, _name) \
  enum _name : _type _name;   \
  enum _name : _type
#endif

typedef NS_ENUM(NSInteger, MLNTileOperation) {
  RequestedFromCache,    ///< A read request from the cache
  RequestedFromNetwork,  ///< A read request from the online source
  LoadFromNetwork,       ///< Tile data from the network has been retrieved
  LoadFromCache,         ///< Tile data from the cache has been retrieved
  StartParse,            ///< Background processing of tile data has been initiated
  EndParse,              ///< Background processing of tile data has been completed
  Error,                 ///< An error occurred while loading the tile
  Cancelled,             ///< Loading of a tile was cancelled
  NullOp,                ///< No operation has taken place
};
