#pragma once
auto g_uwd_builder = std::make_unique<uwd_engine::uwd_builder>( );

namespace spoof_call {
    template<typename T>
    concept VoidOr8Bytes = std::is_same_v<T, void> || sizeof( T ) <= 8;

    template<class F>
    class c_spoof_call {
    public:
        c_spoof_call( const std::uint64_t function_address, uwd_engine::uwd_builder* uwd_builder ) :
            m_config( uwd_builder->generate( function_address ) ),
            m_uwd_builder( uwd_builder ) {
        }

        template<class... Args>
        std::invoke_result_t<F, Args...> Call( Args &&...args ) requires VoidOr8Bytes<std::invoke_result_t<F, Args...>> {
            return m_uwd_builder->template call<std::invoke_result_t<F, Args...>>( &m_config, args... );
        }

        template<class... Args>
        std::invoke_result_t<F, Args...> Call( Args &&...args ) requires ( sizeof( std::invoke_result_t<F, Args...> ) > 8 ) {
            std::invoke_result_t<F, Args...> data;
            m_uwd_builder->template call<void>( &m_config, &data, args... );
            return data;
        }

    private:
        uwd_private::unwind_cfg m_config;
        uwd_engine::uwd_builder* m_uwd_builder;
    };
}

#define spoof(func_expr, ...) \
  ([&]() -> decltype(func_expr(__VA_ARGS__)) { \
    using type = decltype(func_expr); \
    static spoof_call::c_spoof_call<type> unwindable_call(reinterpret_cast<std::uint64_t>(func_expr), g_uwd_builder.get()); \
    return unwindable_call.Call(__VA_ARGS__); \
  })()