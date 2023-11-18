package pl.edu.uw.heroes.players;

import pl.edu.uw.heroes.actions.Action;
import pl.edu.uw.heroes.units.Squad;
import pl.edu.uw.heroes.units.Unit;

import java.util.Collection;

public interface Player {

    Collection<Unit> getUnits();

    Action chooseAction(Collection<Action> actions);

    Squad[] getSquad();

    void addToPlayerUnits(Unit unit) throws Exception;

    Unit getUnitFromSquad(int index) throws Exception;
}
