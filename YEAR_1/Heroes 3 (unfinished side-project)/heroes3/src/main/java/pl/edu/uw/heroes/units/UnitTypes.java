package pl.edu.uw.heroes.units;

import lombok.Getter;

public enum UnitTypes {

    DRAGON(40, 50.0), UNICORN(18, 22), GOLEM(4, 5), SKELETON(1.0, 3), GRIFFIN(3, 6), CENTAUR(1.0, 1.0), PEGASUS(5, 9), GREMLIN(1.0, 2), WOOD_ELF(3, 5), LICH(11, 13);

    @Getter
    protected double damageMin;

    @Getter
    protected double damageMax;

    UnitTypes(double damageMin, double damageMax){
        this.damageMin = damageMin;
        this.damageMax = damageMax;
    }

    @Override
    public String toString() {
        switch(this) {
            case DRAGON -> {
                return "Dragon";
            }
            case UNICORN -> {
                return "Unicorn";
            }
            case GOLEM -> {
                return "Golem";
            }
            case SKELETON -> {
                return "Skeleton";
            }
            case GRIFFIN -> {
                return "Griffin";
            }
            case CENTAUR -> {
                return "Centaur";
            }
            case PEGASUS -> {
                return "Pegasus";
            }
            case GREMLIN -> {
                return "Gremlin";
            }
            case WOOD_ELF -> {
                return "Wood Elf";
            }
            case LICH -> {
                return "Lich";
            }
        }
        return null;
    }
}
