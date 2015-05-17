/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** circular_vector.h
** An STL-Compliant Circular Vector Container. Shares similarities to a Circular
** Buffer Data Structure, except allows access to all elements in the container.
** Contains all the same member functions as C++98 std::vector from the STL,
** except for vector::insert and vector::erase.
** Newly introduced member functions include: push_front and pop_front which
** unlike the counterpart operations in std::vector are O(1) time (Constant).
**
**
** Author: Konrad Janica
** -------------------------------------------------------------------------*/

#ifndef CIRCULAR_VECTOR_HPP_
#define CIRCULAR_VECTOR_HPP_

#include <algorithm> // std::swap, std::max, std::lexicographical_compare, std::equal
#include <stdexcept> // std::invalid_argument, std::out_of_range

// Forward declaration of iterator class
template <typename T_noconst, typename T, typename element_type = typename T::value_type>
class circular_vector_iterator;

// An STL Compliant Circular Vector Container
//   This Data Structure is basically a centered wrapping vector with space at both sides
//   to allow O(1) (constant time) insert/erase(front) as well as O(1) push_back and pop_back.
//   Internal structure is an array with an index starting from the center of capacity
//   and wrapping around to the start of the array.
//   As elements are added to the front and end, the start and end indices move
//   accordingly.
//   When capacity is reached, the storage container will reallocate its storage
//   increasing its capacity to 1.5 * capacity.
//     Like vectors, this makes push_back and push_front amortized time O(1)
//     per insertion.
//   The Default Capacity should be larger than 1 otherwise a Circular Vector is
//   pointless.
template <typename T, typename Alloc = std::allocator<T> >
class circular_vector {
  public:
    // TYPEDEFS:
    typedef circular_vector<T, Alloc>            self_type;
    typedef Alloc                                allocator_type;
    typedef typename Alloc::value_type           value_type;
    typedef typename Alloc::pointer              pointer;
    typedef typename Alloc::const_pointer        const_pointer;
    typedef typename Alloc::reference            reference;
    typedef typename Alloc::const_reference      const_reference;
    typedef typename Alloc::size_type            size_type;
    typedef typename Alloc::difference_type      difference_type;
    // Iterator
    typedef circular_vector_iterator <self_type, self_type> 
      iterator;
    // Const Iterator
    typedef circular_vector_iterator <self_type,const self_type, const value_type> 
      const_iterator;
    // Reverse Iterator
    typedef std::reverse_iterator<iterator>       reverse_iterator;
    // Reverse Const Iterator
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    // CONSTANTS:
    static const size_type kDefaultCapacity = 5;
    enum SelectIndex {
      kStart = 0,
      kEnd = 1,
    };

    // CONSTRUCTORS:
    // @brief  Empty container constructor (default constructor).
    //         Constructs an empty container, with no elements. With a specified amount of 
    //         reserved space.
    // @param  capacity  The starting allocated storage reserve
    // @throws  std::invalid_argument  With negative capacity values
    explicit circular_vector(size_type capacity = kDefaultCapacity, const allocator_type &alloc = allocator_type())
      : size_(0), capacity_(capacity), start_idx_(capacity/2), end_idx_(capacity/2),
      alloc_(alloc), array_(alloc_.allocate(capacity)) {
        if (capacity <= 0) {
          throw std::invalid_argument("invalid capacity");
        }
      };
    // @brief  Fill constructor. Constructs a container with @a n elements. Each element is a copy of @a val.
    // @param  n    The size and capacity of the %circular_vector
    // @param  val  The data value to fill the %circular_vector
    // @throws  std::invalid_argument  With negative size values
    explicit circular_vector(size_type n, const value_type &val, const allocator_type &alloc = allocator_type())
      : size_(0), capacity_(n), start_idx_(n/2), end_idx_(n/2),
      alloc_(alloc), array_(alloc_.allocate(n)) {
        if (n <= 0) {
          throw std::invalid_argument("invalid capacity");
        }
        resize(n, val);
      };

