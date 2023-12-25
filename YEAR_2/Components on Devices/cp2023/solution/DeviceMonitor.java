package cp2023.solution;

import cp2023.base.ComponentId;
import cp2023.base.ComponentTransfer;
import cp2023.base.DeviceId;

import java.util.*;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.Semaphore;
import java.util.stream.IntStream;

public class DeviceMonitor {
    // Semaphores on which a component that reserved the i-th slot waits for the previous component
    // to leave the corresponding slot
    private final Semaphore[] toSlot;

    // isWaitingOnComponent[i] = true if a component has already reserved the i-th slot
    private boolean isWaitingOnComponent[];

    // Map of components that have reserved a slot on the device and are waiting for it to be fully freed
    private Map<ComponentId, Integer> componentWaitingOnSlot = new HashMap<>();

    private final Semaphore _mutex; // Global mutex
    private int availableSlots; // Number of slots currently available
    private DeviceId deviceId;

    // Keeps track of which slot each component occupies, null if it is free
    private ComponentId[] componentPerSlot;

    // Map of awaiting transfers to reserve a slot on the device; ordered by arrival FIFO
    private HashMap<DeviceId, ArrayList<Quintuple<Semaphore, Semaphore, ComponentTransfer,
            CountDownLatch, ComponentTransfer>>> transferMap;

    // Returns index of the smallest available slot in available slots
    private PriorityQueue<Integer> smallestFreeSlot;

    // The head of the ArrayList contains the index of the component that announced its departure from the
    // device the earliest.
    private ArrayList<Integer> futureAvailableSlots = new ArrayList<>();

    public DeviceMonitor(int capacity, ArrayList<ComponentId> componentsOnDevice, DeviceId deviceId,
                         HashMap<DeviceId, ArrayList<Quintuple<Semaphore, Semaphore,
                         ComponentTransfer, CountDownLatch, ComponentTransfer>>> transferMap,
                         Semaphore _mutex) {
        int size;

        // Just in case componentsOnDevice does not exist yet, then it is equal to being empty, therefore 0
        if (componentsOnDevice == null || componentsOnDevice.equals(null)) {
            size = 0;
        } else {
            size = componentsOnDevice.size();
        }

        availableSlots = capacity - size;
        componentPerSlot = new ComponentId[capacity];
        this.deviceId = deviceId;
        this.transferMap = transferMap;
        toSlot = new Semaphore[capacity];
        this._mutex = _mutex;

        Arrays.fill(toSlot, new Semaphore(0, true));

        for (int i = 0; i < size; i++) {
            componentPerSlot[i] = componentsOnDevice.get(i);
        }

        smallestFreeSlot = new PriorityQueue<>();
        smallestFreeSlot.addAll(IntStream.rangeClosed(size, capacity - 1).boxed().toList());
        isWaitingOnComponent = new boolean[capacity];
        Arrays.fill(isWaitingOnComponent, false);
    }

