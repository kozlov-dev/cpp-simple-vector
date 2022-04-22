#pragma once

#include <algorithm>
#include <cassert>
#include <initializer_list>
#include <iostream>
#include <iterator>

#include "array_ptr.h"

class ReserveProxyObj
{
public:
    ReserveProxyObj(size_t capacity_to_reserve)
    {
        capacity_to_reserve_ = capacity_to_reserve;
    }

    size_t capacity() const { return capacity_to_reserve_; }

private:
    size_t capacity_to_reserve_;
};

template <typename Type>
class SimpleVector
{
public:
    using Iterator = Type *;
    using ConstIterator = const Type *;

    SimpleVector() noexcept = default;

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size)
        : SimpleVector(size, std::move(Type()))
    {
    }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type &value) : items_(size),
                                                   size_(size),
                                                   capacity_(size)
    {
        std::fill(begin(), end(), value);
    }

    SimpleVector(size_t size, Type &&value) : items_(size),
                                              size_(size),
                                              capacity_(size)
    {
        for (size_t i = 0; i < size_; ++i)
        {
            items_[i] = std::move(value);
        }
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init) : items_(init.size()),
                                                     size_(init.size()),
                                                     capacity_(init.size())
    {
        std::copy(init.begin(), init.end(), begin());
    }

    SimpleVector(const SimpleVector &other) : items_(other.capacity_),
                                              size_(other.size_),
                                              capacity_(other.capacity_)
    {
        std::copy(other.begin(), other.end(), begin());
    }

    SimpleVector &operator=(const SimpleVector &rhs)
    {
        if (this == &rhs)
        {
            return *this;
        }
        SimpleVector<Type> new_vec(rhs);
        swap(new_vec);
        return *this;
    }

    SimpleVector(SimpleVector &&other)
    {
        items_ = std::move(other.items_);
        std::swap(size_, other.size_);
        other.size_ = 0;

        std::swap(capacity_, other.capacity_);

        other.capacity_ = 0;
    }

    SimpleVector &operator=(SimpleVector &&rhs)
    {
        if (this == &rhs)
        {
            return *this;
        }
        items_ = std::move(rhs.items_);
        std::swap(size_, rhs.size_);
        rhs.size_ = 0;
        std::swap(capacity_, rhs.capacity_);
        rhs.capacity_ = 0;
        return *this;
    }

    SimpleVector(ReserveProxyObj obb)
    {
        ArrayPtr<Type> arr(obb.capacity());
        items_.swap(arr);
        capacity_ = obb.capacity();
    }

    void Reserve(size_t capacity_to_reserve)
    {
        if (capacity_ >= capacity_to_reserve)
            return;

        size_t new_cap = capacity_to_reserve;
        ArrayPtr<Type> new_items(new_cap);
        std::copy(std::make_move_iterator(begin()), std::make_move_iterator(end()), new_items.Get());
        items_.swap(new_items);
        capacity_ = new_cap;
    }

    // Возвращает количество элементов в массиве
    size_t GetSize() const noexcept
    {
        return size_;
    }

    // Возвращает вместимость массива
    size_t GetCapacity() const noexcept
    {
        return capacity_;
    }

    // Сообщает, пустой ли массив
    bool IsEmpty() const noexcept
    {
        return size_ == 0;
    }

    // Возвращает ссылку на элемент с индексом index
    Type &operator[](size_t index) noexcept
    {
        assert(size_);
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type &operator[](size_t index) const noexcept
    {
        assert(size_);
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type &At(size_t index)
    {
        if (index >= size_)
        {
            throw std::out_of_range("");
        }
        return items_[index];
    }
    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type &At(size_t index) const
    {
        if (index >= size_)
        {
            throw std::out_of_range("");
        }
        return items_[index];
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept
    {
        size_ = 0;
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size)
    {
        if (new_size <= size_)
        {
            size_ = new_size;
            return;
        }
        if (new_size <= capacity_)
        {
            for (size_t i = size_; i < new_size; ++i)
            {
                items_[i] = std::move(Type());
            }
            size_ = new_size;
            return;
        }
        size_t new_cap = std::max(new_size, capacity_ * 2);
        ArrayPtr<Type> new_items(new_cap);

        std::copy(std::make_move_iterator(begin()), std::make_move_iterator(end()), new_items.Get());
        for (size_t i = size_; i < new_size; ++i)
        {
            new_items[i] = std::move(Type());
        }

        items_.swap(new_items);
        capacity_ = new_cap;
        size_ = new_size;
    }

    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept
    {
        return Iterator(items_.Get());
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept
    {
        return Iterator(items_.Get() + size_);
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept
    {
        return ConstIterator(items_.Get());
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept
    {
        return ConstIterator(items_.Get() + size_);
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept
    {
        return ConstIterator(items_.Get());
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept
    {
        return ConstIterator(items_.Get() + size_);
    }

    // Добавляет элемент в конец вектора
    // При нехватке места увеличивает вдвое вместимость вектора
    void PushBack(const Type &item)
    {
        const auto pos = size_;
        if (size_ == capacity_)
        {
            Resize(capacity_ + 1);
        }
        else
        {
            ++size_;
        }
        items_[pos] = item;
    }

    void PushBack(Type &&item)
    {
        const auto pos = size_;
        if (size_ == capacity_)
        {
            Resize(capacity_ + 1);
        }
        else
        {
            ++size_;
        }
        items_[pos] = std::move(item);
    }
    // Вставляет значение value в позицию pos.
    // Возвращает итератор на вставленное значение
    // Если перед вставкой значения вектор был заполнен полностью,
    // вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
    Iterator Insert(ConstIterator pos, const Type &value)
    {
        assert(begin() <= pos && pos <= end());

        auto n = std::distance(begin(), Iterator(pos));
        if (capacity_ == 0)
        {
            PushBack(value);
            return begin();
        }
        if (size_ < capacity_)
        {
            for (int i = size_; i > n; --i)
            {
                items_[i] = std::move(items_[i - 1]);
            }
            items_[n] = value;
            ++size_;
            return begin() + n;
        }
        else
        {
            SimpleVector<Type> new_vec(capacity_ * 2);

            std::copy(begin(), begin() + n, new_vec.begin());
            std::copy(begin() + n, end(), new_vec.begin() + n + 1);
            *(new_vec.begin() + n) = value;
            auto old_size = size_;
            swap(new_vec);
            size_ = old_size + 1;
            return begin() + n;
        }
    }

    Iterator Insert(ConstIterator pos, Type &&value)
    {
        assert(begin() <= pos && pos <= end());

        auto n = std::distance(begin(), Iterator(pos));
        if (capacity_ == 0)
        {
            PushBack(std::move(value));
            return begin();
        }
        if (size_ < capacity_)
        {
            for (int i = size_; i > n; --i)
            {
                items_[i] = std::move(items_[i - 1]);
            }
            items_[n] = std::move(value);

            ++size_;
            return begin() + n;
        }
        else
        {
            SimpleVector<Type> new_vec(capacity_ * 2);

            std::copy(std::make_move_iterator(begin()), std::make_move_iterator(begin() + n), new_vec.begin());
            std::copy(std::make_move_iterator(begin() + n), std::make_move_iterator(end()), new_vec.begin() + n + 1);
            *(new_vec.begin() + n) = std::move(value);
            auto old_size = size_;
            swap(new_vec);
            size_ = old_size + 1;
            return begin() + n;
        }
    }

    // "Удаляет" последний элемент вектора. Вектор не должен быть пустым
    void PopBack() noexcept
    {
        assert(size_);
        --size_;
    }

    // Удаляет элемент вектора в указанной позиции
    Iterator Erase(ConstIterator pos)
    {
        assert(begin() <= pos && pos < end());
        assert(size_);
        auto n = std::distance(begin(), Iterator(pos));
        std::copy(std::make_move_iterator(begin() + n + 1), std::make_move_iterator(end()), begin() + n);
        // if (size_ > 0)
        // {
        --size_;
        // }
        return begin() + n;
    }

    // Обменивает значение с другим вектором
    void swap(SimpleVector &other) noexcept
    {
        // items_->swap(*(other.items_));
        items_.swap(other.items_);
        std::swap(this->capacity_, other.capacity_);
        std::swap(this->size_, other.size_);
    }

private:
    ArrayPtr<Type> items_;
    size_t size_ = 0;
    size_t capacity_ = 0;
};

template <typename Type>
inline bool operator==(const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs)
{
    if (lhs.GetSize() != rhs.GetSize())
    {
        return false;
    }
    return std::equal(lhs.begin(), lhs.begin() + lhs.GetSize(), rhs.begin(), rhs.begin() + rhs.GetSize());
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs)
{
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs)
{
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs)
{
    return !(rhs < lhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs)
{
    return rhs < lhs;
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs)
{
    return !(lhs < rhs);
}

ReserveProxyObj Reserve(size_t capacity_to_reserve)
{
    return ReserveProxyObj(capacity_to_reserve);
}