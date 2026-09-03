// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
/**
 * @file tools.test.coroutine_harness-core.cppm
 * @version 0.3.0
 * @date April 12, 2026
 *
 * @copyright © 2026 Jeremy Murphy and any Contributors
 * @par License: @parblock
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License. @endparblock
 *
 * @brief Core coroutine lifecycle tracking probe and handle RAII manager
 *
 * Defines `coroutine_probe` struct and `coroutine_handle_manager` class.
 *
 * Invariants:
 * - A managed `coroutine_handle` is destroyed exactly once.
 * - If a probe is attached, destruction is always recorded.
 * - Destroying an incomplete coroutine without a probe is a logic error. [Attach a probe to allow/observe premature destruction.]
 */

//Module partition interface unit
export module tools.test.coroutine_harness:core;

import std; //NOLINT

export namespace tools::test::coroutine_harness {
    /// @brief Observes coroutine lifecycle events for testing and validation.
    struct coroutine_probe {
        /// @brief Indicates how a coroutine was awaited.
        enum class path : std::uint8_t {
            none,   ///< Not awaited
            lvalue, ///< Awaited as an lvalue
            rvalue, ///< Awaited as an rvalue
        };

        bool done{false};            ///< Final suspend reached
        bool destroyed{false};       ///< Coroutine frame destroyed
        bool awaited{false};         ///< `co_await` invoked
        bool suspended{false};       ///< Suspension occurred
        bool resumed{false};         ///< Resumed after suspension
        bool moved{false};           ///< Task was moved
        path await_path{path::none}; ///< Await path classification
    };

    //Forward declaration of promise type used by task type but defined later
    template<typename T>
    struct test_promise;

    /**
     * @brief RAII wrapper for `std::coroutine_handle`.
     *
     * @tparam T Coroutine result type.
     *
     * @details
     * Ensures safe destruction and enforces lifecycle invariants:
     * - Tracks destruction via probe (if present)
     * - Detects premature destruction of unfinished coroutines
     *
     * @warning Destruction may throw if invariants are violated.
     */
    template<typename T>
    class coroutine_handle_manager {
        using promise_type    = test_promise<T>;
        using probe_ptr       = promise_type::probe_ptr;
        using raw_handle_type = std::coroutine_handle<promise_type>;

        raw_handle_type handle_;

    public:
        coroutine_handle_manager() = default;

        /// @brief Construct from raw coroutine handle.
        explicit coroutine_handle_manager(raw_handle_type handle) : handle_(handle) {}

        /**
         * @brief Destroy managed coroutine.
         *
         * @throws std::logic_error if coroutine is incomplete and no probe is attached.
         */
        //NOLINTNEXTLINE(bugprone-unsafe-to-allow-exceptions): This test support fixture uses a destructor exception to catch an otherwise undetectable class of test failures.
        ~coroutine_handle_manager() noexcept(false)
        {
            if (handle_) {
                bool done{handle_.done()};
                probe_ptr probe{handle_.promise().probe};

                std::exchange(handle_, {}).destroy();

                if (probe) {
                    probe->destroyed = true;
                } else if (!done && (std::uncaught_exceptions() == 0)) {
                    throw std::logic_error(
                        "test_task coroutine frame destroyed before coroutine completion without probe attached"
                    );
                }
            }
        }

        coroutine_handle_manager(const coroutine_handle_manager&)            = delete;
        coroutine_handle_manager& operator=(const coroutine_handle_manager&) = delete;

        coroutine_handle_manager(coroutine_handle_manager&& other) noexcept : handle_(std::exchange(other.handle_, {})) {}

        coroutine_handle_manager&
            operator=(coroutine_handle_manager&& other) noexcept(std::is_nothrow_swappable_v<coroutine_handle_manager>)
        {
            swap(*this, other);
            return *this;
        }

        /// @brief Access underlying handle.
        [[nodiscard]] decltype(auto) get(this auto&& self) { return std::forward_like<decltype(self)>(self.handle_); }

        /// @brief Implicit conversion to raw handle.
        explicit(false) operator raw_handle_type() const { return handle_; }

        /// @brief Check if handle is valid.
        explicit operator bool() const noexcept { return handle_ != nullptr; }

        /// @brief Check completion state.
        [[nodiscard]] bool done() const { return handle_ ? handle_.done() : false; }

        /// @brief Access promise.
        [[nodiscard]] decltype(auto) promise(this auto&& self) { return self.handle_.promise(); }

        friend void swap(coroutine_handle_manager& lhs, coroutine_handle_manager& rhs) noexcept(
            std::is_nothrow_swappable_v<raw_handle_type>
        )
        {
            using std::swap;
            swap(lhs.handle_, rhs.handle_);
        }
    };
} // namespace tools::test::coroutine_harness
