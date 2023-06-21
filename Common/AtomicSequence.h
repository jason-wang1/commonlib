#pragma once
#include <atomic>

#define CACHELINE_SIZE_BYTES 64
#define CACHELINE_PADDING_FOR_ATOMIC_INT64_SIZE (CACHELINE_SIZE_BYTES - sizeof(std::atomic_int64_t))

class AtomicSequence
{
public:
    AtomicSequence(int64_t num = 0L) : _seq(num){};
    ~AtomicSequence(){};
    AtomicSequence(const AtomicSequence &) = delete;
    AtomicSequence(const AtomicSequence &&) = delete;
    void operator=(const AtomicSequence &) = delete;

    void store(const int64_t val)
    {
        _seq.store(val);
    }

    int64_t load()
    {
        return _seq.load();
    }

    int64_t fetch_add(const int64_t increment)
    {
        return _seq.fetch_add(increment);
    }

private:
    char _frontPadding[CACHELINE_SIZE_BYTES];
    std::atomic_int64_t _seq;
    char _backPadding[CACHELINE_PADDING_FOR_ATOMIC_INT64_SIZE];
};