package pl.edu.uw.heroes.actions;

import pl.edu.uw.heroes.board.Field;
import pl.edu.uw.heroes.simulation.GameState;
import pl.edu.uw.heroes.units.Unit;

import java.util.ArrayList;
import java.util.Collection;

public class Move extends UnitAction {

    private final Field destination;

    public Move(Unit unit, Field destination) {
        super(unit);
        this.destination = destination;
    }

    @Override
    public void execute(GameState gameState) {
        unit.doMove(destination);

        Collection<Action> actions = new ArrayList<>();
        actions.add(new EmptyAction(unit));

        for (Field neighbor: unit.getField().getNeighbors()) {
            if (neighbor.getUnit() != null) {
                CloseAttack closeAttack = new CloseAttack(unit, neighbor.getUnit());
                actions.add(closeAttack);
            }
        }

        if(!actions.isEmpty()) {
            Action action = unit.getOwner().chooseAction(actions);
            System.out.println(unit.getOwner() + " chose " + action);
            action.execute(gameState);
        }
    }

    @Override
    public String toString() {
        return "Unit " + unit + " moves from to " + destination;
    }
}
