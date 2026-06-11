// C++23, GCC 14.2
#include <cstddef>
#include <atomic>
#include <utility>

namespace from_scratch
{
    // Non-templated on Deleter: a from-scratch version deletes with `delete`.
    // (std::shared_ptr supports custom deleters by TYPE-ERASING them: the
    // control block has a virtual destroy(), with one derived block type per
    // deleter. Worth saying in an interview; overkill to implement live.)
    struct control_block
    {
        explicit control_block(std::size_t count) : count_{count} {}
        std::atomic<std::size_t> count_;
    };

    template <typename T>
    class shared_ptr
    {
    public:
        shared_ptr() noexcept = default;

        // Taking ownership of a raw pointer => we are the first owner.
        explicit shared_ptr(T* pointer)
            : ptr{pointer},
              ctrl_block{pointer ? new control_block{1} : nullptr}
        {}

        // Copy: share both pointers, bump the count.
        shared_ptr(const shared_ptr& other) noexcept
            : ptr{other.ptr}, ctrl_block{other.ctrl_block}
        {
            if (ctrl_block)
                ctrl_block->count_.fetch_add(1, std::memory_order_relaxed);
        }

        // Copy-assign via copy-and-swap: `other` arrives by VALUE (the copy
        // ctor already bumped the count), we swap our guts into it, and its
        // destructor releases our old state. Self-assignment safe for free.
        shared_ptr& operator=(shared_ptr other) noexcept
        {
            swap(other);
            return *this;
        }

        // Move: steal, leave other empty. No count change — ownership moved,
        // it didn't multiply.
        shared_ptr(shared_ptr&& other) noexcept
            : ptr{std::exchange(other.ptr, nullptr)},
              ctrl_block{std::exchange(other.ctrl_block, nullptr)}
        {}
        // (move-assign is covered by the by-value operator= above:
        //  a moved-from argument binds to it via the move ctor)

        ~shared_ptr() { release(); }

        void reset(T* pointer = nullptr)
        {
            shared_ptr tmp{pointer};   // build new state...
            swap(tmp);                 // ...swap it in; tmp's dtor releases old
        }

        void swap(shared_ptr& other) noexcept
        {
            std::swap(ptr, other.ptr);
            std::swap(ctrl_block, other.ctrl_block);
        }

        std::size_t use_count() const noexcept
        {
            return ctrl_block
                 ? ctrl_block->count_.load(std::memory_order_relaxed)
                 : 0;
        }

        T* get()        const noexcept { return ptr; }
        T* operator->() const noexcept { return ptr; }
        T& operator*()  const noexcept { return *ptr; }
        explicit operator bool() const noexcept { return ptr != nullptr; }

    private:
        void release() noexcept
        {
            if (!ctrl_block) return;
            // fetch_sub returns the PREVIOUS value: 1 means we were the last
            // owner. Decrement-then-separately-check is a race (double free).
            if (ctrl_block->count_.fetch_sub(1, std::memory_order_acq_rel) == 1)
            {
                delete ptr;
                delete ctrl_block;
            }
            ptr = nullptr;
            ctrl_block = nullptr;
        }

        T* ptr{nullptr};
        control_block* ctrl_block{nullptr};
    };
}
