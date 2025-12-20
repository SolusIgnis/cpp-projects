# Changelog.md
## [0.2.0] - September 19, 2025
- Updated `version` across all files to `0.2.0` to mark the official release.
- Updated `release_date` across all files to September 19, 2025, reflecting the current date of the release.

## [0.1.8] - September 18, 2025
### Changed
- Refactored `InputProcessor` with `async_compose`, using `State` enum (`INITIALIZING`, `READING`, `PROCESSING`, `DONE`).
- Added `complete` helper in `InputProcessor` to ensure single completion and `State::DONE` safety.
- Added `ProtocolConfigT` template parameter with default `DefaultProtocolFSMConfig` to `socket` class across all files.
- Added private `async_write_temp_buffer` and `async_report_error` methods to the `socket` class in `telnet-socket.cppm`, with implementations in `telnet-socket-async-impl.cpp`.
- Refactored `async_read_some` and `async_write_*` to use new `InputProcessor` and helper methods. 
- Updated `telnet-socket-sync-impl.cpp` to include `ProtocolConfigT` and use pass-by-value for `TelnetCommand` and `option::id_num` in `write_command`, `write_negotiation`, and `write_subnegotiation`.
- Added private `asio_completion_signature` type alias (`void(std::error_code, std::size_t)`) in `telnet-socket.cppm` to simplify Boost.Asio completion handler signatures in `TelnetSocketConcept` and `socket` class methods.
- Added private template type alias `asio_result_type<CompletionToken> = asio::async_result_t<std::decay_t<CompletionToken>, asio_completion_signature>` to the `socket` class in `telnet-socket.cppm` to simplify asynchronous method return types.
- Enhanced `ProtocolFSM` with `option::id_num`, updated `SubnegotiationHandler`, added `option::make_option`, and implemented `InputProcessor` with `async_compose`.
- Shortened template parameter names in `telnet-socket-async-impl.cpp` and `telnet-socket-impl.cpp` to `NLS` and `PC`.
- Simplified template syntax in `telnet-socket-async-impl.cpp` and `telnet-socket-impl.cpp` using `template<TelnetSocketConcept NLS, ProtocolFSMConfig PC>`.


## [0.1.7] - September 17, 2025
### Changed
- Replaced `TelnetOption` with `option::id_num` across all interface and implementation files for consistency and type safety.
- Updated `SubnegotiationHandler` to use unary signature (`std::function<void(const std::vector<uint8_t>&)>`) in `telnet-options.cppm` and `telnet-protocol_fsm-impl.cpp`.
- Added `option::make_option` factory with `name` parameter for option construction in `telnet-options.cppm` and `telnet-protocol_fsm-impl.cpp`.
- Implemented implicit conversion for `option::id_num` in map lookups and used lambda-based `std::find_if` for option lookups in `telnet-protocol_fsm-impl.cpp`.
- Enhanced `ProtocolFSM` to register default options for unregistered `option::id_num` values during subnegotiation.
- Added checks for `!current_option_` in `ProtocolFSM::process_byte` to return `error::protocol_violation` for invalid state transitions.
- Reordered IAC check in `ProtocolFSM::process_byte` for efficiency.
- Updated all files (`telnet-options.cppm`, `telnet-protocol_fsm-impl.cpp`, `telnet-socket.cppm`, `telnet-socket-impl.cpp`, `telnet-socket-async-impl.cpp`, `telnet-socket-sync-impl.cpp`) to version **0.1.11** and release date **September 17, 2025**.
- Updated comments to reflect `option::id_num` and `ProtocolFSM` behavior (e.g., default option registration).
- Preserved `@todo` notes for Phase 2 tasks (refactor `InputProcessor` to use `async_compose`, review writers for `async_initiate` usage).

## [0.1.6] - September 16, 2025
### Changed
- Replaced generic error codes (`asio::error::operation_aborted`, `asio::error::invalid_argument`, `std::errc::invalid_argument`, `std::errc::not_enough_memory`) with `telnet::error` codes (`protocol_violation`, `invalid_command`, `option_not_available`, `invalid_subnegotiation`, `subnegotiation_overflow`, `input_processor_already_initialized`, `invalid_negotiation`, `internal_error`) across all Telnet module files for improved error specificity.
- Retained `std::errc::not_enough_memory` for `std::bad_alloc` exceptions in synchronous socket methods (`telnet-socket-sync-impl.cpp`) to maintain consistency with existing behavior.
- Updated all files to version 0.1.11 and release date to 2025-09-16.
- Added `import :errors;` to relevant files to access `telnet::error` codes.

## [0.1.5] - September 16, 2025
### Added
- **Subnegotiation Option State**:
  - Added `ProtocolState::SubnegotiationOption` to validate the option byte after `IAC SB` as a `TelnetOption` per RFC 854.
  - Stores the full `Option` object in `current_option_` (changed from `std::optional<TelnetOption>` to `std::optional<Option>`).
  - Validates option via `ProtocolConfig::get_option`; if unregistered, invokes `unknown_option_handler_` and creates a default `Option` with `MAX_SUBNEGOTIATION_SIZE`.
