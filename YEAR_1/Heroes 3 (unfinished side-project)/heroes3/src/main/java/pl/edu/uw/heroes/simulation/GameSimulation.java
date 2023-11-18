package pl.edu.uw.heroes.simulation;

import lombok.AllArgsConstructor;
import pl.edu.uw.heroes.actions.*;
import pl.edu.uw.heroes.board.BFSCalculator;
import pl.edu.uw.heroes.board.Field;
import pl.edu.uw.heroes.players.Player;
import pl.edu.uw.heroes.units.Unit;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Queue;
import java.util.stream.Stream;

@AllArgsConstructor
public class GameSimulation {

    private final GameState state;

    private final BFSCalculator bfsCalculator = new BFSCalculator();

    public boolean executeOneAction() {
        if (state.getPlayerLeft().getUnits().size() == 0 || state.getPlayerRight().getUnits().size() == 0) {
            return false;
        }

        Queue<Unit> units = state.getUnits();
        if (units.isEmpty())
            prepareNextRound();

        Unit unit = units.poll();

        Collection<Action> actions = actionsForUnit(unit);
        Action action = unit.getOwner().chooseAction(actions);
        System.out.println(unit.getOwner() + " chose " + action);
        action.execute(state);
        state.getExecutedActions().add(action);
        return true;
    }

    private void prepareNextRound() {
        Queue<Unit> units = state.getUnits();
        Stream.concat(
                        state.getPlayerLeft().getUnits().stream(),
                        state.getPlayerRight().getUnits().stream()
                )
                .peek(Unit::resetRound)
                .sorted((u1, u2) -> Integer.compare(u2.getSpeed(), u1.getSpeed()))
                .forEach(units::add);
        if (units.isEmpty())
            throw new IllegalStateException("No units on board!");
    }

    private Collection<Action> actionsForUnit(Unit unit) {
        Collection<Action> actions = new ArrayList<>();

        if (!unit.isDead()) {
            actions.add(new Defend(unit));

            boolean canRangeAttack = true;

            if (unit.canWait())
                actions.add(new Wait(unit));

            bfsCalculator.calculatePossibleMoves(unit.getField()).forEach(field -> actions.add(new Move(unit, field)));

            for (Field neighbor : unit.getField().getNeighbors()) {
                if (neighbor.getUnit() != null && neighbor.getUnit().getOwner() != unit.getOwner()) {
                    CloseAttack closeAttack = new CloseAttack(unit, neighbor.getUnit());
                    actions.add(closeAttack);
                    canRangeAttack = false;
                }
            }

            for (SpecialAbility specialAbility : unit.getSpecialAbilities()) {
                Player enemy = unit.getOwner().equals(state.getPlayerLeft()) ? state.getPlayerRight() : state.getPlayerLeft();

                for (Unit troop : enemy.getUnits()) {
                    actions.add(specialAbility.createSpecialAbility(unit, troop));
                }
            }

            if (unit.getStatistics().isRanged()) {
                Player enemy = unit.getOwner().equals(state.getPlayerLeft()) ? state.getPlayerRight() : state.getPlayerLeft();

                for (Unit troop : enemy.getUnits()) {
                    RangeAttack rangeAttack = new RangeAttack(unit, troop);
                    actions.add(rangeAttack);
                }
            }
        } else {
            actions.add(new EmptyAction(unit));
        }
        return actions;
    }

    public void printState() {
        System.out.println("Player left (" + state.getPlayerLeft() + "): ");
        System.out.println(state.getPlayerLeft().getUnits());
        System.out.println("Player right (" + state.getPlayerRight() + "): ");
        System.out.println(state.getPlayerRight().getUnits());
    }
}