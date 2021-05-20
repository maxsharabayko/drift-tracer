#include "metrics_latency.hpp"


namespace dtrace::metrics {

using namespace std;
using namespace chrono;

void latency::submit_sample(const time_point& sample_time, const time_point& current_time)
{
	const auto delay = current_time - sample_time;
	const long long delay_us = duration_cast<microseconds>(delay).count();
	m_latency_max_us = max(m_latency_max_us, delay_us);
	m_latency_min_us = min(m_latency_min_us, delay_us);

	m_latency_avg_us = m_latency_avg_us != -1
		? (m_latency_avg_us * 15 + delay_us) / 16
		: delay_us;
}

void latency::reset()
{
	m_latency_min_us = std::numeric_limits<long long>::max();
	m_latency_max_us = std::numeric_limits<long long>::min();
}

} // namespace dtrace::metrics
