<!-- SPDX-License-Identifier: Apache-2.0 -->
<!-- SPDX-FileCopyrightText: 2025-2026 Jeremy Murphy and any Contributors -->
# Telnet Project TODO
## Version: 0.5.7
## Date: October 30, 2025
## Purpose: This document compiles all `@todo` tasks from the Telnet project files, organized by phase, to guide development in Phase 6 and beyond. The tasks focus on enhancing stream compatibility, TLS support, and interface cleanup, building on the modular structure defined in `telnet.cppm`.

## Development Plan

### Overview
The Telnet project (version 0.5.0) has completed Phases 4 and 5, achieving protocol stability through RFC 1143 compliance, enhanced logging with `std::format`, urgent data/Synch procedure handling (RFC 854), advanced option handling with tagged awaitables, handler registration, and user-initiated negotiation. Phase 6 targets stream compatibility and interface cleanup, while Future Development addresses long-term maintenance and optimization. Phase 6 and later milestones are unordered, with changes requiring explicit approval.

### Phase 6 Milestones
01. [✔️] **Rename `socket` to `stream`** (Completed October 17, 2025):
  - **Task**: Rename `socket` and `:socket` to `stream` and `:stream` respectively.
  - **Steps**:
    - Renamed `socket` class to `stream` along with `NextLayerSocketT` to `NextLayerT`.
    - Renamed `:socket` to `:stream` and rename all files from telnet-socket to telnet-stream.
    - Updated all references in all files to use the new names.
    - Updated all references in this document to use the new names.
  - **Dependencies**: Affects practically everything.
  - **Priority**: Highest (core interface change)
  - **Estimated Effort**: Completed in 0.1 days.

02. [✔️] **Refactor `InputProcessor` Helper Methods (`std::visit`)** (Completed October 18, 2025):
   - **Task**: Refactor overloaded helper methods for `std::visit` within the `InputProcessor::operator()` response handling and add complete Doxygen documentation.
   - **Steps**:
     - Created `do_response` overloaded private methods for each alternative in `ProcessingReturnVariant`.
     - Consolidated `OptionEnablementAwaitable` and `OptionDisablementAwaitable` overloads into a single templated `do_response` using `TaggedAwaitable<Tag, T, Awaitable>` in `telnet-stream.cppm` and `telnet-stream-impl.cpp`.
     - Updated lambda in `std::visit` to dispatch to `do_response`.
   - **Priority**: High (code clarity, maintainability, documentation).
   - **Estimated Effort**: Completed in 1 day.
   
03. [✔️] **Refactor `InputProcessor` Helper Methods (Main Loop)** (Completed October 19, 2025):
  - **Task**: Refactor helper methods to simplify the main processing loop in `InputProcessor::operator()`.
  - **Steps**:
    - Separated `InputProcessor::operator()` into 3 state handlers.
    - Extracted `process_write_error` and `process_fsm_signal` from the main body of the `PROCESSING` state handler.
    - Simplified the `PROCESSING` state for better readability.
    - Reviewed other potential refactoring opportunities and determined the current form has the optimal balance.
  - **Dependencies**: Affects `:stream` (`InputProcessor`).
  - **Priority**: High (code clarity and maintainability).
  - **Estimated Effort**: Completed in 1 day.

04. [✔️] **Refine Stream Constraints** (Completed October 29, 2025):
  - **Task**: Hand-tune `TelnetSocketConcept` (now `LayeredSocketStream`) to match Boost.Asio stream socket requirements.
  - **Steps**:
    - Created a `:concepts` partition to hold type constraints.
    - Analyzed `boost::asio::ip::tcp::socket` interface as well as AsyncReadStream, AsyncWriteStream, SyncReadStream, and SyncWriteStream.
    - Implemented C++20 `concept`s for `AsioAsyncReadStream`, `AsioSyncReadStream`, `AsioAsyncWriteStream`, `AsioSyncWriteStream`, `ExecutorProvider`, `LayeredObject`, `EndpointProvider`, as well as several additional helper `concept`s culminating in `AsioSocket`, `AsioStreamSocket`, and `AsioLayerableStreamSocket`.
    - Implemented C++20 `concept`s for `AsioMutableBufferSequence`, `AsioConstBufferSequence`, and 4 types of `CompletionToken`.
    - Moved all of the above into a new `asio_concepts` primary module interface unit `../asio_concepts/asio_concepts.cppm` as it outgrew the bounds of this project.
    - Updated `LayeredSocketStream` from `telnet-stream.cppm` (moved to `:concepts`) to delegate to `asio_concepts::AsioLayerableStreamSocket`, requiring an `AsioStream` whose `lowest_layer_type` is also an `AsioStreamSocket`.
    - Tested with Boost.Asio TCP sockets using `next_layer_` in `:stream` implementation files.
    - Implemented **basic_stream_socket_wrapper** in **new partition** to ensure `std::error_code` and `std::system_error` interoperability when standalone Asio is not available.
  - **Dependencies**: Affects `:stream` (`telnet-stream-impl.cpp`, `telnet-stream-async-impl.cpp`, `telnet-stream-sync-impl.cpp`).
  - **Priority**: High (ensures stream compatibility before TLS support).
  - **Estimated Effort**: Completed in 10 days.