- **Per-Option Subnegotiation Buffer Limits**:
  - Added `max_subneg_size_` to `Option` (default `MAX_SUBNEGOTIATION_SIZE = 1024`, <=0 for unlimited).
  - Updated `Option` constructor and `register_option` to accept `max_subneg_size`.
  - Modified `handle_state_subnegotiation` and `handle_state_subnegotiation_iac` to use `current_option_->max_subneg_size_` for buffer checks, falling back to `MAX_SUBNEGOTIATION_SIZE` for default `Option`.
- **Subnegotiation Warning Handler**:
  - Added `SubnegotiationWarningHandler` type (`std::function<void(TelnetOption, size_t, size_t)>`) to `DefaultProtocolFSMConfig`.
  - Added `set_subnegotiation_warning_handler` and `warn_subnegotiation_buffer` to trigger warnings at 80% of `max_subneg_size`.
  - Updated `ProtocolFSMConfig` concept to require `SubnegotiationWarningHandler` and related methods.
- Completes Phase 2 task: make subnegotiation buffer size configurable (Task 4).

### Changed
- **Breaking Change**: Changed `ProtocolFSM::current_option_` from `std::optional<TelnetOption>` to `std::optional<Option>` to store the full `Option` object during subnegotiation, avoiding repeated lookups.
- Updated `handle_state_option_negotiation` to set `current_option_` using `ProtocolConfig::get_option`.
- Restored all comments from **0.1.9** to maintain documentation integrity.

### Notes
- Default `max_subneg_size` is 1024 for backward compatibility and safety for unknown options.
- Warning handler triggers at 80% of `max_subneg_size` (configurable via `set_subnegotiation_warning_handler`).
- `SubnegotiationOption` state ensures RFC 854 compliance by validating the option byte after `IAC SB`.
- Completes all Phase 2 tasks (Task 2, Task 3, Task 4).
- Compatible with existing `InputProcessor` and `async_read_some` semantics.
- Storing `Option` in `current_option_` improves performance by avoiding repeated `get_option` calls (O(n) for `std::set<Option>`).

## [0.1.4] - September 15, 2025
### Added
- **Refined ProtocolFSMConfig concept**:
  - Added requirements for nested types `TelnetCommandHandler`, `UnknownOptionHandler`, and `ErrorLogger` in `ProtocolConfig`.
  - Constrained these types to be convertible to `ProtocolFSM<ConfigT>::TelnetCommandHandler`, `ProtocolFSM<ConfigT>::UnknownOptionHandler`, and `ProtocolFSM<ConfigT>::ErrorLogger`, respectively, using `std::convertible_to`.
  - Updated method constraints to use `ProtocolFSM<ConfigT>`'s handler types, ensuring type consistency for custom `ProtocolConfig` implementations.
- **Default error logging**:
  - Implemented default `error_logger_` in `DefaultProtocolFSMConfig::initialize` to log errors to `std::cerr` with error code and byte in hexadecimal.
  - Added error logging in `ProtocolFSM::process_byte` for invalid states (`operation_aborted`), `handle_state_iac` for invalid commands (`invalid_argument`), `handle_state_option_negotiation` for invalid commands/options (`invalid_argument`), and `handle_state_subnegotiation_iac` for missing subnegotiation handlers (`invalid_argument`).
  - Updated `ProtocolFSM::process_byte`, `handle_state_iac`, `handle_state_option_negotiation`, and `handle_state_subnegotiation_iac` to call `ProtocolConfig::log_error` for invalid states (`operation_aborted`), invalid commands/options (`invalid_argument`), and buffer overflows (`message_size`).
  - Updated `ProtocolFSMConfig` concept to require `log_error` instead of `get_error_logger`.
  - Completes Phase 2 task: implement error logging in `process_byte`.
  
## [0.1.3] - September 15, 2025
### Added
- **Refined `DefaultProtocolFSMConfig::get_command_handler`**:
  - Updated `get_command_handler(TelnetCommand)` to return `unknown_command_handler_` if no specific handler is found in `command_handlers_`, reducing the need for explicit `get_unknown_command_handler` calls.
  - Removed `get_unknown_command_handler` from `DefaultProtocolFSMConfig` as it’s now encapsulated in `get_command_handler`.
  - Simplified `ProtocolFSM::handle_state_iac` to use only `get_command_handler`.
  - Updated `ProtocolFSMConfig` concept to remove `get_unknown_command_handler` requirement.
- **Improved `ProtocolFSM` configuration access**:
  - Added `DefaultProtocolFSMConfig::get_command_handler(TelnetCommand)` and `get_option(TelnetOption)` returning `std::optional` for direct, encapsulated access to handlers and options.
  - Removed `get_command_handlers()` and `get_option_registry()` to hide collection types (encapsulates `std::map` and `std::set`).
  - Updated `ProtocolFSM` to use new getters, simplifying `handle_state_iac`, `handle_state_option_negotiation`, and `handle_state_subnegotiation_iac`.
  - Updated `ProtocolFSMConfig` concept to require `get_command_handler` and `get_option`.
