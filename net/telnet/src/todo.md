# Telnet Project TODO
## Version: 0.4.0
## Date: October 03, 2025
## Purpose: This document compiles all `@todo` tasks from the Telnet project files, organized by phase, to guide development in Phase 5 and beyond. The tasks focus on enhancing option handling, negotiation, and socket operations, building on the modular structure defined in `telnet.cppm`.

## Development Plan

### Overview
The Telnet project (version 0.4.0) has completed Phase 4, achieving protocol stability through RFC 1143 compliance and enhanced logging with `std::format`. Phase 5 focuses on improving option handling with enablement/disablement handlers, user-initiated negotiation, and optimized data structures. Phase 6 targets socket compatibility and interface cleanup, while Future Development addresses long-term maintenance and optimization. The `telnet-socket-sync-impl.cpp` file, critical for synchronous wrappers, supports Phase 5 negotiation tasks and Phase 6 TLS support, with potential `sync_await` optimization noted for Future Development.

### Phase 5 Milestones
1. **Review Options List**:
   - **Task**: Review `option::id_num` list for completeness against IANA Telnet Option Registry and RFCs.
   - **Steps**:
     - Cross-reference `option::id_num` in `telnet-options.cppm` with IANA Telnet Option Registry and RFCs (e.g., RFC 855, RFC 856).
     - Identify missing options (e.g., proprietary extensions like `MSDP`, `MCCP2`) or obsolete entries.
     - Update `:options` with new entries and test integration with `:protocol_fsm` (implemented in `telnet-protocol_fsm-impl.cpp`), `:socket` (implemented in `telnet-socket-impl.cpp`, `telnet-socket-async-impl.cpp`, `telnet-socket-sync-impl.cpp`), and `:internal`.
     - Ensure `:errors` covers new error cases (e.g., `option_not_available`).
   - **Dependencies**: Affects `:options`, `:protocol_fsm`, `:socket`, `:internal`, `:errors`.
   - **Priority**: High (ensures comprehensive option support).
   - **Estimated Effort**: 2–3 days (1 for review, 1–1.5 for updates, 0.5–1 for testing).

2. **Review Option Implementation Strategy**:
   - **Task**: Determine which options require internal `ProtocolFSM` implementation vs. library extensions.
   - **Steps**:
     - Analyze options (e.g., `BINARY`, `ECHO`, `NAWS`) for complexity and customization needs.
     - Decide which options (e.g., `BINARY`, `ECHO`) should be implemented internally for performance vs. externally for flexibility.
     - Update `:options`, `:protocol_fsm` (implemented in `telnet-protocol_fsm-impl.cpp`), and `:internal` (for `OptionHandlerRegistry`) to support the chosen model.
     - Test with `:socket` negotiation methods (implemented in `telnet-socket-impl.cpp`, `telnet-socket-async-impl.cpp`, `telnet-socket-sync-impl.cpp`) and `:errors` (e.g., `user_handler_forbidden`).
   - **Dependencies**: Affects `:options`, `:protocol_fsm`, `:socket`, `:internal`, `:errors`.
   - **Priority**: High (defines option handling architecture).
   - **Estimated Effort**: 2–3 days (1 for analysis, 1–1.5 for implementation, 0.5–1 for testing).

3. **Enhance OptionHandlerRecord**:
   - **Task**: Add enablement and disablement handlers to `OptionHandlerRecord`.
   - **Steps**:
     - Define handler types (e.g., `EnablementHandler`, `DisablementHandler`) for `WILL`, `DO`, `WONT`, `DONT` events in `telnet-internal.cppm`.
     - Update `OptionHandlerRegistry` to store and invoke these handlers.
     - Integrate with `:protocol_fsm` (implemented in `telnet-protocol_fsm-impl.cpp`) for state transitions and `:options` for option validation.
     - Test with negotiation scenarios using `:socket` (implemented in `telnet-socket-impl.cpp`, `telnet-socket-async-impl.cpp`, `telnet-socket-sync-impl.cpp`) and `:errors` (e.g., `invalid_negotiation`).
   - **Dependencies**: Affects `:internal`, `:protocol_fsm`, `:options`, `:socket`, `:errors`.
   - **Priority**: High (enhances option state management).
   - **Estimated Effort**: 2–3 days (0.5–1 for handler definition, 1 for integration, 0.5–1 for testing).

