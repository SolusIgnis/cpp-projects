/**
 * @file telnet-options.cppm
 * @version 0.3.0
 * @release_date September 29, 2025
 *
 * @brief Interface for Telnet option handling.
 * @remark Defines `option` class, `option::id_num` enumeration, and associated predicates/handlers.
 *
 * @copyright (c) 2025 [it's mine!]. All rights reserved.
 * @license See LICENSE file for details
 *
 * @remark This module is fully inline. An implementation unit MAY be added in Phase 5 for complex option logic.
 * @see RFC 855 for Telnet option negotiation, `:protocol_fsm` for `option` usage, `:socket` for negotiation operations, `:types` for `TelnetCommand`
 */
module; //Including Boost.Asio in the Global Module Fragment until importable header units are reliable.
#include <boost/asio.hpp>
namespace asio = boost::asio;

//Module partition interface unit
export module telnet:options;

import std; // For std::string, std::vector, std::function, std::optional, std::size_t

export import :types; ///< @see telnet-types.cppm for `byte_t`

export namespace telnet {
    /**
     * @brief Class to encapsulate Telnet option data and negotiation logic.
     *
     * @note Option support is defined by an `option` instance in `ProtocolFSM`; subnegotiation support is indicated by a non-null subnegotiation handler.
     * @remark Comparisons (==, !=, <, >, <=, >=) are based on `id_num`; implicit conversion to `id_num` allows mixed comparisons.
     * @see RFC 855 for Telnet option negotiation, `:protocol_fsm` for `option` usage in the protocol state machine, `:socket` for negotiation operations, `:types` for `TelnetCommand`.
     *
     * @todo Future Development: Use C++26 reflection to populate option names automatically.
     */
    class option {
    public:
        /**
         * @details Nested enumeration of Telnet option IDs as defined in the IANA Telnet Option Registry and MUD-specific extensions.
         * @note All 256 possible `byte_t` values are valid for proprietary extensions.
         *
         * @todo Phase 5: Review options list for completeness.
         * @todo Phase 5: Review options for potential internal implementation or implementation as a library extension.
         */
        enum class id_num : byte_t;

        /**
         * @typedef EnablePredicate
         * @brief Function type for predicates determining local or remote option support.
         *
         * @param id The `option::id_num` to evaluate.
         * @return True if the option is supported, false otherwise.
         */
        using EnablePredicate = std::function<bool(id_num)>;

        /// @brief Constructs an `option` with the given ID and optional parameters.
        explicit option(id_num id, std::string name = "",
                        EnablePredicate local_pred = always_reject,
                        EnablePredicate remote_pred = always_reject,
                        bool subneg_supported = false,
                        size_t max_subneg_size = MAX_SUBNEGOTIATION_SIZE)
            : id_(id), name_(std::move(name)),
              local_predicate_(std::move(local_pred)), remote_predicate_(std::move(remote_pred)),
              supports_subnegotiation_(subneg_supported),
              max_subneg_size_(max_subneg_size) {}

        /// @brief Factory to create common `option` instances.
        static option make_option(id_num id, std::string name,
                                 bool local_supported = false,
                                 bool remote_supported = false,
                                 bool subneg_supported = false,
                                 size_t max_subneg_size = MAX_SUBNEGOTIATION_SIZE) {
            EnablePredicate local_pred = local_supported ? always_accept : always_reject;
            EnablePredicate remote_pred = remote_supported ? always_accept : always_reject;
            return option(id, std::move(name), std::move(local_pred), std::move(remote_pred),
                          subneg_supported, max_subneg_size);
        }
        
        /// @brief Three-way comparison operator for ordering and equality.
        constexpr auto operator<=>(const option& other) const noexcept {
            return id_ <=> other.id_;
        }
        
        /// @brief Three-way comparison operator for ordering and equality.
        constexpr auto operator<=>(option::id_num other_id) const noexcept {
        	return id_ <=> other_id;
        }
        
        /// @brief Implicitly converts to `option::id_num`.
        operator id_num() const noexcept { return id_; }

        /// @brief Gets the Telnet `option::id_num`.
        id_num get_id() const noexcept { return id_; }

        /// @brief Gets the `option` name.
        const std::string& get_name() const noexcept { return name_; }

