/**
 * @file net.telnet-options.cppm
 * @version 0.5.7
 * @release_date October 30, 2025
 *
 * @brief Interface for Telnet option handling.
 * @remark Defines `option` class, `option::id_num` enumeration, and associated predicates/handlers.
 *
 * @copyright (c) 2025 [it's mine!]. All rights reserved.
 * @license See LICENSE file for details
 *
 * @remark This module is fully inline.
 * @see RFC 855 for Telnet option negotiation, `:protocol_fsm` for `option` usage, `:stream` for negotiation operations, `:types` for `telnet::command`
 */

module; //Including Asio in the Global Module Fragment until importable header units are reliable.
#include <asio.hpp>

//Module partition interface unit
export module net.telnet:options;

import std; //NOLINT For std::string, std::vector, std::function, std::optional, std::size_t

export import :types;  ///< @see "net.telnet-types.cppm" for `byte_t`
export import :errors; ///< @see "net.telnet-errors.cppm" for `error` enum

//namespace asio = boost::asio;

export namespace net::telnet {
    /**
     * @brief Class to encapsulate Telnet option data and negotiation logic.
     *
     * @note Option support is defined by an `option` instance in `ProtocolFSM`; subnegotiation support is indicated by a non-null subnegotiation handler.
     * @remark Comparisons (==, !=, <, >, <=, >=) are based on `id_num`; implicit conversion to `id_num` allows mixed comparisons.
     * @see RFC 855 for Telnet option negotiation, `:protocol_fsm` for `option` usage in the protocol state machine, `:stream` for negotiation operations, `:types` for `telnet::command`.
     *
     * @todo Future Development: Use C++26 reflection to populate option names automatically.
     */
    class option {
    public:
        /**
         * @details Nested enumeration of Telnet option IDs as defined in the IANA Telnet Option Registry and MUD-specific extensions.
         * @note All 256 possible `byte_t` values are valid for proprietary extensions.
         */
        enum class id_num : byte_t;

        /**
         * @typedef enable_predicate_type
         * @brief Function type for predicates determining local or remote option support.
         *
         * @param id The `option::id_num` to evaluate.
         * @return True if the option is supported, false otherwise.
         */
        using enable_predicate_type = std::function<bool(id_num /*id*/)>;

        ///@brief Constructs an `option` with the given ID and optional parameters.
        //NOLINTNEXTLINE(google-explicit-constructor)
        option(
            id_num id,
            std::string name                  = "",
            enable_predicate_type local_pred  = always_reject,
            enable_predicate_type remote_pred = always_reject,
            bool subneg_supported             = false,
            size_t max_subneg_size            = max_subnegotiation_buffer_size
        )
            : id_(id),
              name_(std::move(name)),
              local_predicate_(std::move(local_pred)),
              remote_predicate_(std::move(remote_pred)),
              supports_subnegotiation_(subneg_supported),
              max_subneg_size_(max_subneg_size)
        {}

        ///@brief Factory to create common `option` instances.
        static option make_option(
            id_num id,
            std::string name,
            bool local_supported   = false,
            bool remote_supported  = false,
            bool subneg_supported  = false,
            size_t max_subneg_size = max_subnegotiation_buffer_size
        )
        {
            enable_predicate_type local_pred  = local_supported ? always_accept : always_reject;
            enable_predicate_type remote_pred = remote_supported ? always_accept : always_reject;
            return option(
                id, std::move(name), std::move(local_pred), std::move(remote_pred), subneg_supported, max_subneg_size
            );
        }

        ///@brief Three-way comparison operator for ordering and equality.
        [[nodiscard]] constexpr auto operator<=>(const option& other) const noexcept { return id_ <=> other.id_; }

        ///@brief Three-way comparison operator for ordering and equality.
        [[nodiscard]] constexpr auto operator<=>(option::id_num other_id) const noexcept { return id_ <=> other_id; }

        ///@brief Implicitly converts to `option::id_num`.
        [[nodiscard]] operator id_num() const noexcept { return id_; } //NOLINT(google-explicit-constructor)

        ///@brief Gets the Telnet `option::id_num`.
        [[nodiscard]] id_num get_id() const noexcept { return id_; }

        ///@brief Gets the `option` name.
        [[nodiscard]] const std::string& get_name() const noexcept { return name_; }

        ///@brief Evaluates the local predicate to determine if the `option` can be enabled locally.
        [[nodiscard]] bool supports_local() const { return local_predicate_(id_); }

