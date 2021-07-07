
#include <sdm/config.hpp>
#include <sdm/utils/linear_algebra/mapped_vector.hpp>
#include <sdm/utils/struct/pair.hpp>

namespace sdm
{

    template <typename TIndex, typename T>
    double MappedVector<TIndex, T>::PRECISION = config::PRECISION_MAPPED_VECTOR;

    template <typename TIndex, typename T>
    MappedVector<TIndex, T>::MappedVector() : RecursiveMap<TIndex, T>(), default_value_(0.), size_(-1)
    {
    }

    template <typename TIndex, typename T>
    MappedVector<TIndex, T>::MappedVector(T default_value) : RecursiveMap<TIndex, T>(), default_value_(default_value), size_(-1)
    {
    }

    template <typename TIndex, typename T>
    MappedVector<TIndex, T>::MappedVector(long size, T default_value) : RecursiveMap<TIndex, T>(), default_value_(default_value), size_(size)
    {
    }

    template <typename TIndex, typename T>
    MappedVector<TIndex, T>::MappedVector(const MappedVector &v)
        : RecursiveMap<TIndex, T>(v),
          default_value_(v.default_value_),
          size_(v.size_),
          v_indexes(v.v_indexes),
          bmin(v.bmin),
          bmax(v.bmax),
          pmin(v.pmin),
          pmax(v.pmax)
    {
    }

    template <typename TIndex, typename T>
    MappedVector<TIndex, T>::MappedVector(std::initializer_list<value_list_type> vals) : RecursiveMap<TIndex, T>(vals), default_value_(0), size_(-1)
    {
    }

    template <typename TIndex, typename T>
    MappedVector<TIndex, T>::~MappedVector() {}

    template <typename TIndex, typename T>
    T MappedVector<TIndex, T>::norm_1() const
    {
        T v = 0;
        for (const auto &item : *this)
        {
            v += std::abs(item.second);
        }
        return v;
    }

    template <typename TIndex, typename T>
    T MappedVector<TIndex, T>::at(const TIndex &i) const
    {
        if (this->find(i) != this->end())
        {
            auto value = RecursiveMap<TIndex, T>::at(i);
            return value;
        }
        else
        {
            return this->default_value_;
        }
    }

    template <typename TIndex, typename T>
    T MappedVector<TIndex, T>::getValueAt(const TIndex &index) const
    {
        return this->at(index);
    }

    template <typename TIndex, typename T>
    void MappedVector<TIndex, T>::setValueAt(const TIndex &index, const T &value)
    {
        (*this)[index] = value;
    }

    template <typename TIndex, typename T>
    T MappedVector<TIndex, T>::norm_2() const
    {
        T v = 0;
        for (const auto &item : *this)
        {
            v += std::pow(item.second, 2);
        }
        return v;
    }

    template <typename TIndex, typename T>
    const std::pair<TIndex, T> &MappedVector<TIndex, T>::getMin()
    {
        if (!this->bmin)
        {
            T min = std::numeric_limits<T>::max();
            TIndex amin;
            for (const auto &item : *this)
            {
                if (min > item.second)
                {
                    amin = item.first;
                    min = item.second;
                }
            }
            if (min == std::numeric_limits<T>::max())
            {
                amin = TIndex();
                min = this->default_value_;
            }

            this->pmin = {amin, min};
        }

        return this->pmin;
    }

    template <typename TIndex, typename T>
    T MappedVector<TIndex, T>::min()
    {
        return this->getMin().second;
    }

    template <typename TIndex, typename T>
    TIndex MappedVector<TIndex, T>::argmin()
    {
        return this->getMin().first;
    }

    template <typename TIndex, typename T>
    const std::pair<TIndex, T> &MappedVector<TIndex, T>::getMax()
    {
        if (!this->bmax)
        {
            T max = -std::numeric_limits<T>::max();
            TIndex amax;
            for (const auto &item : *this)
            {
                if (max < item.second)
                {
                    amax = item.first;
                    max = item.second;
                }
            }

            if (max == -std::numeric_limits<T>::max())
            {
                amax = TIndex();
                max = this->default_value_;
            }

            this->pmax = {amax, max};
        }

        return this->pmax;
    }

    template <typename TIndex, typename T>
    T MappedVector<TIndex, T>::max()
    {
        return this->getMax().second;
    }

    template <typename TIndex, typename T>
    TIndex MappedVector<TIndex, T>::argmax()
    {
        return this->getMax().first;
    }

    template <typename TIndex, typename T>
    void MappedVector<TIndex, T>::setPrecision(double precision)
    {
        MappedVector<TIndex, T>::PRECISION = precision;
    }

    template <typename TIndex, typename T>
    bool MappedVector<TIndex, T>::is_equal(const MappedVector<TIndex, T> &other, double precision) const
    {
        if (this->size() != other.size())
        {
            return false;
        }
        if (std::abs(this->getDefault() - other.getDefault()) > precision)
        {
            return false;
        }

        for (const auto &v : *this)
        {
            if (other.find(v.first) == other.end())
            {
                return false;
            }
            else
            {
                if (std::abs(other.at(v.first) - v.second) > precision)
                {
                    return false;
                }
            }
        }

        return true;
    }

