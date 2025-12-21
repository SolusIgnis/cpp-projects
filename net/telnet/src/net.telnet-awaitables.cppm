/**
 * @file net.telnet-awaitables.cppm
 * @version 0.5.7
 * @release_date October 30, 2025
 *
 * @brief Partition defining tagged awaitable types for type-safe asynchronous operations in the Telnet library.
 * @remark Defines `TaggedAwaitable` and associated semantic tag structs for option negotiation and subnegotiation handlers.
 *
 * @copyright (c) 2025 [it's mine!]. All rights reserved.
 * @license See LICENSE file for details
 *
 * @see RFC 854 for Telnet protocol, RFC 855 for option negotiation, `:protocol_fsm` for handler usage, `:internal` for handler definitions
 */

module; //Including Asio in the Global Module Fragment until importable header units are reliable.
#include <asio/awaitable.hpp>

// Module partition interface unit
export module net.telnet:awaitables;

import std; //for std::move

//namespace asio = boost::asio;

export namespace net::telnet::awaitables {
    /**
     * @brief Wrapper for an awaitable with a semantic tag for type safety.
     * @tparam Tag The semantic tag type.
     * @tparam T The awaitable's value type (i.e. the "return type" of `co_await`ing it) (e.g., `void`).
     * @tparam Awaitable The underlying awaitable type (default: `boost::asio::awaitable<T>`).
     * @remark Provides implicit conversion to/from the underlying awaitable and supports direct `co_await`.
     * @see `tags` namespace for semantic tag types, `:protocol_fsm`, `:internal`
     */
    template<typename Tag, typename T, typename Awaitable = asio::awaitable<T>>
    class TaggedAwaitable {
    private:
        using awaitable_type = Awaitable; ///< Underlying awaitable type
    
        awaitable_type awaitable_; ///< The wrapped awaitable
    
    public:
        ///@brief Default constructor.
        TaggedAwaitable() = default;
    
        ///@brief Constructs from an awaitable.
        TaggedAwaitable(awaitable_type awaitable) noexcept
            : awaitable_(std::move(awaitable)) {}
    
        ///@brief Implicit conversion to underlying awaitable (lvalue).
        operator awaitable_type&() noexcept { return awaitable_; }
    
        ///@brief Implicit conversion to underlying awaitable (const lvalue).
        operator const awaitable_type&() const noexcept { return awaitable_; }
    
        ///@brief Implicit conversion to underlying awaitable (rvalue).
        operator awaitable_type&&() noexcept { return std::move(awaitable_); }
    
        ///@brief Supports co_await for lvalue.
        auto operator co_await() & noexcept {
            return awaitable_.operator co_await();
        }
    
        ///@brief Supports co_await for const lvalue.
        auto operator co_await() const & noexcept {
            return awaitable_.operator co_await();
        }
    
        ///@brief Supports co_await for rvalue.
        auto operator co_await() && noexcept {
            return std::move(awaitable_).operator co_await();
        }
    }; //class TaggedAwaitable
    /**
     * @fn TaggedAwaitable::TaggedAwaitable(awaitable_type awaitable) noexcept
     * @param awaitable The awaitable to wrap.
     * @note Implicit conversion from the underlying type allows direct returns from Boost.Asio asynchronous operations.
     */

    /// @brief Semantic tag `struct`s to specialize `TaggedAwaitable`. @see `TaggedAwaitable`
    namespace tags {
        /// @brief Tag to specialize `TaggedAwaitable` for option enablement handlers. @see `TaggedAwaitable`
        struct OptionEnablementTag {};
        
        /// @brief Tag to specialize `TaggedAwaitable` for option disablement handlers. @see `TaggedAwaitable`
        struct OptionDisablementTag {};
        
        /// @brief Tag to specialize `TaggedAwaitable` for subnegotiation handlers. @see `TaggedAwaitable`
        struct SubnegotiationTag {};
    } //namespace tags
    
    /**
     * @typedef OptionEnablementAwaitable
     * @brief Awaitable type for option enablement handlers.
     * @see `TaggedAwaitable`, `tags::OptionEnablementTag`, `:internal` (`OptionHandlerRegistry`), `:protocol_fsm` (for use)
     */
    using OptionEnablementAwaitable = TaggedAwaitable<tags::OptionEnablementTag, void>;
     
    /**
     * @typedef OptionDisablementAwaitable
     * @brief Awaitable type for option disablement handlers.
     * @see `TaggedAwaitable`, `tags::OptionDisablementTag`, `:internal` (`OptionHandlerRegistry`), `:protocol_fsm` (for use)
     */
    using OptionDisablementAwaitable = TaggedAwaitable<tags::OptionDisablementTag, void>;
     
    /**
     * @typedef SubnegotiationAwaitable
     * @brief Awaitable type for subnegotiation handlers.
     * @see `TaggedAwaitable`, `tags::SubnegotiationTag`, `:internal` (`OptionHandlerRegistry`), `:protocol_fsm` (for use)
     */
    using SubnegotiationAwaitable = TaggedAwaitable<tags::SubnegotiationTag, std::tuple<const option&, std::vector<byte_t>>>;
} //namespace net::telnet::awaitables
