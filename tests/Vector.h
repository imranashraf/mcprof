 #pragma once
 
#include "stdio.h"
#include <cstring>
#include <cassert>

#include <stdlib.h>
#include <string.h>

#define USE_MALLOC_WRAPPERS

#include "malloc_wrap.h"

template <class Type> class Vector
{
  unsigned int capacity_;
  unsigned int size_;
  Type *data_;

public:

  Vector()
  {
    data_ = 0;
    capacity_ = 0;
    size_ = 0;
  }
  
  Vector(unsigned int capacity)
  {
    data_ = 0;
    size_ = 0;
    init(capacity_);
  }
  
  void setZero()
  {
    if (data_ != 0)
    {
      memset(data_, 0, size*sizeof(Type));
    }
  }
  
  inline unsigned int getCapacity()
  {
    return capacity_;
  }
  
  inline unsigned int size()
  {
    return size_;
  }  
  
  void clear()
  {
    size_ = 0;
  }
  
  void init(unsigned int capacity)
  {
    clear();
    
    if (data_ != 0)
    {
      free(data_);
    }
    
    capacity_ = capacity;
    data_ = (Type*)malloc(capacity_ * sizeof(Type));
  }
  
  inline const Type operator()(unsigned int index) const
  {
    #if __i386__ || __amd64__
    //assert (index < size_);
    #endif
    return data_[index];
  }
  
  inline Type& operator()(unsigned int index)
  {
    #if __i386__ || __amd64__
    //assert (index < size_);
    #endif
    return data_[index];
  } 
  
  void push_back(Type value)
  {
    #if __i386__ || __amd64__
    //printf("%d %d\n", size_, capacity_);
    //assert (size_ < capacity_);
    #endif
    data_[size_] = value;
    size_++;
  }

  inline Type* getStorage()
  {
    return data_;
  }

  ~Vector()
  {
    if (data_ != 0)
    {
      delete[] data_;
    }
  }
};