        /// @brief Evaluates the local predicate to determine if the `option` can be enabled locally.
        bool supports_local() const noexcept { return local_predicate_(id_); }

        /// @brief Evaluates the remote predicate to determine if the `option` can be enabled remotely.
        bool supports_remote() const noexcept { return remote_predicate_(id_); }

        /// @brief Gets the maximum subnegotiation buffer size.
        size_t max_subnegotiation_size() const noexcept { return max_subneg_size_; }

        /// @brief Checks if the `option` supports subnegotiation.
        bool supports_subnegotiation() const noexcept { return supports_subnegotiation_; }

        /// @brief Predicate that always accepts the `option`.
        static bool always_accept(id_num) noexcept { return true; }

        /// @brief Predicate that always rejects the `option`.
        static bool always_reject(id_num) noexcept { return false; }

    private:
        static constexpr size_t MAX_SUBNEGOTIATION_SIZE = 1024;
        
        const id_num id_;
        const std::string name_;
        
        const EnablePredicate local_predicate_;
        const EnablePredicate remote_predicate_;
        
        const bool supports_subnegotiation_;
        
        const size_t max_subneg_size_;
    }; //class option
    
    /**
     * @fn explicit option::option(id_num id, std::string name, EnablePredicate local_pred, EnablePredicate remote_pred, bool subneg_supported, size_t max_subneg_size)
     *
     * @param id The Telnet `option::id_num`.
     * @param name The option name (default empty; populated in C++26? with reflection).
     * @param local_pred Predicate for local support (default `always_reject`).
     * @param remote_pred Predicate for remote support (default `always_reject`).
     * @param subneg_supported Does the `option` support subnegotiation handler registration (default `false`).
     * @param max_subneg_size Maximum subnegotiation buffer size (<=0 for unlimited; default `MAX_SUBNEGOTIATION_SIZE`).
     */
    /**
     * @fn static option option::make_option(id_num id, std::string name, bool local_supported, bool remote_supported, bool subneg_supported, size_t max_subneg_size)
     *
     * @param id The Telnet `option::id_num` byte.
     * @param name The option name (required for debuggability).
     * @param local_supported Whether the `option` is supported locally (default false).
     * @param remote_supported Whether the `option` is supported remotely (default false).
     * @param subneg_supported Whether the `option` supports subnegotiation handlers (default false).
     * @param max_subneg_size Maximum subnegotiation buffer size (default `MAX_SUBNEGOTIATION_SIZE`).
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
     * @see IANA Telnet Option Registry, RFC 855 for Telnet option negotiation, `:protocol_fsm` for `option` processing, `:socket` for negotiation operations.
     */
    enum class option::id_num : byte_t {
        BINARY                             = 0x00, ///< Binary Transmission (@see RFC 856)
        ECHO                               = 0x01, ///< Echo (@see RFC 857)
        RECONNECTION                       = 0x02, ///< Reconnection (NIC 15391 of 1973)
        SUPPRESS_GO_AHEAD                  = 0x03, ///< Suppress Go Ahead (@see RFC 858)
        APPROX_MESSAGE_SIZE_NEGOTIATION    = 0x04, ///< Approx Message Size Negotiation (NIC 15393 of 1973)
        STATUS                             = 0x05, ///< Status (@see RFC 859)
        TIMING_MARK                        = 0x06, ///< Timing Mark (@see RFC 860)
        REMOTE_CONTROLLED_TRANS_AND_ECHO   = 0x07, ///< Remote Controlled Trans and Echo (@see RFC 726)
        OUTPUT_LINE_WIDTH                  = 0x08, ///< Output Line Width (NIC 20196 of August 1978)
        OUTPUT_PAGE_SIZE                   = 0x09, ///< Output Page Size (NIC 20197 of August 1978)
        OUTPUT_CARRIAGE_RETURN_DISPOSITION = 0x0A, ///< Output Carriage-Return Disposition (@see RFC 652)
        OUTPUT_HORIZONTAL_TAB_STOPS        = 0x0B, ///< Output Horizontal Tab Stops (@see RFC 653)
        OUTPUT_HORIZONTAL_TAB_DISPOSITION  = 0x0C, ///< Output Horizontal Tab Disposition (@see RFC 654)
        OUTPUT_FORMFEED_DISPOSITION        = 0x0D, ///< Output Formfeed Disposition (@see RFC 655)
        OUTPUT_VERTICAL_TABSTOPS           = 0x0E, ///< Output Vertical Tabstops (@see RFC 656)
        OUTPUT_VERTICAL_TAB_DISPOSITION    = 0x0F, ///< Output Vertical Tab Disposition (@see RFC 657)
        OUTPUT_LINEFEED_DISPOSITION        = 0x10, ///< Output Linefeed Disposition (@see RFC 658)
        EXTENDED_ASCII                     = 0x11, ///< Extended ASCII (@see RFC 698)
        LOGOUT                             = 0x12, ///< Logout (@see RFC 727)
        BYTE_MACRO                         = 0x13, ///< Byte Macro (@see RFC 735)
        DATA_ENTRY_TERMINAL                = 0x14, ///< Data Entry Terminal (@see RFC 1043, RFC 732)
        SUPDUP                             = 0x15, ///< SUPDUP (@see RFC 736, RFC 734)
        SUPDUP_OUTPUT                      = 0x16, ///< SUPDUP Output (@see RFC 749)
        SEND_LOCATION                      = 0x17, ///< Send Location (@see RFC 779)
        TERMINAL_TYPE                      = 0x18, ///< Terminal Type (@see RFC 1091)
        END_OF_RECORD                      = 0x19, ///< End of Record (@see RFC 885)
        TACACS_USER_IDENTIFICATION         = 0x1A, ///< TACACS User Identification (@see RFC 927)
        OUTPUT_MARKING                     = 0x1B, ///< Output Marking (@see RFC 933)
        TERMINAL_LOCATION_NUMBER           = 0x1C, ///< Terminal Location Number (@see RFC 946)
        TELNET_3270_REGIME                 = 0x1D, ///< Telnet 3270 Regime (@see RFC 1041)
        X_3_PAD                            = 0x1E, ///< X.3 PAD (@see RFC 1053)
        NEGOTIATE_ABOUT_WINDOW_SIZE        = 0x1F, ///< Negotiate About Window Size (@see RFC 1073)
        TERMINAL_SPEED                     = 0x20, ///< Terminal Speed (@see RFC 1079)
        REMOTE_FLOW_CONTROL                = 0x21, ///< Remote Flow Control (@see RFC 1372)
        LINEMODE                           = 0x22, ///< Linemode (@see RFC 1184)
        X_DISPLAY_LOCATION                 = 0x23, ///< X Display Location (@see RFC 1096)
        ENVIRONMENT_OPTION                 = 0x24, ///< Environment Option (@see RFC 1408)
        AUTHENTICATION_OPTION              = 0x25, ///< Authentication Option (@see RFC 2941)
        ENCRYPTION_OPTION                  = 0x26, ///< Encryption Option (@see RFC 2946)
        NEW_ENVIRONMENT_OPTION             = 0x27, ///< New Environment Option (@see RFC 1572)
        TN3270E                            = 0x28, ///< TN3270E (@see RFC 2355)
        XAUTH                              = 0x29, ///< XAUTH (Rob Earhart)
        CHARSET                            = 0x2A, ///< CHARSET (@see RFC 2066)
        TELNET_REMOTE_SERIAL_PORT          = 0x2B, ///< Telnet Remote Serial Port (Robert Barnes)
        COM_PORT_CONTROL_OPTION            = 0x2C, ///< Com Port Control Option (@see RFC 2217)
        TELNET_SUPPRESS_LOCAL_ECHO         = 0x2D, ///< Telnet Suppress Local Echo (Wirt Atmar)
        TELNET_START_TLS                   = 0x2E, ///< Telnet Start TLS (Michael Boe)
        KERMIT                             = 0x2F, ///< KERMIT (@see RFC 2840)
        SEND_URL                           = 0x30, ///< SEND-URL (David Croft)
        FORWARD_X                          = 0x31, ///< FORWARD_X (Jeffrey Altman)
        /* Range 0x32-0x44 Unused per IANA */
        MSDP                               = 0x45, ///< MUD Server Data Protocol
        /* Range 0x46-0x54 Unused per IANA */
        MCCP1                              = 0x55, ///< MUD Client Compression Protocol v.1
        MCCP2                              = 0x56, ///< MUD Client Compression Protocol v.2
        /* Range 0x57-0x89 Unused per IANA */
        TELOPT_PRAGMA_LOGON                = 0x8A, ///< TELOPT PRAGMA LOGON (Steve McGregory)
        TELOPT_SSPI_LOGON                  = 0x8B, ///< TELOPT SSPI LOGON (Steve McGregory)
        TELOPT_PRAGMA_HEARTBEAT            = 0x8C, ///< TELOPT PRAGMA HEARTBEAT (Steve McGregory)
        /* Range 0x8D-0xFE Unused per IANA */
        EXTENDED_OPTIONS_LIST              = 0xFF  ///< Extended-Options-List (@see RFC 861)
    }; //enum class option::id_num
    