- **Templated `ProtocolFSM` on a configuration class**:
  - Introduced `DefaultProtocolFSMConfig` as a class to encapsulate options, handlers, and thread safety primitives.
  - Templated `ProtocolFSM` on `ConfigT`, defaulting to `DefaultProtocolFSMConfig`.
  - Added public nested `ProtocolConfig` type alias for `ConfigT`, enabling clean API (e.g., `ProtocolFSM::ProtocolConfig::set_unknown_command_handler`, `ProtocolFSM::ProtocolConfig::set_error_logger`).
  - Moved configuration-related static members (`command_handlers_`, `option_registry_`, `error_logger_`, etc.) to `DefaultProtocolFSMConfig`.
  - Added static getters in `DefaultProtocolFSMConfig` for encapsulated, thread-safe access using `std::shared_lock`.
  - Removed forwarding methods from `ProtocolFSM`, relying on `ProtocolConfig` for configuration management.
  - Moved `set_error_logger` to `DefaultProtocolFSMConfig` with a getter, preparing for future error logging implementation.
  - Added C++20 concept `ProtocolFSMConfig` to constrain configuration types.
  - Updated `socket` to use `ProtocolFSM<DefaultProtocolFSMConfig>` by default.
  - Removed `@todo Phase2: template ProtocolFSM on a configuration class to register options and handlers`.
  - Removed `@todo Phase2: add std::shared_lock around ConfigT member reads in process_byte` as getters provide thread-safe access.
  - Removed `@todo Phase2: implement error logging with detailed context` as `set_error_logger` is now in `DefaultProtocolFSMConfig`, with implementation deferred.

## [0.1.2] - September 15, 2025
### Changed
- **Refactored `ProtocolFSM::process_byte` for improved maintainability**:
  - Split the large `switch` statement into helper methods (`handle_state_normal`, `handle_state_iac`, `handle_state_option_negotiation`, `handle_state_subnegotiation`, `handle_state_subnegotiation_iac`) to handle each protocol state.
  - Added `change_state` method to centralize state transitions and ensure `current_command_`, `current_option_`, and `subnegotiation_buffer_` are cleared when transitioning to `ProtocolState::Normal`.
  - Removed `@todo Phase2: consider refactoring with helper methods` from `telnet-protocol_fsm.cppm` as the task is complete.
  - Maintained `noexcept` and tuple return type for compatibility with `InputProcessor` and `async_read_some`.
- Updated file headers to reflect version `0.1.2`.

## [0.1.1] - September 14, 2025
### Changed
- Reorganized `telnet/telnet-socket-sync-impl.cpp` methods, grouping throwing overloads and noexcept overloads with implementation comments.
- Refactored `telnet/telnet.cppm` to only export partitions (`:types`, `:options`, `:protocol_fsm`, `:socket`).
- Created new `telnet:types` partition for `TelnetCommand`.
- Moved `TelnetOption` to `telnet:options` and `TelnetSocketConcept` to `telnet:socket`.
- Added global module fragments with `#include <boost/asio.hpp>` and `namespace asio = boost::asio;` in all module units.
- Changed `telnet:socket` to use `export import :types;` and `export import :options;` to make `TelnetCommand` and `TelnetOption` visible in its public interface.
- Added explicit `import telnet:<partition>; // module partition interface unit` in implementation units (`telnet-protocol_fsm-impl.cpp`, `telnet-socket-impl.cpp`, `telnet-socket-async-impl.cpp`, `telnet-socket-sync-impl.cpp`) for portability and clarity.
- Renamed partition `telnet:telnet_socket` to `telnet:socket` for simplicity.

### Fixed
- Corrected reference parameters in `telnet:socket` for rvalues/lvalues and const-ness.

## [0.1.0] - September 14, 2025
### Changed
- Updated version across `telnet.cppm` and `telnet_socket.cppm` to `0.1.0` to mark the official release.
- Updated `release_date` in `telnet.cppm` and `telnet_socket.cppm` to September 14, 2025, reflecting the current date of the release.
- Promoted `0.1.0[rc1]` changes to the official release, including all `@todo` items added for Phase 2 and Phase 3 planning (e.g., refactoring, safety improvements, options/subnegotiation enhancements).
- Integrated comprehensive `@todo` tags with `Phase2:` and `Phase3:` prefixes into code comments in `telnet.cppm` and `telnet_socket.cppm` for future development tracking.

## [0.1.0[rc1]] - September 13, 2025
### Changed
- Updated version across `telnet.cppm` and `telnet_socket.cppm` to `0.1.0[rc1]` to mark the release candidate for the 0.1.0 milestone.
- Updated `release_date` in `telnet.cppm` to September 13, 2025, reflecting the current date of the release candidate.

