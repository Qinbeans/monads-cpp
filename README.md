# Monads for C++

This is a thought expirement to bring monads seen in Javascript and Golang to C++.

```go
value, ok := os.LookupEnv("MY_VARIABLE")
```

While C++ doesn't support env look up out of the box, we can still create similar code.

```c++
auto [value, ok] = []() {
  char* env = getenv("MY_VARIABLE");
  if (env == nullptr) return ["", false];
  return [env, true];
}();
```
