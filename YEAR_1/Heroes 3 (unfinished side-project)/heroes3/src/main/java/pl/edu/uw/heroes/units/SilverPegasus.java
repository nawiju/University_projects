package pl.edu.uw.heroes.units;

import lombok.ToString;
import pl.edu.uw.heroes.players.Player;

import static pl.edu.uw.heroes.units.UnitTypes.*;

@ToString
public class SilverPegasus extends Unit {

    public SilverPegasus(Player owner) {
        super(owner, GRIFFIN, null, new UnitStatistics(true, 12, 9, 10, 0, PEGASUS.getDamageMin(), PEGASUS.getDamageMax(), 30, false, false), null);
    }

    @Override
    public void doDefend() {
        this.defense = this.statistics.getDefense() * 1.15;
    }
}