05. [✔️] **Move ProtocolFSMConfig** (Completed October 30, 2025):
  - **Task**: Move `ProtocolFSMConfig` and `DefaultProtocolFSMConfig` to a `:concepts` or `:protocol_config` partition.
  - **Steps**:
    - Created a new partition `:protocol_config` in `telnet-protocol_config.cppm`.
    - Moved `ProtocolFSMConfig` concept to `:concepts`.
    - Moved `DefaultProtocolFSMConfig` class to `:protocol_config`.
    - Updated import/export statements in `:protocol_fsm`.
    - Testd compatibility with existing `ProtocolFSM` usage.
  - **Dependencies**: Affects `:protocol_fsm`, `:telnet`.
  - **Priority**: Medium (improves modularity).
  - **Estimated Effort**: Completed in 0.5 days.

06. [✔️] **Remove OptionHandlerRegistry Operator** (Completed October 30, 2025):
  - **Task**: Remove `OptionHandlerRegistry::operator[]` if no use cases found.
  - **Steps**:
    - Analyzed usage of `operator[]` in `:protocol_fsm`, `:stream`, and other components (no callers).
    - Removed from `telnet-internal.cppm` to remove the function.
  - **Dependencies**: Affects `:internal`.
  - **Priority**: Medium (interface cleanup).
  - **Estimated Effort**: Completed in 0.5 days.

07. [❌️] **Implement Generic `spawn_handler`** (Closed NAD, October 30, 2025):
  - **Task**: Generalize the coroutine spawning mechanism (`spawn_handler`) to handle any `TaggedAwaitable<Tag, T>` type, enabling uniform async execution and result handling across handler categories.
  - **Steps**:
    - Define a templated `spawn_handler(Self&& self, TaggedAwaitable<Tag, T, Awaitable>&& task)` function that:
      - Uses `asio::co_spawn()` with `next_layer_.get_executor()` and preserves error propagation semantics.
      - Supports both `void` and non-`void` return types via `if constexpr` and `std::is_void_v<T>`.
      - Implements a generic `handle_response(Self&&, TaggedAwaitable<Tag, T, Awaitable>&&)` overload that forwards tagged awaitables to `spawn_handler`.
    - Ensure proper forwarding of `Self` (the completion handler) through move semantics.
    - Refactor existing handler launch sites (e.g., subnegotiation, option handlers) to use the unified `spawn_handler` instead of specialized inline `co_spawn` calls.
    - Add overloads or tag-based specializations if certain handler types need pre/post-processing.
  - **Dependencies**: Depends on **Implement Tagged Awaitables**; affects `:stream`, `:awaitables`, `:protocol_fsm`, `:internal`.
  - **Priority**: Medium (code reuse, consistency, and future extensibility).
  - **Estimated Effort**: Completed in 0 days. (Prior refactoring made this unnecessary/impractical.)
  
08. [ ] **Implement Tagged Predicates for telnet::option Local/Remote Enablement Predicates**
  - **Task**:
  - **Steps**:
    - Define a nested template class for `option::enable_predicate` to own the `std::function`.
    - Implement a function call operator to invoke the stored function.
    - Use tags for local and remote with `using` aliases to create distinct types for the local predicate and remote predicate.
    - Update constructor call sites to use the new types.
  - **Dependencies**: Affects `:options`, `:protocol_config`.
  - **Priority**: Medium (cleans up telnet::option constructor parameters)
  - **Estimated Effort**: 1 day.
  
09. [ ] **Investigate TaggedAwaitable Tags**:
  - **Task**: Investigate whether the tag types for TaggedAwaitable need to be declared or defined to be used.
  - **Steps**:
    - See if the tag types work as incomplete types or if they need definitions.
  - **Dependencies**: `:awaitables`
  - **Priority**: Low (curiosity and minor redundancy reduction)
  - **Estimated Effort**: 1 hour

10. [✔️] **Refine Socket Concepts for MutableBufferSequence, ConstBufferSequence, and CompletionToken Method Parameters**:
  - **Task**: Use the unevaluated immediately-invoked lambda trick from the Orcs/Heroes CPPCon24 talk to refine the socket concepts from the `net.asio_concepts` module.
  - **Steps**:
    - Identify concepts with templated member functions.
    - Use the IIFE lambda trick to model the template parameters in the concept definitions.
  - **Dependencies**: `net.asio_concepts` and `:concepts`
  - **Priority**: Medium (concept refinement is useful ahead of TLS implementation)
  - **Estimated Effort**: Completed in 1 day
  
