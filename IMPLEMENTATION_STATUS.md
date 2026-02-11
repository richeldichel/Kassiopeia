# Kassiopeia Parallelization Implementation Status

## Current Status: SEQUENTIAL EXECUTION

**Important**: The current implementation uses a thread pool infrastructure but executes events **sequentially** (one at a time) due to thread safety constraints with shared components.

### Why Sequential?
Root components have internal mutable state that is not thread-safe. To prevent race conditions and double-free errors, the entire event execution is protected by a mutex, serializing execution.

### Recommendation
Use `number_of_threads="1"` (default). Setting higher values currently provides no performance benefit and adds small overhead.

## Path to True Parallelization
Requires refactoring root components to be stateless or implementing deep cloning. The thread pool infrastructure is ready once components are thread-safe.

See full documentation in PARALLELIZATION.md
