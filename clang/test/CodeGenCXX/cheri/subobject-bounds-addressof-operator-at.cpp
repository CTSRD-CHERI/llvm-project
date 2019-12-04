// RUNNOT: %cheri128_purecap_cc1 -cheri-bounds=subobject-safe -O2 -std=c++17 -emit-llvm %s -o - -Rcheri-subobject-bounds -ast-dump -fcolor-diagnostics
// RUN: %cheri128_purecap_cc1 -cheri-bounds=subobject-safe -O2 -std=c++17 -emit-llvm %s -o /dev/null -Rcheri-subobject-bounds -verify

// A reduced version of the original test case: pointer_traits<char*>::pointer_to(std::string::__data[0])

// Some code in libc++ testsuite uses &string[0] to get a pointer to the internal data
// This code is wrong (it will yield a 1 byte pointer) and should use .data() or .c_str()

// Check that we can diagnose addressof on operator[] and suggest using .data() or .c_str

class string {
public:
  const char &operator[](int) const;
  char &operator[](int);

  char &at(int);
  const char &at(int) const;

  char *data();
  const char *data() const;
};

void do_stuff(const char *) {}
void do_stuff(const int *) {}

void test_string(const string &const_str, string &mutable_str) {
  do_stuff(&const_str[0]); // expected-warning{{pointer created from reference returned by 'operator[]' will be bounded to a single element}}
  // expected-remark@-1{{not setting bounds for pointer to 'const char' (source is a C++ reference and therefore should already have sub-object bounds)}}
  // expected-note@-2{{add an explicit cast to 'const char & __capability'}}

  do_stuff(&mutable_str[0]);  // expected-warning{{pointer created from reference returned by 'operator[]' will be bounded to a single element}}
  // expected-remark@-1{{not setting bounds for pointer to 'char' (source is a C++ reference and therefore should already have sub-object bounds)}}
  // expected-note@-2{{add an explicit cast to 'char & __capability'}}

  do_stuff(&const_str.at(0));  // expected-warning{{pointer created from reference returned by 'at' will be bounded to a single element}}
  // expected-remark@-1{{not setting bounds for pointer to 'const char' (source is a C++ reference and therefore should already have sub-object bounds)}}
  // expected-note@-2{{add an explicit cast to 'const char & __capability'}}
  do_stuff(&mutable_str.at(0));  // expected-warning{{pointer created from reference returned by 'at' will be bounded to a single element}}
  // expected-remark@-1{{not setting bounds for pointer to 'char' (source is a C++ reference and therefore should already have sub-object bounds)}}
  // expected-note@-2{{add an explicit cast to 'char & __capability'}}

  // These are fine
  do_stuff(const_str.data());
  do_stuff(mutable_str.data());

  // Explicit cast to reference should not warn:
  do_stuff(&(const char&)const_str.at(0)); // expected-remark{{not setting bounds for pointer to 'const char' (source is a C++ reference and therefore should already have sub-object bounds)}}
  do_stuff(&(const char&)const_str[0]); // expected-remark{{not setting bounds for pointer to 'const char' (source is a C++ reference and therefore should already have sub-object bounds)}}
  do_stuff(&(char&)mutable_str.at(0)); // expected-remark{{not setting bounds for pointer to 'char' (source is a C++ reference and therefore should already have sub-object bounds)}}
  do_stuff(&(char&)mutable_str[0]); // expected-remark{{not setting bounds for pointer to 'char' (source is a C++ reference and therefore should already have sub-object bounds)}}
}

template <typename T>
class vector {
public:
  const T &operator[](int) const;
  T &operator[](int);

  T &at(int);
  const T &at(int) const;

  T *data();
  const T *data() const;

  T& front();
  const T& front() const;
  T& back();
  const T& back() const;
};

