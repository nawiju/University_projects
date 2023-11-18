package pl.edu.uw.heroes.board;

import lombok.Getter;
import lombok.Setter;
import pl.edu.uw.heroes.units.Unit;

import java.util.Collection;
import java.util.HashMap;
import java.util.Map;

public class Field {

    @Getter
    private final Position position;

    private final Map<Direction, Field> neighbors = new HashMap<>();

    @Getter
    @Setter
    private Unit unit;

    public Field(Position position) {
        this.position = position;
    }

    public boolean isEmpty() {
        return unit == null;
    }

    public void addNeighbor(Direction direction, Field field) {
        neighbors.put(direction, field);
    }

    public Collection<Field> getNeighbors() {
        return neighbors.values();
    }

    @Override
    public String toString() {
        return position.toString();
    }
}