4. **Add Shared Mutex to OptionHandlerRegistry**:
   - **Task**: Add a `std::shared_mutex` to `OptionHandlerRegistry` if registration is not confined to initialization time.
   - **Steps**:
     - Evaluate whether `OptionHandlerRegistry` in `telnet-internal.cppm` allows runtime registration (e.g., post-initialization updates).
     - Add `std::shared_mutex` to protect `handlers_` map for concurrent access, supporting shared reads and exclusive writes.
     - Update `get_handler`, `set_handler`, and other methods to use shared/exclusive locks as needed.
     - Test thread safety with concurrent registration and handler invocation in `:protocol_fsm` and `:socket`.
     - Update documentation in `telnet-internal.cppm` to reflect thread-safety guarantees.
   - **Dependencies**: Affects `:internal`, `:protocol_fsm`, `:socket`.
   - **Priority**: Medium (ensures thread safety if runtime registration is needed).
   - **Estimated Effort**: 2–3 days (0.5 for evaluation, 1–1.5 for implementation, 0.5–1 for testing).

5. **Restructure Option Negotiation Results for Handlers**:
   - **Task**: Invoke enablement/disablement handlers from `OptionHandlerRecord` after state transitions.
   - **Steps**:
     - Modify `ProtocolFSM::handle_state_option_negotiation` in `telnet-protocol_fsm-impl.cpp` to call handlers post-transition.
     - Update `ProcessingReturnVariant` to include handler invocation results.
     - Test with options like `ECHO` and `NAWS` using `:socket` (implemented in `telnet-socket-impl.cpp`, `telnet-socket-async-impl.cpp`, `telnet-socket-sync-impl.cpp`).
   - **Dependencies**: Depends on `:internal` (handler enhancements), `:options`, `:protocol_fsm`, `:socket`, `:errors`.
   - **Priority**: High (enhances flexibility for option-specific behavior).
   - **Estimated Effort**: 2–3 days (1 for implementation, 1–1.5 for testing, 0.5 for documentation).

6. **Restructure Option Negotiation for Internal Implementation**:
   - **Task**: Implement core options (e.g., `BINARY`, `ECHO`) directly in `ProtocolFSM`.
   - **Steps**:
     - Identify core options for internal implementation to optimize performance.
     - Update `handle_state_option_negotiation` in `telnet-protocol_fsm-impl.cpp` to handle these options directly, bypassing external handlers.
     - Test performance and correctness with `:socket` (implemented in `telnet-socket-impl.cpp`, `telnet-socket-async-impl.cpp`, `telnet-socket-sync-impl.cpp`) and `:options`.
   - **Dependencies**: Depends on `:options` (option review), `:internal`, `:protocol_fsm`, `:socket`, `:errors`.
   - **Priority**: High (improves performance for core options).
   - **Estimated Effort**: 2–3 days (1 for implementation, 1–1.5 for testing, 0.5 for documentation).

7. **Option Registry Optimization**:
   - **Task**: Optimize `option_registry` for large option sets by evaluating `std::set` vs. `std::vector`.
   - **Steps**:
     - Benchmark `std::set` lookup (O(log n)) in `option_registry` (`telnet-options.cppm`) for typical option counts (10–50).
     - Evaluate `std::vector` with pre-filled indices (O(1) lookup) for up to 256 options.
     - Update `option_registry` if needed and test with `:protocol_fsm` and `:socket`.
   - **Dependencies**: Affects `:options`, `:protocol_fsm`, `:socket`.
   - **Priority**: Medium (performance optimization, depends on option review).
   - **Estimated Effort**: 2–3 days (1 for benchmarking, 1 for implementation, 0.5–1 for testing).

8. **Option Negotiation Method**:
   - **Task**: Add a method for user code to request/offer options, validating state and sending negotiation sequences.
   - **Steps**:
     - Add `request_option` to `telnet::socket` in `telnet-socket.cppm`, validating via `ProtocolFSM` and sending IAC `WILL`/`DO` using `write_negotiation` or `async_write_negotiation`.
     - Use `OptionStatusRecord` (`:internal`) and `option_registry` (`:options`) for state and option lookup.
     - Handle errors with `:errors` (e.g., `invalid_negotiation`, `option_not_available`).
     - Test with options like `ECHO`, `NAWS` per RFC 855 using `InputProcessor` and `escape_telnet_output`.
   - **Dependencies**: Requires `:protocol_fsm`, `:options`, `:types`, `:errors`, `:internal`, `:socket`.
   - **Priority**: High (core feature for Telnet protocol).
   - **Estimated Effort**: 3–4 days (1 for implementation, 1.5–2 for testing, 0.5 for documentation).

