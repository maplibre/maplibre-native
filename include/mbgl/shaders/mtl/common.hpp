#pragma once

#define MLN_MTL_UNIFORM_BLOCK(idx, vert, frag, struc) \
    { idx, vert, frag, sizeof(struc), MLN_STRINGIZE(struc) }
