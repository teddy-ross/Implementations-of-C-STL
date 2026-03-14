template <typename T>
constexpr T&& forward(typename std::remove_reference<T>::type& t) noexcept{
  return static_cast<T&&>(t);
}

template <typename T>
constexpr typename std::remove_reference<T>::type&& move(T&& t) noexcept{
  return (static_cast<typename std::remove_reference<T>::type&&> (t));
}



int main(){
  return 0;
}