    // @brief  Range constructor. Constructs a container with as many elements as the range [first,last), 
    //         with each element constructed from its corresponding element in that range, in the same order.
    // @param  first  The initial position to start the copy from
    // @param  last   The final exclusive position of the copy range
    template <class InputIterator>
      circular_vector(InputIterator first, InputIterator last, const allocator_type& alloc = allocator_type())
      : size_(0), capacity_(last-first), start_idx_((last-first)/2), end_idx_((last-first)/2),
      alloc_(alloc), array_(alloc_.allocate(last-first)) {
        try {
          assign(first,last);
        } catch (...) {
          clear();
          alloc_.deallocate(array_, capacity_);
          throw std::length_error("out of memory");
        }
      }
    // @brief  Copy constructor. Constructs a container with a copy of each of the elements in x, in the same order.
    // @param  x  Another vector object of the same type (with the same class template arguments T and Alloc), whose contents are copied.
    // @throws  std::length_error  Upon catching any exception while assigning memory
    circular_vector(const circular_vector &x)
      : size_(0), capacity_(x.capacity()), start_idx_(x.capacity()/2), end_idx_(x.capacity()/2),
      alloc_(x.get_allocator()), array_(x.get_allocator().allocate(x.capacity())) {
        try {
          assign(x.begin(), x.end());
        } catch (...) {
          clear();
          alloc_.deallocate(array_, capacity_);
          throw std::length_error("out of memory");
        }
      }
    // @brief  Move constructor. Assigns new contents to the container, replacing its current contents, and modifying its size accordingly.
    // @param  x  A vector object of the same type (i.e., with the same template parameters, T and Alloc).
    circular_vector &operator = (const self_type &x) {
      size_ = 0;
      capacity_ = x.capacity();
      start_idx_ = x.capacity()/2;
      end_idx_ = x.capacity()/2;
      array_ = alloc_.allocate(x.capacity());

      assign(x.begin(), x.end());
      return *this;
    }

    // DECONSTRUCTORS:
    ~circular_vector() {
      clear();
      alloc_.deallocate(array_, capacity_);
    };

    // ITERATORS
    // begin(), An iterator referring to array_[0], i.e. the first element
    // @warn  Iterator should be repositioned upon capacity reallocation or after push_front call
    iterator         begin()              { return iterator(this, 0); }
    const_iterator   begin() const        { return const_iterator(this, 0); }
    // end(), An iterator referring to array_[size()], i.e. past-the-end element
    // @warn  Iterator should be repositioned upon capacity reallocation or after push_back call
    iterator         end()                { return iterator(this, size()); }
    const_iterator   end() const          { return const_iterator(this, size()); }
    // rbegin()
    // @warn  Iterator should be repositioned upon capacity reallocation or after push_back call
    reverse_iterator rbegin()             { return reverse_iterator(end()); }
    const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
    // rend()
    // @warn  Iterator should be repositioned upon capacity reallocation or after push_front call
    reverse_iterator rend()               { return reverse_iterator(begin()); }
    const_reverse_iterator rend() const   { return const_reverse_iterator(begin()); }

    // ALLOCATORS:
    // @brief  Returns a copy of the allocator object associated with the @circular_vector
    // @return  Read-only (constant) allocator
    allocator_type get_allocator() const  { return alloc_; };

    // CAPACITIES:
    // @brief  Returns the amount of elements in the %circular_vector
    // @return  Read-only (constant) circular size
    size_type size() const     { return size_; };
    // @brief  Returns the maximum number of elements that the %circular_vector can hold
    //         during dynamic allocation mode
    // @return  Read-only (constant) maximum size
    size_type max_size() const { return alloc_.max_size(); };
    // @brief  Resizes the %circular_vector to specified size
    // @param  n  Number of elements the %circular_vector should contain.
    // @param  val  The value of the element to fill the extra size
    // @warn  This function changes the actual content of the container by inserting or
    //        erasing elements from it (unless @a n = size()).
    //        If the number is smaller than the %circular_vector's current size the
    //        %circular_vector is truncated, otherwise default (or specified) elements
    //        are appended until size reaches @a n size.
    //        If capacity needs to increase => capacity becomes @a n.
    //        Capacity never shrinks.
    void resize(size_type n, const value_type &val = value_type()) {
      if (n > size()) {
        if (n > capacity()) {
          circular_vector temp(n); // Capacity = n
          temp.assign(begin(), end());
          swap(temp);
        }
        // Push new valued elements until size() = n
        while (n != size()) {
          push_back(val);
        }
      } else if (n < size()) {
        // A more efficient method of doing this can be used
        // but must ensure wrapped arrays are addressed properly.
        while (n != size())
          pop_back();
      }
      // else n == size() => do nothing
    };
    // @brief  Returns size of allocated storage capacity
    size_type capacity() const { return capacity_; };
    // @brief  Returns true if there are elements in the %circular_vector
    // @return  Read-only (constant) True iff end index is in default state 
    bool empty() const         { return !size_; };
    // @brief  Request that the %circular_vector capacity be at least enough to contain
    //         n elements. This function has no effect on the %circular_vector size and
    //         cannot alter its elements.
    // @warn  If @a n is greater than the current %circular_vector capacity, the function
    //        causes the container to reallocate its storage increasing its
    //        capacity to @a n (or greater). O(n) time and space required when this occurs
    void reserve(size_type n) {
      if (capacity() < n) {
        circular_vector temp( std::max<size_type>(n, capacity() * 1.5) );
        temp.assign(begin(), end());
        swap(temp);
      }
    }

