/**********************************************************************************

 Infomap software package for multi-level network clustering

 Copyright (c) 2013, 2014 Daniel Edler, Martin Rosvall
 
 For more information, see <http://www.mapequation.org>
 

 This file is part of Infomap software package.

 Infomap software package is free software: you can redistribute it and/or modify
 it under the terms of the GNU Affero General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Infomap software package is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Affero General Public License for more details.

 You should have received a copy of the GNU Affero General Public License
 along with Infomap software package.  If not, see <http://www.gnu.org/licenses/>.

**********************************************************************************/

#ifndef VECTOR_MAP_H_
#define VECTOR_MAP_H_

#include <vector>
#include <limits>
#include <map>

namespace infomap {


template<typename T>
class VectorMap
{
public:
    
	VectorMap(unsigned int capacity = 0) :
        m_capacity(capacity),
        m_values(capacity),
        m_redirect(capacity, 0),
        m_maxOffset(std::numeric_limits<unsigned int>::max() - 1 - capacity)
    {}

    void startRound() {
        if (m_size > 0) {
            m_offset += m_capacity;
            m_size = 0;
        }
        if (m_offset > m_maxOffset)
		{
            m_redirect.assign(m_capacity, 0);
			m_offset = 1;
		}
    }

    void add(unsigned int index, T value) 
    {
        if (isSet(index))
        {
            m_values[m_redirect[index] - m_offset] += value;
            // auto tmp = m_values[m_redirect[index] - m_offset];
            // m_values[m_redirect[index] - m_offset] = value;
            // m_values[m_redirect[index] - m_offset] += tmp;
        }
        else
        {
            m_redirect[index] = m_offset + m_size;
            m_values[m_size] = value;
            ++m_size;
        }
    }

    bool isSet(unsigned int index)
    {
        return m_redirect[index] >= m_offset;
    }

    unsigned int size()
    {
        return m_size;
    }

    T& operator[](unsigned int index)
    {
        return m_values[m_redirect[index] - m_offset];
    }

    std::vector<T>& values()
    {
        return m_values;
    }

private:
    unsigned int m_capacity = 0;
    std::vector<T> m_values;
	std::vector<unsigned int> m_redirect;
	unsigned int m_maxOffset = std::numeric_limits<unsigned int>::max() - 1;
	unsigned int m_offset = 1;
    unsigned int m_size = 0;
};


template<typename T>
class SimpleMap
{
public:
    
    SimpleMap(unsigned int capacity = 0) :
        m_capacity(capacity)
    {}

    void startRound() {
        m_map.clear();
        // m_map.rehash(m_capacity);
        m_dirty = true;
    }

    void add(unsigned int index, T value) 
    {
        m_map[index] += value;
        m_dirty = true;
    }

    unsigned int size()
    {
        return m_map.size();
    }

    T& operator[](unsigned int index)
    {
        return m_map[index];
    }

    std::vector<T>& values()
    {
        if (m_dirty) {
            m_values.resize(m_map.size());
            unsigned int i = 0;
            for (auto& it : m_map) {
                m_values[i++] = it.second;
            }
            m_dirty = false;
        }
        return m_values;
    }

private:
    // std::unordered_map<unsigned int, T> m_map;
    std::map<unsigned int, T> m_map;
    unsigned int m_capacity = 0;
    std::vector<T> m_values;
    bool m_dirty = true;
};

}

#endif /* VECTOR_MAP_H_ */
