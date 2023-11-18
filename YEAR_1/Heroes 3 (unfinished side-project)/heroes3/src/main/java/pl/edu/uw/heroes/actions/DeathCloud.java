package pl.edu.uw.heroes.actions;

import pl.edu.uw.heroes.board.Field;
import pl.edu.uw.heroes.simulation.GameState;
import pl.edu.uw.heroes.units.Unit;

public class DeathCloud extends Attack{
    public DeathCloud(Unit unit, Unit attackedUnit) {
        super(unit, attackedUnit);
    }

    @Override
    public void execute(GameState gameState) {
        attackedUnit.isAttacked(calculateDamage());

        for (Field field: attackedUnit.getField().getNeighbors()) {
            if (field.getUnit() != null) {
                field.getUnit().isAttacked(calculateDamage());
            }
        }
    }

    @Override
    public String toString() {
        return "Lich " + unit + " uses death cloud on " + attackedUnit;
    }
}
