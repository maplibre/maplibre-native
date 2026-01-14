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

package com.example.time;

/**
 * Represents duration in time (both positive and negative).
 * <p>
 *     The duration is represented as number of seconds (see {@link #getSeconds()})
 *     and number of nanonseconds in a second (see {@link #getNano()}).
 * <p>
 *     Duration can be created from various units of time by calling on of
 *     {@code of*} methods. The {@code to*} family of methods convert duration
 *     to a value expressed in desired unit of time.
 */
public final class Duration implements Comparable<Duration> {
    private long mSeconds;
    private int mNanos;

    private static long NANOS_PER_SECOND = 1000000000;
    private static int NANOS_PER_MILLIS = 1000000;
    private static long MILLIS_PER_SECOND = 1000;

    private Duration(long seconds, int nanos) {
        mSeconds = seconds;
        mNanos = nanos;
    }

    private static long exactAdd(long v1, long v2) throws ArithmeticException {
        if (v2 < 0 && v1 < Long.MIN_VALUE - v2) {
            throw new ArithmeticException("Integer underflow");
        } else if (v2 > 0 && v1 > Long.MAX_VALUE - v2) {
            throw new ArithmeticException("Integer overflow");
        }
        return v1 + v2;
    }

    private static long exactMultiply(long v1, long v2) throws ArithmeticException {
        if (v2 == -1 && v1 == Long.MIN_VALUE || v1 == -1 && v2 == Long.MIN_VALUE) {
            throw new ArithmeticException("Integer overflow");
        }
        if (v2 > 0 && v1 > Long.MAX_VALUE / v2) {
            throw new ArithmeticException("Integer overflow");
        }
        if (v2 > 0 && v1 < Long.MIN_VALUE / v2) {
            throw new ArithmeticException("Integer underflow");
        }
        if (v2 < -1 && v1 > Long.MIN_VALUE / v2) {
            throw new ArithmeticException("Integer underflow");
        }
        if (v2 < -1 && v1 < Long.MAX_VALUE / v2) {
            throw new ArithmeticException("Integer overflow");
        }
        return v1 * v2;
    }

    private static long divFloor(long x, long y) {
        long result = x / y;
        if ((x ^ y) < 0 && (result * y != x)) {
            result--;
        }
        return result;
    }

    private static long modFloor(long x, long y) {
        long result = x % y;
        if ((x ^ y) < 0 && result != 0) {
            result += y;
        }
        return result;
    }

    /**
     *
     * @return The nanoseconds component of this duration.
     */
    public int getNano() {
        return mNanos;
    }

    /**
     *
     * @return The seconds component of this duration.
     */
    public long getSeconds() {
        return mSeconds;
    }

    /**
     * Creates a duration representing specified number of days.
     * A Day is assumed to always be 24 hours.
     *
     * @param days The number of days.
     * @return The Duration representing the specified number of days.
     * @throws ArithmeticException if the input is outside the range possible to
     *                             represent by a Duration
     */
    public static Duration ofDays(long days) throws ArithmeticException {
        return ofHours(exactMultiply(days, 24));
    }

    /**
     * Creates a duration representing specified number of hours.
     * An hour is assumed to always be 60 minutes.
     *
     * @param hours The number of hours.
     * @return The Duration representing the specified number of hours.
     * @throws ArithmeticException if the input is outside the range possible to
     *                             represent by a Duration
     */
    public static Duration ofHours(long hours) throws ArithmeticException {
        return ofMinutes(exactMultiply(hours, 60));
    }

    /**
     * Creates a duration representing specified number of hours.
     * A minute is assumed to always be 60 seconds.
     *
     * @param minutes The number of minutes.
     * @return The Duration representing the specified number of minutes.
     * @throws ArithmeticException if the input is outside the range possible to
     *                             represent by a Duration
     */
    public static Duration ofMinutes(long minutes) throws ArithmeticException {
        return ofSeconds(exactMultiply(minutes, 60));
    }

    /**
     * Creates a duration representing specified number of seconds.
     *
     * @param seconds The number of seconds.
     * @return The Duration representing the specified number of seconds.
     */
    public static Duration ofSeconds(long seconds) {
        return new Duration(seconds, 0);
    }

    /**
     * Creates a duration representing specified number of seconds and an adjustment in nanoseconds.
     *
     * @param seconds The number of seconds.
     * @param nanoAdjustment The nanosecond adjustment to the number of seconds.
     * @return The Duration representing the specified number of seconds, adjusted.
     */
    public static Duration ofSeconds(long seconds, long nanoAdjustment) {
        long secs = exactAdd(seconds, divFloor(nanoAdjustment, NANOS_PER_SECOND));
        int nanos = (int) modFloor(nanoAdjustment, NANOS_PER_SECOND);
        return new Duration(secs, nanos);
    }