## [0.0.12] - September 13, 2025
### Changed
- Replaced the placeholder `InputProcessor` implementation with a fully developed nested class template in `socket`, integrating asynchronous Telnet input processing with `async_read_some`.
- Updated `async_read_some` to instantiate and use `InputProcessor` with perfect forwarding, replacing the direct `process_telnet_input` call and detached `async_write_negotiation` handling.
- Moved `is_processing_ = true` inside the `if (!is_processing_)` block in `InputProcessor::operator()(const std::error_code& ec, std::size_t bytes_transferred)` to serve as an initialization step, closing the block early.
- Moved the `for` loop and result propagation logic into the trunk of the control flow in `InputProcessor::operator()(const std::error_code& ec, std::size_t bytes_transferred)`.
- Removed reinitializations of `begin`, `end`, `write_it`, and `read_it` from the trunk, as they are now managed as private member iterators (`buf_begin_`, `buf_end_`, `write_it_`, `read_it_`).
- Removed the empty `else` branch from `if (!is_processing_)`, as the trunk now handles all processing.
- Removed the `else` from `if (!is_processing_)` in `operator()(const std::error_code& ec, std::size_t bytes_transferred)`, as the `if (!completion_handler_)` throw guarantees this as the else path.
- Moved the `if (ec)` error handling before the `if (!is_processing_)` branch, ensuring consistent error propagation regardless of state.
- Removed the `enum class ProcessorState` and `change_state` method from `InputProcessor`, simplifying state management.
- Replaced `current_state_` with a `bool is_processing_` initialized to `false` to track the processing state.
- Updated `operator()(const std::error_code& ec, std::size_t bytes_transferred)` to use an `if-else` chain instead of a `switch`, deducing states from `completion_handler_` and `is_processing_`.
- Removed the `change_state(ProcessorState::READING)` call from `operator()(CompletionHandler handler)`, relying on `is_processing_` for state transitions.
- Eliminated the `WRITING` state, as it was redundant.
- Adjusted the `UNINITIALIZED` state check to `if (!completion_handler_)` with a `std::logic_error` throw.
- Integrated the `process_telnet_input` logic directly into the trunk of `operator()(const std::error_code& ec, std::size_t bytes_transferred)` in `InputProcessor`, replacing the method call with inline processing using `fsm_.process_byte`.
- Adapted the inline logic to propagate errors and results via `completion_handler_`, maintaining the original flow.
- Added `++read_it_;` before the `parent_socket_.async_write_negotiation` call in `InputProcessor` to increment the read iterator, ensuring it advances past the negotiation-triggering byte and mimics a `continue` on successful write completion.
- Replaced the `asio::co_spawn` call with a direct call to `parent_socket_.async_write_negotiation` in `InputProcessor` when handling `negotiation_response`, binding the `parent_socket_`'s executor using `asio::bind_executor(parent_socket_.get_executor(), shared_from_this())`.
- Added a `return` statement after `async_write_negotiation` to exit the function after initiating the async write.
- Confirmed that `is_processing_` being set before the `async_write_negotiation` call allows the `for` loop to resume without reinitialization on write completion, rendering the `PROCESSING` and `WRITING` states redundant.
- Added standard Doxygen-style documentation comments to the `InputProcessor` class:
  - Class-level comment with `@brief`, `@tparam`, and `@note` describing its role in Telnet input processing and unified state management.
  - `///` inline comments for private members (`parent_socket_`, `buffers_`, `is_processing_`, `completion_handler_`, `buf_begin_`, `buf_end_`, `write_it_`, `read_it_`) detailing their purposes.
- Updated the documentation for `operator()(CompletionHandler handler)` to refer to it as the "Initiator callback" with an appropriate `@brief` description.
- Added `@see ProtocolFSM::process_byte in telnet.cppm` to the documentation of `operator()(const std::error_code& ec, std::size_t bytes_transferred)` to cross-reference the byte processing logic.
- Deprecated `process_telnet_input` with a note to use `InputProcessor` for integrated async processing.
- Updated `async_read_some` to pass `InputProcessor<MutableBufferSequence, asio::default_completion_token_t<executor_type>>::create(*this, std::move(buffers))` directly as the initiator argument to `asio::async_initiate`, replacing the lambda wrapper for a more concise and efficient initiation.
- Renamed the `Socket` class to `socket` to align with Boost.Asio naming conventions for public socket types, updating all references and documentation accordingly.
- Replaced all references to the template parameter `LowerLayerSocketT` with the type alias `next_layer_type` where appropriate (e.g., constructor parameter, member variable, and `ProtocolFSM` template), maintaining `lowest_layer_type` as a distinct alias for `LowerLayerSocketT::lowest_layer_type`.
- Updated the `InputProcessor` initiator callback `operator()(CompletionHandler handler)` to `operator()(CompletionHandler&& handler)` to align with Boost.Asio’s `async_initiate` convention for forwarding completion handlers.
- Bumped version to 0.0.12 across `telnet_socket.cppm` to reflect the comprehensive refactoring and enhancement of input processing.

## [0.0.11] - September 12, 2025
### Changed
- Enhanced `ProtocolFSM` in `telnet.cppm` by using `std::call_once` with a `static std::once_flag` to ensure `initialize` is executed only once by the first instance, preventing subsequent instances from overwriting static members (`command_handlers_`, `unknown_command_handler_`, `unknown_option_handler_`, `option_registry_`).
- Made the `initialize` method private in `ProtocolFSM` to restrict its accessibility and ensure it’s only called via `std::call_once` during construction.
- Bumped version to 0.0.11 across `telnet.cppm` to reflect the one-time initialization improvement and access control change.

### Fixed
- Restored explicit `uint8_t` hexadecimal values to `TelnetCommand` and `TelnetOption` enumerations in `telnet.cppm`, aligning with RFC 854 and the IANA Telnet Option Registry (e.g., `IAC = 0xFF`, `BINARY = 0x00`, `EXTENDED_OPTIONS_LIST = 0xFF`).
- Corrected `TelnetOption` values to match the IANA registry provided, converting decimal values to hexadecimal (e.g., `NEGOTIATE_ABOUT_WINDOW_SIZE = 0x1F`, `TELOPT_PRAGMA_LOGON = 0x8A`), resolving discrepancies from previous assignments.

