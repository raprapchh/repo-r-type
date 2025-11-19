Conclusion protocol benchmark :

UDP was chosen because it sends immediately, requires no connection, and fits real-time updates where outdated data must be overwritten instantly. A minimal benchmark confirmed this: UDP averaged ~5.9 µs per send and ~22.4 µs round-trip, with no connection cost. TCP averaged ~6.6 µs per send, ~22.2 µs round-trip, but added a ~262 µs connection time.
TCP was not retained because its connection step and ordered delivery introduce blocking that breaks real-time gameplay.
UDP is therefore the better fit for our project.

Results :

```
➜  benchmark_protocol git:(main) ✗ ./documentation/benchmark/benchmark_protocol/bench_protocol udp
UDP average send time: 5947.28 ns per packet
UDP average round-trip time: 22456.9 ns per packet
UDP payload size: 392 bytes
➜  benchmark_protocol git:(main) ✗ ./documentation/benchmark/benchmark_protocol/bench_protocol tcp
TCP average send time: 6617.79 ns per packet
TCP average round-trip time: 22273 ns per packet
TCP connect time: 262455 ns
TCP payload size: 392 bytes
```
