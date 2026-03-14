// implementation of std::array

namespace my_array{

  template <typename T>
  class Array  {
  public:
    Array();
    Array(Array &&) = default;
    Array(const Array &) = default;
    Array &operator=(Array &&) = default;
    Array &Array=(const Array &) = default;
    ~Array();

  private:
    
    T* data_{nullptr};

    
  };


  
}









int main(){

  return 0;
}