### Phase 7 Milestones
01. [ ] **Implement a strand in `telnet::stream`**:
  - **Task**: Implement a strand as the executor type for `telnet::stream`.
  - **Steps**:
    - Change `telnet::stream::executor_type` to `asio::strand<typename next_layer_type::executor_type>`.
    - Create a `strand_executor_` member initialized in the `telnet::stream` constructor.
    - Change `telnet::stream::get_executor` to return the strand.
  - **Dependencies**: Affects `:stream`.
  - **Priority**: High (enhances protocol correctness and thread-safety by explicitly sequencing operations)
  - **Estimated Effort**: 1 day.
  
02. [ ] **Refactor Asynchronous Write Methods to Use the Side Buffer**:
  - **Task**: Implement an `OutputProcessor` class to manage writes through the side buffer, and send all writes through it.
  - **Steps**:
    - Implement an `OutputProcessor` class analogous to `InputProcessor`.
    - Refactor each asynchronous write method to use the `OutputProcessor` instead of direct calls to `next_layer_.async_write_some`.
    - Implement queueing within the `OutputProcessor` to coalesce pended writes into a single underlying call.
    - Implement a flag in `stream::context_` to prevent writes during the Abort Output command processing.
  - **Dependencies**: Affects `:stream`.
  - **Priority**: High (enhances safety by controlling buffer lifetimes)
  - **Estimated Effort**: 2 day.

03. [ ] **TLS Support**:
  - **Task**: Implement option `TELNET_START_TLS` using `LayeredTLSStream` for conditional TLS support.
  - **Steps**:
    - Define `LayeredTLSStream` for `boost::asio::ssl::stream`.
    - Modify `telnet::stream` to handle TLS operations (e.g., handshake) via templates or runtime checks.
    - Use `:errors` or `boost::asio::error` for TLS errors.
    - Test with TCP and SSL sockets for backward compatibility.
    - Update documentation in `telnet-stream.cppm`.
  - **Dependencies**: Depends on **Refine Socket Constraints**, `:stream`, `:errors`.
  - **Priority**: Medium (enhances security, optional for core functionality).
  - **Estimated Effort**: 4–5 days (1 for design, 2–2.5 for implementation, 1–1.5 for testing).

04. [ ] **Half-Duplex Support**:
  - **Task**: Add optional half-duplex support per RFC 854 for legacy peers.
  - **Steps**:
    - Evaluate legacy peer requirements.
    - Update `ProtocolFSM`, `stream`, and `stream::InputProcessor` to handle half-duplex constraints (i.e., `TelnetCommand::GA` handling).
    - Test with legacy peers and update `:stream`.
  - **Dependencies**: Affects `:protocol_fsm`, `:stream`, potentially `:errors`.
  - **Priority**: Low (depends on legacy peer needs).
  - **Estimated Effort**: 2–3 days (1 for evaluation, 1–1.5 for implementation, 0.5 for testing).

### Future Development Milestones
- [ ] **Module Export Cleanup**:
  - **Task**: Ensure internal types (e.g., `OptionHandlerRegistry`, `ProtocolFSM`) are not exported.
  - **Steps**:
    - Audit exported partitions (`:types`, `:errors`, `:options`, `:protocol_fsm`, `:stream`, `:internal`).
    - Move internal types to non-exported partitions or implementation files.
    - Update `telnet.cppm` and test encapsulation.
  - **Dependencies**: Affects all partitions.
  - **Priority**: Low (optimization, deferred until after Phase 6).
  - **Estimated Effort**: 2–3 days (1 for audit, 1 for refactoring, 0.5–1 for testing).

- [ ] **Error Conditions**:
  - **Task**: Define and integrate `std::error_condition` mappings for both `error` and `protocol_signal`.
  - **Steps**:
    - Create a new `enum class condition` to represent higher-level Telnet semantic groups (e.g., `short_read`, `buffer_command`, `protocol_interrupt`).
    - Implement `default_error_condition()` in the Telnet error category to map specific `error` values to their canonical `condition`.
    - Optionally override `equivalent()` to support multi-condition equivalence (e.g., `AO` may match both `buffer_signal` and `protocol_interrupt`).
    - Update relevant handlers and `InputProcessor` logic to interpret or log conditions appropriately.
    - Add unit tests verifying that condition equivalence works as intended (`ec == condition::short_read`, etc.).
  - **Dependencies**: Extends `:stream` error category; affects all Telnet error propagation, async read loops, and handler signaling.
  - **Priority**: Medium (semantic clarity and error handling consistency).
  - **Estimated Effort**: 1–2 days (0.5–1 for category updates, 0.5–1 for integration and testing).