    template <typename TIndex, typename T>
    bool MappedVector<TIndex, T>::operator==(const MappedVector<TIndex, T> &other) const
    {
        return this->is_equal(other, PRECISION);
    }

    template <typename TIndex, typename T>
    bool MappedVector<TIndex, T>::operator!=(const MappedVector<TIndex, T> &v2) const
    {
        return (!this->operator==(v2));
    }

    template <typename TIndex, typename T>
    bool MappedVector<TIndex, T>::operator<(const MappedVector &v2) const
    {
        for (const auto &item : *this)
        {
            if (item.second > v2.at(item.first))
            {
                return false;
            }
        }
        for (const auto &item : v2)
        {
            if (item.second < this->at(item.first))
            {
                return false;
            }
        }
        return true;
    }

    template <typename TIndex, typename T>
    T MappedVector<TIndex, T>::operator*(const MappedVector &v2) const
    {
        return this->dot(v2);
    }

    template <typename TIndex, typename T>
    T MappedVector<TIndex, T>::operator^(const MappedVector &v2) const
    {
        return this->dot(v2);
    }

    template <typename TIndex, typename T>
    T MappedVector<TIndex, T>::dot(const MappedVector &v2) const
    {
        T product = 0;

        for (const auto &item : *this)
        {
            product += item.second * v2.at(item.first);
        }

        return product;
    }

    template <typename TIndex, typename T>
    size_t MappedVector<TIndex, T>::size() const
    {
        return RecursiveMap<TIndex, T>::size();
    }

    template <typename TIndex, typename T>
    T MappedVector<TIndex, T>::getDefault() const
    {
        return this->default_value_;
    }

    template <typename TIndex, typename T>
    void MappedVector<TIndex, T>::setDefault(double default_value)
    {
        this->default_value_ = default_value;
    }

    template <typename TIndex, typename T>
    void MappedVector<TIndex, T>::setIndexes()
    {
        if (this->v_indexes.size() == 0)
        {
            for (const auto &p_i_v : *this)
            {
                this->v_indexes.push_back(p_i_v.first);
            }
        }
    }

    template <typename TIndex, typename T>
    std::vector<TIndex> MappedVector<TIndex, T>::getIndexes() const
    {
        // assert( (this->v_indexes.size()>0) );
        // return this->v_indexes;
        std::vector<TIndex> indexes = {};
        for (const auto &p_i_v : *this)
        {
            indexes.push_back(p_i_v.first);
        }
        return indexes;
    }

    template <typename TIndex, typename T>
    std::string MappedVector<TIndex, T>::str() const
    {
        std::ostringstream res;
        std::string size = ((this->size_ > 0) ? std::to_string(this->size_) : "?");
        res << "[" << size << "]";
        res << "(";
        for (const auto &val : *this)
        {
            res << val.first << " : " << val.second << ", ";
        }
        res << "default"
            << " : " << this->default_value_;
        res << ")";
        return res.str();
    }

    template <typename TIndex, typename T>
    template <class Archive>
    void MappedVector<TIndex, T>::serialize(Archive &archive, const unsigned int)
    {
        using boost::serialization::make_nvp;

        archive &boost::serialization::base_object<RecursiveMap<TIndex, T>>(*this);
        archive &make_nvp("default_value", default_value_);
        archive &make_nvp("size", size_);
        archive &make_nvp("PRECISION", PRECISION);

        archive &make_nvp("bmin", bmin);
        archive &make_nvp("bmax", bmax);

        if (v_indexes.size() > 0)
            archive &make_nvp("vector_indexes", v_indexes);

        if (bmin)
        {
            archive &make_nvp("pair_min", pmin);
        }
        if (bmax)
        {
            archive &make_nvp("pair_max", pmax);
        }
    }

} // namespace sdm

namespace std
{
    template <typename S, typename V>
    struct hash<sdm::MappedVector<S, V>>
    {
        typedef sdm::MappedVector<S, V> argument_type;
        typedef std::size_t result_type;
        inline result_type operator()(const argument_type &in) const
        {
            return this->operator()(in, sdm::MappedVector<S, V>::PRECISION);
        }

        inline result_type operator()(const argument_type &in, double precision) const
        {
            size_t seed = 0;
            std::map<S, V> ordered(in.begin(), in.end());
            std::map<S, int> rounded;
            for (const auto &pair_item_value : in)
            {
                rounded.emplace(pair_item_value.first, lround(precision * pair_item_value.second));
            }
            for (const auto &v : rounded)
            {
                //Combine the hash of the current vector with the hashes of the previous ones
                sdm::hash_combine(seed, v);
            }
            return seed;
        }
    };
}