    /**
     * @brief Thread-safe registry for managing `option` instances in the protocol state machine.
     * @remark Used by `ProtocolFSM` to store and query supported Telnet options.
     * @remark The `std::initializer_list` constructor enforces sorted input by `option::id_num` at compile time using `static_assert`, ensuring O(n) `std::set` construction. Unsorted inputs cause compilation failure. All accessor methods are atomic via `std::shared_mutex`, supporting concurrent reads and exclusive writes.
     * @warning Methods are guaranteed atomic by `std::shared_mutex`, but chaining operations is NOT thread-safe; however `get` followed by `upsert` provides snapshot consistency.
     * @todo Phase 5: Review likely upper bounds of n (vs theoretical bound of n=256) in light of scalability of `std::set` for large option sets and consider alternative containers (e.g., pre-filled vector) if lookup performance becomes a bottleneck.
     * @see RFC 855 for Telnet option negotiation, `:protocol_fsm` for `option` usage in the protocol state machine, `:socket` for negotiation operations, `:types` for `TelnetCommand`, `option` for option details.
     */
    class option_registry {
    public:
        /// @brief Constructs a registry from a sorted initializer list of `option` instances.
        option_registry(std::initializer_list<option> init) {
            static_assert(
                std::is_sorted(init.begin(), init.end(), std::less<>{}),
                "Initializer list must be sorted by option::id_num"
            );
            registry_ = std::set<option, std::less<>>(init.begin(), init.end());
        } //option_registry(std::initializer_list<option>)
    