- [ ] **Switch to `std::byte`**:
  - **Task**: Switch `byte_t` from `std::uint8_t` to `std::byte` when C++23 support improves.
  - **Steps**:
    - Update `byte_t` alias in `telnet-types.cppm`.
    - Refactor `:stream`, `:protocol_fsm`, `:options`, `:internal`.
    - Test compatibility with `TelnetCommand`, `option_registry`, `OptionHandlerRegistry`.
  - **Dependencies**: Affects `:stream`, `:protocol_fsm`, `:options`, `:internal`.
  - **Priority**: Low (depends on C++23 adoption).
  - **Estimated Effort**: 2–3 days (1 for refactoring, 1–1.5 for testing, 0.5 for documentation).

- [ ] **C++26 Reflection for Option Names**:
  - **Task**: Use C++26 reflection to auto-populate `option` names.
  - **Steps**:
    - Prototype reflection to generate `std::string` names for `option::id_num`.
    - Add customization point for user-set names.
    - Update `option` constructor and `make_option` in `telnet-options.cppm`.
    - Test with `:protocol_fsm`, `:stream`, `:internal`.
  - **Dependencies**: Affects `:options`, depends on C++26 support.
  - **Priority**: High/Deferred (improves logging, depends on C++26).
  - **Estimated Effort**: 2–3 days (1 for prototyping, 1–1.5 for implementation, 0.5 for testing).

- [ ] **C++26 Async Framework Compatibility**:
  - **Task**: Monitor compatibility with evolving Boost.Asio and `std::execution`.
  - **Steps**:
    - Review Boost.Asio and C++ standard library updates quarterly.
    - Prototype async methods with C++26 `std::execution`.
    - Test with `:stream` async operations.
  - **Dependencies**: Affects `:stream`, indirectly synchronous methods via `sync_await`.
  - **Priority**: Low (long-term maintenance).
  - **Estimated Effort**: 3–4 days (1 for review, 1.5–2 for prototyping, 0.5–1 for testing).

- [ ] **Enhanced Logging with Levels**:
  - **Task**: Add log levels (e.g., trace, debug, info, warning, error) to the logging system.
  - **Steps**:
    - Add an `enum class` for log levels.
    - Add `Logger` type alias as a future replacement for `ErrorLogger`.
    - Add `ProtocolConfig::log` function as a future replacement for `ProtocolConfig::log_error` with support for logging levels.
    - Deprecate `ProtocolConfig::log_error` and replace it in `ProtocolFSMConfig` with the new facility.
    - Modify all `log_error` calls into `log` calls to specify levels (or allow for a default level).
    - Add configuration for log level filtering.
    - Remove the deprecated `ProtocolConfig::log_error`.
    - Remove any `ErrorLogger` or `log_error` references.
    - Review the module for additional logging opportunities at non-error levels.
  - **Dependencies**: Affects `:protocol_fsm`, `:stream`, `:errors`.
  - **Priority**: Medium.
  - **Estimated Effort**: 2–3 days.

- [ ] **Make `async_write_negotiation` Private**:
  - **Task**: Make `telnet::stream::async_write_negotiation` private after refactoring callers.
  - **Steps**:
    - Refactor callers (`InputProcessor::operator()`) to use internal methods or declare as `friend`.
    - Update `telnet-stream.cppm` to make `async_write_negotiation` private.
    - Test negotiation functionality.
  - **Dependencies**: Affects `:stream` (`telnet-stream-impl.cpp`).
  - **Priority**: Low (interface cleanup, deferred until after Phase 6).
  - **Estimated Effort**: 1–2 days (0.5 for refactoring, 0.5–1 for testing).

### Completed Tasks
- [✔️] **Phase 4: Logging Enhancement** (Completed October 02, 2025):
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

- [✔️] **Phase 4: RFC 1143 Compliance** (Completed October 03, 2025):
  - Implemented RFC 1143’s Q Method in `handle_state_option_negotiation` (`telnet-protocol_fsm-impl.cpp`), using `OptionStatusRecord` (`telnet-internal.cppm`) to track states (`NO`, `YES`, `WANTNO`, `WANTYES`) and queue bits.
  - Handled redundant requests (`WILL`/`DO` in `YES`, `WONT`/`DONT` in `NO`) with `error::invalid_negotiation` logs to prevent loops.
  - Ensured robustness against rapid renegotiation for options designed for frequent state changes.
  - Integrated with `:stream` for sending responses via `async_write_negotiation` (`telnet-stream-async-impl.cpp`) and `write_negotiation` (`telnet-stream-sync-impl.cpp`).
  - Added comprehensive Catch2 unit tests for all Q Method state transitions, edge cases (e.g., redundant requests, unregistered options), and rapid renegotiation scenarios.
  - Updated `README.md`, `telnet-protocol_fsm-impl.cpp`, `telnet-internal.cppm` to document RFC 1143 compliance and logging conventions.
  - Updated `undefined_subnegotiation_handler` in `telnet-internal.cppm` to log `cmd: SB (0xFA), opt: {}` (non-directional per RFC 854).