        ///@brief Evaluates the remote predicate to determine if the `option` can be enabled remotely.
        [[nodiscard]] bool supports_remote() const { return remote_predicate_(id_); }

        ///@brief Evaluates the predicate for the designated direction to determine if the `option` can be enabled in that direction.
        [[nodiscard]] bool supports(negotiation_direction direction) const
        {
            return (direction == negotiation_direction::remote) ? supports_remote() : supports_local();
        }

        ///@brief Gets the maximum subnegotiation buffer size.
        [[nodiscard]] size_t max_subnegotiation_size() const noexcept { return max_subneg_size_; }

        ///@brief Checks if the `option` supports subnegotiation.
        [[nodiscard]] bool supports_subnegotiation() const noexcept { return supports_subnegotiation_; }

        ///@brief Predicate that always accepts the `option`.
        [[nodiscard]] static bool always_accept(id_num /*unused*/) noexcept { return true; }

        ///@brief Predicate that always rejects the `option`.
        [[nodiscard]] static bool always_reject(id_num /*unused*/) noexcept { return false; }

    private:
        static constexpr size_t max_subnegotiation_buffer_size = 1024;

        id_num id_;
        std::string name_;

        enable_predicate_type local_predicate_;
        enable_predicate_type remote_predicate_;

        bool supports_subnegotiation_;

        size_t max_subneg_size_;
    }; //class option

    /**
     * @fn explicit option::option(id_num id, std::string name, enable_predicate_type local_pred, enable_predicate_type remote_pred, bool subneg_supported, size_t max_subneg_size)
     *
     * @param id The Telnet `option::id_num`.
     * @param name The option name (default empty; populated in C++26? with reflection).
     * @param local_pred Predicate for local support (default `always_reject`).
     * @param remote_pred Predicate for remote support (default `always_reject`).
     * @param subneg_supported Does the `option` support subnegotiation handler registration (default `false`).
     * @param max_subneg_size Maximum subnegotiation buffer size (<=0 for unlimited; default `max_subnegotiation_size`).
     */
    /**
     * @fn static option option::make_option(id_num id, std::string name, bool local_supported, bool remote_supported, bool subneg_supported, size_t max_subneg_size)
     *
     * @param id The Telnet `option::id_num` byte.
     * @param name The option name (required for debuggability).
     * @param local_supported Whether the `option` is supported locally (default false).
     * @param remote_supported Whether the `option` is supported remotely (default false).
     * @param subneg_supported Whether the `option` supports subnegotiation handlers (default false).
     * @param max_subneg_size Maximum subnegotiation buffer size (default `max_subnegotiation_size`).
     * @return An `option` instance configured for the given `option::id_num`.
     */
    /**
     * @fn constexpr auto option::operator<=>(const option& other) const noexcept
     *
     * @param other The other `option` to compare with.
     * @return The result of comparing `id_` values.
     */
    /**
     * @fn constexpr auto option::operator<=>(option::id_num other_id) const noexcept
     *
     * @param other_id The other `option::id_num` to compare with.
     * @return The result of comparing `id_num` values.
     */
    /**
     * @fn option::operator id_num() const noexcept
     *
     * @return The `option`’s ID.
     *
     * @remark Enables implicit conversion to `option::id_num` for use in option identification.
     */
    /**
     * @fn id_num option::get_id() const noexcept
     *
     * @return The `option`'s ID.
     */
    /**
     * @fn const std::string& option::get_name() const noexcept
     *
     * @return The user-provided name or empty string.
     */
    /**
     * @fn bool option::supports_local() const noexcept
     *
     * @return True if the `option` can be enabled locally, false otherwise.
     */
    /**
     * @fn bool option::supports_remote() const noexcept
     *
     * @return True if the `option` can be enabled remotely, false otherwise.
     */
    /**
     * @fn bool option::supports(negotiation_direction direction) const noexcept
     *
     * @param direction The direction in question for support.
     * @return True if the `option` can be enabled in the designated direction, false otherwise.
     */
    /**
     * @fn size_t option::max_subnegotiation_size() const noexcept
     *
     * @return The maximum subnegotiation buffer size (<=0 for unlimited).
     */
    /**
     * @fn bool option::supports_subnegotiation() const noexcept
     *
     * @return True if subnegotiation handlers are allowed, false otherwise.
     */
    /**
     * @fn static bool option::always_accept(id_num) noexcept
     *
     * @param id The `option::id_num` to evaluate (unused).
     * @return True.
     */
    /**
     * @fn static bool option::always_reject(id_num) noexcept
     *
     * @param id The `option::id_num` to evaluate (unused).
     * @return False.
     */