    /**
     * Creates a duration representing specified number of milliseconds.
     *
     * @param milliseconds The number of milliseconds.
     * @return The Duration representing the specified number of milliseconds.
     */
    public static Duration ofMillis(long milliseconds) {
        return ofNanos(milliseconds * NANOS_PER_MILLIS);
    }

    /**
     * Creates a duration representing specified number of nanoseconds.
     *
     * @param nanoseconds The number of nanoseconds.
     * @return The Duration representing the specified number of nanoseconds.
     */
    public static Duration ofNanos(long nanoseconds) {
        long secs = nanoseconds / NANOS_PER_SECOND;
        int nanos = (int) (nanoseconds % NANOS_PER_SECOND);
        if (nanos < 0) {
            nanos += NANOS_PER_SECOND;
            secs--;
        }
        return new Duration(secs, nanos);
    }

    /**
     * Converts this duration to nanoseconds.
     *
     * @return Total number of nanoseconds in this duration.
     * @throws ArithmeticException if the resulting value cannot be represented
     *                             by {@code long} type.
     */
    public long toNanos() throws ArithmeticException {
        return exactAdd(exactMultiply(mSeconds, NANOS_PER_SECOND), mNanos);
    }

    /**
     * Gets the nanoseconds part of this duration. Equals to {@link #getNano()}.
     *
     * @return The nanoseconds part of this duration, value from 0 to 999999999.
     */
    public int toNanosPart() {
        return mNanos;
    }

    /**
     * Converts this duration to milliseconds. Any data past milliseconds precision is
     * simply discarded. There is no mathematical rounding, so a duration
     * of 999999 nanoseconds will still be converted to 0 milliseconds.
     *
     * @return Total number of milliseconds in this duration.
     * @throws ArithmeticException if the resulting value cannot be represented
     *                             by {@code long} type.
     */
    public long toMillis() throws ArithmeticException {
        return exactAdd(exactMultiply(mSeconds, MILLIS_PER_SECOND), mNanos / NANOS_PER_MILLIS);
    }

    /**
     * Gets the milliseconds part of this duration.
     *
     * @return The milliseconds part of this duration.
     */
    public int toMillisPart() {
        return mNanos / NANOS_PER_MILLIS;
    }

    /**
     * Converts this duration to seconds. Any data past seconds precision is
     * simply discarded. There is no mathematical rounding, so a duration
     * of 999 milliseconds will still be converted to 0 seconds.
     *
     * @return Total number of seconds in this duration.
     */
    public long toSeconds() {
        return mSeconds;
    }

    /**
     * Gets the seconds part of this duration.
     *
     * @return The seconds part of this duration, value from 0 to 59.
     */
    public int toSecondsPart() {
        return (int) (mSeconds % 60);
    }

    /**
     * Converts this duration to minutes. Any data past minute precision is
     * simply discarded. There is no mathematical rounding, so a duration
     * of 59 seconds and 999 milliseconds will still be converted to 0 minutes.
     *
     * @return Total number of minutes in this duration.
     */
    public long toMinutes() {
        return mSeconds / 60;
    }

    /**
     * Gets the minutes part of this duration.
     *
     * @return The minutes part of this duration, value from 0 to 59.
     */
    public int toMinutesPart() {
        return (int) (toMinutes() % 60);
    }

    /**
     *
     * Converts this duration to hours. Any data past hour precision is
     * simply discarded. There is no mathematical rounding, so a duration
     * of 59 minutes and 59 seconds will still be converted to 0 hours.
     *
     * @return The number of full hours in this duration.
     */
    public long toHours() {
        return toMinutes() / 60;
    }

    /**
     * Gets the hours part of this duration.
     *
     * @return The hours part of this duration, value from 0 to 23.
     */
    public int toHoursPart() {
        return (int) (toHours() % 24);
    }

    /**
     * Converts this duration to days. Any data past day precision is
     * simply discarded. There is no mathematical rounding, so a duration
     * of 23 hours 59 minutes and 59 seconds will still be converted to 0 days.
     * Day is always assumed to be 24 hours.
     *
     * @return The number of full days in this duration.
     */
    public long toDays() {
        return toHours() / 24;
    }

    /**
     * Same as {@link #toDays()}.
     *
     * @return The number of full days in this duration.
     */
    public long toDaysPart() {
        return toDays();
    }

    @Override
    public int compareTo(Duration duration) {
        int result = 0;
        if (mSeconds < duration.mSeconds) {
            result = -1;
        } else if (mSeconds > duration.mSeconds) {
            result = 1;
        }
        if (result == 0) {
            if (mNanos < duration.mNanos) {
                result = -1;
            } else if (mNanos > duration.mNanos) {
                result = 1;
            }
        }
        return result;
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        Duration duration = (Duration) o;
        return mSeconds == duration.mSeconds && mNanos == duration.mNanos;
    }

    @Override
    public int hashCode() {
        return java.util.Objects.hash(mSeconds, mNanos);
    }
}