- [❌️] **Phase 5: Add Shared Mutex to `OptionHandlerRegistry`** (Closed NAD, October 2025):
  - **Task**: Add a `std::shared_mutex` to `OptionHandlerRegistry` if registration is not confined to initialization time.
  - **Steps**:
    - Evaluated and deemed implementation unnecessary due to single-strand execution.
  - **Dependencies**: Affects `:internal`, `:protocol_fsm`, `:stream`.
  - **Priority**: Medium.
  - **Estimated Effort**: Completed in 0 days (evaluation only).

- [✔️] **Phase 5: Review Options List** (Completed October 3, 2025):
  - **Task**: Review `option::id_num` list for completeness against IANA Telnet Option Registry and RFCs.
  - **Steps**:
    - Cross-referenced `option::id_num` in `telnet-options.cppm` with IANA Telnet Option Registry and RFCs (e.g., RFC 855, RFC 856).
    - Identified and added missing niche options (e.g., GSSAPI, PUEBLO, SSL_DEPRECATED, MCP) for comprehensive coverage of standard, RFC-defined, and MUD/proprietary extensions.
    - Updated `:options` with new entries, including support predicates and documentation.
    - Tested integration with `:protocol_fsm` (implemented in `telnet-protocol_fsm-impl.cpp`), `:stream` (implemented in `telnet-stream-impl.cpp`, `telnet-stream-async-impl.cpp`, `telnet-stream-sync-impl.cpp`), and `:internal`.
    - Ensured `:errors` covers new error cases (e.g., `option_not_available`).
  - **Dependencies**: Affects `:options`, `:protocol_fsm`, `:stream`, `:internal`, `:errors`.
  - **Priority**: High (ensures comprehensive option support).
  - **Estimated Effort**: Completed in 2 days.

- [✔️] **Phase 5: Review Option Implementation Strategy** (Completed October 4, 2025):
  - **Task**: Determine which options require internal `ProtocolFSM` implementation vs. library extensions.
  - **Steps**:
    - Analyzed options (e.g., `BINARY`, `ECHO`, `NAWS`) for complexity and customization needs.
    - Decided which options (e.g., `BINARY`, `ECHO`) should be implemented internally for performance vs. externally for flexibility.
    - Updated `:options`, `:protocol_fsm` (implemented in `telnet-protocol_fsm-impl.cpp`), and `:internal` (for `OptionHandlerRegistry`) to support the chosen model.
    - Tested with `:stream` negotiation methods (implemented in `telnet-stream-impl.cpp`, `telnet-stream-async-impl.cpp`, `telnet-stream-sync-impl.cpp`) and `:errors` (e.g., `user_handler_forbidden`).
  - **Dependencies**: Affects `:options`, `:protocol_fsm`, `:stream`, `:internal`, `:errors`.
  - **Priority**: High (defines option handling architecture).
  - **Estimated Effort**: Completed in 2 days.

- [✔️] **Phase 5: Restructure `stream` with Side Buffers** (Completed October 6, 2025):
  - **Task**: Add input and output side buffers to `stream` for internal buffering of reads and writes.
  - **Steps**:
    - Added `asio::streambuf` members to `stream` providing input and output side buffers.
    - Restructured `InputProcessor` to commit next_layer reads into the input side buffer and consume its data when `ProtocolFSM` processes the bytes.
    - Adjusted `InputProcessor` to fill the user buffer as `process_byte` forwards bytes out of the input side buffer.
    - Ensured `InputProcessor` guards against overruns in the user's provided buffer.
  - **Dependencies**: `:stream`.
  - **Priority**: Critical (necessary for RFC 854 and RFC 1123 compliance; pre-requisite for next task).
  - **Estimated Effort**: Completed in 1 day.

- [✔️] **Phase 5: Implement "Soft EOF" Error Codes for Completing Reads Early** (Completed October 7, 2025):
  - **Task**: Create error codes for EOL, EOR, and GA conditions to allow `async_read_some` to signal an early read completion.
  - **Steps**:
    - Added `error::end_of_line`, `error::end_of_record`, and `error::go_ahead` codes in `:errors`.
    - Modified `InputProcessor` to check for these codes from `process_byte` in order to complete the operation (and propagate the codes) before exhausting the data in the input side buffer.
    - Modified `ProtocolFSM` to return these codes to signal the respective conditions back to `InputProcessor`.
  - **Dependencies**: Affects `:errors`, `:protocol_fsm`, and `:stream`.
  - **Priority**: Critical (necessary for RFC 854 and RFC 1123 compliance; pre-requisite for internal implementation of EOR and half-duplex mode).
  - **Estimated Effort**: Completed in 1 day.