    /**
     * @brief Enumeration of Telnet option ID bytes.
     *
     * @see IANA Telnet Option Registry, RFC 855 for Telnet option negotiation, `:protocol_fsm` for `option` processing, `:stream` for negotiation operations.
     *
     * @remark The formatting of this protocol registry enumeration is intentionally fixed to preserve column alignment of TelOpt names, TelOpt numbers, and description/reference comments. clang-format is disabled within the braces of the enumeration.
     */
    enum class option::id_num : byte_t {
        // clang-format off: preserve column alignment
        /* ======================================================================================================================================== *
         *         TelOpt Name          TelOpt Number            Description/Reference                                                              *
         * ======================================================================================================================================== */
        binary                             = 0x00, ///< Binary Transmission (@see RFC 856)
        echo                               = 0x01, ///< Echo (@see RFC 857)
        reconnection                       = 0x02, ///< Reconnection (NIC 15391 of 1973)
        suppress_go_ahead                  = 0x03, ///< Suppress Go Ahead (@see RFC 858)
        approx_message_size_negotiation    = 0x04, ///< Approx Message Size Negotiation (NIC 15393 of 1973)
        status                             = 0x05, ///< Status (@see RFC 859)
        timing_mark                        = 0x06, ///< Timing Mark (@see RFC 860)
        remote_controlled_trans_and_echo   = 0x07, ///< Remote Controlled Trans and Echo (@see RFC 726)
        output_line_width                  = 0x08, ///< Output Line Width (NIC 20196 of August 1978)
        output_page_size                   = 0x09, ///< Output Page Size (NIC 20197 of August 1978)
        output_carriage_return_disposition = 0x0A, ///< Output Carriage-Return Disposition (@see RFC 652)
        output_horizontal_tab_stops        = 0x0B, ///< Output Horizontal Tab Stops (@see RFC 653)
        output_horizontal_tab_disposition  = 0x0C, ///< Output Horizontal Tab Disposition (@see RFC 654)
        output_formfeed_disposition        = 0x0D, ///< Output Formfeed Disposition (@see RFC 655)
        output_vertical_tabstops           = 0x0E, ///< Output Vertical Tabstops (@see RFC 656)
        output_vertical_tab_disposition    = 0x0F, ///< Output Vertical Tab Disposition (@see RFC 657)
        output_linefeed_disposition        = 0x10, ///< Output Linefeed Disposition (@see RFC 658)
        extended_ascii                     = 0x11, ///< Extended ASCII (@see RFC 698)
        logout                             = 0x12, ///< Logout (@see RFC 727)
        byte_macro                         = 0x13, ///< Byte Macro (@see RFC 735)
        data_entry_terminal                = 0x14, ///< Data Entry Terminal (@see RFC 1043, RFC 732)
        supdup                             = 0x15, ///< SUPDUP (@see RFC 736, RFC 734)
        supdup_output                      = 0x16, ///< SUPDUP Output (@see RFC 749)
        send_location                      = 0x17, ///< Send Location (@see RFC 779)
        terminal_type                      = 0x18, ///< Terminal Type (@see RFC 1091) (Extended by "MTTS" MUD Terminal Type Standard)
        end_of_record                      = 0x19, ///< End of Record (@see RFC 885)
        tacacs_user_identification         = 0x1A, ///< TACACS User Identification (@see RFC 927)
        output_marking                     = 0x1B, ///< Output Marking (@see RFC 933)
        terminal_location_number           = 0x1C, ///< Terminal Location Number (@see RFC 946)
        telnet_3270_regime                 = 0x1D, ///< Telnet 3270 Regime (@see RFC 1041)
        x_3_pad                            = 0x1E, ///< X.3 PAD (@see RFC 1053)
        negotiate_about_window_size        = 0x1F, ///< Negotiate About Window Size (@see RFC 1073)
        terminal_speed                     = 0x20, ///< Terminal Speed (@see RFC 1079)
        remote_flow_control                = 0x21, ///< Remote Flow Control (@see RFC 1372)
        linemode                           = 0x22, ///< Linemode (@see RFC 1184)
        x_display_location                 = 0x23, ///< X Display Location (@see RFC 1096)
        environment_option                 = 0x24, ///< Environment Option (@see RFC 1408)
        authentication                     = 0x25, ///< Authentication Option (@see RFC 2941)
        encrypt_deprecated                 = 0x26, ///< Encryption Option (@see RFC 2946) (Deprecated in favor of TLS)
        new_environment_option             = 0x27, ///< New Environment Option (@see RFC 1572) (Extended by "MNES" MUD New Environment Standard)
        tn3270e                            = 0x28, ///< TN3270E (@see RFC 2355)
        xauth                              = 0x29, ///< XAUTH (Rob Earhart)
        charset                            = 0x2A, ///< CHARSET (@see RFC 2066)
        telnet_remote_serial_port          = 0x2B, ///< Telnet Remote Serial Port (Robert Barnes)
        com_port_control_option            = 0x2C, ///< Com Port Control Option (@see RFC 2217)
        telnet_suppress_local_echo         = 0x2D, ///< Telnet Suppress Local Echo (Wirt Atmar)
        telnet_start_tls                   = 0x2E, ///< Telnet Start TLS (Michael Boe)
        kermit                             = 0x2F, ///< KERMIT (@see RFC 2840)
        send_url                           = 0x30, ///< SEND-URL (David Croft)
        forward_x                          = 0x31, ///< FORWARD_X (Jeffrey Altman)
        /* Range 0x32-0x44 Unused per IANA */
        msdp                               = 0x45, ///< MUD Server Data Protocol
        mssp                               = 0x46, ///< MUD Server Status Protocol
        /* Range 0x47-0x4E Unused per IANA */
        gssapi                             = 0x4F, ///< Generic Security Service API (@see RFC 2942)
        /* Range 0x50-0x54 Unused per IANA */
        mccp1                              = 0x55, ///< MUD Client Compression Protocol v.1
        mccp2                              = 0x56, ///< MUD Client Compression Protocol v.2
        mccp3                              = 0x57, ///< MUD Client Compression Protocol v.3
        /* Range 0x58-0x59 Unused per IANA */
        msp                                = 0x5A, ///< MUD Sound Protocol
        mxp                                = 0x5B, ///< MUD eXtension Protocol
        /* Option 0x5C Unused per IANA */
        zmp                                = 0x5D, ///< Zenith MUD Protocol
        pueblo                             = 0x5E, ///< Pueblo Protocol (1998)
        /* Range 0x5F-0x65 Unused per IANA */
        aardwolf_102                       = 0x66, ///< Aardwolf MUD Channel 102 Protocol
        /* Range 0x67-0x89 Unused per IANA */
        telopt_pragma_logon                = 0x8A, ///< TELOPT PRAGMA LOGON (Steve McGregory)
        telopt_sspi_logon                  = 0x8B, ///< TELOPT SSPI LOGON (Steve McGregory)
        telopt_pragma_heartbeat            = 0x8C, ///< TELOPT PRAGMA HEARTBEAT (Steve McGregory)
        /* Range 0x8D-0x90 Unused per IANA */
        ssl_deprecated                     = 0x91, ///< SSL Legacy Implementation (Deprecated in favor of TLS)
        /* Range 0x92-0x9F Unused per IANA */
        mcp                                = 0xA0, ///< MUD Client Protocol (2002)
        /* Range 0xA1-0xC7 Unused per IANA */
        atcp                               = 0xC8, ///< Achaea Telnet Communication Protocol
        gmcp                               = 0xC9, ///< Generic MUD Communication Protocol (aka ATCP2)
        /* Range 0xCA-0xFE Unused per IANA */        
        extended_options_list              = 0xFF  ///< Extended-Options-List (@see RFC 861)
        // clang-format on
    }; //enum class option::id_num