### Added
- Added MUD-specific options to `TelnetOption` enumeration: `MSDP = 0x45` (MUD Server Data Protocol), `MCCP1 = 0x55` (MUD Client Compression Protocol v.1), and `MCCP2 = 0x56` (MUD Client Compression Protocol v.2) based on user-provided values.
- Added a comment `/* Options 50-137 (Bytes 0x32-0x89) Unassigned by IANA */` in the `TelnetOption` enumeration to document the unassigned range between `FORWARD_X` and `TELOPT_PRAGMA_LOGON`.
- Added a comment `/* Options 141-254 (Bytes 0x8D-0xFE) Unassigned by IANA */` in the `TelnetOption` enumeration to document the unassigned range between `TELOPT_PRAGMA_HEARTBEAT` and `EXTENDED_OPTIONS_LIST`.
- Added a private nested class template `InputProcessor` in `Socket` with template parameters `MutableBufferSequence` and `CompletionHandler`, including a public static `create()` method using perfect forwarding with `std::make_shared`, and declared `std::shared_ptr<InputProcessor>` as a friend class to enforce shared-pointer-only instantiation.

## [0.0.10] - September 12, 2025
### Changed
- Refactored `ProtocolFSM` constructor in `telnet.cppm` by introducing a new `static initialize()` method to handle registration of default options and handlers, called from the constructor to centralize initialization logic and improve maintainability.
- Removed redundant mutex lock from `ProtocolFSM` constructor, relying on `initialize()` and downstream methods for thread safety.
- Removed `std::lock_guard` from `initialize()` and replaced `option_registry_.insert(...)` calls with `register_option(...)` to leverage its existing thread-safe locking, correcting the previous refactoring approach.
- Bumped version to 0.0.10 across `telnet.cppm` to reflect the constructor and locking refactoring.

## [0.0.9] - September 12, 2025
### Added
- Added `sync_await` private utility method in `Socket` to synchronously await `asio::awaitable` operations, using a temporary `io_context` and `jthread` to capture results or exceptions, supporting both `void` and non-`void` return types.

### Changed
- Refactored synchronous methods (`read_some`, `write_some`, `write_command`, `write_negotiation`, `write_subnegotiation`) to centralize logic:
  - Moved core implementation to asynchronous counterparts (`async_read_some`, `async_write_some`, `async_write_command`, `async_write_negotiation`, `async_write_subnegotiation`).
  - Updated throwing synchronous overloads to use `sync_await` with `asio::use_awaitable`, returning the result directly and throwing `std::system_error` on error, with parameters forwarded using `std::forward`.
  - Reimplemented `noexcept` synchronous overloads to wrap the throwing overloads, catching `std::system_error`, `std::bad_alloc`, and unexpected exceptions, setting the `std::error_code&` out-parameter and returning `0` on failure, also using `std::forward` for parameters.
- Bumped version to 0.0.9 across `telnet_socket.cppm` to reflect the major refactoring of synchronous method implementations.
- Added temporary handling of `negotiation_response` in `Socket::process_telnet_input`: checks if `negotiation_response` is not `std::nullopt`, unpacks the `TelnetCommand` and `TelnetOption`, and uses `asio::co_spawn` to initiate `async_write_negotiation` on a detached thread, serving as a placeholder until a more integrated solution is implemented.
- Removed deprecated `process_byte__broken` and `send_negotiation_response` methods from `ProtocolFSM` in `telnet.cppm`, simplifying the API and relying on the updated `process_byte` and `async_write_negotiation` implementations.

### Fixed
- Corrected the return type of `async_*` methods with `asio::use_awaitable` to `asio::awaitable<std::size_t>`, aligning with Boost.Asio’s default behavior of throwing `std::system_error` on error, and removed redundant tuple unpacking in throwing overloads.
- Ensured consistent parameter forwarding in all synchronous methods to optimize for rvalues and move-only types.
- Fixed `Socket::async_write_some` to call the completion handler with the error code and `0` bytes written when `escape_telnet_output` returns an error, ensuring proper error propagation instead of leaving the handler uninvoked.

## [0.0.8] - September 11, 2025
### Added
- Added input validation in `Socket::async_write_subnegotiation` to check if the Telnet option is enabled via FSM, returning `invalid_argument` if not.
- Wrapped vector operations in `Socket::async_write_subnegotiation` with `try...catch` to handle memory allocation errors, mapping `bad_alloc` to `not_enough_memory` and other exceptions to `operation_aborted`.
- Renamed tuple unpacking in `Socket::async_write_subnegotiation` from `[ec, result]` to `[ec, _]` to indicate the second element is discarded.
- Added synchronous `Socket::read_some` and `Socket::write_some` methods with `noexcept` behavior, taking an `error_code&` out-parameter. Implemented as wrappers around `async_read_some` and `async_write_some` using a local `io_context` and `io_context::run()` to block until completion, preserving existing async logic without duplication.
- Added synchronous `Socket::write_command`, `write_negotiation`, and `write_subnegotiation` methods with `noexcept` behavior, taking an `error_code&` out-parameter. Implemented as wrappers around `async_write_command`, `async_write_negotiation`, and `async_write_subnegotiation` using a local `io_context` and `io_context::run()` to block until completion, preserving existing async logic without duplication.
- Added overloads for `Socket::read_some`, `write_some`, `write_command`, `write_negotiation`, and `write_subnegotiation` without `error_code&` out-parameters. These overloads forward to the `error_code&` versions, throwing `std::system_error` with the error code on failure, providing an exception-based interface while reusing existing synchronous implementations.