- [✔️] **Phase 5: Refactor Soft EOF Codes** (Completed October 9, 2025):
  - **Task**: Refactor "soft EOF" codes into a separate enumeration/category and add entries for EC, EL, AO, IP, BRK, and DM for `process_byte` to return as error_code processing signals.
  - **Steps**:
    - Created a separate `processing_signal` enum in `:errors` for EOL, EOR, GA, and new codes (EC = Erase Character, EL = Erase Line, AO = Abort Output, IP = Interrupt Process, BRK = Break, DM = Data Mark).
    - Updated `telnet_error_category` to handle the new enum.
    - Modified `process_byte` and handlers (e.g., `handle_state_iac` for new commands) to return the new codes if enabled.
    - Updated `InputProcessor` to propagate the new codes for early completion.
  - **Dependencies**: Affects `:errors`, `:protocol_fsm`, `:stream`.
  - **Priority**: High (enhances processing signals for more commands; necessary for Synch/DM correctness).
  - **Estimated Effort**: Completed in 1 day.

- [✔️] **Phase 5: Centralize Command Processing in FSM** (Completed October 10, 2025):
  - **Task**: Update command processing to handle ALL commands internally by returning `error_code` objects representing the interrupting command from `process_byte`. The calling implementation layer will manage the interaction with side buffers, urgent processing state, and user code.
  - **Steps**:
    - Modified `ProtocolFSM::handle_state_iac` to return dedicated `error_code` values for commands like `GA`, `AO`, `IP`, `BRK`, etc. (using the refactored soft EOF codes from **Refactor Soft EOF Codes**).
    - The calling layer (`telnet::stream::InputProcessor`) was made responsible for managing side buffers, urgent processing state, and notifying user code based on the returned error code.
    - For EC and EL, `InputProcessor` adjusts the `write_it_` position within the side buffer if and only if it contains data. If empty, it propagates the EC or EL `error_code` to signal the user code for application-level handling.
  - **Dependencies**: Depends on **Refactor Soft EOF Codes**. Affects `:protocol_fsm` and `:stream` (`InputProcessor`).
  - **Priority**: High (architecture cleanup, aligns with **Eliminate User Command Handlers**).
  - **Estimated Effort**: Completed in 2 days.

- [✔️] **Phase 5: Eliminate User Command Handlers** (Completed October 10, 2025):
  - **Task**: Eliminate registration and management of user command handlers when they become superfluous.
  - **Steps**:
    - Removed `CommandProcessor` concept and `command_handler_registry_` from the FSM.
    - Eliminated the `default` case in `handle_state_iac` that checked `command_handler_registry_`.
    - Updated user-facing APIs that exposed command registration.
    - Added `default_error_condition` for error mappings in `:errors`.
    - Added `deferred_error` parameter to `InputProcessor` for error storage.
    - Refactored side buffers and `deferred_transport_error_` into `context_type` struct to organize the `InputProcessor`'s contextual state.
  - **Dependencies**: Depends on **Centralize Command Processing in FSM**. Affects `:protocol_fsm` and `:internal`.
  - **Priority**: High (interface cleanup and reduction of complexity).
  - **Estimated Effort**: Completed in 2 days.

- [✔️] **Phase 5: Urgent Data/Synch Procedure Handling (OOB)** (Completed October 12, 2025):
  - **Task**: Implement Telnet Synch procedure handling using Out-of-Band (OOB) data for urgent processing.
  - **Steps**:
    - Set `out_of_band_inline` on `lowest_layer()` in `stream` constructor (`telnet-stream-impl.cpp`) using non-throwing `set_option` with `std::error_code&`, logging errors via `ProtocolConfig::log_error` (RFC 854).
    - Added `urgent_data_tracker` class in `telnet-stream.cppm` with `UrgentDataState` enum (`NO_URGENT_DATA`, `HAS_URGENT_DATA`, `UNEXPECTED_DATA_MARK`) and `std::atomic<UrgentDataState>` to track Synch mode, replacing `std::atomic<bool> urgent_flag_`.
    - Added `launch_wait_for_urgent_data` in `stream` to call `async_receive` with `message_out_of_band` on `lowest_layer()` using a 0-byte buffer, guarded by `!context_.urgent_data_state`.
    - Added `async_send_synch` and both `send_synch` overloads in `stream` to send Telnet Synch (three NUL bytes, one as OOB, followed by IAC DM).
    - Added private `async_send_nul` helper to send a single NUL byte, with optional OOB flag.
    - Updated `InputProcessor::operator()` to:
      - Discard non-IAC bytes in Synch mode.
      - Call `urgent_data_state.saw_data_mark()` on `DM` and relaunch OOB wait.
      - Clear `output_side_buffer` and call `async_send_synch` on `abort_output`, deferring `processing_signal::abort_output` for application-level notification (RFC 854).
    - Updated `ProtocolFSM::handle_state_iac` to return `processing_signal::data_mark` for `DM` (RFC 854).
    - Added logging in `urgent_data_tracker::saw_urgent` and `saw_data_mark` for unexpected states (`UNEXPECTED_DATA_MARK` in `saw_urgent`, `NO_URGENT_DATA` or `UNEXPECTED_DATA_MARK` in `saw_data_mark`).
    - Added `deferred_processing_signal` to `context_type` for deferring `processing_signal::abort_output`.
    - Updated `TelnetSocketConcept` in `telnet-stream.cppm` to include `async_receive`, `async_send`, and `set_option` for OOB support.
  - **Dependencies**: Affects `:stream` (`InputProcessor`), `:protocol_fsm` (for DM handling), `:errors`. Depends on **Refactor Soft EOF Codes**.
  - **Priority**: High (critical Telnet feature for interrupt/abort).
  - **Estimated Effort**: Completed in 3 days.

