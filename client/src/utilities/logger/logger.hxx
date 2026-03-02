#pragma once

namespace logging {
    template<typename... Args>
    inline void print( const char* format, Args... args ) {
        auto now = std::chrono::system_clock::now( );
        std::time_t time = std::chrono::system_clock::to_time_t( now );

        tm local_tm;
        localtime_s( &local_tm, &time );

        printf( oxorany( "\x1b[38;2;233;179;225m[\x1b[38;2;226;181;228m%02d\x1b[38;2;219;183;231m/\x1b[38;2;212;185;234m%02d\x1b[38;2;205;187;237m/\x1b[38;2;198;189;240m%04d\x1b[38;2;191;191;243m \x1b[38;2;184;193;246m%02d\x1b[38;2;177;195;249m:\x1b[38;2;170;197;252m%02d\x1b[38;2;163;199;255m:\x1b[38;2;153;179;255m%02d]\x1b[0m " ),
            local_tm.tm_mon + 1,
            local_tm.tm_mday,
            local_tm.tm_year + 1900,
            local_tm.tm_hour,
            local_tm.tm_min,
            local_tm.tm_sec );
        printf( oxorany( "\x1b[37m" ) );
        printf( format, args... );
        printf( oxorany( "\x1b[0m\n" ) );
    }
}