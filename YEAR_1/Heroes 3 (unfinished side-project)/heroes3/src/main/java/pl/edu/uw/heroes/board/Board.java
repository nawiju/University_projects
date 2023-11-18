package pl.edu.uw.heroes.board;

import java.util.Collection;
import java.util.HashMap;
import java.util.Map;

public class Board {

    private final int width, height;

    private final Map<Position, Field> fields = new HashMap<>();

    public Board(int width, int height) {
        this.width = width;
        this.height = height;
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                Position position = new Position(i, j);
                fields.put(position, new Field(position));
            }
        }
        addNeighbors();
    }

    private void addNeighbors() {
        for (Field field : fields.values()) {
            for (Direction direction : Direction.values()) {
                Field neighbor = fields.get(direction.move(field.getPosition()));
                if (neighbor != null)
                    field.addNeighbor(direction, neighbor);
            }
        }
    }

    public Collection<Field> getFields() {
        return fields.values();
    }

    public Field getField(Position position) {
        return fields.get(position);
    }
}