    /**
     * @brief Thread-safe registry for managing `option` instances in the protocol state machine.
     * @remark Used by `ProtocolFSM` to store and query supported Telnet options.
     * @remark The `std::initializer_list` constructor enforces sorted input by `option::id_num` at compile time using `static_assert`, ensuring O(n) `std::set` construction. Unsorted inputs cause compilation failure. All accessor methods are atomic via `std::shared_mutex`, supporting concurrent reads and exclusive writes.
     * @warning Methods are guaranteed atomic by `std::shared_mutex`, but chaining operations is NOT thread-safe; however `get` followed by `upsert` provides snapshot consistency.
     * @see RFC 855 for Telnet option negotiation, `:protocol_fsm` for `option` usage in the protocol state machine, `:stream` for negotiation operations, `:types` for `telnet::command`, `option` for option details.
     */
    class option_registry {
    public:
        ///@brief Constructs a registry from a sorted initializer list of `option` instances.
        option_registry(std::initializer_list<option> init)
        {
            ///@todo Figure out if this can be done as a constant expression.
            //static_assert(
            // std::is_sorted(init.begin(), init.end(), std::less<>{}),
            // "Initializer list must be sorted by option::id_num"
            //);
            registry_ = std::set<option, std::less<>>(init.begin(), init.end());
        } //option_registry(std::initializer_list<option>)