9. **Async Negotiation Awaiting**:
   - **Task**: Support asynchronous awaiting of negotiation responses as a composed operation or via callbacks.
   - **Steps**:
     - Decide between `asio::async_compose` and callback-based approach.
     - Integrate with `ProtocolFSM` and `OptionStatusRecord` to track responses (e.g., IAC `WONT`/`DONT`) using `async_read_some` and `read_some`.
     - Use `:options` for validation and `:errors` for response errors.
     - Test with negotiation scenarios (e.g., successful/rejected options).
     - Update documentation in `telnet-socket.cppm` and `:protocol_fsm`.
   - **Dependencies**: Depends on option negotiation method, `:protocol_fsm`, `:socket`, `:types`, `:options`, `:errors`, `:internal`.
   - **Priority**: Medium (enhances usability, depends on negotiation method).
   - **Estimated Effort**: 3–4 days (1 for design, 1.5–2 for implementation/testing, 0.5 for documentation).
   - **Notes**: Evaluate trade-offs between composed async operations (integrated) vs. callbacks (flexible).

### Phase 6 Milestones
- **Refine Socket Constraints**:
   - **Task**: Hand-tune `TelnetSocketConcept` to match Boost.Asio stream socket requirements.
   - **Steps**:
     - Analyze `boost::asio::ip::tcp::socket` requirements (e.g., `async_read_some`, `async_write_some`, `get_executor`).
     - Update `TelnetSocketConcept` in `telnet-socket.cppm` to remove over-constraints or add missing requirements.
     - Test with Boost.Asio TCP sockets using `next_layer_` in `:socket` implementation files.
   - **Dependencies**: Affects `:socket` (implemented in `telnet-socket-impl.cpp`, `telnet-socket-async-impl.cpp`, `telnet-socket-sync-impl.cpp`).
   - **Priority**: High (ensures socket compatibility before TLS support).
   - **Estimated Effort**: 2–3 days (1 for analysis, 1–1.5 for implementation, 0.5 for testing).

- **TLS Support**:
   - **Task**: Add `TLSTelnetSocketConcept` for conditional TLS support.
   - **Steps**:
     - Define `TLSTelnetSocketConcept` for `boost::asio::ssl::stream`.
     - Modify `telnet::socket` to handle TLS operations (e.g., handshake) via templates or runtime checks.
     - Use `:errors` for TLS errors (e.g., `internal_error` for handshake failures).
     - Test with TCP and SSL sockets for backward compatibility.
     - Update documentation in `telnet-socket.cppm`.
   - **Dependencies**: Depends on refined `TelnetSocketConcept`, `:socket`, `:errors`.
   - **Priority**: Medium (enhances security, optional for core functionality).
   - **Estimated Effort**: 4–5 days (1 for design, 2–2.5 for implementation, 1–1.5 for testing).

- **Remove OptionHandlerRegistry Operator**:
   - **Task**: Remove `OptionHandlerRegistry::operator[]` if no use cases found.
   - **Steps**:
     - Analyze usage of `operator[]` in `:protocol_fsm` and other components.
     - Remove both overloads from `telnet-internal.cppm` if unused.
     - Update `:protocol_fsm` and `:options` if needed.
     - Test to ensure no functionality breaks.
   - **Dependencies**: Affects `:internal`, `:protocol_fsm`, potentially `:options`, `:socket`.
   - **Priority**: Medium (interface cleanup, depends on use case analysis).
   - **Estimated Effort**: 1–2 days (0.5 for analysis, 0.5–1 for implementation, 0.5 for testing).

- **Move ProtocolFSMConfig**:
   - **Task**: Move `ProtocolFSMConfig` and `DefaultProtocolFSMConfig` to a `:concepts` or `:protocol_config` partition.
   - **Steps**:
     - Create a new partition in `telnet-protocol_fsm.cppm`.
     - Move `ProtocolFSMConfig` concept and `DefaultProtocolFSMConfig` class.
     - Update import/export statements in `:protocol_fsm` and `:telnet`.
     - Test compatibility with existing `ProtocolFSM` usage.
   - **Dependencies**: Affects `:protocol_fsm`, `:telnet`.
   - **Priority**: Medium (improves modularity).
   - **Estimated Effort**: 1–2 days (0.5 for refactoring, 0.5–1 for testing, 0.5 for documentation).

