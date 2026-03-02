namespace paging {
    class cr3_guard_t {
    public:
        explicit cr3_guard_t( std::uint64_t new_cr3 )
            : m_previous_cr3( m_target_cr3 ) {
            if ( new_cr3 != 0 ) {
                attach_to_process( new_cr3 );
            }
        }

        ~cr3_guard_t( ) {
            attach_to_process( m_previous_cr3 );
        }

        cr3_guard_t( const cr3_guard_t& ) = delete;
        cr3_guard_t& operator=( const cr3_guard_t& ) = delete;

        cr3_guard_t( cr3_guard_t&& ) = delete;
        cr3_guard_t& operator=( cr3_guard_t&& ) = delete;

        std::uint64_t previous( ) const {
            return m_previous_cr3;
        }

        std::uint64_t current( ) const {
            return m_target_cr3;
        }

    private:
        std::uint64_t m_previous_cr3;
    };

    cr3_guard_t scoped_attach( std::uint64_t new_cr3 ) {
        return cr3_guard_t( new_cr3 );
    }
}