        ///@brief Constructs a registry from a pre-constructed `std::set` of `option` instances.
        explicit option_registry(std::set<option, std::less<>>&& init) : registry_(std::move(init)) {}

        ///@brief Retrieves an `option` by its ID.
        std::optional<option> get(option::id_num opt_id) const noexcept
        {
            std::shared_lock<std::shared_mutex> lock(mutex_);
            auto iter = registry_.find(opt_id);
            if (iter != registry_.end()) {
                return *iter;
            } else {
                return std::nullopt;
            }
        } //get(option::id_num)

        ///@brief Checks if an `option` is present in the registry.
        [[nodiscard]] bool has(option::id_num opt_id) const noexcept
        {
            std::shared_lock<std::shared_mutex> lock(mutex_);
            return (registry_.find(opt_id) != registry_.end());
        } //has(option::id_num)

        ///@brief Inserts or updates an `option` in the registry.
        const option& upsert(const option& opt)
        {
            std::lock_guard<std::shared_mutex> lock(mutex_);
            auto [add_result, success] = registry_.insert(opt);
            if (success) {
                return *add_result;
            } else {
                //Use iterator from erase as hint to insert new option at same position, optimizing insertion to O(1)
                auto replace_result = registry_.insert(registry_.erase(add_result), opt);
                return *replace_result;
            }
        } //upsert(const option&)

        ///@brief Inserts or updates an `option` with error handling.
        void upsert(const option& opt, std::error_code& ec) noexcept
        {
            try {
                upsert(opt); // the mutex is locked inside this call
            } catch (const std::system_error& e) {
                ec = e.code();
            } catch (std::bad_alloc& e) {
                ec = make_error_code(std::errc::not_enough_memory);
            } catch (...) {
                ec = make_error_code(telnet::error::internal_error);
            }
        } //upsert(const option&, std::error_code&)

        ///@brief Inserts or updates an `option` constructed from arguments.
        template<typename... Args>
        const option& upsert(option::id_num opt_id, Args&&... args)
        {
            return upsert(option{opt_id, std::forward<Args>(args)...});
        } //upsert(option::id_num, Args...)

    private:
        std::set<option, std::less<>> registry_;
        mutable std::shared_mutex mutex_;
    }; //class option_registry

