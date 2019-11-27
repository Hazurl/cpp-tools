#pragma once

#include <chrono>
#include <thread>
#include <fstream>
#include <filesystem>

namespace prf {


struct Watch {

    inline Watch()
        : start_time(std::chrono::high_resolution_clock::now()) {}

    inline void restart() {
        start_time = std::chrono::high_resolution_clock::now();
    }

    template<typename Precision = std::chrono::microseconds>
    inline auto get_duration_from_start() {
        return std::chrono::duration_cast<Precision>(std::chrono::high_resolution_clock::now() - start_time);
    }

    std::chrono::time_point<std::chrono::high_resolution_clock> start_time;
};

template<typename Precision>
using duration_from_precision_t = decltype(std::declval<Watch>().get_duration_from_start<Precision>());

template<typename F>
struct Deferred {

    static_assert(std::is_invocable_v<F>, "Callback not callable");

    template<typename Arg, std::enable_if_t<std::is_constructible_v<F, Arg&&>>* = nullptr>
    inline Deferred(Arg&& arg) 
        : callback(std::forward<Arg>(arg)) {}

    inline ~Deferred() {
        callback();
    } 

    F callback;

};

template<typename F>
Deferred(F&&) -> Deferred<F>;

template<typename F, typename Precision = std::chrono::microseconds>
struct ScopedWatch : Watch {

    using duration_t = duration_from_precision_t<Precision>;

    static_assert(std::is_invocable_v<F, duration_t>, "Callback not callable with the associated duration");

    template<typename Arg, std::enable_if_t<std::is_constructible_v<F, Arg&&>>* = nullptr>
    inline ScopedWatch(Arg&& arg) 
        : callback(std::forward<Arg>(arg)) {}

    inline ~ScopedWatch() {
        callback(get_duration_from_start<Precision>());
    } 

    F callback;

};

template<typename F>
ScopedWatch(F&&) -> ScopedWatch<F>;







template<typename T>
using ticks_t = typename T::rep;

using ticks_ns_t = ticks_t<std::chrono::nanoseconds>;
using ticks_us_t = ticks_t<std::chrono::microseconds>;
using ticks_ms_t = ticks_t<std::chrono::milliseconds>;

std::size_t hash_thread_id(std::thread::id id = std::this_thread::get_id()) {
    return std::hash<std::thread::id>{}(id);
}




struct DurationEvent {
    const char* name;
    std::size_t group_id;

    ticks_ns_t start;
    ticks_ns_t end;
};

struct InstantEvent {
    const char* name;
    std::size_t group_id;

    ticks_ns_t time;
};




template<typename P = std::chrono::nanoseconds>
inline ticks_t<P> get_ticks() {
    return std::chrono::duration_cast<P>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}

inline auto starting_ticks = get_ticks();


inline InstantEvent make_instant_event(const char* name, std::size_t group_id = hash_thread_id()) {
    return {
        name, group_id, get_ticks() - starting_ticks
    };
}

inline DurationEvent make_duration_event(const char* name, std::size_t group_id = hash_thread_id()) {
    auto now = get_ticks() - starting_ticks;
    return {
        name, group_id, now, now
    };
}

inline DurationEvent end_duration_event(DurationEvent event) {
    event.end = get_ticks() - starting_ticks;
    return event;
}



inline std::ostream& operator<<(std::ostream& os, InstantEvent const& event) {
    return os << ",{\"name\":\"" << event.name << "\",\"ph\":\"i\", \"pid\":0,\"tid\":" << event.group_id << ",\"ts\":" << (event.time / 1000.) << ",\"s\":\"g\"}\n";
}

inline std::ostream& operator<<(std::ostream& os, DurationEvent const& event) {
    return os << ",{\"name\":\"" << event.name << "\",\"ph\":\"X\", \"pid\":0,\"tid\":" << event.group_id << ",\"ts\":" << (event.start / 1000.) << ",\"dur\":" << (event.end - event.start) / 1000. << "}\n";
}




struct EndProfile{};
struct BeginProfile {
    const char* unit = "ms";
};

inline std::ostream& operator<<(std::ostream& os, BeginProfile const& info) {
    return os << "{\"displayTimeUnit\":\"" << info.unit << "\",\"traceEvents\":[{}\n";
}

inline std::ostream& operator<<(std::ostream& os, EndProfile const&) {
    return os << "]}";
}





template<typename T>
inline auto start_profiling(T& collector, const char* name, std::size_t group_id = hash_thread_id()) {
    return Deferred([event = make_duration_event(name, group_id), &collector] () {
        collector.write(end_duration_event(event));
    });
} 

template<typename T>
inline void emit_instant_event(T& collector, const char* name, std::size_t group_id = hash_thread_id()) {
    collector.write(make_instant_event(name, group_id));
} 





struct FileCollector {

    inline FileCollector(std::filesystem::path const filename) : file(filename) {
        file << BeginProfile{};
    }

    inline ~FileCollector() {
        file << EndProfile{};
    }

    inline void write(InstantEvent const& instant_event) {
        file << instant_event;
    }

    inline void write(DurationEvent const& duration_event) {
        file << duration_event;
    }

    std::ofstream file;
};



#ifndef PRF_NO_GLOBAL_COLLECTOR
    #ifndef PRF_OUTPUT_FILE
        #define PRF_OUTPUT_FILE "profiler_output_file.json"
    #endif

inline prf::FileCollector collector(PRF_OUTPUT_FILE);

#endif

#define PROFILE_SCOPE_TO(name, collector) auto _profiler ## _ ## __LINE__ = ::prf::start_profiling(collector, name)
#define PROFILE_SCOPE(name) PROFILE_SCOPE_TO(name, ::prf::collector)

#define PROFILE_FUNCTION_TO(collector) PROFILE_SCOPE_TO(__PRETTY_FUNCTION__, collector)
#define PROFILE_FUNCTION() PROFILE_FUNCTION_TO(::prf::collector)

#define EMIT_EVENT_TO(msg, collector) ::prf::emit_instant_event(collector, msg)
#define EMIT_EVENT(msg) EMIT_EVENT_TO(msg, ::prf::collector)





}