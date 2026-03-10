#include <cstddef>
#include <iostream>
#include <stdexcept>

namespace my_string{

class String;

class StringView {
public:
    static constexpr std::size_t npos = static_cast<std::size_t>(-1);

    // ── Constructors ────────────────────────────────────────────
    constexpr StringView() noexcept : data_{nullptr}, size_{0} {}
    constexpr StringView(const char* s) : data_{s}, size_{s ? std::strlen(s) : 0} {}
    constexpr StringView(const char* s, std::size_t len) : data_{s}, size_{len} {}
    StringView(const String& s) noexcept;  // declaration only

    // Rule of zero — default copy/move/destructor are all fine
    // because we don't own anything

    // ── Element access ──────────────────────────────────────────
    constexpr const char& operator[](std::size_t i) const { return data_[i]; }
    constexpr const char& at(std::size_t i) const {
        if (i >= size_) throw std::out_of_range("StringView::at");
        return data_[i];
    }
    constexpr const char* data() const { return data_; }

    // ── Capacity ────────────────────────────────────────────────
    constexpr std::size_t size() const { return size_; }
    constexpr std::size_t length() const { return size_; }
    constexpr bool empty() const { return size_ == 0; }

    // ── Operations ──────────────────────────────────────────────
    constexpr StringView substr(std::size_t pos, std::size_t count = npos) const {
        if (pos > size_) throw std::out_of_range("substr pos out of range");
        return StringView(data_ + pos, std::min(count, size_ - pos));
    }

    constexpr std::size_t find(char ch, std::size_t pos = 0) const {
        for (std::size_t i = pos; i < size_; ++i) {
            if (data_[i] == ch) return i;
        }
        return npos;
    }

    // ── Modifiers (narrow the view, not the data) ───────────────
    constexpr void remove_prefix(std::size_t n) {
        data_ += n;
        size_ -= n;
    }
    constexpr void remove_suffix(std::size_t n) {
        size_ -= n;
    }

    // ── Comparison ──────────────────────────────────────────────
    friend bool operator==(StringView a, StringView b) {
        return a.size_ == b.size_ && std::memcmp(a.data_, b.data_, a.size_) == 0;
    }
    friend bool operator!=(StringView a, StringView b) { return !(a == b); }
    friend bool operator<(StringView a, StringView b) {
        int cmp = std::memcmp(a.data_, b.data_, std::min(a.size_, b.size_));
        return cmp < 0 || (cmp == 0 && a.size_ < b.size_);
    }

    friend std::ostream& operator<<(std::ostream& os, StringView s) {
        os.write(s.data_, s.size_);
        return os;
    }
   
private:
    const char* data_;
    std::size_t size_;
};

class String {
public:
    static constexpr std::size_t npos = static_cast<std::size_t>(-1);

    // ── Constructors ────────────────────────────────────────────
    //
    // 1 allocate
    // 2 fill
    // 3 terminate
    //
    String(): data_{new char[1]},  size_{0}, capacity_{0}{
      data_[0] = '\0';
    }
    String(const char* s){
      size_ = s ? std::strlen(s) : 0;
      capacity_ = size_;
      data_ = new char[capacity_ + 1];
      if (s != nullptr){
        std::memcpy(data_, s, size_);
      }
      data_[size_] = '\0';
    }
    String(std::size_t count, char ch): data_(new char[count + 1]), size_{count}, capacity_{count}{
      std::memset(data_, ch, count);
      data_[count] = '\0';
    }

    // ── Rule of Five ────────────────────────────────────────────
    //
    String(const String& other): data_(new char[other.size_ + 1]), size_{other.size_}, capacity_{other.size_} {std::memcpy(data_, other.data_, size_ + 1);}
      
    String(String&& other) noexcept{
      data_ = std::exchange(other.data_,nullptr);
      size_ = std::exchange(other.size_, 0);
      capacity_ = std::exchange(other.capacity_, 0);
    }
    ~String(){
      delete[] data_;
    }

