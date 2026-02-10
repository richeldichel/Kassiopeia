# Parallelization in Kassiopeia Example Simulations

This document explains how to use the parallelization feature in Kassiopeia example simulations.

## Overview

Kassiopeia now supports multi-threaded event processing. All example XML files have been updated to include the `number_of_threads` parameter, which allows you to control the level of parallelization.

## Usage

### Basic Usage

By default, simulations run with a single thread (sequential execution). To enable parallelization, set the `threads` parameter when running a simulation:

```bash
# Run with 4 threads
Kassiopeia DipoleTrapSimulation.xml --override threads=4

# Run with 8 threads
Kassiopeia QuadrupoleTrapSimulation.xml --override threads=8

# Run with default (1 thread, sequential)
Kassiopeia AnalyticSimulation.xml
```

### XML Configuration

The `number_of_threads` parameter is configured in each simulation file as:

```xml
<ks_simulation
    ...
    events="[events:1]"
    number_of_threads="[threads:1]"
    ...
/>
```

The `[threads:1]` syntax means:
- Variable name: `threads`
- Default value: `1` (single-threaded)
- Can be overridden via command line with `--override threads=<N>`

### Recommended Thread Counts

- **Single-threaded (1 thread)**: Default, deterministic results
- **Multi-core systems**: Set to your CPU core count (e.g., 4, 8, 16)
- **Maximum**: Typically no benefit beyond your physical core count

Example for a 4-core system:
```bash
Kassiopeia DipoleTrapSimulation.xml --override events=1000 --override threads=4
```

## Performance Considerations

### When Parallelization Helps

Parallelization is most beneficial for:
- **Many independent events**: Monte Carlo simulations with hundreds or thousands of events
- **CPU-bound simulations**: Complex trajectory calculations, many particle interactions
- **Simple field configurations**: Analytic fields without caching

### When to Use Single-Threaded Mode

Use `threads=1` (default) when:
- **Exact reproducibility needed**: Results must be identical between runs
- **Cached field solvers**: Using KEMField with cached charge density solvers
- **Small event counts**: Overhead exceeds benefits for < 100 events
- **I/O-heavy simulations**: Frequent output writing limits speedup

### Expected Performance

With optimal conditions:
- 4 threads: ~3.5-4x speedup
- 8 threads: ~7-7.5x speedup
- Overhead: ~5% from context switching and synchronization

## Examples

### DipoleTrapSimulation with Parallelization

```bash
# Run 1000 events with 4 threads
Kassiopeia DipoleTrapSimulation.xml --override events=1000 --override threads=4
```

### QuadrupoleTrapSimulation with Maximum Parallelization

```bash
# Run 5000 events with 8 threads
Kassiopeia QuadrupoleTrapSimulation.xml --override events=5000 --override threads=8
```

### AnalyticSimulation (Best for Parallelization)

```bash
# Analytic fields are thread-safe, good for parallel execution
Kassiopeia AnalyticSimulation.xml --override events=10000 --override threads=8
```

## Limitations and Warnings

### Field Caching

Electric and magnetic field objects are shared across threads. Field solvers with mutable caches (like KEMField cached solvers) may have race conditions.

**Recommendation**: Use simple analytic fields for parallel execution, or use `threads=1` with cached solvers.

### Random Number Generation

The global random number generator is shared across threads, which can lead to non-deterministic ordering of random numbers.

**Impact**: Different runs with the same seed may produce slightly different results in parallel mode.

**Recommendation**: For exact reproducibility, use `threads=1`.

### Event Ordering

Events complete in non-deterministic order in parallel mode (though event IDs remain sequential).

**Recommendation**: Use event IDs for analysis, not file order.

## Troubleshooting

### Simulation Slower with Multiple Threads

Possible causes:
- Too many threads for available cores (use physical core count)
- I/O bottleneck (reduce output frequency)
- Small event count (overhead exceeds benefits)

### Different Results with Same Seed

This is expected in parallel mode due to shared RNG. Use `threads=1` for deterministic results.

### Crashes or Errors

If you encounter crashes with parallel execution:
1. Try with `threads=1` to verify the simulation works
2. Check if you're using cached field solvers (may not be thread-safe)
3. Report the issue with details about your configuration

## More Information

For detailed technical documentation, see:
- `PARALLELIZATION.md` in the root directory
- `IMPLEMENTATION_STATUS.md` for technical details
- Documentation/gh-pages/source/kassiopeia_simulation.rst

## Testing

Unit tests for parallelization are located in:
- `UnitTest/Kassiopeia/Source/Parallelization.cxx`

Run tests with:
```bash
UnitTestKassiopeia --gtest_filter=ParallelizationTest.*
```
