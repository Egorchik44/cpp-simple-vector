#pragma once

#include "array_ptr.h"

#include <cassert>
#include <initializer_list>
#include <stdexcept>


class ReserveProxyObj {
public:
    ReserveProxyObj(const size_t capacity_to_reserve)
        :class_assistant_(capacity_to_reserve)
    {
    }

    size_t GetSize() {
        return class_assistant_;
    }

private:
    size_t class_assistant_;
};

ReserveProxyObj Reserve(const size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size)
        : SimpleVector(size, Type{})
    {
    }

    // Конструктор с вызовом функции Reserve
    explicit SimpleVector(ReserveProxyObj obj)
    {
        Reserve(obj.GetSize());
    }

    SimpleVector(const SimpleVector& other)
        :items_(other.size_)
        , size_(other.size_)
        , capacity_(other.size_)
    {
        std::copy(other.begin(), other.end(), items_.Get());
    }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value)
        : items_(size)
        , size_(size)
        , capacity_(size)
    {
        std::fill(items_.Get(), items_.Get() + size_, value);
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init)
        :items_(init.size())
        , size_(init.size())
        , capacity_(init.size())
    {
        std::copy(init.begin(), init.end(), items_.Get());
    }

    // конструктор перемещения
    SimpleVector(SimpleVector&& other)
        : items_(other.capacity_)
    {
        swap(other);
    }
    SimpleVector& operator=(const SimpleVector& rhs) {
        if (this != &rhs) {
            if (rhs.IsEmpty()) {
                Clear();
                return *this;
            }
            SimpleVector<Type> arr_ptr(rhs.size_);
            std::copy(rhs.begin(), rhs.end(), arr_ptr.begin());
            arr_ptr.capacity_ = capacity_;
            swap(arr_ptr);
        }
        return *this;
    }

    // Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept {
        assert(index <= capacity_);
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        assert(index <= capacity_);
        return items_[index];
    }

    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept {
        return Iterator{ items_.Get() };
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept {
        return Iterator{ &items_[size_] };
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept {
        return ConstIterator{ items_.Get() };
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept {
        return ConstIterator{ &items_[size_] };
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept {
        return ConstIterator{ items_.Get() };
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept {
        return ConstIterator{ &items_[size_] };
    }

    // Возвращает количество элементов в массиве
    size_t GetSize() const noexcept {
        return size_;
    }

    // Возвращает вместимость массива
    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    // Сообщает, пустой ли массив
    bool IsEmpty() const noexcept {
        return size_ == 0;
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("Work with error date!");
        }
        else {
            return items_[index];
        }
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("Work with error date!");
        }
        else {
            return items_[index];
        }
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        size_ = 0;
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Fill(Iterator first, Iterator last) {
        assert(first < last);
        for (; first != last; ++first) {
            *first = std::move(Type());
        }
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size) {
        if (new_size <= size_) {
            size_ = new_size;
        }
        if (new_size <= capacity_) {
            Fill(items_.Get() + size_, items_.Get() + size_ + new_size);
        }
        if (new_size > capacity_) {
            size_t new_capacity = std::max(new_size, capacity_ * 2);
            ArrayPtr<Type> arr_ptr(new_capacity);
            Fill(arr_ptr.Get(), arr_ptr.Get() + new_capacity);
            std::move(items_.Get(), items_.Get() + capacity_, arr_ptr.Get());
            items_.swap(arr_ptr);

            size_ = new_size;
            capacity_ = new_capacity;
        }
    }

    void Reserve(size_t new_capacity) {
        if (new_capacity > capacity_) {
            SimpleVector<Type> tmp_(new_capacity);
            std::copy(cbegin(), cend(), tmp_.begin());
            tmp_.size_ = size_;
            swap(tmp_);
        }
    }

    void PushBack(const Type& item) {
        if (size_ > capacity_) {
            auto new_capacity = std::max(size_t(1), 2 * capacity_); //защита, если capacity_=0
            ArrayPtr<Type> arr_ptr(new_capacity);
            std::fill(arr_ptr.Get(), arr_ptr.Get() + new_capacity, Type());
            std::copy(items_.Get(), items_.Get() + size_, arr_ptr.Get());
            arr_ptr[size_] = item;
            items_.swap(arr_ptr);
            capacity_ = new_capacity;
        }
        items_[size_] = std::move(item);
        ++size_;
    }

    void PushBack(Type&& item) {
        if (size_ + 1 > capacity_) {
            size_t new_capacity = std::max(size_ + 1, capacity_ * 2);
            ArrayPtr<Type> temp(new_capacity);
            std::move(items_.Get(), items_.Get() + size_, temp.Get());
            items_.swap(temp);
            capacity_ = new_capacity;
        }
        items_[size_] = std::move(item);
        ++size_;
    }

    // Вставляет значение value в позицию pos.
    // Возвращает итератор на вставленное значение
    // Если перед вставкой значения вектор был заполнен полностью,
    // вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
    Iterator Insert(ConstIterator pos, const Type& value) {
    assert(pos >= begin() && pos <= end());

    size_t element = std::distance(cbegin(), pos);
    if (capacity_ == 0) {
        ArrayPtr<Type> app_ptr(1);
        app_ptr[element] = value;
        items_.swap(app_ptr);
        ++capacity_;
    }
    else if (size_ < capacity_) {
        std::copy_backward(items_.Get() + element, items_.Get() + size_,
            items_.Get() + size_ + 1);
        items_[element] = value;
    }
    else {
        size_t new_capacity = std::max(size_ + 1, capacity_ * 2);
        ArrayPtr<Type> app_ptr(capacity_);
        std::copy(items_.Get(), items_.Get() + size_,
            app_ptr.Get());
        std::copy_backward(items_.Get() + element, items_.Get() + size_,
            app_ptr.Get() + size_ + 1);
        app_ptr[element] = value;
        items_.swap(app_ptr);
        capacity_ = new_capacity;
    }
    ++size_;

    return &items_[element];
}



// метод Insert перемещением
Iterator Insert(Iterator pos, Type&& value) {
    assert(pos >= begin() && pos <= end());

    size_t element = std::distance(begin(), pos);
    if (capacity_ == 0) {
        ArrayPtr<Type> app_ptr(1);
        app_ptr[element] = std::move(value);
        items_.swap(app_ptr);
        ++capacity_;
    }
    else if (size_ < capacity_) {
        std::move_backward(items_.Get() + element, items_.Get() + size_,
            items_.Get() + size_ + 1);
        items_[element] = std::move(value);
    }
    else {
        size_t new_capacity = std::max(size_ + 1, capacity_ * 2);
        ArrayPtr<Type> app_ptr(capacity_);
        std::move(items_.Get(), items_.Get() + size_,
            app_ptr.Get());
        std::move_backward(items_.Get() + element, items_.Get() + size_,
            app_ptr.Get() + size_ + 1);
        app_ptr[element] = std::move(value);
        items_.swap(app_ptr);
        capacity_ = new_capacity;
    }
    ++size_;

    return &items_[element];
}


    // "Удаляет" последний элемент вектора. Вектор не должен быть пустым
    void PopBack() noexcept {
        assert(!IsEmpty());
        --size_;
    }

    // Удаляет элемент вектора в указанной позиции
    Iterator Erase(ConstIterator pos) {
        assert(pos >= cbegin() && pos <= cend());
        auto element = std::distance(cbegin(), pos);
        std::move(items_.Get() + element + 1, items_.Get() + size_, items_.Get() + element);
        --size_;
        return &items_[element];
    }

    // Обменивает значение с другим вектором
    void swap(SimpleVector& other) noexcept {
        std::swap(capacity_, other.capacity_);
        std::swap(size_, other.size_);
        items_.swap(other.items_);
    }

private:
    ArrayPtr<Type> items_;
    size_t size_ = 0;
    size_t capacity_ = 0;

};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(rhs < lhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return (rhs < lhs);
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(rhs < lhs);
}