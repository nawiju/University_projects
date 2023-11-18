package pl.edu.uw.heroes.actions;

import lombok.AllArgsConstructor;
import pl.edu.uw.heroes.units.Unit;

@AllArgsConstructor
public abstract class UnitAction implements Action {

    protected final Unit unit;
}
