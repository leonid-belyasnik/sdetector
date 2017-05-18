#include "seeker.h"
#include <algorithm>

using namespace T1;

namespace {
	static std::vector<int> calc_z(const uint8_t* rs, size_t len)
	{
		std::vector<int> z;
		z.resize(len);
		z[0] = (int)len;
		std::size_t l = 0, r = 0;
		std::size_t j;

		for (std::size_t i = 1; i < len; i++)
		{
			if (i > r)
			{
				for (j = 0; ((j + i) < len) && (rs[i + j] == rs[j]); j++);

				z[i] = (int)j;
				l = i;
				r = (i + j - 1);
			}
			else
			{
				if (z[i - l] < (r - i + 1))
				{
					z[i] = z[i - l];
				}
				else
				{
					for (j = 1; ((j + r) < len) && (rs[r + j] == rs[r - i + j]); j++);

					z[i] = int(r - i + j);
					l = i;
					r = (r + j - 1);
				}
			}
		}

		return z;
	}
}

std::vector<int> BMSeeker::compute_suffshift(const uint8_t* s, int m)
{
	std::vector<int> suffshift(m + 1, m);

	std::vector<uint8_t> vrs(s, s + m);
	std::reverse(vrs.begin(), vrs.end());
	uint8_t* rs = vrs.data();

	std::vector<int> z = calc_z(rs, vrs.size());

	for (int j = m - 1; j > 0; j--)
	{
		suffshift[m - z[j]] = j;
	}
	for (int j = 1, r = 0; j <= m - 1; j++)
	{
		if (j + z[j] == m)
		{
			for (; r <= j; r++)
			{
				if (suffshift[r] == m) 
					suffshift[r] = j;
			}
		}
	}

	return suffshift;
}

int BMSeeker::find(const SeekData& seek_set)
{
	if (!is_opened() ||
		seek_set.vb_trace.empty() ||
		size() < seek_set.vb_trace.size()
		)
	{
		return -1;
	}

	size_t size_data = size();
	size_t size_trace = seek_set.vb_trace.size();
	const uint8_t* raw_data = data();

	int i = 0, j, bound = 0; 
	while (i <= size_data - size_trace) 
	{
		for (j = int(size_trace - 1); j >= bound && seek_set.vb_trace[j] == raw_data[i + j]; j--);

		if (j < bound) 
		{
			return i;
		}
		else 
		{
			bound = 0;
		}

		i += seek_set.vn_suffshift[j + 1];
	}

	return -1;
}