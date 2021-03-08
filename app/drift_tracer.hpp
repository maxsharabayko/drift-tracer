#pragma once
#include "stdafx.hpp"

/// This class is useful in every place where
/// the time drift should be traced. It's currently in use in every
/// solution that implements any kind of TSBPD.
template<unsigned MAX_SPAN, int MAX_DRIFT, bool CLEAR_ON_UPDATE = true>
class DriftTracer
{
    int64_t  m_qDrift;
    int64_t  m_qOverdrift;

    int64_t  m_qDriftSum;
    unsigned m_uDriftSpan;

public:
    DriftTracer()
        : m_qDrift(0)
        , m_qOverdrift(0)
        , m_qDriftSum(0)
        , m_uDriftSpan(0)
    {}

    bool update(int64_t driftval)
    {
        m_qDriftSum += driftval;
        ++m_uDriftSpan;

        // NOTE: In SRT this condition goes after if (m_uDriftSpan < MAX_SPAN).
        // I moved it here to calculate accumulated overdrift.
        if (CLEAR_ON_UPDATE)
            m_qOverdrift = 0;

        if (m_uDriftSpan < MAX_SPAN)
            return false;

        // Calculate the median of all drift values.
        // In most cases, the divisor should be == MAX_SPAN.
        m_qDrift = m_qDriftSum / m_uDriftSpan;

        // And clear the collection
        m_qDriftSum = 0;
        m_uDriftSpan = 0;

        // In case of "overdrift", save the overdriven value in 'm_qOverdrift'.
        // In clear mode, you should add this value to the time base when update()
        // returns true. The drift value will be since now measured with the
        // overdrift assumed to be added to the base.
        if (std::abs(m_qDrift) > MAX_DRIFT)
        {
            m_qOverdrift = m_qDrift < 0 ? -MAX_DRIFT : MAX_DRIFT;
            m_qDrift -= m_qOverdrift;
        }

        // printDriftOffset(m_qOverdrift, m_qDrift);

        // Timebase is separate
        // m_qTimeBase += m_qOverdrift;

        return true;
    }

    // For group overrides
    void forceDrift(int64_t driftval)
    {
        m_qDrift = driftval;
    }

    // These values can be read at any time, however if you want
    // to depend on the fact that they have been changed lately,
    // you have to check the return value from update().
    //
    // IMPORTANT: drift() can be called at any time, just remember
    // that this value may look different than before only if the
    // last update() returned true, which need not be important for you.
    //
    // CASE: CLEAR_ON_UPDATE = true
    // overdrift() should be read only immediately after update() returned
    // true. It will stay available with this value until the next time when
    // update() returns true, in which case the value will be cleared.
    // Therefore, after calling update() if it retuns true, you should read
    // overdrift() immediately an make some use of it. Next valid overdrift
    // will be then relative to every previous overdrift.
    //
    // CASE: CLEAR_ON_UPDATE = false
    // overdrift() will start from 0, but it will always keep track on
    // any changes in overdrift. By manipulating the MAX_DRIFT parameter
    // you can decide how high the drift can go relatively to stay below
    // overdrift.
    int64_t drift() const { return m_qDrift; }
    int64_t overdrift() const { return m_qOverdrift; }
};