    // MODIFIERS:
    // @brief  Fills a %circular_vector with copies of the elements in the
    //         range [start, last)
    // @param  start  An input iterator
    // @param  last   An input iterator
    // @warn  The assignment completely changes the %circular_vector and the
    //        resulting %circular_vector's size is the same as the number of 
    //        elements assigned. Old data will be lost
    template <typename iter>
      void assign(iter start, iter last) {
        if (size() != 0) 
          clear();
        while (start != last) {
          push_back(*start);
          ++start;
        }
      }
    // @brief  Fills a %circular_vector with the specified value in the
    //         range [0, n)
    // @param  n    Number of elements to be assigned
    // @param  val  Value to be assigned
    // @warn  The assignment completely changes the %circular_vector and the
    //        resulting %circular_vector's size is the same as the number of 
    //        elements assigned. Old data will be lost
    void assign(size_type n, const value_type &val) {
      if (size() != 0)
        clear();
      while (n != 0) {
        push_back(val);
        --n;
      }
    }
    // @brief  Removes the first indexed element
    // @warn  Undefined behaviour when calling on an empty %circular_vector
    void pop_front() {
      alloc_.destroy(array_ + start_idx_);
      increment(kStart);
    }
    // @brief  Removes the last indexed element
    // @warn  Undefined behaviour when calling on an empty %circular_vector
    void pop_back() {
      decrement(kEnd);
      alloc_.destroy(array_ + end_idx_);
    }
    // @brief  Adds an element to the head of the %circular_vector
    //         and decrements the start index
    // @param  val  Element to be added
    // @warn  If capacity has been reached, the function causes the container to
    //        reallocate its storage increasing its capacity to 1.5 * capacity.
    //        O(n) time and space required when this occurs
    void push_front(const value_type &val) {
      if (end_idx_ == start_idx_ && !empty()) {
        reserve(capacity() * 1.5);
      } else if (end_idx_ == start_idx_) {
        // Do a push_back only on empty case
        push_back(val);
        return;
      }

      decrement(kStart);
      array_[start_idx_] = val;
    }
    // @brief  Adds an element to the tail of the %circular_vector
    // @param  val  Element to be added
    // @warn  If capacity has been reached, the function causes the container to
    //        reallocate its storage increasing its capacity to 1.5 * capacity.
    //        O(n) time and space required when this occurs
    void push_back(const value_type &val) {
      if (end_idx_ == start_idx_ && !empty())
        reserve(capacity() * 1.5);

      array_[end_idx_] = val;
      increment(kEnd);
    }
    // @brief  Exchanges the content of the container by the content of x, which is
    //         another %circular_vector object of the same type. Sizes may differ.
    // @param  x  The %circular_vector of the same type to swap with.
    void swap(circular_vector &x) {
      std::swap(size_,       x.size_);
      std::swap(capacity_,   x.capacity_);
      std::swap(start_idx_,  x.start_idx_);
      std::swap(end_idx_,    x.end_idx_);
      std::swap(array_,      x.array_);
    }
    // @brief  Removes all elements from the @circular_vector (which are destroyed),
    //         leaving the container with a size of 0.
    // @warn  If the elements themselves are pointers, the pointed-to memory is not
    //        touched in any way. Managing the pointer is the user's responsibility.
    void clear() {
      for (size_type x = 0; x < capacity(); ++x) {
        alloc_.destroy(array_ + x);
      }
      start_idx_ = capacity() / 2;
      end_idx_ = capacity() /2;
      size_ = 0;
    }