    // This function manages all the actions of a transfer before it can execute its prepare()
    public void reserveOnDevice(ComponentId component, ComponentTransfer transfer, HashMap<ComponentId,
                                Quintuple<Semaphore, Semaphore, ComponentTransfer, CountDownLatch,
                                ComponentTransfer>> transfersInCycle) {
        try {
            _mutex.acquire();
            // Case 1: There is at least one slot currently available
            // The transfer can just write itself into the slot
            if (availableSlots > 0) {
                availableSlots--;
                // Component reserves slot
                componentPerSlot[smallestFreeSlot.poll()] = component;
                _mutex.release();
            } else {
                // Case 2: There are no slots currently available
                Semaphore newAwaitingSemaphore = new Semaphore(0, true);
                Quintuple<Semaphore, Semaphore, ComponentTransfer, CountDownLatch, ComponentTransfer> awaitingTransfer =
                        new Quintuple(newAwaitingSemaphore, null, transfer, null, null);

                // The transfer is added into the global map of awaiting transfers to their destination devices
                transferMap.get(deviceId).add(awaitingTransfer);

                // Checks if it completes a cycle
                ArrayList<Quintuple<Semaphore, Semaphore, ComponentTransfer, CountDownLatch, ComponentTransfer>>
                        currentPath = DFS(deviceId, transferMap, awaitingTransfer);

                // Case 2.1: The transfer does not complete a cycle
                if (currentPath.isEmpty()) {
                    // Case 2.1.1: There are no slots that are scheduled to be released
                    if (futureAvailableSlots.isEmpty()) {
                        _mutex.release();

                        // Transfer waits on this semaphore until it is "awoken" by a departing component
                        newAwaitingSemaphore.acquire();

                        // When it wakes up, it checks if it is part of a cycle
                        // If it is not and it has been woken up, it "skips" to case 2.1.2
                        if (awaitingTransfer.getFifth() != null) {
                            // Checks if it has to wake up another transfer that is part of its cycle
                            if (awaitingTransfer.getSecond() != null) {
                                awaitingTransfer.getSecond().release();
                            } else {
                                // The transfer is the last element of the cycle
                                _mutex.release();
                            }
                            return; // It can execute its prepare() without checking if slots will be available
                        }
                    }

                    // Case 2.1.2: There are slots scheduled to be freed
                    // Transfer reserves the first slot that had been signalled to be freed up
                    int reservedSlot = futureAvailableSlots.remove(0);

                    componentWaitingOnSlot.put(component, reservedSlot);

                    // Remove transfer from map of awaiting transfers
                    for (Quintuple<Semaphore, Semaphore, ComponentTransfer, CountDownLatch, ComponentTransfer>
                            quintuple : transferMap.get(deviceId)) {
                        if (quintuple.getThird().getComponentId().equals(component)) {
                            transferMap.get(deviceId).remove(quintuple);
                            break;
                        }
                    }
                    _mutex.release();
                } else {
                    // Case 2.2: The transfer completes the cycle
                    currentPath.add(0, awaitingTransfer);

                    // Creates a countDownLatch for all transfers part of the cycle
                    CountDownLatch countDownLatch = new CountDownLatch(currentPath.size());

                    for (int i = 0; i < currentPath.size(); i++) {
                        Quintuple<Semaphore, Semaphore, ComponentTransfer, CountDownLatch, ComponentTransfer> quintuple
                                = currentPath.get(i);

                        for (Quintuple<Semaphore, Semaphore, ComponentTransfer, CountDownLatch, ComponentTransfer>
                                subQuintuple : transferMap.get(quintuple.getThird().getDestinationDeviceId())) {

                            if (subQuintuple.getThird().getComponentId()
                                    .equals(quintuple.getThird().getComponentId())) {
                                subQuintuple.setFourth(countDownLatch);

                                // Sets the semaphore the transfer will release when awoken in case 2.1.1
                                if (i < currentPath.size() - 1) {
                                    subQuintuple.setSecond(currentPath.get((1 + i) % currentPath.size()).getFirst());
                                }
                                subQuintuple.setFifth(currentPath.get((1 + i) % currentPath.size()).getThird());

                                transfersInCycle.put(subQuintuple.getThird().getComponentId(), subQuintuple);
                                transferMap.get(quintuple.getThird().getDestinationDeviceId()).remove(quintuple);
                                break;
                            }
                        }
                    }

                    // Released the appropriate semaphore
                    currentPath.get(0).getSecond().release();
                }
            }
        } catch (InterruptedException e) {
            throw new RuntimeException("panic: unexpected thread interruption");
        }
    }

    // This function is executed after prepare() and manages all the actions to do with writing the component into
    // the appropriate slot on the device
    public void addToDevice(ComponentId component) {
        try {
            _mutex.acquire();

            if (componentWaitingOnSlot.containsKey(component)) {
                int reservedSlot = componentWaitingOnSlot.get(component);
                componentWaitingOnSlot.remove(component);

                // Signals that the slot is reserved (as you, my dear, shall see later, this should be done here
                // rather than earlier)
                isWaitingOnComponent[reservedSlot] = true;

                // If the slot is still not available as the previous component has not yet been removed from the
                // device as it had not finished its prepare(), this transfer must wait
                if (componentPerSlot[reservedSlot] != null) {
                    _mutex.release();
                    toSlot[reservedSlot].acquire();
                }

                // The slot is fully available and can be taken
                isWaitingOnComponent[reservedSlot] = false;
                componentPerSlot[reservedSlot] = component;
            }
            _mutex.release();
        } catch (InterruptedException e) {
            throw new RuntimeException("panic: unexpected thread interruption");
        }
    }

    // This function signals that a slot shall be available
    public void signalFutureSlotAvailability(ComponentId component) {
        try {
            _mutex.acquire();

            int componentIndex = findComponentIndex(component);
            futureAvailableSlots.add(componentIndex);

            // Releases the transfer that has waited the longest to enter the device
            if (!transferMap.get(deviceId).isEmpty()) {
                Quintuple<Semaphore, Semaphore, ComponentTransfer, CountDownLatch, ComponentTransfer> awakenedTransfer
                        = transferMap.get(deviceId).remove(0);
                awakenedTransfer.getFirst().release();
            } else {
                _mutex.release();
            }
        } catch (InterruptedException e) {
            throw new RuntimeException("panic: unexpected thread interruption");
        }
    }