- **Review Subnegotiation Overflow Handling**:
   - **Task**: Enhance `subnegotiation_overflow` handling for custom error handlers.
   - **Steps**:
     - Update `ProtocolFSM::handle_state_subnegotiation` and `handle_state_subnegotiation_iac` in `telnet-protocol_fsm-impl.cpp` to support custom error handlers.
     - Modify `ProtocolConfig::log_error` to invoke custom handlers.
     - Test with large subnegotiation buffers.
   - **Dependencies**: Affects `:protocol_fsm`, `:errors`, `:internal`.
   - **Priority**: Medium (enhances error handling flexibility).
   - **Estimated Effort**: 1–2 days (0.5 for implementation, 0.5–1 for testing, 0.5 for documentation).

### Future Development Milestones
- **Module Export Cleanup**:
   - **Task**: Ensure internal types (e.g., `OptionHandlerRegistry`, `ProtocolFSM`) are not exported.
   - **Steps**:
     - Audit exported partitions (`:types`, `:errors`, `:options`, `:protocol_fsm`, `:socket`, `:internal`).
     - Move internal types to non-exported partitions or implementation files.
     - Update `telnet.cppm` and test encapsulation.
   - **Dependencies**: Affects all partitions.
   - **Priority**: Low (optimization, deferred until after Phase 5/6).
   - **Estimated Effort**: 2–3 days (1 for audit, 1 for refactoring, 0.5–1 for testing).

- **Switch to `std::byte`**:
   - **Task**: Switch `byte_t` from `std::uint8_t` to `std::byte` when C++23 support improves.
   - **Steps**:
     - Update `byte_t` alias in `telnet-types.cppm`.
     - Refactor `:socket`, `:protocol_fsm`, `:options`, `:internal`.
     - Test compatibility with `TelnetCommand`, `option_registry`, `OptionHandlerRegistry`.
   - **Dependencies**: Affects `:socket`, `:protocol_fsm`, `:options`, `:internal`.
   - **Priority**: Low (depends on C++23 adoption).
   - **Estimated Effort**: 2–3 days (1 for refactoring, 1–1.5 for testing, 0.5 for documentation).

- **C++26 Reflection for Option Names**:
   - **Task**: Use C++26 reflection to auto-populate `option` names.
   - **Steps**:
     - Prototype reflection to generate `std::string` names for `option::id_num`.
     - Add customization point for user-set names.
     - Update `option` constructor and `make_option` in `telnet-options.cppm`.
     - Test with `:protocol_fsm`, `:socket`, `:internal`.
   - **Dependencies**: Affects `:options`, depends on C++26 support.
   - **Priority**: High/Deferred (improves logging, depends on C++26).
   - **Estimated Effort**: 2–3 days (1 for prototyping, 1–1.5 for implementation, 0.5 for testing).

- **Async Framework Compatibility**:
   - **Task**: Monitor `asio::async_result_t` compatibility with Boost.Asio and `std::execution`.
   - **Steps**:
     - Review Boost.Asio and C++ standard library updates quarterly.
     - Prototype async methods with C++ coroutines or `std::execution`.
     - Test with `:socket` async operations.
   - **Dependencies**: Affects `:socket`, indirectly synchronous methods via `sync_await`.
   - **Priority**: Low (long-term maintenance).
   - **Estimated Effort**: 3–4 days (1 for review, 1.5–2 for prototyping, 0.5–1 for testing).

- **Custom Executor Support**:
   - **Task**: Use `asio::get_associated_executor` for custom executors in `telnet::socket`.
   - **Steps**:
     - Update async methods in `:socket` to use `asio::get_associated_executor`.
     - Test with custom executors (e.g., thread pools).
     - Update documentation in `telnet-socket.cppm`.
   - **Dependencies**: Affects `:socket` async methods.
   - **Priority**: Low (enhances flexibility).
   - **Estimated Effort**: 2–3 days (1 for implementation, 1–1.5 for testing, 0.5 for documentation).

- **Half-Duplex Support**:
   - **Task**: Add optional half-duplex support per RFC 854 for legacy peers.
   - **Steps**:
     - Evaluate legacy peer requirements.
     - Update `ProtocolFSM` to handle half-duplex constraints (e.g., `SUPPRESS_GO_AHEAD`).
     - Test with legacy peers and update `:socket`.
   - **Dependencies**: Affects `:protocol_fsm`, `:socket`, potentially `:errors`.
   - **Priority**: Low (depends on legacy peer needs).
   - **Estimated Effort**: 2–3 days (1 for evaluation, 1–1.5 for implementation, 0.5 for testing).