    String& operator=(String other) noexcept{ // copy-and-swap
      if (this == &other) return *this;

      swap(other);
      return *this;
    } 
    // ── Element access ──────────────────────────────────────────
    char& operator[](std::size_t i){
      return data_[i];
    }
    const char& operator[](std::size_t i) const{
      return data_[i];
    }
    char& at(std::size_t i){
      if (i >= size_){
        throw std::out_of_range("Index farther than size of string");
      }
      return data_[i];
    }
    const char* c_str() const{
      return data_;
    }
    const char* data() const{
      return data_;
    }
    char* data(){
      return data_;
    }

    // ── Capacity ────────────────────────────────────────────────
    std::size_t size() const{
      return size_;
    }
    std::size_t length() const{
      return size_;
    }
    std::size_t capacity() const{
      return capacity_;
    }
    bool empty() const{
      return (size_ == 0);
    };
    void reserve(std::size_t new_cap){
      if (new_cap <= capacity_) return;
      reallocate(new_cap);
    }

    // ── Modifiers ───────────────────────────────────────────────
    void clear(){
      size_ = 0;
      data_[0] = '\0';
    }
    void push_back(char ch){
      append(&ch, 1);
    }
    void pop_back(){
      if (size_ > 0){
        size_ -= 1;
      }
      data_[size_] = '\0';
    }
    String& operator+=(const String& other){
      append(other.data_, other.size_);
      return *this;
  
    }
    String& operator+=(const char* s){
      append(s, std::strlen(s));
      return *this;
    }
    String& operator+=(char ch){
      append(&ch, 1);
      return *this;
    }
    String& operator+=(StringView sv) {
      append(sv.data(), sv.size());
      return *this;
    }
    
    operator StringView() const noexcept { return StringView(data_, size_); }

    String substr(std::size_t pos, std::size_t count = npos) const {
      if (pos > size_) throw std::out_of_range("substr pos out of range");
      std::size_t actual = std::min(count, size_ - pos);
      String result;
      result.reserve(actual);
      result.append(data_ + pos, actual);
      return result;
    }

    std::size_t find(char ch, std::size_t pos = 0) const {
      for (std::size_t i = pos; i < size_; ++i) {
        if (data_[i] == ch) return i;
      }
    return npos;
    }    
    
    void swap(String& other) noexcept{
      std::swap(data_, other.data_);
      std::swap(size_, other.size_);
      std::swap(capacity_, other.capacity_);
    }

    // ── Comparison (hidden friends) ─────────────────────────────
    friend bool operator==(const String& a, const String& b){
      return a.size_ == b.size_ && std::memcmp(a.data_, b.data_, a.size_) == 0;
    }
    friend bool operator!=(const String& a, const String& b){ 
      return !(a == b);
    }
    friend bool operator<(const String& a, const String& b){
          int cmp = std::memcmp(a.data_, b.data_, std::min(a.size_, b.size_));
          return cmp < 0 || (cmp == 0 && a.size_ < b.size_);
    }

    // ── Concatenation ───────────────────────────────────────────
    friend String operator+(const String& a, const String& b){
          String result;
          result.reserve(a.size_ + b.size_);
          result.append(a.data_, a.size_);
          result.append(b.data_, b.size_);
          return result;
}
    

    // ── Stream output ───────────────────────────────────────────
    friend std::ostream& operator<<(std::ostream& os, const String& s){
      os.write(s.data_, s.size_);
      return os;
    }



private:
    char*       data_;
    std::size_t size_;
    std::size_t capacity_;

    void reallocate(std::size_t new_cap){
      char* memory = new char [new_cap + 1]; // allocate new memory 
      std::memcpy(memory, data_, size_ + 1); // copy old data. std::memcpy(destination, source, numberOfBytes)
      delete [] data_; // delete old data
      data_ = memory; // sub in old data
      capacity_ = new_cap; // set capacity_ to new capacity 
    }

    void append(const char* s, std::size_t len){
      if(size_ + len > capacity_){ // check if size_ + len_ exceeds our capacity and make adjustment if so
        reallocate(std::max(size_ + len, capacity_ * 2));
      }
      std::memcpy(size_ + data_, s,len); // copy new 
      size_ += len;
      data_[size_] = '\0';
    }
};

 inline StringView::StringView(const String& s) noexcept
     : data_{s.data()}, size_{s.size()} {}

}

   

int main(){
  
  auto variable = my_string::String{"Hello"};
  std::cout << variable << '\n';

  return 0;
}
