package it.cnr.istc.pst.oratio;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.IdentityHashMap;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.Objects;
import java.util.stream.Stream;

import it.cnr.istc.pst.oratio.GraphListener.State;

public class Solver implements Scope, Env {

    static {
        System.loadLibrary("solver-api");
    }
    public static final String BOOL = "bool";
    public static final String INT = "int";
    public static final String REAL = "real";
    public static final String TP = "tp";
    public static final String STRING = "string";
    public static final String IMPULSE = "Impulse";
    public static final String INTERVAL = "Interval";
    public static final String ORIGIN = "origin";
    public static final String HORIZON = "horizon";
    public static final String AT = "at";
    public static final String START = "start";
    public static final String END = "end";

    private final long native_handle;
    final Map<String, Field> fields = new LinkedHashMap<>();
    final Map<String, Collection<Method>> methods = new LinkedHashMap<>();
    final Map<String, Type> types = new LinkedHashMap<>();
    final Map<String, Predicate> predicates = new LinkedHashMap<>();
    final Map<String, Item> exprs = new LinkedHashMap<>();
    final Map<Item, String> expr_names = new IdentityHashMap<>();
    private final Map<String, Item> all_items = new HashMap<>();
    private final Map<String, Type> all_types = new HashMap<>();
    private final Collection<GraphListener> graph_listeners = new ArrayList<>();
    private final Collection<StateListener> state_listeners = new ArrayList<>();
    private final Collection<ExecutorListener> executor_listeners = new ArrayList<>();

    public Solver() {
        native_handle = new_instance();
    }

    private native long new_instance();

    public native void dispose();

    @Override
    public Solver getSolver() {
        return this;
    }

    @Override
    public Scope getScope() {
        return this;
    }

    @Override
    public Field getField(String name) throws NoSuchFieldException {
        return fields.get(name);
    }

    @Override
    public Map<String, Field> getFields() {
        return Collections.unmodifiableMap(fields);
    }

    @Override
    public Method getMethod(final String name, final Type... parameter_types) throws NoSuchMethodException {
        if (Stream.of(parameter_types).anyMatch(Objects::isNull))
            throw new IllegalArgumentException("parameter types cannot be null");
        boolean isCorrect;
        if (methods.containsKey(name))
            for (Method m : methods.get(name))
                if (m.pars.length == parameter_types.length) {
                    isCorrect = true;
                    for (int i = 0; i < m.pars.length; i++)
                        if (!m.pars[i].type.isAssignableFrom(parameter_types[i])) {
                            isCorrect = false;
                            break;
                        }
                    if (isCorrect)
                        return m;
                }

        // not found
        throw new NoSuchMethodException(name);
    }

    @Override
    public Collection<Method> getMethods() {
        Collection<Method> c_methods = new ArrayList<>(methods.size());
        methods.values().forEach(ms -> c_methods.addAll(ms));
        return Collections.unmodifiableCollection(c_methods);
    }

    void defineMethod(final Method method) {
        if (!methods.containsKey(method.name))
            methods.put(method.name, new ArrayList<>());
        methods.get(method.name).add(method);
    }

    @Override
    public Type getType(String name) throws ClassNotFoundException {
        Type type = types.get(name);
        if (type != null)
            return type;

        // not found
        throw new ClassNotFoundException(name);
    }

    @Override
    public Map<String, Type> getTypes() {
        return Collections.unmodifiableMap(types);
    }

    @Override
    public Predicate getPredicate(final String name) throws ClassNotFoundException {
        Predicate predicate = predicates.get(name);
        if (predicate != null)
            return predicate;

        // not found
        throw new ClassNotFoundException(name);
    }

    @Override
    public Map<String, Predicate> getPredicates() {
        return Collections.unmodifiableMap(predicates);
    }

    @Override
    public Item get(String name) throws NoSuchFieldException {
        Item item = exprs.get(name);
        if (item != null)
            return item;

        // not found
        throw new NoSuchFieldException(name);
    }

    @Override
    public Map<String, Item> getExprs() {
        return Collections.unmodifiableMap(exprs);
    }

    String guessName(final Item itm) {
        return expr_names.get(itm);
    }

    public native void read(String script);

    public native void read(String[] files);

    public native void solve();

    private void fireFlawCreated(final long id, final long[] causes, final String label, final byte state,
            final int position_lb, final int position_ub) {
        State c_state = State.values()[state];
        Bound position = new Bound(position_lb, position_ub);
        graph_listeners.stream().forEach(l -> l.flawCreated(id, causes, label, c_state, position));
    }

    private void fireFlawStateChanged(final long id, final byte state) {
        State c_state = State.values()[state];
        graph_listeners.stream().forEach(l -> l.flawStateChanged(id, c_state));
    }

    private void fireFlawCostChanged(final long id, final long cost_num, final long cost_den) {
        Rational cost = new Rational(cost_num, cost_den);
        graph_listeners.stream().forEach(l -> l.flawCostChanged(id, cost));
    }

    private void fireFlawPositionChanged(final long id, final int position_lb, final int position_ub) {
        Bound position = new Bound(position_lb, position_ub);
        graph_listeners.stream().forEach(l -> l.flawPositionChanged(id, position));
    }

    private void fireCurrentFlaw(final long id) {
        graph_listeners.stream().forEach(l -> l.currentFlaw(id));
    }

    private void fireResolverCreated(final long id, final long effect, final String label, final long cost_num,
            final long cost_den, final byte state) {
        State c_state = State.values()[state];
        Rational cost = new Rational(cost_num, cost_den);
        graph_listeners.stream().forEach(l -> l.resolverCreated(id, effect, cost, label, c_state));
    }

    private void fireResolverStateChanged(final long id, final byte state) {
        State c_state = State.values()[state];
        graph_listeners.stream().forEach(l -> l.resolverStateChanged(id, c_state));
    }

    private void fireCurrentResolver(final long id) {
        graph_listeners.stream().forEach(l -> l.currentResolver(id));
    }

    private void fireCausalLinkAdded(final long flaw, final long resolver) {
        graph_listeners.stream().forEach(l -> l.causalLinkAdded(flaw, resolver));
    }

    private void fireLog(String log) {
        state_listeners.stream().forEach(l -> l.log(log));
    }

    private void fireInit() {
        state_listeners.stream().forEach(l -> l.init());
    }

    private void fireStateChanged() {
        state_listeners.stream().forEach(l -> l.stateChanged());
    }

    private void fireTick(final long current_time_num, final long current_time_den) {
        Rational current_time = new Rational(current_time_num, current_time_den);
        executor_listeners.stream().forEach(l -> l.tick(current_time));
    }

    private void fireStartingAtoms(final long[] atoms) {
        executor_listeners.stream().forEach(l -> l.startingAtoms(atoms));
    }

    private void fireEndingAtoms(final long[] atoms) {
        executor_listeners.stream().forEach(l -> l.endingAtoms(atoms));
    }

    public void addGraphListener(GraphListener l) {
        graph_listeners.add(l);
    }

    public void removeGraphListener(GraphListener l) {
        graph_listeners.remove(l);
    }

    public void addStateListener(StateListener l) {
        state_listeners.add(l);
    }

    public void removeStateListener(StateListener l) {
        state_listeners.remove(l);
    }

    public void addExecutorListener(ExecutorListener l) {
        executor_listeners.add(l);
    }

    public void removeExecutorListener(ExecutorListener l) {
        executor_listeners.remove(l);
    }
}
