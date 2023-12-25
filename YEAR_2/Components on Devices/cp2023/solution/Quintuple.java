package cp2023.solution;

// A class that permits me to hold five elements of different types in one structure
public class Quintuple<T, V, G, H, I> {
    private T first;
    private V second;
    private G third;
    private H fourth;
    private I fifth;
    public Quintuple(T first, V second, G third, H fourth, I fifth) {
        this.first = first;
        this.second = second;
        this.third = third;
        this.fourth = fourth;
        this.fifth = fifth;
    }

    public T getFirst() {
        return this.first;
    }

    public V getSecond() {
        return this.second;
    }

    public G getThird() {
        return this.third;
    }

    public H getFourth() {
        return this.fourth;
    }
    public I getFifth() { return this.fifth; }

    public void setFirst(T first) {
        this.first = first;
    }

    public void setSecond(V second) {
        this.second = second;
    }

    public void setThird(G third) {
        this.third = third;
    }

    public void setFourth(H fourth) {
        this.fourth = fourth;
    }

    public void setFifth(I fifth) { this.fifth = fifth; }
}