### Changed
- Updated `Socket::async_write_subnegotiation` to reserve buffer space, append `IAC SB opt`, escape the subnegotiation data, and append `IAC SE` in a single cohesive flow.
- Bumped version to 0.0.8 across `telnet.cppm` and `telnet_socket.cppm` to reflect the completion of subnegotiation implementation and error handling enhancements.
- Renamed `ProtocolFSM::process_byte` to `process_byte__broken` and `ProtocolFSM::async_process_byte` to `process_byte`, making the latter the primary method suitable for both synchronous and asynchronous flows after decoupling the `socket` parameter. Updated `process_byte` to return a `std::tuple<std::error_code, bool, std::optional<std::tuple<TelnetCommand, TelnetOption>>>` to include a negotiation response option.
- Updated `ProtocolFSM::make_negotiation_response` to be `const noexcept`, removed the unused `socket` parameter, and adjusted it to return a `std::tuple<TelnetCommand, TelnetOption>` for use in the new `process_byte` return tuple.
- Modified `ProtocolFSM::process_byte` (now `process_byte__broken`) and `process_byte` to remove the `socket` parameter from handler invocations (e.g., `unknown_command_handler_`, `unknown_option_handler_`), allowing users to capture `socket` in lambdas for custom responses.

### Fixed
- Ensured `async_write_subnegotiation` uses the out-parameter approach for `escape_telnet_output` without unnecessary buffer copies.
- Fixed `Socket::async_write_some` to capture the `escaped_data` vector by `std::move` instead of by reference, preventing a dangling reference when the stack-allocated vector goes out of scope.

### Deprecated
- Marked `ProtocolFSM::process_byte__broken` as deprecated, recommending the use of `process_byte` for a robust implementation suitable for both synchronous and asynchronous flows.
- Marked `ProtocolFSM::send_negotiation_response` as deprecated, suggesting the use of the `response` from `process_byte` with `make_negotiation_response` to handle negotiation responses asynchronously.

## [0.0.7] - September 10, 2025
- Bumped version to 0.0.7 to reflect recent enhancements.
- Updated `async_read_some` and `process_telnet_input` in `telnet_socket` to use `noexcept`, improving performance by eliminating exception handling overhead.
- Modified `process_telnet_input` to return a `std::tuple<std::error_code, std::size_t>` (mimicking Boost.Asio’s `[ec, bytes_transferred]` pattern) instead of throwing `std::logic_error`, with `operation_aborted` returned for invariant violations (e.g., write iterator exceeding buffer end).
- Enhanced `async_read_some` to propagate the tuple result directly, removing try-catch and `asio::post` for efficiency while maintaining thread safety with `bind_executor`.
- Integrated `buffers_iterator` in `process_telnet_input` to treat the `MutableBufferSequence` as a continuous range, resolving non-contiguous buffer handling for Telnet processing.
- Updated documentation to reflect the new return type and `noexcept` behavior in `telnet_socket`.
- Updated `ProtocolFSM::process_byte` to be `noexcept`, replacing the `std::logic_error` throw for invalid states with a `std::tuple<std::error_code, bool>` return (e.g., `{operation_aborted, false}`). The method now resets to `Normal` state and returns an error code on unreachable states, aligning with the tuple-based error handling strategy.
- Renamed the `state_` member of `ProtocolFSM` to `current_state_` for improved clarity in representing the current FSM state.
- Reworked `Socket::send_telnet_response` into `async_send_telnet_response` to support asynchronous operation with completion handling. Updated to use `next_layer_` instead of `lowest_layer_` for I/O to respect protocol layering, and added IAC escaping for any `ConstBufferSequence`, improving flexibility and error management.
- Separated IAC escaping from writing by introducing a templated `Socket::escape_telnet_output`, which takes a `ConstBufferSequence` constrained by `asio::const_buffer_sequence` and returns a vector with 0xFF bytes duplicated. Updated `async_send_telnet_response` to use this method, maintaining use of `next_layer_` for I/O to respect protocol layering. Simplified the escaping loop to use `asio::buffers_begin` and `asio::buffers_end` with `buffers_iterator` for direct byte-wise iteration.
- Renamed `async_send_telnet_response` to `async_write_some` to align with Boost.Asio conventions, maintaining Telnet-specific IAC escaping via `escape_telnet_output` and use of `next_layer_` for I/O to respect protocol layering. Separated IAC escaping from writing with `escape_telnet_output`, which uses `asio::buffers_begin` and `asio::buffers_end` with `buffers_iterator` for direct byte-wise iteration.
- Added `Socket::async_write_command` to asynchronously send a Telnet command by constructing a buffer with {IAC, static_cast<uint8_t>(cmd)} and using `next_layer_` for I/O to respect protocol layering.
- Added an overload for `Socket::async_write_command` that takes a `TelnetCommand` and a `TelnetOption`, constructing a buffer with {IAC, static_cast<uint8_t>(cmd), static_cast<uint8_t>(opt)} and using `next_layer_` for I/O to respect protocol layering. Added validation to set the error code to `std::errc::invalid_argument` if the command is not WILL, WONT, DO, or DONT.
- Added an overload for `Socket::async_write_negotiation` that takes a `TelnetCommand` and a `TelnetOption`, constructing a buffer with {IAC, static_cast<uint8_t>(cmd), static_cast<uint8_t>(opt)} and using `next_layer_` for I/O to respect protocol layering. Added validation to set the error code to `std::errc::invalid_argument` if the command is not WILL, WONT, DO, or DONT.
- Added a stub for `Socket::async_write_subnegotiation` that takes a `TelnetOption` and a `const std::vector<uint8_t>& subnegotiation_buffer`, currently a no-op with a TODO for implementing proper subnegotiation framing (IAC SB, option, data, IAC SE) and additional infrastructure.
- Moved `Socket::escape_telnet_output` to the private section to restrict it as an internal utility method for Telnet-specific IAC escaping.
- Added a private overload for `Socket::escape_telnet_output` that takes a `std::vector<uint8_t>& escaped_data` and a `const ConstBufferSequence&` parameter, modifying and returning a reference to the provided vector with escaped data, throwing std::bad_alloc on memory allocation failure. Refactored the original `escape_telnet_output` overload to delegate to the new overload after reserving capacity, returning an empty vector if memory allocation fails. Ensured `async_write_some` uses the original overload that creates a new buffer, reserving it for general use.
- Modified `Socket::escape_telnet_output` overloads to return `std::tuple<std::error_code, std::vector<uint8_t>>` for the original overload and `std::tuple<std::error_code, std::vector<uint8_t>&>` for the out-parameter overload, handling std::bad_alloc with not_enough_memory and unexpected exceptions with operation_aborted error codes. Declared both overloads noexcept after adding catch(...) blocks. Updated `async_write_some` to use the original overload, checking the error code and proceeding only on success. Ensured the out-parameter overload is reserved for subnegotiation use.
- Modified `Socket::escape_telnet_output` overloads to return `std::tuple<std::error_code, std::vector<uint8_t>>` for the original overload and `std::tuple<std::error_code, std::vector<uint8_t>&>` for the out-parameter overload, handling std::bad_alloc with not_enough_memory and unexpected exceptions with operation_aborted error codes. Optimized the original overload to return the tuple directly from the delegation call, eliminating intermediate variables. Declared both overloads noexcept after adding catch(...) blocks. Updated `async_write_some` to use the original overload, checking the error code and proceeding only on success. Ensured the out-parameter overload is reserved for subnegotiation use. Fixed typo in `process_telnet_input` loop iteration.
- Added three accessor methods to `ProtocolFSM`: `is_locally_enabled(TelnetOption)`, `is_remotely_enabled(TelnetOption)`, and `is_enabled(TelnetOption)`. The first two check local and remote enablement states respectively, returning false for unregistered options. The third returns the logical OR of the first two, providing a combined enablement check. All methods are marked noexcept.
- Removed `noexcept` from `Socket::async_read_some` to reflect potential exceptions from Boost.Asio's completion token implementation.

