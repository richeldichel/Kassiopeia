# Parallelization in Kassiopeia

This document describes the parallel processing capabilities added to Kassiopeia.

## Overview

Kassiopeia now supports multi-threaded event processing, allowing multiple particle events to be simulated concurrently on multi-core systems. This can significantly improve performance for simulations with many independent events.

## Usage

To enable parallel processing, add the `number_of_threads` parameter to your simulation configuration:

```xml
<ks_simulation
    name="my_simulation"
    run="1"
    seed="12345"
    events="1000"
    number_of_threads="4"
    ...
/>
```

### Parameters

- **number_of_threads**: Integer value specifying the number of worker threads (default: 1)
  - Set to 1 for single-threaded execution (original behavior)
  - Set to the number of CPU cores for optimal performance
  - Values greater than the number of cores may not provide additional benefit

## Implementation Details

### Thread Safety

The parallelization is implemented at the event level:

1. **Thread Pool**: A pool of worker threads processes events from a shared queue
2. **Event Workers**: Each thread has its own event, track, and step objects to avoid conflicts
3. **Mutex Protection**: File I/O operations and run statistics updates are protected by mutexes
4. **Random Number Generation**: Particle generation is synchronized to maintain reproducibility

### Performance Considerations

- **Best for**: Simulations with many independent events (Monte Carlo simulations)
- **Scalability**: Linear speedup expected up to the number of CPU cores
- **Overhead**: Small overhead for thread management and synchronization
- **I/O Bottleneck**: File writing is serialized, which may limit speedup for I/O-intensive simulations

### Limitations

1. **Event Order**: Events may complete in a different order than single-threaded execution
2. **Random Number Generation**: The global random number generator (KRandom singleton) is shared across threads
   - Different runs with the same seed may produce slightly different results in parallel mode
   - This is because random numbers are consumed in a non-deterministic order due to thread scheduling
   - For exact reproducibility, use `number_of_threads="1"` (single-threaded mode)
   - Statistical distributions across large numbers of events should remain consistent
3. **Cache Sharing**: File caches and other shared resources are accessed through mutex locks

## Examples

### Single-threaded (default behavior)
```xml
<ks_simulation events="1000" />
```

### Multi-threaded with 8 threads
```xml
<ks_simulation events="1000" number_of_threads="8" />
```

## Troubleshooting

### Performance Not Scaling

If you don't see performance improvements:

1. Check that you have enough events to parallelize (at least 10x the thread count)
2. Verify your system has multiple CPU cores available
3. Consider I/O overhead - try reducing output frequency
4. Monitor CPU usage to ensure threads are actually running in parallel

### Different Results

If results differ from single-threaded execution:

1. This is expected for random number sequences due to parallel event processing
2. Statistical distributions should remain consistent
3. For exact reproducibility, use `number_of_threads="1"`

## Technical Details

### Architecture

- **KSRoot**: Main simulation loop modified to support parallel execution
- **EventWorker**: Structure containing per-thread simulation objects
- **ThreadWorkerFunction**: Worker thread function that processes events from the queue
- **Mutex Locks**: Three mutexes protect shared resources:
  - `fQueueMutex`: Protects event queue access
  - `fRunUpdateMutex`: Protects run statistics updates
  - `fWriterMutex`: Protects file I/O operations

### Code Changes

Key files modified:
- `Kassiopeia/Simulation/Include/KSRoot.h`
- `Kassiopeia/Simulation/Source/KSRoot.cxx`
- `Kassiopeia/Simulation/Include/KSSimulation.h`
- `Kassiopeia/Simulation/Source/KSSimulation.cxx`
- `Kassiopeia/Bindings/Simulation/Source/KSSimulationBuilder.cxx`

## Future Improvements

Potential enhancements for future versions:

1. Thread-local random number generators for better independence
2. Lock-free queue for better queue performance
3. Parallel I/O using separate output files per thread
4. Work stealing for better load balancing
5. GPU acceleration for trajectory calculations