        /// @brief Constructs a registry from a pre-constructed `std::set` of `option` instances.
        option_registry(std::set<option, std::less<>>&& init) : registry_(std::move(init)) {}

        /// @brief Retrieves an `option` by its ID.
        std::optional<option> get(option::id_num opt_id) const noexcept {
            std::shared_lock<std::shared_mutex> lock(mutex_);
            auto it = registry_.find(opt_id);
            if (it != registry_.end()) {
                return *it;
            } else {
                return std::nullopt;
            }
        } //get(option::id_num)

        /// @brief Checks if an `option` is present in the registry.
        bool has(option::id_num opt_id) const noexcept {
            std::shared_lock<std::shared_mutex> lock(mutex_);
            return (registry_.find(opt_id) != registry_.end());
        } //has(option::id_num)

        /// @brief Inserts or updates an `option` in the registry.
        const option& upsert(const option& opt) {
            std::lock_guard<std::shared_mutex> lock(mutex_);
            auto [add_result, success] = registry_.insert(opt);
            if (success) {
                return *add_result;
            } else {
                // Use iterator from erase as hint to insert new option at same position, optimizing insertion to O(1)
                auto [replace_result, _] = registry_.insert(registry_.erase(add_result), opt);
                return *replace_result;
            }
        } //upsert(const option&)

        /// @brief Inserts or updates an `option` with error handling.
        void upsert(const option& opt, std::error_code& ec) noexcept {
            try {
                upsert(opt); // the mutex is locked inside this call
            } catch (const std::system_error& e) {
                ec = e.code();
            } catch (std::bad_alloc) {
                ec = make_error_code(std::errc::not_enough_memory);
            } catch (...) {
                ec = make_error_code(error::internal_error);
            }
        } //upsert(const option&, std::error_code&)

        /// @brief Inserts or updates an `option` constructed from arguments.
        template<typename... Args>
        const option& upsert(option::id_num opt_id, Args&&... args) {
            return upsert(option{opt_id, std::forward<Args>(args)...});
        } //upsert(option::id_num, Args...)
    
    private:
        std::set<option, std::less<>> registry_;
        mutable std::shared_mutex mutex_;
    }; // class option_registry
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
} //namespace telnet