- [✔️] **Phase 5: Implement Tagged Awaitables** (Completed October 14, 2025):
  - **Task**: Introduce a `TaggedAwaitable<Tag, T, Awaitable>` template system to improve type safety, readability, and maintainability of Telnet asynchronous handler responses.
  - **Steps**:
    - Created `:awaitables` partition in `telnet-awaitables.cppm`.
    - Defined `TaggedAwaitable<Tag, T, Awaitable>` wrapping `asio::awaitable<T>` (or another awaitable type) with implicit conversion operators to/from `asio::awaitable<T>`.
    - Created tag structs: `OptionEnablementTag`, `OptionDisablementTag`, `SubnegotiationTag`.
    - Defined type aliases: `OptionEnablementAwaitable`, `OptionDisablementAwaitable`, `SubnegotiationAwaitable` as `TaggedAwaitable<Tag, void, asio::awaitable<void>>`.
    - Tested with `:stream` for subnegotiation handling and `:errors` for error cases (e.g., `user_handler_not_found`).
  - **Dependencies**: Affects `:awaitables`, `:stream`, `:protocol_fsm`, `:internal`.
  - **Priority**: Medium (code clarity, maintainability, and extensibility).
  - **Estimated Effort**: Completed in 1 day.

- [✔️] **Phase 5: Enhance `OptionHandlerRecord`** (Completed October 14, 2025):
  - **Task**: Add enablement and disablement handlers to `OptionHandlerRecord`.
  - **Steps**:
    - Defined `OptionEnablementHandler` and `OptionDisablementHandler` type aliases in `telnet-protocol_fsm.cppm` for `WILL`/`DO` and `WONT`/`DONT` events.
    - Updated `OptionHandlerRecord` in `telnet-internal.cppm` to store `enablement_handler`, `disablement_handler`, and `subnegotiation_handler`.
    - Added `register_handlers`, `unregister_handlers`, `handle_enablement`, `handle_disablement` methods to `OptionHandlerRegistry` in `telnet-internal.cppm`.
    - Added `register_option_handlers` and `unregister_option_handlers` in `ProtocolFSM` (`telnet-protocol_fsm.cppm`) to manage handlers via `option::id_num`.
    - Added `register_option_handlers` and `unregister_option_handlers` in `telnet::stream` (`telnet-stream.cppm`), supporting `option::id_num` with implicit conversion from `option`.
    - Tested with negotiation scenarios (e.g., `WILL ECHO`, `DO NAWS`) using `:stream` and `:errors` (e.g., `invalid_negotiation`, `user_handler_not_found`).
  - **Dependencies**: Affects `:internal`, `:protocol_fsm`, `:options`, `:stream`, `:errors`, `:awaitables`.
  - **Priority**: High (enhances option state management).
  - **Estimated Effort**: Completed in 1 day.

- [✔️] **Phase 5: Invoke Option Negotiation Handlers** (Completed October 15, 2025):
  - **Task**: Invoke enablement/disablement handlers from `OptionHandlerRecord` after state transitions.
  - **Steps**:
    - Updated `ProcessingReturnVariant` in `telnet-protocol_fsm.cppm` to include `OptionEnablementAwaitable` and `OptionDisablementAwaitable`.
    - Modified `ProtocolFSM::handle_state_option_negotiation` in `telnet-protocol_fsm-impl.cpp` to call `OptionHandlerRegistry::handle_enablement` or `handle_disablement` after state transitions.
    - Updated `telnet::stream::InputProcessor` in `telnet-stream-impl.cpp` to handle `OptionEnablementAwaitable` and `OptionDisablementAwaitable` via `std::visit` like we do for `SubnegotiationHandler`.
  - **Dependencies**: Depends on `:internal`, `:options`, `:protocol_fsm`, `:stream`, `:errors`, `:awaitables`.
  - **Priority**: High (enhances flexibility for option-specific behavior).
  - **Estimated Effort**: Completed in 1 day.