- **Optimize Synchronous I/O Performance**:
   - **Task**: Reduce `sync_await` overhead in `telnet-socket-sync-impl.cpp`.
   - **Steps**:
     - Evaluate `sync_await` performance (new `io_context` and thread per call).
     - Explore reusing `io_context` or optimizing thread management.
     - Test synchronous operations under high load.
     - Update `sync_await` and synchronous methods.
   - **Dependencies**: Affects `:socket` (implemented in `telnet-socket-impl.cpp`, `telnet-socket-sync-impl.cpp`).
   - **Priority**: Low (deferred until after Phase 5/6).
   - **Estimated Effort**: 2–3 days (1 for evaluation, 1–1.5 for implementation, 0.5 for testing).

### Completed Tasks
- **Phase 4: Logging Enhancement** (Completed October 02, 2025)
  - Updated `ProtocolConfig::log_error` in `telnet-protocol_fsm.cppm` to use `std::format` with signature `static void log_error(const std::error_code& ec, std::format_string<auto> fmt, Args&&... args)`.
  - Modified all `log_error` calls in `telnet-protocol_fsm-impl.cpp` to use format strings (e.g., `"byte: 0x{:02x}, cmd: {}, opt: {}, dir: {}"`).
  - Removed old `log_error` method and renamed `NEW_log_error` to `log_error`.
  - Added `TelnetCommand` formatter in `telnet-types.cppm` (default: `name (0xXX)`, `{:n}` → `name`, `{:x}` → `0xXX`, invalid → `UNKNOWN`).
  - Added `option` formatter in `telnet-options.cppm` (default: `0xXX (name)`, `{:n}` → `name` or `unknown`, `{:x}` → `0xXX`).
  - Added `NegotiationDirection` formatter in `telnet-types.cppm` (formats as `local` or `remote`).
  - Replaced `"none"_sv` with `"N/A"_sv` across all `log_error` calls for `std::nullopt` cases.
  - Added Doxygen documentation for `std::formatter` specializations in `telnet-types.cppm` and `telnet-options.cppm`.
  - Ensured `:errors` includes necessary error codes (e.g., `invalid_negotiation`, `protocol_violation`, `subnegotiation_overflow`).
  - Tested format correctness and performance under typical and high-load scenarios.
- **Phase 4: RFC 1143 Compliance** (Completed October 03, 2025)
  - Implemented RFC 1143’s Q Method in `handle_state_option_negotiation` (`telnet-protocol_fsm-impl.cpp`), using `OptionStatusRecord` (`telnet-internal.cppm`) to track states (`NO`, `YES`, `WANTNO`, `WANTYES`) and queue bits.
  - Handled redundant requests (`WILL`/`DO` in `YES`, `WONT`/`DONT` in `NO`) with `error::invalid_negotiation` logs to prevent loops.
  - Ensured robustness against rapid renegotiation for options designed for frequent state changes.
  - Integrated with `:socket` for sending responses via `async_write_negotiation` (`telnet-socket-async-impl.cpp`) and `write_negotiation` (`telnet-socket-sync-impl.cpp`).
  - Added comprehensive Catch2 unit tests for all Q Method state transitions, edge cases (e.g., redundant requests, unregistered options), and rapid renegotiation scenarios.
  - Updated `README.md`, `telnet-protocol_fsm-impl.cpp`, `telnet-internal.cppm` to document RFC 1143 compliance and logging conventions.
  - Updated `undefined_subnegotiation_handler` in `telnet-internal.cppm` to log `cmd: SB (0xFA), opt: {}` (non-directional per RFC 854).

### Notes
- Version 0.5.0 marks the start of Phase 5, focusing on option handling and negotiation enhancements.
- Log formats use default `{}` specifiers for `TelnetCommand` (`name (0xXX)`), `option` (`0xXX (name)`), and `NegotiationDirection` (`local` or `remote`), with `"N/A"_sv` for `std::nullopt` cases.
- The new `shared_mutex` task for `OptionHandlerRegistry` ensures thread safety for runtime registration, aligning with Phase 5’s focus on robust option handling.
- Next focus: Detailed planning for Phase 5, prioritizing option review, handler enhancements, and user-initiated negotiation.
*Last updated: October 03, 2025*