    // ELEMENT ACCESS:
    // @brief  Provides access to the data contained in %circular_vector
    // @param n The index of the element for which data should be accessed
    // @return  Read/write reference to data
    // @warn  Calling this function with an argument @a n that is out of range
    //        causes undefined behaviour
    reference operator [] (size_type n)             { return normalize(n); };
    // @brief  Provides access to the data contained in %circular_vector
    // @param n The index of the element for which data should be accessed
    // @return  Read/write reference to data
    // @warn  Calling this function with an argument @a n that is out of range
    //        causes undefined behaviour
    const_reference operator [] (size_type n) const { return normalize(n); };
    // @brief  Provides access to the data contained in %circular_vector
    // @param n The index of the element for which data should be accessed
    // @return  Read/write reference to data
    // @throw  std::out_of_range  If @a n is an invalid index
    reference at(size_type n) {
      if (n > size()-1)
        throw std::out_of_range("index larger than last index");
      if (n < 0)
        throw std::out_of_range("negative index");
      return normalize(n);
    };
    // @brief  Provides access to the data contained in %circular_vector
    // @param n The index of the element for which data should be accessed
    // @return  Read-only (constant) reference to data
    // @throw  std::out_of_range  If @a n is an invalid index
    const_reference at(size_type n) const {
      if (n > size()-1)
        throw std::out_of_range("index larger than last index");
      if (n < 0)
        throw std::out_of_range("negative index");
      return normalize(n);
    };
    // @return  Read/Write reference to the first indexed element 
    //          in %circular_vector
    // @warn  Calling this function on an empty container causes undefined
    //        behaviour
    reference front()              { return array_[start_idx_]; };
    // @return  Read-only (constant) reference to the first indexed element 
    //          in %circular_vector
    // @warn  Calling this function on an empty container causes undefined
    //        behaviour
    const_reference front() const  { return array_[start_idx_]; };
    // @return  Read/Write reference to the last indexed element 
    //          in %circular_vector
    // @warn  Calling this function on an empty container causes undefined
    //        behaviour
    reference back()               { return *(end()-1); };
    // @return  Read-only (constant) reference to the last indexed element 
    //          in %circular_vector
    // @warn  Calling this function on an empty container causes undefined
    //        behaviour
    const_reference back()  const  { return *(end()-1); };


  private:
    // Number of elements in the %circular_vector
    size_type size_;
    // Currently allocatd memory of the %circular_vector
    size_type capacity_;
    // Index of the start of the %circular_vector in array_
    size_type start_idx_;
    // Index of the end of the %circular_vector in array_
    size_type end_idx_;
    // Defined Memory Allocator
    allocator_type alloc_;
    // The Data Storage Array
    value_type * array_;

    // HELPER FUNCTIONS:
    // @brief  Increments the specified index and changes size appropriately
    // @param  index  The enum representing 0 - start_idx_ or 1 - end_idx_
    void increment(size_type index) {
      switch(index) {
        case kStart:
          start_idx_ = (start_idx_ + 1) % capacity();
          --size_;
          break;
        case kEnd:
          end_idx_ = (end_idx_ + 1) % capacity_;
          ++size_;
          break;
        default:
          throw std::invalid_argument("invalid enumerator");
      }
    }
    // @brief  Decrements the specified index and changes size appropriately
    // @param  index  The enum representing 0 - start_idx_ or 1 - end_idx_
    void decrement(size_type index) {
      switch(index) {
        case kStart:
          if (start_idx_ == 0)
            start_idx_ = capacity() - 1;
          else
            --start_idx_;
          ++size_;
          break;
        case kEnd:
          if (end_idx_ == 0)
            end_idx_ = capacity() - 1;
          else
            --end_idx_;
          --size_;
          break;
        default:
          throw std::invalid_argument("invalid enumerator");
      }
    }
    // @brief  Returns the given index normalized to the %circular_vector wrapping
    // @param n The index of the element for which data should be accessed
    // @return  Read/write reference to data
    // @warn  Calling this function with an argument @a n that is out of range
    //        causes undefined behaviour
    reference normalize(size_type n) const {
      return array_[(start_idx_ + n) % capacity()];
    }
};

