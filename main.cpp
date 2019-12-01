#include <thread>
#include <unordered_map>
#include <vector>

//#define PRF_OUTPUT_FILE "real_output_2.json"
// Name of the file to output

//#define PRF_NO_GLOBAL_COLLECTOR
// The library won't create a global collector
// which means
//      PROFILE_SCOPE
//      PROFILE_FUNCTION
//      EMIT_EVENT
// won't work, as they use the global collector
// You must then use their equivalent
//      PROFILE_SCOPE_TO
//      PROFILE_FUNCTION_TO
//      EMIT_EVENT_TO

// Remember to define those before including the profiler header

#include <cpp-tools/dbg.hpp>
#include <cpp-tools/profiler.hpp>

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
    }
}