## [0.0.5/0.0.6] - September 09, 2025
- Bumped version to 0.0.6 (and initially 0.0.5) to reflect ongoing development and updates.
- Documented the decision to use `std::set` for the `option_registry_` set in `ProtocolFSM` instead of `std::unordered_map`.
  - Rationale: The `TelnetOption` enumeration currently contains 50 options, providing acceptable O(log n) lookup performance with `std::set`. The choice is based on the infrequent addition of new Telnet options over the past 20 years (e.g., RFC 2840 from 2000 being the last major update), suggesting that significant growth (e.g., to 100+ options) is unlikely. This avoids the memory and initialization overhead of `std::unordered_map`, which would only be justified with a much larger set. A future review is recommended if the option count exceeds 75.
- Enhanced thread safety in `ProtocolFSM`:
  - Added a single static `std::mutex` (`fsm_mutex_`) to protect static members (`command_handlers_`, `unknown_command_handler_`, `option_registry_`) during write operations (e.g., `register_handler`, `register_option`).
  - Implemented `std::lock_guard` in registration methods and the constructor to ensure thread safety, with negligible overhead in a single-threaded context.
  - Documented the mutex usage, noting that reads (e.g., during `process_byte`) are assumed immutable post-initialization and do not require locking.
- Refactored option enablement state management in `ProtocolFSM`:
  - Removed `is_locally_enabled_` and `is_remotely_enabled_` from the `Option` class to address mutability issues in `std::set`.
  - Added per-instance `std::map<TelnetOption, bool> local_enabled_` and `std::map<TelnetOption, bool> remote_enabled_` to track option enablement state per connection.
  - Updated `process_byte` to use these maps for state management, ensuring per-session negotiation aligns with Telnet’s RFC 854 behavior.
  - Documented the separation of option definitions (`option_registry_`) from enablement state, using `TelnetOption` as a consistent key.
- Optimized subnegotiation buffer management in `ProtocolFSM`:
  - Pre-reserved `subnegotiation_buffer_` with `(MAX_SUBNEGOTIATION_SIZE / 8)` (128 bytes) in the constructor to limit reallocations for small to medium subnegotiation streams.
  - Retained the dynamic `std::vector<uint8_t>` approach, allowing growth up to `MAX_SUBNEGOTIATION_SIZE` (1024 bytes) while minimizing memory waste.
  - Added a `TODO` to revisit the subnegotiation buffer strategy (e.g., size, type) when implementing options for specific use cases like NAWS or MSDP.
