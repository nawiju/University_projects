package pl.edu.uw.heroes.actions;

import pl.edu.uw.heroes.simulation.GameState;
import pl.edu.uw.heroes.units.Unit;

public class EmptyAction extends UnitAction {
    public EmptyAction(Unit unit) {
        super(unit);
    }

    @Override
    public void execute(GameState gameState) {

    }

    @Override
    public String toString() {
        return "Do nothing more.";
    }
}
