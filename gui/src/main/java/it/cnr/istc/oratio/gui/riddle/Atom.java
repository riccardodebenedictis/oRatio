/*
 * Copyright (C) 2018 Riccardo De Benedictis <riccardo.debenedictis@istc.cnr.it>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
package it.cnr.istc.oratio.gui.riddle;

/**
 *
 * @author Riccardo De Benedictis <riccardo.debenedictis@istc.cnr.it>
 */
public class Atom extends Item {

    private final AtomState state;
    private final Item tau;

    Atom(Core core, Predicate predicate, final AtomState state, final Item tau) {
        super(core, predicate);
        this.state = state;
        this.tau = tau;
    }

    @Override
    public Predicate getType() {
        return (Predicate) super.getType();
    }

    public AtomState getState() {
        return state;
    }

    public Item getTau() {
        return tau;
    }

    public enum AtomState {
        Active, Inactive, Unified
    }
}
