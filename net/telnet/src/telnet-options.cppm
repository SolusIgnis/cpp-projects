/**
 * @file telnet-options.cppm
 * @version 0.2.0
 * @release_date September 19, 2025
 * @brief Interface for Telnet option handling.
 * @remark Defines option class, option::id_num enumeration, and associated predicates/handlers.
 * @copyright (c) 2025 [it's mine!]. All rights reserved.
 * @license See LICENSE file for details
 *
 * @remark This module is fully inline. An implementation unit MAY be added in Phase 3 for complex option logic.
 * @see RFC 855 for Telnet option negotiation, telnet:protocol_fsm for option usage, telnet:socket for negotiation operations, telnet:types for Telnet commands
 */
module; //Including Boost.Asio in the Global Module Fragment until importable header units are reliable.
#include <boost/asio.hpp>
namespace asio = boost::asio;

//Module partition interface unit
export module telnet:options;

import std;        // For std::string, std::vector, std::function, std::optional, std::size_t
import std.compat; // For std::uint8_t

export namespace telnet {
    /**
     * @brief Class to encapsulate Telnet option data and negotiation logic.
     * @note Option support is defined by an option instance in ProtocolFSM; subnegotiation support is indicated by a non-null subnegotiation handler.
     * @remark Comparisons (==, !=, <, >, <=, >=) are based on id_num; implicit conversion to id_num allows mixed comparisons.
     * @todo Phase 3: Revisit virtual methods (e.g., `option_base` with `handle_subnegotiation`) if custom option handlers require polymorphic behavior.
     * @todo Phase 3: Consider `std::vector<SubnegotiationHandler>` if distinct, independent actions cannot be combined into a single lambda.
     * @todo Future Development: Use C++23 reflection to populate option names automatically.
     * @see RFC 855 for Telnet option negotiation, telnet:protocol_fsm for option usage in the protocol state machine, telnet:socket for negotiation operations, telnet:types for Telnet commands.
     */
    class option {
    public:
        /**
         * @brief Nested enumeration of Telnet option IDs as defined in the IANA Telnet Option Registry and MUD-specific extensions.
         * @note All 256 possible std::uint8_t values are valid for proprietary extensions.
         * @todo Phase 3: Review options list for completeness.
         * @todo Phase 3: Review options for potential internal implementation or implementation as a library extension.
         * @see IANA Telnet Option Registry, RFC 855 for Telnet option negotiation, telnet:protocol_fsm for option processing, telnet:socket for negotiation operations.
         */
        enum class id_num : std::uint8_t;

        using EnablePredicate = std::function<bool(id_num)>;
        using SubnegotiationHandler = std::function<void(const id_num, const std::vector<std::uint8_t>&)>;

        /**
         * @brief Constructs an option with the given ID and optional parameters.
         * @param id The Telnet option ID.
         * @param name The option name (default empty; populated in Phase 3 with reflection).
         * @param local_pred Predicate for local support (default rejects).
         * @param remote_pred Predicate for remote support (default rejects).
         * @param subneg_handler Handler for subnegotiation data (default nullopt).
         * @param max_subneg_size Maximum subnegotiation buffer size (<=0 for unlimited; default MAX_SUBNEGOTIATION_SIZE).
         */
        explicit option(id_num id, std::string name = "",
                        EnablePredicate local_pred = always_reject,
                        EnablePredicate remote_pred = always_reject,
                        std::optional<SubnegotiationHandler> subneg_handler = std::nullopt,
                        size_t max_subneg_size = MAX_SUBNEGOTIATION_SIZE)
            : id_(id), name_(std::move(name)),
              local_predicate_(std::move(local_pred)), remote_predicate_(std::move(remote_pred)),
              subnegotiation_handler_(std::move(subneg_handler)),
              max_subneg_size_(max_subneg_size) {}

