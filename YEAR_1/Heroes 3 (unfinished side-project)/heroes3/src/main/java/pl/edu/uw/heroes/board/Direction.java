package pl.edu.uw.heroes.board;

public enum Direction {

    UPPER_LEFT,
    UPPER_RIGHT,
    LEFT,
    RIGHT,
    LOWER_LEFT,
    LOWER_RIGHT;

    public Direction opposite() {
        return switch (this) {
            case UPPER_LEFT -> LOWER_RIGHT;
            case UPPER_RIGHT -> LOWER_LEFT;
            case LEFT -> RIGHT;
            case RIGHT -> LEFT;
            case LOWER_LEFT -> UPPER_RIGHT;
            case LOWER_RIGHT -> UPPER_LEFT;
        };
    }

    public Position move(Position position) {
        int width = position.width();
        int height = position.height();
        switch (this) {
            case UPPER_LEFT, UPPER_RIGHT -> height--;
            case LEFT -> width--;
            case RIGHT -> width++;
            case LOWER_LEFT, LOWER_RIGHT -> height++;
        }
        if (height % 2 == 0) {
            switch (this) {
                case UPPER_RIGHT, LOWER_RIGHT -> width++;
            }
        } else {
            switch (this) {
                case UPPER_LEFT, LOWER_LEFT -> width--;
            }
        }
        return new Position(height, width);
    }
}