- Deferred virtual methods in `Option`:
  - Retained the current callback-based design (`local_predicate`, `remote_predicate`, `subnegotiation_handler_`) as sufficiently flexible.
  - Added a `TODO` to revisit virtual methods (e.g., `OptionBase` with `handle_subnegotiation`) if custom option handlers require complex logic or inheritance.
- Deferred multiple subnegotiation handlers in `Option`:
  - Kept the single `subnegotiation_handler_` approach, leveraging lambdas for flexible processing and data passing to other contexts.
  - Added a `TODO` to consider `std::vector<SubnegotiationHandler>` if distinct, independent actions cannot be combined into a single lambda.
- Refactored `IAC` state handling in `ProtocolFSM::process_byte`:
  - Streamlined the command handling logic by setting `current_command_` with a cast and using a `switch` statement.
  - Handled `WILL`, `WONT`, `DO`, `DONT` with a fallthrough to `OptionNegotiation`, `SB` to `Subnegotiation`, and `SE` as an error to `Normal`.
  - Implemented a default case to search `command_handlers_`, fall back to `unknown_command_handler_` if present, or discard and return to `Normal` if absent.
  - Ensured the command byte is discarded with `return false`.
  - Added `[[fallthrough]]` annotations to document intentional fallthrough from `WILL`, `WONT`, and `DO` to `DONT`.
- Renamed static members in `ProtocolFSM` for clarity:
  - Changed `options_` to `option_registry_` to better reflect its role as a registry of implemented options.
  - Changed `default_handler_` to `unknown_command_handler_` to clarify its purpose as a fallback for unhandled commands.
  - Updated corresponding setter methods (`set_default_handler` to `set_unknown_command_handler`) and documentation.
- Added `unknown_option_handler_` to `ProtocolFSM`:
  - Introduced a static `UnknownOptionHandler` type alias and member, defaulting to a logging lambda (`std::cout << "Unknown option: " << value << "\n"`) similar to `unknown_command_handler_`.
  - Added `set_unknown_option_handler` method with `std::lock_guard` for thread safety.
  - Integrated the handler in `OptionNegotiation` to log unknown options before rejecting them with a `DONT` or `WONT` response.
- Improved state consistency in `ProtocolFSM`:
  - Modified `change_state` to reset `current_command_`, `current_option_`, and `subnegotiation_buffer_` when transitioning to `Normal` state.
  - Removed explicit resets from individual cases (e.g., `OptionNegotiation`, `Subnegotiation`, `SubnegotiationIAC`) to centralize cleanup, enhancing robustness and reducing code duplication.
  - Omitted empty entry/exit action branches in `change_state` to simplify the implementation.
- Enhanced state machine robustness in `ProtocolFSM`:
  - Added a `default:` case in `switch (state_)` to throw `std::logic_error` for invalid states, improving error detection.
- Optimized `process_byte` in `ProtocolFSM`:
  - Removed redundant `return false` after `switch (state_)`, as all cases now terminate with a return or throw, improving code clarity and eliminating unnecessary execution paths.
- Ensured RFC 854 compliance in `ProtocolFSM`:
  - Explicitly initialized `state_` to `ProtocolState::Normal` in the constructor, aligning with RFC 854's initial state requirement.
  - Removed `@warning` about uninitialized `state_`, as the default initialization eliminates this risk.
- Improved documentation for maintainability in `ProtocolFSM`:
  - Enhanced class-level documentation with expanded `@note` on initialization and thread safety, and a detailed `@example` including option registration.
  - Added comprehensive `@param`, `@return`, `@throws`, and `@note` tags to methods (`process_byte`, `change_state`, `send_negotiation_response`).
  - Documented handler types (`TelnetCommandHandler`, `UnknownOptionHandler`) with usage examples and thread safety notes.
  - Added constraints and edge case guidance to registration and setter methods.
- Added placeholder implementation for `send_negotiation_response`:
  - Included a basic synchronous response mechanism with documentation, pending replacement with actual async I/O logic.
- Integrated actual `send_negotiation_response` implementation:
  - Replaced the placeholder with an async implementation using `asio::async_write`, adhering to RFC 854 negotiation responses.
  - Updated documentation to reflect async behavior, potential `std::system_error` throws, and socket lifetime warnings.

## [0.0.4] - September 07, 2025
- Initial implementation of `ProtocolFSM` with basic state machine logic.
- Added support for core Telnet commands (NOP, AYT) with default handlers.
- Implemented option negotiation framework with `Option` class.
- Introduced `telnet_socket` integration for I/O operations.

## [0.0.3] - September 07, 2025
- Added support for subnegotiation states (Subnegotiation, SubnegotiationIAC).
- Implemented buffer management for subnegotiation data with `MAX_SUBNEGOTIATION_SIZE`.
- Fixed edge case handling for IAC escaping in normal data.

## [0.0.2] - September 06, 2025
- Introduced `TelnetSocketConcept` to enforce socket compatibility.
- Added initial set of Telnet options (BINARY, ECHO, SUPPRESS_GO_AHEAD, NAWS).
- Implemented basic command handling with `command_handlers_` map.

## [0.0.1] - TBD
- Initial skeleton of the `telnet` module.
- Defined `TelnetCommand` and `TelnetOption` enumerations.
- Created basic structure for `ProtocolFSM` class. for `ProtocolFSM` class.