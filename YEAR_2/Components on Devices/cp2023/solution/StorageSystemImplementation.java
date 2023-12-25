package cp2023.solution;

import cp2023.base.ComponentId;
import cp2023.base.ComponentTransfer;
import cp2023.base.DeviceId;
import cp2023.base.StorageSystem;
import cp2023.exceptions.*;

import java.util.*;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.Semaphore;

public class StorageSystemImplementation implements StorageSystem {
    private Map<DeviceId, Integer> deviceTotalSlots;
    private Map<ComponentId, DeviceId> componentPlacement;
    private Map<DeviceId, DeviceMonitor> deviceMonitorMap = new HashMap<>();
    private Set<ComponentId> componentsOperatedOn = new HashSet<>();

    // Global map of all the transfers awaiting to their destination device
    private HashMap<DeviceId, ArrayList<Quintuple<Semaphore, Semaphore, ComponentTransfer, CountDownLatch,
            ComponentTransfer>>> awaitingTransfers = new HashMap<>();
    private HashMap<ComponentId, Quintuple<Semaphore, Semaphore, ComponentTransfer, CountDownLatch,
            ComponentTransfer>> transfersInCycle = new HashMap<>();
    private Semaphore _mutex = new Semaphore(1, true);

    public StorageSystemImplementation(Map<DeviceId, Integer> deviceTotalSlots,
                                       Map<ComponentId, DeviceId> componentPlacement)
            throws IllegalArgumentException {

        if (deviceTotalSlots == null || componentPlacement == null ) {
            throw new IllegalArgumentException("Null input variables.");
        }

        if (deviceTotalSlots.isEmpty()) {
            throw new IllegalArgumentException("No devices in system.");
        }

        this.componentPlacement = new HashMap<>();
        this.deviceTotalSlots = new HashMap<>();

        this.componentPlacement.putAll(componentPlacement);
        this.deviceTotalSlots.putAll(deviceTotalSlots);

        Map<DeviceId, ArrayList<ComponentId>> componentsOnDevice = validateInput();

        // Initialize deviceMonitorMap and awaitingTransfers
        this.deviceTotalSlots.forEach((key, value) -> {
            deviceMonitorMap.put(key, new DeviceMonitor(value, componentsOnDevice.get(key),
                    key, awaitingTransfers, _mutex));
            awaitingTransfers.put(key, new ArrayList<>());
        });
    }

    @Override
    public void execute(ComponentTransfer transfer) throws TransferException, IllegalArgumentException {
        // Input validation. If an exception is thrown, will not continue executing this code.
        validTransferType(transfer);
        validDevice(transfer);
        componentAlreadyExists(transfer);
        componentDoesNotExist(transfer);
        componentAlreadyAtDestination(transfer);

        ComponentId component = transfer.getComponentId();
        DeviceId source = transfer.getSourceDeviceId();
        DeviceId destination = transfer.getDestinationDeviceId();

        try {
            _mutex.acquire();
            if (componentsOperatedOn.contains(component)) {
                throw new ComponentIsBeingOperatedOn(component);
            }
            componentsOperatedOn.add(transfer.getComponentId());
            _mutex.release();
        } catch (InterruptedException e) {
            throw new RuntimeException("panic: unexpected thread interruption");
        }

        // Checks which type transfer has been requested
        if (destination == null) {
            deletionTransfer(transfer, source);
            return;
        }

        if (source == null) {
            additionTransfer(transfer, destination);
            return;
        }

        moveTransfer(transfer, destination, source);
    }

    private void moveTransfer(ComponentTransfer transfer, DeviceId destination, DeviceId source) {
        ComponentId componentId = transfer.getComponentId();

        deviceMonitorMap.get(destination).reserveOnDevice(componentId, transfer, transfersInCycle);
        transfer.prepare();

        try {
            _mutex.acquire();
            Quintuple<Semaphore, Semaphore, ComponentTransfer, CountDownLatch, ComponentTransfer> quintuple =
                    transfersInCycle.get(componentId);
            _mutex.release();

            if (quintuple != null) {
                // Case 1: the transfer is part of a cycle, and it has been allowed to fully execute its prepare()
                quintuple.getFourth().countDown();
                // Awaits on the countDownLatch until all transfers have completed their prepare()
                quintuple.getFourth().await();
                deviceMonitorMap.get(destination).addToDeviceInCycle(quintuple.getFifth()
                        .getComponentId(), quintuple.getThird().getComponentId(), quintuple,
                        componentPlacement, componentsOperatedOn, transfersInCycle);
            } else {
                // Case 2: the transfer is not part of a cycle
                deviceMonitorMap.get(source).signalFutureSlotAvailability(componentId);
                deviceMonitorMap.get(source).removeFromDevice(componentId);
                deviceMonitorMap.get(destination).addToDevice(componentId);

                // Update the data regarding the state of the system
                // This is done after perform as only then has the component been actually moved
                try {
                    _mutex.acquire();
                    componentPlacement.replace(componentId, destination);
                    componentsOperatedOn.remove(componentId);
                    _mutex.release();
                } catch (InterruptedException e) {
                    throw new RuntimeException("panic: unexpected thread interruption");
                }
            }

        } catch (InterruptedException e) {
            throw new RuntimeException("panic: unexpected thread interruption");
        }

        transfer.perform();
    }

