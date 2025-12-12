/*
 * Copyright (C) 2016-2022 HERE Europe B.V.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * SPDX-License-Identifier: Apache-2.0
 * License-Filename: LICENSE
 */

package com.example;

import java.util.HashMap;
import java.util.Map;

/**
 * @hidden
 */
public final class HashMapBuilder<K, V> {
    private Map<K, V> map = new HashMap<>();

    /**
     * @hidden
     * @return The reference to self for chaining.
     * @param key The key with which the specified value is to be associated.
     * @param value The value to be associated with the specified key.
     */
    public HashMapBuilder<K, V> put(K key, V value) {
        map.put(key, value);
        return this;
    }

    /**
     * @hidden
     * @return The built map.
     */
    public Map<K, V> build() {
        return map;
    }
}
