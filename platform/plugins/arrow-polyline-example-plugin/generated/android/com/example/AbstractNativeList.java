/*
 * Copyright (C) 2016-2021 HERE Europe B.V.
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

import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;
import java.util.ListIterator;
import java.util.NoSuchElementException;
import java.util.Objects;

/**
 * <p>Internal base abstract class for List implementations backed by a native object.
 *
 * @hidden
 */
public abstract class AbstractNativeList<T> extends NativeBase implements List<T> {

    /**
     * @hidden
     */
    private final class NativeIterator implements ListIterator<T> {

        private int index = 0;

        @Override
        public boolean hasNext() { return index < size(); }

        @Override
        public T next() {
            if (!hasNext()) {
                throw new NoSuchElementException();
            }

            final T result = get(index);
            index += 1;
            return result;
        }

        @Override
        public boolean hasPrevious() { return index >= 0; }

        @Override
        public T previous() {
            if (!hasPrevious()) {
                throw new NoSuchElementException();
            }

            final T result = get(index);
            index -= 1;
            return result;
        }

        @Override
        public int nextIndex() { return index + 1; }

        @Override
        public int previousIndex() { return index - 1; }
        @Override
        public void remove() { throw new UnsupportedOperationException(); }
        @Override
        public void set(T e) { throw new UnsupportedOperationException(); }
        @Override
        public void add(T e) { throw new UnsupportedOperationException(); }
    }

    private Integer _size = null;

    protected AbstractNativeList(final long nativeHandle, final Disposer disposer) {
        super(nativeHandle, disposer);
    }

    protected abstract int obtainSize();

    private List<Object> toArrayList() {
        List<Object> arrayList = new ArrayList<>();
        for (final T element: this) arrayList.add(element);
        return arrayList;
    }

    @Override
    public int size() {
        if (_size == null) _size = obtainSize();
        return _size;
    }

    @Override
    public boolean isEmpty() { return size() == 0; }

    @Override
    public boolean contains(Object o) { return toArrayList().contains(o); }

    @Override
    public Iterator<T> iterator() { return listIterator(0); }

    @Override
    public Object[] toArray() { return toArrayList().toArray(); }

    @Override
    public <T> T[] toArray(T[] a) { return toArrayList().toArray(a); }

    @Override
    public boolean add(T t) { throw new UnsupportedOperationException(); }
    @Override
    public boolean remove(Object o) { throw new UnsupportedOperationException(); }

    @Override
    public boolean containsAll(Collection<?> c) { return toArrayList().containsAll(c); }

    @Override
    public boolean addAll(Collection<? extends T> c) { throw new UnsupportedOperationException(); }
    @Override
    public boolean addAll(int index, Collection<? extends T> c) { throw new UnsupportedOperationException(); }
    @Override
    public boolean removeAll(Collection<?> c) { throw new UnsupportedOperationException(); }
    @Override
    public boolean retainAll(Collection<?> c) { throw new UnsupportedOperationException(); }
    @Override
    public void clear() { throw new UnsupportedOperationException(); }

    @Override
    public T set(int index, T element) { throw new UnsupportedOperationException(); }
    @Override
    public void add(int index, T element) { throw new UnsupportedOperationException(); }
    @Override
    public T remove(int index) { throw new UnsupportedOperationException(); }
    @Override
    public int indexOf(Object o) { throw new UnsupportedOperationException(); }
    @Override
    public int lastIndexOf(Object o) { throw new UnsupportedOperationException(); }

    @Override
    public ListIterator<T> listIterator() { return new NativeIterator(); }

    @Override
    public ListIterator<T> listIterator(int index) {
        final ListIterator<T> result = listIterator();
        for (int i = 0; i < index; i++) {
            if (!result.hasNext()) break;
            result.next();
        }
        return result;
    }

    @Override
    public List<T> subList(int fromIndex, int toIndex) {
        List<T> arrayList = new ArrayList<>();
        for (int i = fromIndex; i < toIndex; i++) {
            arrayList.add(get(i));
        }
        return arrayList;
    }
}