    private void additionTransfer(ComponentTransfer transfer, DeviceId destination) {
        ComponentId componentId = transfer.getComponentId();

        deviceMonitorMap.get(destination).reserveOnDevice(componentId, transfer, transfersInCycle);
        transfer.prepare();

        deviceMonitorMap.get(destination).addToDevice(componentId);
        transfer.perform();

        // Update the data regarding the state of the system
        try {
            _mutex.acquire();
            componentPlacement.put(componentId, destination);
            componentsOperatedOn.remove(componentId);
            _mutex.release();
        } catch (InterruptedException e) {
            throw new RuntimeException("panic: unexpected thread interruption");
        }
    }

    private void deletionTransfer(ComponentTransfer transfer, DeviceId source) {
        ComponentId componentId = transfer.getComponentId();

        deviceMonitorMap.get(source).signalFutureSlotAvailability(componentId);
        transfer.prepare();

        deviceMonitorMap.get(source).removeFromDevice(componentId);
        transfer.perform();

        // Update the data regarding the state of the system
        try {
            _mutex.acquire();
            componentPlacement.remove(componentId);
            componentsOperatedOn.remove(componentId);
            _mutex.release();
        } catch (InterruptedException e) {
            throw new RuntimeException("panic: unexpected thread interruption");
        }
    }

    private Map<DeviceId, ArrayList<ComponentId>> validateInput() {
        HashMap<DeviceId, ArrayList<ComponentId>> filledDevices = new HashMap<>();

        for (Map.Entry<DeviceId, Integer> device : this.deviceTotalSlots.entrySet()) {
            if (device.equals(null) || device.getValue() == null || device.getValue().equals(null)
                    || device.getValue() <= 0) {
                throw new IllegalArgumentException("No slots on device " + device.getKey().toString()
                        + " exception.");
            }
        }

        // Checks if each component is assigned a device that actually exists and if no device has been assigned
        // more components than its capacity.
        for (Map.Entry<ComponentId, DeviceId> component : componentPlacement.entrySet()) {
            DeviceId device = component.getValue();
            ComponentId componentId = component.getKey();

            if (component.equals(null) || componentId.equals(null) || componentId == null
                    || component.getValue() == null || component.getValue().equals(null)
                    || component.getKey() == null) {
                throw new IllegalArgumentException("Component incorrectly declared");
            }

            if (this.deviceTotalSlots.get(device) == null || this.deviceTotalSlots.get(device).equals(null)) {
                throw new IllegalArgumentException("Component" + componentId.toString()
                        + " assigned to nonexistent device exception.");
            }

            if (filledDevices.get(device) == null) {
                ArrayList<ComponentId> components = new ArrayList<>();
                components.add(componentId);
                filledDevices.put(device, components);
            } else {
                filledDevices.get(device).add(componentId);
            }

            if (filledDevices.get(device).size() > this.deviceTotalSlots.get(device)) {
                throw new IllegalArgumentException("Too many components assigned to device "
                        + device + "!");
            }
        }

        return filledDevices;
    }

    // Functions validating the transfer, the devices, the component, inputted arguments, etc.
    private void validTransferType(ComponentTransfer transfer) throws TransferException {
        if (transfer.getDestinationDeviceId() == null && transfer.getSourceDeviceId() == null) {
            throw new IllegalTransferType(transfer.getComponentId());
        }
    }

    private void validDevice(ComponentTransfer transfer) throws TransferException {
        DeviceId source = transfer.getSourceDeviceId();
        DeviceId destination = transfer.getDestinationDeviceId();

        // If source is null then this is an adding operation, thus valid.
        if (source != null && !this.deviceTotalSlots.containsKey(source)) {
            throw new DeviceDoesNotExist(source);
        }

        // If destination is null, then this is a deleting operation, hence valid.
        if (destination != null && !this.deviceTotalSlots.containsKey(destination)) {
            throw new DeviceDoesNotExist(destination);
        }
    }

    private void componentAlreadyExists(ComponentTransfer transfer) throws TransferException {
        ComponentId componentId = transfer.getComponentId();
        DeviceId deviceId;

        try {
            _mutex.acquire();
            deviceId = componentPlacement.get(componentId);
            _mutex.release();
        } catch (InterruptedException e) {
            throw new RuntimeException("panic: unexpected thread interruption");
        }

        // If componentPlacement.get(componentId) returns null, then the component does not exist in the system
        // assuming that in the componentPlacement, its value cannot be null.
        if (transfer.getSourceDeviceId() == null && deviceId != null) {
            throw new ComponentAlreadyExists(componentId, deviceId);
        }
    }

    private void componentDoesNotExist(ComponentTransfer transfer) throws TransferException {
        DeviceId source = transfer.getSourceDeviceId();
        DeviceId currentDevice;

        try {
            _mutex.acquire();
            currentDevice = componentPlacement.get(transfer.getComponentId());
            _mutex.release();
        } catch (InterruptedException e) {
            throw new RuntimeException("panic: unexpected thread interruption");
        }

        // If source is null then it is an adding operation hence it is valid if the component does not exist.
        // If the currentDevice is not equal to the source device, that is also an illegal transfer.
        if (source != null && (!source.equals(currentDevice) ||
                !componentPlacement.containsKey(transfer.getComponentId()))) {
            throw new ComponentDoesNotExist(transfer.getComponentId(), source);
        }
    }

    private void componentAlreadyAtDestination(ComponentTransfer transfer) throws TransferException {
        DeviceId source = transfer.getSourceDeviceId();
        DeviceId destination = transfer.getDestinationDeviceId();

        if (source != null && source.equals(destination)) {
            throw new ComponentDoesNotNeedTransfer(transfer.getComponentId(), source);
        }
    }
}