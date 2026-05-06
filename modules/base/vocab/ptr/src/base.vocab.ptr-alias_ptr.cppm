        {}
        
        ///@brief Rebinds the `alias_ptr` to another object.
        alias_ptr& assign(reference source) noexcept
        alias_ptr& operator=(reference source) noexcept
            requires (!std::is_void_v<T>)
        {
            address_ = std::addressof(source);
        ///@brief (Conversion) Assigns from another `alias_ptr` according to underlying pointer conversions.
        template<typename U>
            requires (!std::same_as<U, T>) && std::convertible_to<std::add_pointer_t<U>, pointer>
        constexpr alias_ptr& assign(const alias_ptr<U>& source) noexcept
        {
            address_ = source.get();
            return *this;
        ///@brief Assigns from a raw `pointer`.
        template<typename P>
            requires std::is_pointer_v<std::remove_cvref_t<P>> && std::convertible_to<std::decay_t<P>, pointer>
        constexpr alias_ptr& assign(P&& source)
        {
            address_ = source;
            return *this;
                  && requires(Pointer<Element, Args...> ptr) {
                         { std::as_const(ptr).get() } -> std::convertible_to<pointer>;
                     }
        {
            address_ = source.get();
            return *this;
            swap(lhs.address_, rhs.address_);
        }
        
        template<typename P>
        alias_ptr& operator=(P&& source) requires requires { assign(std::forward<P>(source)); }
        {
            return assign(std::forward<P>(source));
        //================================================================================
        // Constructors and Assignment Operators: Nullable Implementation
        //================================================================================
        alias_ptr(std::nullptr_t null) :  address_(null) {}

        ///@brief Assignment from `nullptr` rebinds to null.
        alias_ptr& assign(std::nullptr_t null) { address_ = null; return *this; }

        //================================================================================
        // Deleted Constructors and Assignment Operators: No Aliasing Temporaries
            ;

        ///@brief Deleted assignment from `rvalue_reference` to discourage dangling by rejecting direct binding to temporaries.
        alias_ptr& assign(rvalue_reference) =
            delete /*("Assignment from `rvalue_reference` deleted to discourage dangling by rejecting direct binding to temporaries.")*/
            ;

                      && requires(Pointer<Element, Args...> ptr) {
                             { std::as_const(ptr).get() } -> std::convertible_to<pointer>;
                         }
            delete /*("Assignment from pointer-like object rvalue deleted to discourage dangling by rejecting direct binding to temporaries.")*/
            ;

        ///@brief Deleted assignment from C-array to prevent array-to-pointer decay.
        template<typename AnyCArray>
            requires std::is_array_v<AnyCArray>
        alias_ptr& operator=(AnyCArray&) =
            delete /*("Assignment from C-array deleted to prevent array-to-pointer decay. To point to the first element, alias it explicitly.")*/
            ;

     * @remark This constructor does not transfer ownership and does not affect the lifetime of the underlying object.
     */
    /**
     * @fn alias_ptr& alias_ptr::assign(reference source) noexcept
     * @fn alias_ptr& alias_ptr::operator=(reference source) noexcept
     *
     * @param source The object to reference.
     * @return Reference to `*this`.
     * @remark Rebinds the pointer without affecting ownership or lifetime.
     */
    /**
     * @overload alias_ptr& alias_ptr::assign(const alias_ptr<U>& source) noexcept
     *
     * @tparam U The element type, with its pointer convertible to `pointer`, of the source `alias_ptr`.
     *
     * @remark Enables type erasure by converting `alias_ptr<U>` to `alias_ptr<void>` when applicable.
     */
    /**
     * @overload alias_ptr& alias_ptr::assign(P&& source)
     *
     * @tparam P The raw pointer type which must decay to a type convertible to `pointer`.
     *
     * @remark Does not affect the lifetime of the referenced object.
     */
    /**
     *
     * @tparam Pointer A class template modeling a pointer-like type.
     * @tparam Element The element type of the source pointer.