- [✔️] **Phase 5: Restructure Option Negotiation for Internal Implementation** (Completed October 15, 2025):
  - **Task**: Implement core options (e.g., `BINARY`, `ECHO`) directly in `ProtocolFSM` for performance, bypassing external handlers.
  - **Steps**:
    - Identify core options for internal implementation to optimize performance.
    - Update `handle_state_option_negotiation` in `telnet-protocol_fsm-impl.cpp` to handle these options directly, bypassing external handlers.
    - Test performance and correctness with `:stream` (implemented in `telnet-stream-impl.cpp`, `telnet-stream-async-impl.cpp`, `telnet-stream-sync-impl.cpp`) and `:options`.
  - **Dependencies**: Depends on `:options`, `:internal`, `:protocol_fsm`, `:stream`, `:errors`.
  - **Priority**: High (improves performance for core options).
  - **Estimated Effort**: 0 days (implicitly completed by previous efforts).

- [✔️] **Phase 5: Option Negotiation Method** (Completed October 16, 2025):
  - **Task**: Add a method for user code to request/offer an option that validates current state, sets the pending flag in the FSM, and writes the negotiation sequence.
  - **Steps**:
    - Added a `request_option` (and inverse `disable_option`) method to `ProtocolFSM` to validate the availability of an option and update the FSM option state to `WANTYES` or `WANTNO` and manage the `OPPOSITE` queue bit.
    - Added `async_request_option` to `telnet::stream` in `telnet-stream.cppm` and send `IAC` `WILL`/`DO` using `async_write_negotiation`.
    - Added a complementary `async_disable_option` to `telnet::stream` method for user code to disable an option and send `IAC` `WONT`/`DONT` via `async_write_negotiation`.
    - Used `OptionStatusDB` (`:internal`) and `option_registry` (`:options`) for state and option lookup.
  - **Dependencies**: Requires `:protocol_fsm`, `:options`, `:types`, `:errors`, `:internal`, `:stream`.
  - **Priority**: High (core feature for Telnet protocol).
  - **Estimated Effort**: Completed in 1 day.

- [✔️] **Phase 5: Implement Status Option Internally** (Completed October 17, 2025):
  - **Task**: Implement `option::id_num::STATUS` internally.
  - **Steps**:
    - Developed a subnegotiation handler `handle_status_subnegotiation` in `telnet-protocol_fsm-impl.cpp` to report option status per RFC 859.
    - Integrated with `OptionStatusDB` for state retrieval.
    - Updated `SubnegotiationAwaitable` to return a tuple of `option` and `std::vector<byte_t>` to allow the handler to respond with the `IS` list when receiving `SEND`.
  - **Dependencies**: Affects `:stream`, `:protocol_fsm`, `:options`, `:internal`.
  - **Priority**: Medium (enhances diagnostic capabilities).
  - **Estimated Effort**: Completed in 2–3 days (1 for implementation, 1–1.5 for testing, 0.5 for documentation).

- [❌️] **Phase 5: Option Registry Optimization** (Closed NAD, October 17, 2025):
  - **Task**: Optimize `option_registry` for large option sets by evaluating `std::set` vs. `std::vector`.
  - **Steps**:
    - Evaluated `std::vector` with pre-filled indices (O(1) lookup) for vs. `std::set` (O(lg n) lookup) for memory use vs time complexity.
    - Decided to stay with `std::set` to avoid substantial memory overhead for minimal speed gains.
  - **Dependencies**: Affects `:options`, `:protocol_fsm`, `:stream`.
  - **Priority**: Low (performance optimization, depends on option review).
  - **Estimated Effort**: Completed in 0.5 days.

- [❌️] **Phase 5: Async Negotiation Awaiting** (Closed NAD, October 17, 2025):
  - **Task**: Enable asynchronously awaiting negotiation request responses as a composed operation until the remote endpoint responds.
  - **Steps**:
    - Evaluated modifying `async_request_option` to await peer responses using `OptionHandlerRegistry` handlers or `asio::async_compose`.
    - Decided to retain existing handler-based approach due to unified logic for user-initiated and peer-initiated negotiations, protocol compliance (no timeouts per RFC 854), and minimal complexity over direct awaiting.
  - **Dependencies**: Depends on **Option Negotiation Method**, `:protocol_fsm`, `:stream`, `:types`, `:options`, `:errors`, `:internal`, `:awaitables`.
  - **Priority**: Medium (enhances usability).
  - **Estimated Effort**: Completed in 0.5 days.

### Notes
- Version 0.5.7 reflects the completion of Phase 5, including tagged awaitables, enhanced `OptionHandlerRecord`, and user-initiated option negotiation.
- Log formats use default `{}` specifiers for `TelnetCommand` (`name (0xXX)`), `option` (`0xXX (name)`), and `NegotiationDirection` (`local` or `remote`), with `"N/A"_sv` for `std::nullopt` cases.
- Next focus: Phase 6 Milestone 8, **TLS**
*Last updated: October 30, 2025*