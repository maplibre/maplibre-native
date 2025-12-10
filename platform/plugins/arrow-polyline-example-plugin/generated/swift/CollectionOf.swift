// -------------------------------------------------------------------------------------------------
// Copyright (C) 2016-2021 HERE Europe B.V.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0
// License-Filename: LICENSE
//
// -------------------------------------------------------------------------------------------------

/// Custom collection implementation.
public class CollectionOf<T> : Collection {
    public typealias Element = T
    public typealias Index = UInt64

    private(set) public var startIndex: Index
    private(set) public var endIndex: Index

    private let elementGetter: (Index) -> Element
    private let releaser: () -> Void

    /// :nodoc:
    internal let c_handle: _baseRef

    /// :nodoc:
    internal init(handle: _baseRef, size: Index, elementGetter: @escaping (Index) -> Element, releaser: @escaping () -> Void) {
        self.startIndex = 0
        self.endIndex = size
        self.elementGetter = elementGetter
        self.releaser = releaser
        self.c_handle = handle
    }

    deinit {
        releaser()
    }

    public func index(after i: Index) -> Index {
        return i+1
    }

    public subscript(position: Index) -> Element {
        return elementGetter(position)
    }
}
