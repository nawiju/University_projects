package pl.edu.uw.heroes.units;

import lombok.ToString;
import pl.edu.uw.heroes.players.Player;

import static pl.edu.uw.heroes.units.UnitTypes.*;

@ToString
public class RoyalGriffin extends Unit {
    public RoyalGriffin(Player owner) {
        super(owner, GRIFFIN, null, new UnitStatistics(true, 9, 9, 9, 0, GRIFFIN.getDamageMin(), GRIFFIN.getDamageMax(), 25, false, false), null);
    }

    @Override
    public void doDefend() {
        this.defense = this.statistics.getDefense() * 1.05;
    }
}
