package pl.edu.uw.heroes.units;
import lombok.Getter;

public record UnitStatistics(@Getter boolean flying, @Getter int speed, @Getter int attack, @Getter double defense,
                             @Getter int arrows, @Getter double damageMin, @Getter double damageMax, @Getter int health, @Getter boolean ranged, @Getter boolean areaAttack) {


}