    // This function handles all the updating of the data related to a component being removed from a device
    public void removeFromDevice(ComponentId component) {
        try {
            _mutex.acquire();

            int componentIndex = findComponentIndex(component);

            assert(componentIndex != -1);

            if (isWaitingOnComponent[componentIndex] && !futureAvailableSlots.contains(componentIndex)) {
                // Case 1: there exists a component that has reserved the slot that will be freed by the current
                // transfer, and it has performed its prepare()
                componentPerSlot[componentIndex] = null;
                toSlot[componentIndex].release();
            } else if (!futureAvailableSlots.contains(componentIndex)) {
                // Case 2: there exists a component that has reserved this slot but has not finished its
                // prepare() therefore, it needs to set componentPerSlot[componentIndex] = null so that the transfer
                // does not wait forever on the appropriate semaphore toSlot in addToDevice()
                componentPerSlot[componentIndex] = null;
                _mutex.release();
            } else {
                // Case 3: no component is waiting on this component to be removed
                smallestFreeSlot.add(componentIndex);

                // Removes the index of the slot from futureAvailableSlots
                if (futureAvailableSlots.contains(componentIndex)) {
                    futureAvailableSlots.remove(Integer.valueOf(componentIndex));
                }
                componentPerSlot[componentIndex] = null;
                availableSlots++;

                _mutex.release();
            }
        } catch (InterruptedException e) {
            throw new RuntimeException("panic: unexpected thread interruption");
        }
    }

    // This function handles all the data updating related to managing a cycle in the system
    public void addToDeviceInCycle(ComponentId replaced, ComponentId replacing,
                                   Quintuple<Semaphore, Semaphore, ComponentTransfer, CountDownLatch,
                                   ComponentTransfer> quintuple, Map<ComponentId, DeviceId> componentPlacement,
                                   Set<ComponentId> componentsOperatedOn, HashMap<ComponentId, Quintuple<Semaphore,
                                   Semaphore, ComponentTransfer, CountDownLatch, ComponentTransfer>> transfersInCycle) {
        try {
            _mutex.acquire();

            int componentIndex = findComponentIndex(replaced);
            componentPerSlot[componentIndex] = replacing;
            transferMap.get(deviceId).remove(quintuple);
            componentPlacement.replace(replacing, deviceId);
            componentsOperatedOn.remove(replacing);
            transfersInCycle.remove(replacing);

            _mutex.release();
        } catch (InterruptedException e) {
            throw new RuntimeException("panic: unexpected thread interruption");
        }
    }

    // Helper function
    private int findComponentIndex(ComponentId componentId) {
        for (int i = 0; i < componentPerSlot.length; i++) {
            if (componentId.equals(componentPerSlot[i])) {
                return i;
            }
        }
        return -1;
    }

    // DFS that returns the path of the cycle
    private ArrayList<Quintuple<Semaphore, Semaphore, ComponentTransfer, CountDownLatch, ComponentTransfer>>
            DFS(DeviceId deviceId, HashMap<DeviceId, ArrayList<Quintuple<Semaphore, Semaphore, ComponentTransfer,
            CountDownLatch, ComponentTransfer>>> transferMap, Quintuple<Semaphore, Semaphore, ComponentTransfer,
            CountDownLatch, ComponentTransfer> firstNode) {

        HashSet<DeviceId> visited = new HashSet<>();
        Stack<DeviceId> toVisit = new Stack<>();
        Stack<Quintuple<Semaphore, Semaphore, ComponentTransfer, CountDownLatch, ComponentTransfer>> currentPath
                = new Stack<>();
        Stack<Boolean> toKeep = new Stack<>();

        toVisit.push(deviceId);
        toVisit.push(firstNode.getThird().getSourceDeviceId());
        currentPath.push(null); // Dummy node
        currentPath.push(firstNode);
        toKeep.push(true);
        toKeep.push(true);

        while (!toVisit.empty()) {
            DeviceId currentNode = toVisit.pop();
            Boolean keep = toKeep.pop();

            if (keep) {
                toKeep.push(false);
                toVisit.push(currentNode);
            }
            else {
                currentPath.pop();
                continue;
            }

            if (deviceId.equals(currentNode))
                break;

            if (!visited.contains(currentNode)) {
                ArrayList<Quintuple<Semaphore, Semaphore, ComponentTransfer, CountDownLatch, ComponentTransfer>>
                        awaitingTransfers = transferMap.get(currentNode);
                if (currentNode == null)
                    break;

                for (int i = awaitingTransfers.size() - 1; i >= 0; i--) {
                    if (awaitingTransfers.get(i).getThird().getSourceDeviceId() != null
                            && awaitingTransfers.get(i).getFourth() == null) {
                        toVisit.push(awaitingTransfers.get(i).getThird().getSourceDeviceId());
                        currentPath.push(awaitingTransfers.get(i));
                        toKeep.push(true);
                    }
                }
                visited.add(currentNode);
            }
        }

        ArrayList<Quintuple<Semaphore, Semaphore, ComponentTransfer, CountDownLatch, ComponentTransfer>>
                currentPathForwards = new ArrayList<>();

        while (!currentPath.empty()) {
            // The cycle is represented by all the corresponding quintuplets to all the false values in toKeep
            if (!toKeep.peek()) {
                // Adds to the front of the list
                currentPathForwards.add(currentPath.peek());
            }
            currentPath.pop();
            toKeep.pop();
        }

        // Removes the phantom node
        currentPathForwards.remove(currentPathForwards.size() - 1);

        return currentPathForwards;
    }
}