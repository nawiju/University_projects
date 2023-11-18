package pl.edu.uw.heroes.board;

import java.util.*;
import java.util.stream.Collectors;

public class BFSCalculator {

    public Collection<Field> calculatePossibleMoves(Field start) {
        Set<Field> visited = new HashSet<>();
        List<Field> currentLevel = new ArrayList<>();
        List<Field> nextLevel = new ArrayList<>();

        visited.add(start);
        currentLevel.add(start);

        for (int movesLeft = start.getUnit().getSpeed(); movesLeft > 0; movesLeft--) {
            for (Field field : currentLevel) {
                for (Field neighbor : field.getNeighbors()) {
                    if ((neighbor.isEmpty() || start.getUnit().isFlying()) && !visited.contains(neighbor)) {
                        visited.add(neighbor);
                        nextLevel.add(neighbor);
                    }
                }
            }
            currentLevel = nextLevel;
            nextLevel = new ArrayList<>();
        }

        visited.remove(start);
        return visited.stream()
                .filter(Field::isEmpty)
                .collect(Collectors.toSet());
    }
}
