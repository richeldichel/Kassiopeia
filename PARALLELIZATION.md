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
3. **Field Solver Caches**: Electric and magnetic field solvers may have internal caches
   - **IMPORTANT**: Current implementation shares field objects across threads
   - Field calculations should be thread-safe (read-only after initialization)
   - However, if field solvers use caching, this could cause race conditions
   - For simulations using cached field solvers, single-threaded mode is recommended until thread-safe caching is implemented
4. **Cache Sharing**: File caches and other shared resources are accessed through mutex locks

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

1. **Thread-Safe Field Caching**: Add mutex protection to field solver caches
   - Clone field objects for each thread with independent caches
   - Implement lock-free cache data structures for better performance
   - Add thread-local caching for field calculations
2. **Thread-Local Random Number Generators**: Provide independent RNG for each thread
   - Better statistical independence between events
   - Improved reproducibility in parallel mode
3. **Lock-Free Queue**: Replace mutex-protected event queue with lock-free implementation
   - Better queue performance and scalability
4. **Parallel I/O**: Support separate output files per thread
   - Reduce I/O bottleneck
   - Merge files after simulation completes
5. **Work Stealing**: Implement work stealing for better load balancing
   - Handle variable event processing times more efficiently
6. **GPU Acceleration**: Offload trajectory calculations to GPU
   - Potential for massive parallelization of physics calculations

### Field Solver Thread Safety (Priority)

The current implementation shares electric and magnetic field objects across all threads. While this works for simple field configurations, it may cause issues with cached field solvers:

**Known Issues:**
- KEMField cached charge density solvers may have race conditions
- Field calculation caches are not mutex-protected
- Multiple threads accessing the same cache simultaneously can cause data corruption

**Recommended Actions:**
1. For cached field solvers: Use single-threaded mode (`number_of_threads="1"`)
2. For simple/analytic fields without caching: Parallel mode should be safe
3. Future work: Add mutex protection or thread-local caches to field solvers

**Implementation Plan:**
- Add `KSMutex` members to field solver classes that use caching
- Protect cache read/write operations with RAII lock guards
- Consider cloning field objects for each thread with independent caches
- Performance testing to measure mutex contention impact
