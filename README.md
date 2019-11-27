# CPP-Tools

## Profiler

```cpp
#include <thread>

using namespace std::chrono_literals;

void foo() {
    PROFILE_FUNCTION();
    std::this_thread::sleep_for(1000ms);
}

void bar() {
    PROFILE_FUNCTION();
    std::this_thread::sleep_for(100ms);
    EMIT_EVENT("Processing something");
    std::this_thread::sleep_for(500ms);
}

int main() {
    {
        PROFILE_FUNCTION();


        {
            PROFILE_SCOPE("in_main");
            foo();
            EMIT_EVENT("Sending something");
            std::this_thread::sleep_for(500ms);
        }

        bar();
        bar();
        EMIT_EVENT("Receiving something");
        std::this_thread::sleep_for(500ms);
        bar();
    }
}
```

You can then open "profiler_output_file.json" in chrome://tracing.

## Print everything

```cpp
std::vector<int> v;
for(int i=0; i < 10; ++i) {
    v.push_back(i);
}

DBG("vector: ", v);

std::variant<int, std::vector<int>> v2(v);
DBG("variant: ", v2);

std::tuple<int, std::vector<int>, std::variant<int, std::vector<int>>> v3(0, v, v2);
DBG("tuple: ", v3);

std::unordered_map<int, std::vector<int>> m;
m.insert({0, v});
m.insert({42, {123, 456}});
DBG("unordered_map: ", m);
```