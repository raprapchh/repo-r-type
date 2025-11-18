Conclusion storage benchmark :

JSON was chosen because it is more reliable, easier to maintain, and better supported in modern C++. A small benchmark showed faster parsing (0.060 ms for JSON vs 0.185 ms for YAML). YAML was not retained due to indentation issues, more complex parsing, and lower performance. JSON is therefore a better fit for our project.

Results :

```
➜  bench_stockage git:(dev) ✗ ./bench_yaml
YAML parse time: 0.185474 ms
➜  bench_stockage git:(dev) ✗ ./bench_json
JSON parse time: 0.060645 ms
```