        /**
         * @brief Factory to create common option instances.
         * @param id The Telnet option ID byte.
         * @param name The option name (required for debuggability).
         * @param local_supported Whether the option is supported locally (default false).
         * @param remote_supported Whether the option is supported remotely (default false).
         * @param subneg_handler Handler for subnegotiation data (default nullopt).
         * @param max_subneg_size Maximum subnegotiation buffer size (default MAX_SUBNEGOTIATION_SIZE).
         * @return An option instance configured for the given TelOpt number.
         */
        static option make_option(id_num id, std::string name,
                                 bool local_supported = false, bool remote_supported = false,
                                 std::optional<SubnegotiationHandler> subneg_handler = std::nullopt,
                                 size_t max_subneg_size = MAX_SUBNEGOTIATION_SIZE) {
            EnablePredicate local_pred = local_supported ? always_accept : always_reject;
            EnablePredicate remote_pred = remote_supported ? always_accept : always_reject;
            return option(id, std::move(name), std::move(local_pred), std::move(remote_pred),
                          std::move(subneg_handler), max_subneg_size);
        }

        /**
         * @brief Implicit conversion to id_num.
         * @return The option's ID.
         */
        operator id_num() const noexcept { return id_; }

        /**
         * @brief Three-way comparison operator for ordering and equality.
         * @param other The other option to compare with.
         * @return The result of comparing id_ values.
         */
        auto operator<=>(const option& other) const noexcept {
            return id_ <=> other.id_;
        }

        /**
         * @brief Gets the option ID.
         * @return The Telnet option ID.
         */
        id_num get_id() const noexcept { return id_; }

        /**
         * @brief Gets the option name.
         * @return The user-provided name or empty string.
         */
        const std::string& get_name() const noexcept { return name_; }

        /**
         * @brief Checks if the option is supported locally.
         * @return True if the option is supported locally, false otherwise.
         */
        bool supports_local() const noexcept { return local_predicate_(id_); }

        /**
         * @brief Checks if the option is supported remotely.
         * @return True if the option is supported remotely, false otherwise.
         */
        bool supports_remote() const noexcept { return remote_predicate_(id_); }

        /**
         * @brief Handles subnegotiation data if a handler is set.
         * @param data The subnegotiation data.
         */
        void handle_subnegotiation(const std::vector<uint8_t>& data) const noexcept {
            if (subnegotiation_handler_) {
                (*subnegotiation_handler_)(id_, data);
            }
        }

        /**
         * @brief Gets the maximum subnegotiation buffer size.
         * @return The maximum buffer size (<=0 for unlimited).
         */
        size_t max_subnegotiation_size() const noexcept { return max_subneg_size_; }

        /**
         * @brief Checks if the option supports subnegotiation.
         * @return True if a non-null subnegotiation handler is set, false otherwise.
         */
        bool supports_subnegotiation() const noexcept { return subnegotiation_handler_.has_value(); }

        /**
         * @brief Sets the subnegotiation handler for this option.
         * @param handler The new subnegotiation handler (nullopt to disable).
         */
        void set_subnegotiation_handler(std::optional<SubnegotiationHandler> handler) noexcept {
            subnegotiation_handler_ = std::move(handler);
        }

        /**
         * @brief Predicate that always accepts the option.
         * @return True.
         */
        static bool always_accept(id_num) noexcept { return true; }

        /**
         * @brief Predicate that always rejects the option.
         * @return False.
         */
        static bool always_reject(id_num) noexcept { return false; }

    private:
        static constexpr size_t MAX_SUBNEGOTIATION_SIZE = 1024;
        
        id_num id_;
        std::string name_;
        
        EnablePredicate local_predicate_;
        EnablePredicate remote_predicate_;
        
        std::optional<SubnegotiationHandler> subnegotiation_handler_;
        
        size_t max_subneg_size_;
    }; //class option

    /**
     * @brief Enumeration of Telnet option ID bytes
     * @see IANA Telnet Option Registry, RFC 855 for Telnet option negotiation, telnet:protocol_fsm for option processing, telnet:socket for negotiation operations.
     */
    enum class option::id_num : std::uint8_t {
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
} //namespace telnet