// RELATIONAL OPERATORS:
// a==b
template <typename T, typename Alloc>
bool operator==(const circular_vector<T, Alloc> &a, const circular_vector<T, Alloc> &b) {
  return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin());
}
// a!=b which is equivalent to !(a==b)
template <typename T, typename Alloc>
bool operator != (const circular_vector<T, Alloc> &a, const circular_vector<T, Alloc> &b) {
  return !(a==b);
}
// a<b
template <typename T, typename Alloc>
bool operator < (const circular_vector<T, Alloc> &a, const circular_vector<T, Alloc> &b) {
  return std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end());
}
// a>b
template <typename T, typename Alloc>
bool operator > (const circular_vector<T, Alloc> &a, const circular_vector<T, Alloc> &b) {
  return std::lexicographical_compare(b.begin(), b.end(), a.begin(), a.end());
}
// a<=b which is equivalent to !(b<a)
template <typename T, typename Alloc>
bool operator <= (const circular_vector<T, Alloc> &a, const circular_vector<T, Alloc> &b) {
  return !(b<a);
}
// a>=b which is equivalent to !(a<b)
template <typename T, typename Alloc>
bool operator >= (const circular_vector<T, Alloc> &a, const circular_vector<T, Alloc> &b) {
  return !(a<b);
}

// The iterator type for the %circular_vector container.
//   The following template class provides all variants of forward/reverse/const/noconst
//   iterators using template properties.
//   It is enough to instantiate it using %circular_vector
//   @sample usage: circular_vector<int>::iterator it = foo.begin();
//                  foo++... etc.
template <typename T_noconst, typename T, typename element_type>
class circular_vector_iterator {
  public:
    typedef circular_vector_iterator<T_noconst,T,element_type> self_type;

    typedef std::random_access_iterator_tag     iterator_category;
    typedef typename T::value_type              value_type;
    typedef typename T::size_type               size_type;
    typedef typename T::pointer                 pointer;
    typedef typename T::const_pointer           const_pointer;
    typedef typename T::reference               reference;
    typedef typename T::const_reference         const_reference;
    typedef typename T::difference_type         difference_type;

    circular_vector_iterator(T * cbuffer, size_type index)
      : carray_(cbuffer), index_(index) {};

    // Converting a non-const iterator to a const iterator
    circular_vector_iterator(const circular_vector_iterator<T_noconst,
        T_noconst, typename T_noconst::value_type> &other)
      : carray_(other.carray_), index_(other.index_) {};

    friend class circular_vector_iterator<const T, const T, const element_type>;

    // Use compiler generated copy constructor, copy assignment operator
    // and destructor

    element_type &operator * ()  { return (*carray_)[index_]; };
    element_type *operator -> () { return &(operator * ()); };

    self_type &operator ++ () {
      index_++;
      return *this;
    }
    self_type operator ++ (int) {
      self_type temp(*this);
      ++(*this);
      return temp;
    }

    self_type &operator -- () {
      index_--;
      return *this;
    }
    self_type operator -- (int) {
      self_type temp(*this);
      --(*this);
      return temp;
    }

    self_type operator + (difference_type n) const {
      self_type temp(*this);
      temp.index_ += n;
      return temp;
    }
    self_type &operator += (difference_type n) {
      index_ += n;
      return *this;
    }

    self_type operator - (difference_type n) const {
      self_type temp(*this);
      temp.index_ -= n;
      return temp;
    }
    self_type &operator -= (difference_type n) {
      index_ -= n;
      return *this;
    }

    difference_type operator - (const self_type &c) const {
      return index_ - c.index_;
    }

    bool operator == (const self_type &other) const {
      return index_ == other.index_ && carray_ == other.carray_;
    }
    bool operator != (const self_type &other) const {
      return index_ != other.index_ && carray_ == other.carray_;
    }
    bool operator >( const self_type &other) const {
      return index_ > other.index_;
    }
    bool operator >= (const self_type &other) const {
      return index_ >= other.index_;
    }
    bool operator <( const self_type &other) const {
      return index_ < other.index_;
    }
    bool operator <= (const self_type &other) const {
      return index_ <= other.index_;
    }

  private:
    T * carray_;
    size_type  index_;
};

template <typename circular_vector_iterator_t>
circular_vector_iterator_t operator + (const typename circular_vector_iterator_t::difference_type &a,
    const circular_vector_iterator_t &b) {
  return circular_vector_iterator_t(a) + b;
}

template <typename circular_vector_iterator_t>
circular_vector_iterator_t operator - (const typename circular_vector_iterator_t::difference_type &a,
    const circular_vector_iterator_t &b) {
  return circular_vector_iterator_t(a) - b;
}

#endif