    /**
     * @fn option_registry::option_registry(std::initializer_list<option> init)
     *
     * @param init Initializer list of `option` instances, must be sorted by `option::id_num`.
     *
     * @pre `init` Initializer list MUST be sorted by `option::id_num` in ascending order (enforced at compile time by `static_assert`).
     * @remark Ensures O(n) construction of the internal `std::set` by requiring sorted input.
     * @remark Unsorted inputs cause compilation failure, guaranteeing performance for compile-time configurations.
     */
    /**
     * @fn option_registry::option_registry(std::set<option, std::less<>>&& init)
     *
     * @param init A `std::set` of `option` instances, moved into the registry.
     *
     * @remark Allows advanced use cases where options are pre-sorted or dynamically generated before registry creation.
     */
    /**
     * @fn std::optional<option> option_registry::get(option::id_num opt_id) const noexcept
     *
     * @param opt_id The `option::id_num` to query.
     * @return `std::optional` containing the `option` if found, or `std::nullopt` if not.
     *
     * @remark Thread-safe via `std::shared_mutex` (shared lock).
     * @remark Performs O(log n) lookup.
     */
    /**
     * @fn bool option_registry::has(option::id_num opt_id) const noexcept
     *
     * @param opt_id The `option::id_num` to check.
     * @return True if the `option` exists, false otherwise.
     *
     * @remark Thread-safe via `std::shared_mutex` (shared lock).
     * @remark Performs O(log n) lookup.
     */
    /**
     * @fn const option& option_registry::upsert(const option& opt)
     *
     * @param opt The `option` to insert or update.
     * @return Reference to the inserted or updated `option` in the registry.
     *
     * @remark Thread-safe via `std::shared_mutex` (exclusive lock).
     * @remark Performs O(log n) insertion or replacement, using `erase` result iterator as a hint to optimize `insert` performance during replacement.
     */
    /**
     * @overload void option_registry::upsert(const option& opt, std::error_code& ec) noexcept
     * @copydoc const option& option_registry::upsert(const option& opt)
     *
     * @param opt The `option` to insert or update.
     * @param[out] ec Error code set on failure (e.g., `std::errc::not_enough_memory`).
     *
     * @note Catches exceptions and sets appropriate error codes for robust runtime use.
     */
    /**
     * @overload const option& option_registry::upsert(option::id_num opt_id, Args&&... args)
     * @copydoc const option& option_registry::upsert(const option& opt)
     *
     * @tparam Args Types for `args` forwarded to `option` constructor.
     * @param opt_id The `option::id_num` for the `option`.
     * @param args Arguments to construct an `option` (forwarded to `option` constructor).
     * @return Reference to the inserted or updated `option`.
     *
     * @remark Simplifies runtime `option` creation by forwarding arguments to the `option` constructor.
     */
} //namespace net::telnet

export namespace std {
    /**
     * @brief Formatter specialization for `::net::telnet::option` to support `std::format`.
     * @remark Formats `option` values for use in `ProtocolConfig::log_error` during option negotiation and subnegotiation.
     * @see `:protocol_fsm` for logging usage.
     */
    template<>
    struct formatter<::net::telnet::option, char> {
    private:
        char presentation_ = 'd'; ///< Format specifier: 'd' for 0xXX (name), 'n' for name only, 'x' for hex only.
    public:
        /**
         * @brief Parses the format specifier for `option`.
         * @param ctx The format parse context.
         * @return Iterator pointing to the end of the parsed format specifier.
         * @throws std::format_error if the specifier is invalid (not 'd', 'n', or 'x').
         * @remark Supports 'd' (default: 0xXX (name)), 'n' (name only), and 'x' (hex only, 0xXX).
         */
        //NOLINTNEXTLINE(readability-convert-member-functions-to-static): The std::formatter interface doesn't allow this to be static.
        constexpr auto parse(format_parse_context& ctx)
        {
            auto view = std::ranges::subrange(ctx.begin(), ctx.end());
            if (!view.empty()) {
                const char c = view.front(); //NOLINT(readability-identifier-length): Idiomatic
                if (c == 'n' || c == 'x' || c == 'd') {
                    presentation_ = c;
                    view          = view.advance(1);
                }
            }
            if (!view.empty() && view.front() != '}') {
                throw std::format_error("Invalid format specifier for telnet::command");
            }
            return view.begin();
        } //parse(format_parse_context&)

        /**
         * @brief Formats a `::net::telnet::option` value.
         * @param opt The `option` to format.
         * @param ctx The format context.
         * @return Output iterator after formatting.
         * @details Formats as:
         * - 'd': "0xXX (name)" (e.g., "0x00 (Binary Transmission)").
         * - 'n': "name" (e.g., "Binary Transmission" or "unknown" if empty).
         * - 'x': "0xXX" (e.g., "0x00").
         */
        //NOLINTNEXTLINE(readability-convert-member-functions-to-static): The std::formatter interface doesn't allow this to be static.
        template<typename FormatContext>
        auto format(const ::net::telnet::option& opt, FormatContext& ctx) const
        {
            if (presentation_ == 'n') {
                return std::format_to(ctx.out(), "{}", opt.get_name().empty() ? "unknown" : opt.get_name());
            } else if (presentation_ == 'x') {
                return std::format_to(ctx.out(), "0x{:02x}", std::to_underlying(opt.get_id()));
            } else { // 'd' (default: 0xXX (name))
                return std::format_to(
                    ctx.out(),
                    "0x{:02x} ({})",
                    std::to_underlying(opt.get_id()),
                    opt.get_name().empty() ? "unknown" : opt.get_name()
                );
            }
        } //format(const ::net::telnet::option&, FormatContext&)
    }; //class formatter<::net::telnet::option>
} //namespace std
