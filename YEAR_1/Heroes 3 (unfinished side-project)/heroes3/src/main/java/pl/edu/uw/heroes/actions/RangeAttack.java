package pl.edu.uw.heroes.actions;

import pl.edu.uw.heroes.simulation.GameState;
import pl.edu.uw.heroes.units.Unit;

public class RangeAttack extends Attack {

    public RangeAttack(Unit unit, Unit attackedUnit) {
        super(unit, attackedUnit);
    }

    @Override
    public void execute(GameState gameState) {
        double differenceHeight = Math.pow(unit.getField().getPosition().height() - attackedUnit.getField().getPosition().height(), 2);
        double differenceHorizontal = Math.pow(unit.getField().getPosition().width() - attackedUnit.getField().getPosition().width(), 2);
        double distance = Math.round(Math.sqrt(differenceHorizontal + differenceHeight));

        if (distance > 10) {
            attackedUnit.isAttacked(calculateDamage() * 0.5);
        } else {
            attackedUnit.isAttacked(calculateDamage());
        }
    }

    @Override
    public String toString() {
        return "Unit " + unit + " range attacks " + attackedUnit;
    }
}
