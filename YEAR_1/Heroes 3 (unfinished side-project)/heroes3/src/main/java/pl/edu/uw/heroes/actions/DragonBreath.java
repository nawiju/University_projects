package pl.edu.uw.heroes.actions;

import pl.edu.uw.heroes.board.Field;
import pl.edu.uw.heroes.simulation.GameState;
import pl.edu.uw.heroes.units.Unit;

import java.util.stream.Stream;

public class DragonBreath extends Attack {
    public DragonBreath(Unit unit, Unit attackedUnit) {
        super(unit, attackedUnit);
    }

    @Override
    public void execute(GameState gameState) {
        int difference = unit.getField().getPosition().width() - attackedUnit.getField().getPosition().width();

        attackedUnit.isAttacked(calculateDamage());

        if (difference > 0) {
            Stream<Field> fieldStream = attackedUnit.getField().getNeighbors().stream()
                    .filter(f -> f.getPosition().width() == attackedUnit.getField().getPosition().width() + 1 && f.getPosition().height() == attackedUnit.getField().getPosition().height())
                    .peek(x -> x.getUnit().isAttacked(calculateDamage()));;
        } else {
            Stream<Field> fieldStream = attackedUnit.getField().getNeighbors().stream()
                    .filter(f -> f.getPosition().width() == attackedUnit.getField().getPosition().width() - 1 && f.getPosition().height() == attackedUnit.getField().getPosition().height())
                    .peek(x -> x.getUnit().isAttacked(calculateDamage()));
        }
    }

    @Override
    public String toString() {
        return "Dragon " + unit + " uses dragon breath on " + attackedUnit;
    }
}
