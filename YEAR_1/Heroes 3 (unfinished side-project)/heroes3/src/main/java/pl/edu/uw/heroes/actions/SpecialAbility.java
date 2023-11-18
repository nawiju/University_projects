package pl.edu.uw.heroes.actions;

import pl.edu.uw.heroes.units.Unit;

public enum SpecialAbility {
   DRAGON_BREATH, DEATH_CLOUD;

   public Attack createSpecialAbility(Unit unit, Unit attackedUnit) {
       switch(this) {
           case DRAGON_BREATH -> {
               return new DragonBreath(unit, attackedUnit);
           }
           case DEATH_CLOUD -> {
               return new DeathCloud(unit, attackedUnit);
           }
       }
       return null;
   }
}
