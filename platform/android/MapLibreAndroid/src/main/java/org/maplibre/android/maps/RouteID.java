package org.maplibre.android.maps;

final public class RouteID {
    private Integer id;
    public RouteID(int id) {
        this.id = id;
    }
    public int getId() {
        return id;
    }
    public boolean isValid() {
        return id != -1;
    }
}
