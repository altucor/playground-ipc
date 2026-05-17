# playground-ipc

## Building

```
mkdir -p build && cd build
```

### Debug build
```
cmake ../src -DCMAKE_BUILD_TYPE=Debug && cmake --build .
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