template<typename T>
void test_vector(const vector<T> &const_vec, vector<T> &mutable_vec) {
  do_stuff(&const_vec[0]); // expected-warning{{pointer created from reference returned by 'operator[]' will be bounded to a single element}}
  // expected-remark@-1{{not setting bounds for pointer to 'const int' (source is a C++ reference and therefore should already have sub-object bounds)}}
  // expected-note@-2{{add an explicit cast to 'const int & __capability'}}

  do_stuff(&mutable_vec[0]);  // expected-warning{{pointer created from reference returned by 'operator[]' will be bounded to a single element}}
  // expected-remark@-1{{not setting bounds for pointer to 'int' (source is a C++ reference and therefore should already have sub-object bounds)}}
  // expected-note@-2{{add an explicit cast to 'int & __capability'}}

  do_stuff(&const_vec.at(0));  // expected-warning{{pointer created from reference returned by 'at' will be bounded to a single element}}
  // expected-remark@-1{{not setting bounds for pointer to 'const int' (source is a C++ reference and therefore should already have sub-object bounds)}}
  // expected-note@-2{{add an explicit cast to 'const int & __capability'}}
  do_stuff(&mutable_vec.at(0));  // expected-warning{{pointer created from reference returned by 'at' will be bounded to a single element}}
  // expected-remark@-1{{not setting bounds for pointer to 'int' (source is a C++ reference and therefore should already have sub-object bounds)}}
  // expected-note@-2{{add an explicit cast to 'int & __capability'}}

  // These are fine
  do_stuff(const_vec.data());
  do_stuff(mutable_vec.data());

  // Check that we also warn for front() and back()
  do_stuff(&const_vec.front());  // expected-warning{{pointer created from reference returned by 'front' will be bounded to a single element}}
  // expected-remark@-1{{not setting bounds for pointer to 'const int' (source is a C++ reference and therefore should already have sub-object bounds)}}
  // expected-note@-2{{add an explicit cast to 'const int & __capability'}}
  do_stuff(&mutable_vec.front());  // expected-warning{{pointer created from reference returned by 'front' will be bounded to a single element}}
  // expected-remark@-1{{not setting bounds for pointer to 'int' (source is a C++ reference and therefore should already have sub-object bounds)}}
  // expected-note@-2{{add an explicit cast to 'int & __capability'}}
  do_stuff(&const_vec.back());  // expected-warning{{pointer created from reference returned by 'back' will be bounded to a single element}}
  // expected-remark@-1{{not setting bounds for pointer to 'const int' (source is a C++ reference and therefore should already have sub-object bounds)}}
  // expected-note@-2{{add an explicit cast to 'const int & __capability'}}
  do_stuff(&mutable_vec.back());  // expected-warning{{pointer created from reference returned by 'back' will be bounded to a single element}}
  // expected-remark@-1{{not setting bounds for pointer to 'int' (source is a C++ reference and therefore should already have sub-object bounds)}}
  // expected-note@-2{{add an explicit cast to 'int & __capability'}}

  // Explicit cast to reference should not warn:
  do_stuff(&(const T&)const_vec.at(0)); // expected-remark{{not setting bounds for pointer to 'const int' (source is a C++ reference and therefore should already have sub-object bounds)}}
  do_stuff(&(const T&)const_vec.front()); // expected-remark{{not setting bounds for pointer to 'const int' (source is a C++ reference and therefore should already have sub-object bounds)}}
  do_stuff(&(const T&)const_vec.back()); // expected-remark{{not setting bounds for pointer to 'const int' (source is a C++ reference and therefore should already have sub-object bounds)}}
  do_stuff(&(const T&)const_vec[0]); // expected-remark{{not setting bounds for pointer to 'const int' (source is a C++ reference and therefore should already have sub-object bounds)}}
  do_stuff(&(T&)mutable_vec.at(0)); // expected-remark{{not setting bounds for pointer to 'int' (source is a C++ reference and therefore should already have sub-object bounds)}}
  do_stuff(&(T&)mutable_vec.front()); // expected-remark{{not setting bounds for pointer to 'int' (source is a C++ reference and therefore should already have sub-object bounds)}}
  do_stuff(&(T&)mutable_vec.back()); // expected-remark{{not setting bounds for pointer to 'int' (source is a C++ reference and therefore should already have sub-object bounds)}}
  do_stuff(&(T&)mutable_vec[0]); // expected-remark{{not setting bounds for pointer to 'int' (source is a C++ reference and therefore should already have sub-object bounds)}}


}

void test_vector_impl(const vector<int> &const_vec, vector<int> &mutable_vec) {
  test_vector<int>(const_vec, mutable_vec);
  // expected-remark@-1{{not setting bounds for reference to 'const vector<int>' (source is a C++ reference and therefore should already have sub-object bounds)}}
  // expected-remark@-2{{not setting bounds for reference to 'vector<int>' (source is a C++ reference and therefore should already have sub-object bounds)}}
}


// This previously caused crashes
class c {
public:
  c &operator*();
  c *operator->() { return &**this; } // expected-remark{{not setting bounds for pointer to 'c' (source is a C++ reference and therefore should already have sub-object bounds)}}
  void func();
};

void d(c e) {
  e->func();
}
