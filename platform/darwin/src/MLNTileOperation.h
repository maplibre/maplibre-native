#import <Foundation/Foundation.h>

#ifndef NS_ENUM
#define NS_ENUM(_type, _name) \
  enum _name : _type _name;   \
  enum _name : _type
#endif

typedef NS_ENUM(NSInteger, MLNTileOperation) {
    MLNTileOperationRequestedFromCache,    ///< A read request from the cache
    MLNTileOperationRequestedFromNetwork,  ///< A read request from the online source
    MLNTileOperationLoadFromNetwork,       ///< Tile data from the network has been retrieved
    MLNTileOperationLoadFromCache,         ///< Tile data from the cache has been retrieved
    MLNTileOperationStartParse,            ///< Background processing of tile data has been initiated
    MLNTileOperationEndParse,              ///< Background processing of tile data has been completed
    MLNTileOperationError,                 ///< An error occurred while loading the tile
    MLNTileOperationCancelled,             ///< Loading of a tile was cancelled
    MLNTileOperationNullOp,                ///< No operation has taken place
};
