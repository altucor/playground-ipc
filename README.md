# playground-ipc

## Building

```
mkdir -p build && cd build
```

### Debug build
```
cmake ../src -DCMAKE_BUILD_TYPE=Debug && cmake --build .
```

### Release build
```
cmake ../src -DCMAKE_BUILD_TYPE=Release && cmake --build .
```

### Debug & Unit Tests build
```
cmake ../src -DCMAKE_BUILD_TYPE=Debug -DENABLE_TESTS=ON && cmake --build .
```

## Testing

```
cd build
ctest
```

### Debug & Sanitizers & Unit Tests build
```
cmake ../src -DCMAKE_BUILD_TYPE=Debug -DENABLE_TESTS=ON -DPRODUCER_WITH_SANITIZERS=ON -DCONSUMER_WITH_SANITIZERS=ON && cmake --build .
```


# Best runs

## Release build O3, pipe write kChunkSize 64 bytes, payload size 1000 bytes, session duration 5s, MacBook M1 Pro on charging cable
```
[Consumer] Started
[Client]  Opened pipe socket: 0x3
[Client]  Starting
[Session] Receiver thread started
[Session] Packet processor thread started
[Session] Performance monitor thread started
[Session] Monitor:: Packets per second: 53320
[Session] Monitor:: Bytes per second: 54649933
[Session] Monitor:: Packet receive total(valid): 53619
[Session] ------------------------------------
[Session] Monitor:: Packets per second: 53708
[Session] Monitor:: Bytes per second: 55050700
[Session] Monitor:: Packet receive total(valid): 107600
[Session] ------------------------------------
[Session] Monitor:: Packets per second: 55497
[Session] Monitor:: Bytes per second: 56881350
[Session] Monitor:: Packet receive total(valid): 163325
[Session] ------------------------------------
[Session] Monitor:: Packets per second: 54979
[Session] Monitor:: Bytes per second: 56355525
[Session] Monitor:: Packet receive total(valid): 218565
[Session] ------------------------------------
[Consumer] Finished
[Session] Packet processor thread finished
[Packet] Failed to read sequence index value
[Session] Warning got invalid packet, discarding
[Session] Receiver thread finished
[Session] Monitor:: Packets per second: 55234
[Session] Monitor:: Bytes per second: 56614850
[Session] Monitor:: Packet receive total(valid): 272965
[Session] ------------------------------------
[Session] Performance monitor thread finished
[Session] Absolute maximum packets per second: 55497
[Session] Absolute maximum bytes per second: 56881350
[Session] Total packet receive(valid): 272965
```

## Release build O3, pipe write kChunkSize 1024 bytes, payload size 1000 bytes, session duration 5s, MacBook M1 Pro on charging cable

```
[Consumer] Started
[Client]  Opened pipe socket: 0x3
[Client]  Starting
[Session] Packet processor thread started
[Session] Performance monitor thread started
[Session] Receiver thread started
[Session] Monitor:: Packets per second: 56581
[Session] Monitor:: Bytes per second: 57989383
[Session] Monitor:: Packet receive total(valid): 56908
[Session] ------------------------------------
[Session] Monitor:: Packets per second: 62156
[Session] Monitor:: Bytes per second: 63711958
[Session] Monitor:: Packet receive total(valid): 119394
[Session] ------------------------------------
[Session] Monitor:: Packets per second: 62505
[Session] Monitor:: Bytes per second: 64070692
[Session] Monitor:: Packet receive total(valid): 182201
[Session] ------------------------------------
[Session] Monitor:: Packets per second: 61582
[Session] Monitor:: Bytes per second: 63121550
[Session] Monitor:: Packet receive total(valid): 244045
[Session] ------------------------------------
[Session] Packet processor thread finished
[Packet] Failed to read payload of size: 1000
[Consumer] Finished
[Session] Warning got invalid packet, discarding
[Session] Receiver thread finished
[Session] Monitor:: Packets per second: 61582
[Session] Monitor:: Bytes per second: 63121550
[Session] Monitor:: Packet receive total(valid): 305143
[Session] ------------------------------------
[Session] Performance monitor thread finished
[Session] Absolute maximum packets per second: 62505
[Session] Absolute maximum bytes per second: 64070692
[Session] Total packet receive(valid): 305143
```
