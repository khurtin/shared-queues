#ifndef SHARED_QUEUES_CALLABLE_H
#define SHARED_QUEUES_CALLABLE_H

namespace shared_queues
{

template<typename KEY, typename VALUE>
class callable_base
{
public:
    virtual void operator()(const KEY&, const VALUE&) = 0;
    virtual bool operator!() const = 0;
    virtual ~callable_base() {}
}; // class callable_base

template<typename T, typename KEY, typename VALUE>
class callable : public callable_base<KEY, VALUE>
{
public:
    callable(std::shared_ptr<T> object, std::function<void(T*, const KEY&, const VALUE&)>&& func) noexcept : m_object(object), m_func(std::move(func))
    {
    }

    void operator()(const KEY& key, const VALUE& value) override
    {
        std::shared_ptr<T> ptr = m_object.lock();
        if (!ptr)
            return;
        m_func(ptr.get(), key, value);
    }

    bool operator!() const override
    {
        return true;
    }

private:
    std::weak_ptr<T> m_object;
    std::function<void(T*, const KEY&, const VALUE&)> m_func;
}; // class callable

template<typename KEY, typename VALUE>
class callable_fake : public callable_base<KEY, VALUE>
{
public:
    void operator()(const KEY&, const VALUE&) override
    {
    }

    bool operator!() const override
    {
        return false;
    }
};

} // namespace shared_queues

#endif // SHARED_QUEUES_CALLABLE_H
