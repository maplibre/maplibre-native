package org.maplibre.android.annotations;

import org.maplibre.android.style.layers.PropertyValue;

import org.mockito.ArgumentMatcher;

import java.util.Arrays;

class PropertyValueMatcher implements ArgumentMatcher<PropertyValue> {

    private final PropertyValue wanted;

    PropertyValueMatcher(PropertyValue wanted) {
        this.wanted = wanted;
    }

    @Override
    public boolean matches(PropertyValue argument) {
        if (argument == wanted) {
            return true;
        } else if (!argument.name.equals(wanted.name)) {
            return false;
        } else if (argument.getValue() != null && wanted.getValue() != null) {
            return argument.getValue().equals(wanted.getValue());
        } else if (argument.getExpression() != null && wanted.getExpression() != null) {
            return Arrays.deepEquals(argument.getExpression().toArray(), wanted.getExpression().toArray());
        } else {
            return argument.getValue() == null && wanted.getValue() == null
                && argument.getExpression() == null && wanted.getExpression() == null;
        }
    }
}
