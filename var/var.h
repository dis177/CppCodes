#include "config.h"

#if !defined(__VAR_H__) && defined(__VAR_CONFIG_H__)
#define __VAR_H__

#include <string>
#include <functional>
#include <typeindex>
#include <iostream>


#ifdef REGISTRY
#define __VAR_TYPES__ __VAR_FUNDAMENTAL_TYPES__, REGISTRY
#else
#define __VAR_TYPES__ __VAR_FUNDAMENTAL_TYPES__
#endif

#if defined(_MSC_VER)
# define VAR_INLINE inline
#elif defined(__GNUC__) || defined(__clang__)
# define VAR_INLINE __attribute__((always_inline)) inline
# include <cxxabi.h>
#endif

using var = Var::var;

namespace registry {
  template <class _Tp>
  using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<_Tp>>;

  template <class... Args>
  struct types_counter {
    static constexpr size_t size = sizeof...(Args);
  };

#if defined(VAR_USE_TYPES) || defined(VAR_USE_FILTERED_TYPES)
  template <class _First, class... _Rest>
  struct var_types {
    using first = _First;
    using rest = var_types<_Rest...>;
    static constexpr size_t size = 1 + rest::size;
    template <class _Tp>
    static var_types<_First, _Rest..., _Tp> append();
  };
  template <class _First>
  struct var_types<_First> {
    using first = _First;
    constexpr static size_t size = 1;
    template <class _Tp>
    static var_types<_First, _Tp> append();
  };

  template <size_t N, class _Types>
  struct types_visitor {
    using type = typename types_visitor<N - 1, typename _Types::rest>::type;
  };
  template <class _Types>
  struct types_visitor<0, _Types> {
    using type = typename _Types::first;
  };
#else
  template <size_t N, class _First, class... _Rest>
  struct types_visitor {
    using type = types_visitor<N - 1, _Rest...>::type;
  };
  template <class _First, class... _Rest>
  struct types_visitor<0, _First, _Rest...> {
    using type = _First;
  };
#endif

#ifdef VAR_USE_FILTERED_TYPES

  template <class _Types>
  struct types_filter {
    template <class _Tp, size_t... N>
    static auto append(std::index_sequence<N...>) {
      if constexpr (((!std::is_same_v<_Tp, typename types_visitor<N, _Types>::type>) && ...)) {
        return _Types::template append<_Tp>();
      }
      else {
        [[unlikely]]
        return _Types();
      }
    }
    template <class _Tp>
    using append_type = decltype(
      append<_Tp>(std::make_index_sequence<_Types::size>())
      );
  };

  template <class _Types, class _First, class... _Rest>
  struct get_filtered_types {
    using types = get_filtered_types<typename types_filter<_Types>::template append_type<_First>, _Rest...>::types;
  };

  template <class _Types, class _First>
  struct get_filtered_types<_Types, _First> {
    using types = types_filter<_Types>::template append_type<_First>;
  };

#endif

#if defined(VAR_USE_TYPES) || defined(VAR_USE_FILTERED_TYPES)
  struct types_registered {
    using types =
#  if defined(VAR_USE_FILTERED_TYPES)
      get_filtered_types<var_types<bool>, __VAR_TYPES__>::types;
#  elif defined(VAR_USE_TYPES)
      var_types<__VAR_TYPES__>;
#  endif
    using ops = var_types<__VAR_OPBIN_TYPES__>;
  };
#endif

  template <size_t N>
  using registered_types_visitor_t =

#if defined(VAR_USE_FILTERED_TYPES) || defined(VAR_USE_TYPES)

    types_visitor<N, types_registered::types>::type;

#else
    types_visitor<N, __VAR_TYPES__>::type;

#endif

  template <size_t N>
  using registered_ops_visitor_t =
#if defined(VAR_USE_FILTERED_TYPES) || defined(VAR_USE_TYPES)
    types_visitor<N, types_registered::ops>::type;
#else
    types_visitor<N, __VAR_OPBIN_TYPES__>::type;
#endif

  static constexpr size_t types_size = types_counter<__VAR_TYPES__>::size;
  static constexpr size_t ops_size = types_counter<__VAR_OPBIN_TYPES__>::size;
}

namespace Var {
  using namespace registry;

  class var;
  class list;


  template <class _Tp>
  using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<_Tp>>;


  // Get the type name of a variable by template
  template <class _Tp>
  [[nodiscard]] std::string getTypeNameByTemplate() {
#if defined(_MSC_VER)
    if constexpr (std::is_same_v<std::string, _Tp>) {
      return "std::string";
    }
    else if constexpr (std::is_class_v<_Tp>) {
      std::string name = typeid(_Tp).name();
      return name.substr(name.find_first_of(' ') + 1);
    }
    else {
      return typeid(_Tp).name();
    }
#elif defined(__GNUC__) || defined(__clang__)
    if constexpr (std::is_same_v<std::string, _Tp>) {
      return "std::string";
    }
    else {
      return abi::__cxa_demangle(typeid(_Tp).name(), nullptr, nullptr, nullptr);
    }
#endif
  }

  // Abstract base class of var_holder
  // var_holder 的抽象基类
  struct var_base {
    virtual std::ostream& __out__(std::ostream& o) const = 0;
    virtual VAR_INLINE constexpr size_t __typesize__() const noexcept = 0;
    virtual VAR_INLINE const std::string __typename__() const noexcept = 0;
    virtual VAR_INLINE std::type_index __index__() const noexcept = 0;
    virtual VAR_INLINE const std::type_info& __info__() const noexcept = 0;
    [[nodiscard]] virtual VAR_INLINE const void* __addr__() const noexcept = 0;
    [[nodiscard]] virtual VAR_INLINE void* __addr__() noexcept = 0;
    [[nodiscard]] virtual VAR_INLINE var_base* __clone__() const = 0;
    virtual ~var_base() = default;
  };

  // the holder of var
  template <class _Tp>
  struct var_holder : var_base {

    using value_type = registry::remove_cvref_t<_Tp>;
    using item_type = std::conditional_t<std::is_array_v<value_type>,
      std::remove_all_extents_t<value_type>, void>;
    using decayed_type = std::decay_t<value_type>;

    value_type* _data;

    var_holder() = default;

    template <class U>
    var_holder(U&& val)
      requires (
    !std::is_same_v<value_type, var> &&
      !std::is_same_v<value_type, list> &&
      !std::is_array_v<value_type>&&
      std::is_copy_assignable_v <value_type>&&
      std::is_copy_constructible_v<value_type>
      ) : _data(new value_type(val)) {}

    template <size_t N, class U>
    var_holder(const U(&val)[N])
      requires (
    !std::is_same_v<char, U>&&
      std::is_array_v<value_type>&&
      std::is_copy_assignable_v<item_type>&&
      std::is_copy_constructible_v<item_type>
      ) : _data((value_type*)(new item_type[N]))
    {
      item_type* data_ptr = reinterpret_cast<item_type*>(_data);
      const item_type* val_ptr = reinterpret_cast<const item_type*>(val);
      for (size_t i = 0; i < N; i++) {
        data_ptr[i] = val_ptr[i];
      }
    }

    template <size_t N>
    var_holder(const char(&val)[N]) noexcept
      : _data((value_type*)(new value_type(val))) {}

    template <class U>
    var_holder(U&& val) requires (std::is_same_v<value_type, var>)
      : _data(new value_type(val))
    {
      throw std::runtime_error("var_holder constructor: var is not allowed.");
    }

    var_holder(const list& ls)
      : _data(new value_type(ls)) {}
    var_holder(list&& ls)
			: _data(new value_type(std::move(ls))) {}

    ~var_holder() {
      if constexpr (std::is_array_v<value_type>) {
        delete[] _data;
      }
      else {
        delete _data;
      }
      _data = nullptr;
    }

    std::ostream& __out__(std::ostream& os) const override {
      if constexpr (std::is_array_v<value_type> && !std::is_same_v<char, item_type>)
      {
        size_t size = sizeof(value_type) / sizeof(item_type);
        if (!size) {
          return os;
        }
        os << '[' << (*_data)[0];
        for (size_t idx = 1; idx < size; ++idx) {
          os << ", " << (*_data)[idx];
        }
        return os << ']';
      }
      else {
        return os << *_data;
      }
    }
    VAR_INLINE constexpr size_t __typesize__() const noexcept override {
      return sizeof(value_type);
    }
    VAR_INLINE const std::string __typename__() const noexcept override {
      return getTypeNameByTemplate<value_type>();
    }
    VAR_INLINE std::type_index __index__() const noexcept override {
      return std::type_index(typeid(value_type));
    }
    VAR_INLINE const std::type_info& __info__() const noexcept override {
      return typeid(value_type);
    }
    [[nodiscard]] VAR_INLINE void* __addr__() noexcept override {
      return _data;
    }
    [[nodiscard]] VAR_INLINE const void* __addr__() const noexcept override {
      return _data;
    }
    [[nodiscard]] VAR_INLINE var_base* __clone__() const noexcept override {
      return new var_holder<_Tp>{ *_data };
    }
  };

  // template deduction guide
  // 模板类型推导指导

  template <class U>
  var_holder(U&&) -> var_holder<std::remove_reference_t<U> >;

  template <size_t N, class U>
  var_holder(const U(&)[N]) -> var_holder<U[N]>;

  template <size_t N>
  var_holder(const char(&)[N]) -> var_holder<std::string>;

  var_holder(const list&)->var_holder<list>;

  // operator types for binary operator functions
  // 二元运算符函数类型
  using calculator_func = var_base * (*)(const var_base&, const var_base&);
  // operators function mapping
  // 运算符函数映射
  using op_list = std::unordered_map<std::type_index, calculator_func>;

  // operator space mapping with type, type, function
  // get operator function by two types
  // 运算符空间映射
  // 通过两个类型获取运算符函数
  struct op_space {
  public:
    // set operator function among registered types
    // 在注册类型中设置运算符函数
    op_space() {
      set_op_space(std::make_index_sequence<ops_size>());
    }

    // base calculate function
    // 基础计算函数
    template <class func, class A, class B>
    static var_base* cal_function(const var_base& a, const var_base& b) {
      const var_holder<A>* ptra = dynamic_cast<const var_holder<A>*>(&a);
      const var_holder<B>* ptrb = dynamic_cast<const var_holder<B>*>(&b);

      auto res = func()(*(ptra->_data), *(ptrb->_data));
      return new var_holder<decltype(res)>(res);
    }

    struct op_map {
      std::unordered_map<std::type_index, op_list> _map;
      // check if type is registered
      // 检查类型是否注册
      bool has_type(const std::type_index& a) const {
        return _map.find(a) != _map.end();
      }
      // check if there is operator function between two registered types
      // 检查两个注册类型之间是否有特定运算符函数
      bool has_op(const std::type_index& a, const std::type_index& b) {
        if (has_type(a) && has_type(b) && nullptr != _map[a][b]) {
          return true;
        }
        return false;
      }
      // set operator function between two registered types
      // 设置两个注册类型之间的特定运算符函数
      void set_op(calculator_func func, const std::type_index& a, const std::type_index& b) {
        _map[a][b] = func;
      }
      // get operator function between two registered types
      // 获取两个注册类型之间的特定运算符函数
      calculator_func get_op(const std::type_index& a, const std::type_index& b) {
        if (has_op(a, b)) {
          return _map[a][b];
        }
        return nullptr;
      }
    };
    
    // check if there is operator function between two registered types and set it
    // 检查两个注册类型之间是否有特定运算符函数并设置
    template <size_t N, class A, class B>
    void set_op() {
      using op = registered_ops_visitor_t<N>;
      if constexpr (std::is_invocable_v<op, A, B>) {
        _space[N].set_op(cal_function<op, A, B>, std::type_index(typeid(A)), std::type_index(typeid(B)));
      }
      else {
        _space[N].set_op(nullptr, std::type_index(typeid(A)), std::type_index(typeid(B)));
      }
    };

    // set one operator function among a registered type and all registered types
    // 设置一个注册类型与所有注册类型之间的特定运算符函数
    template <size_t N, size_t A, size_t... Bs>
    void set_op_list(std::index_sequence<Bs...>) {
      (set_op<N, registered_types_visitor_t<A>,
        registered_types_visitor_t<Bs>>(), ...);
    }
    
    // set one opearator function among all registered types
    // 设置所有注册类型之间的特定运算符函数
    template <size_t OP, size_t... As>
    void set_op_map(std::index_sequence<As...>) {
      (set_op_list<OP, As>(std::make_index_sequence<types_size>()), ...);
    }
    // set all operator functions among all registered types
    // 设置所有注册类型之间的所有运算符函数
    template <size_t... OPs>
    void set_op_space(std::index_sequence<OPs...>) {
      (set_op_map<OPs>(std::make_index_sequence<types_size>()), ...);
    }

    op_map& operator[] (size_t idx) {
      return _space[idx];
    }
    const op_map& operator[] (size_t idx) const {
      return _space[idx];
    }
  public:

    // storage of operator functions
    // 运算符函数存储
    op_map _space[ops_size];
  };

  class var {
  private:
    static op_space _op_sapce;
    var_base* _holder;

    void set_holder(var_base* v) {
      if (_holder != nullptr) {
        delete _holder;
      }
      _holder = v;
    }
  public:
    var(std::initializer_list<var> init_ls);
    template <class _Tp>
    var(_Tp&& val)
      requires (!std::is_same_v<std::remove_reference_t<_Tp>, var>)
    : _holder(new var_holder(std::forward<_Tp>(val))) {}
    var() noexcept : var(0) {};
    var(var&& val) noexcept : _holder(val._holder) { val._holder = nullptr; }
    var(const var& val) : _holder(val._holder->__clone__()) {}
    ~var() { delete _holder;  _holder = nullptr; }

    const var& operator=(const var& val) {
      delete _holder;
      _holder = val._holder->__clone__();
      return *this;
    }
    // get the type's sizs of the variable
    // 获取变量的类型大小
    VAR_INLINE size_t TypeSize() const noexcept {
      return this->_holder->__typesize__();
    }
    // get the type name of the variable
    // 获取变量的类型名称
    VAR_INLINE const std::string TypeName() const {
      return this->_holder->__typename__();
    }
    // get the type index of the variable
    // 获取变量的类型索引
    VAR_INLINE std::type_index TypeIndex() const noexcept {
      return this->_holder->__index__();
    }
    // get the type_info reference of the variable
    // 获取变量的type_info引用
    VAR_INLINE const std::type_info& TypeInfo() const noexcept {
      return this->_holder->__info__();
    }
    // get the address of the holder
    [[nodiscard]] VAR_INLINE var_base* Address() noexcept {
      return _holder;
    }
    [[nodiscard]] VAR_INLINE const var_base* Address() const noexcept {
      return _holder;
    }
    // used for output in Var::list
    // 用于在列表中的输出
    std::ostream& __out__(std::ostream& os) const {
      if (TypeInfo() == typeid(std::string)) {
        os << '"';
        return _holder->__out__(os) << '"';
      }
      else if (TypeInfo() == typeid(char)) {
        os << '\'';
        return _holder->__out__(os) << '\'';
      }
      else {
        return _holder->__out__(os);
      }
    }

    friend std::ostream& operator<<(std::ostream& os, const var& v) {
      return v._holder->__out__(os);
    }
    
    // cast the variable to a specific type
    // 将变量转换为特定类型
    template <class _Tp>
    static _Tp var_cast(const var& val) {
      if (typeid(_Tp) != val.TypeInfo()) {
        throw std::runtime_error("var_cast:\nbad cast.");
      }
      return *(dynamic_cast<var_holder<_Tp>*>(val._holder)->_data);
    }

    // assign operator functions
    // 分配运算符函数
    template <size_t OP>
    var assign(const var& other) const {
      var res = 0;
      auto func = _op_sapce[OP].get_op(this->TypeIndex(), other.TypeIndex());
      if (func == nullptr) {
        throw std::runtime_error("get_op: ops not found!\n");
      }
      res.set_holder(func(*(this->_holder), *other._holder));
      return res;
    }

    var operator + (const var& other) const {
      return assign<OP_PLUS>(other);
    }
    var operator - (const var& other) const {
      return assign<OP_MINUS>(other);
    }
    var operator * (const var& other) const {
      return assign<OP_MUL>(other);
    }
    var operator / (const var& other) const {
      return assign<OP_DIV>(other);
    }
    var operator % (const var& other) const {
      return assign<OP_MOD>(other);
    }
    var operator == (const var& other) const {
      return assign<OP_EQ>(other);
    }
    var operator != (const var& other) const {
      return assign<OP_NEQ>(other);
    }
    var operator > (const var& other) const {
      return assign<OP_GREATER>(other);
    }
    var operator < (const var& other) const {
      return assign<OP_LESS>(other);
    }
    var operator >= (const var& other) const {
      return assign<OP_GEQ>(other);
    }
    var operator <= (const var& other) const {
      return assign<OP_LEQ>(other);
    }
    var operator && (const var& other) const {
      return assign<OP_L_AND>(other);
    }
    var operator || (const var& other) const {
      return assign<OP_L_OR>(other);
    }
    var operator & (const var& other) const {
      return assign<OP_B_AND>(other);
    }
    var operator | (const var& other) const {
      return assign<OP_B_OR>(other);
    }
    var operator ^ (const var& other) const {
      return assign<OP_B_XOR>(other);
    }
  };

  // get type name
  [[nodiscard]] const std::string type(const var& val) {
    return val.TypeName();
  }

  op_space var::_op_sapce;

  class list {
  private:
    std::vector<var> _vars;

    std::ostream& __out__(std::ostream& os) const {
      os << '[';
      if (!_vars.empty()) {
        auto it = _vars.begin();
        (*it).__out__(os);
        for (++it; it != _vars.end(); ++it) {
          os << ", ";
          (*it).__out__(os);
        }
      }
      return os << ']';
    }
  public:
    using iterator = std::vector<var>::iterator;
    using const_iterator = std::vector<var>::const_iterator;
    using reverse_iterator = std::vector<var>::reverse_iterator;
    using const_reverse_iterator = std::vector<var>::const_reverse_iterator;

    list(std::initializer_list<var> ls) noexcept : _vars(ls) {}
    list() = default;
    list(const list& other) noexcept : _vars(other._vars) {}
    list(list&& other) noexcept : _vars(other._vars) {}
    ~list() noexcept {}

    // use c++ style api
    // 使用c++风格接口

    VAR_INLINE size_t size() const noexcept {
      return _vars.size();
    }
    VAR_INLINE size_t capacity() const noexcept {
      return _vars.capacity();
    }
    constexpr var& at(size_t pos) {
      return _vars.at(pos);
    }
    constexpr const var& at(size_t pos) const {
      return _vars.at(pos);
    }
    VAR_INLINE void push_back(const var& val) {
      _vars.push_back(val);
    }
    VAR_INLINE void pop_back() noexcept {
      _vars.pop_back();
    }
    constexpr var& front() noexcept {
      return _vars.front();
    }
    constexpr const var& front() const noexcept {
      return _vars.front();
    }
    constexpr var& back() noexcept {
      return _vars.back();
    }
    constexpr const var& back() const noexcept {
      return _vars.back();
    }
    constexpr iterator begin() noexcept {
      return _vars.begin();
    }
    constexpr const_iterator begin() const noexcept {
      return _vars.begin();
    }
    constexpr iterator end() noexcept {
      return _vars.end();
    }
    constexpr const_iterator end() const noexcept {
      return _vars.end();
    }
    constexpr const_iterator cbegin() const noexcept {
      return _vars.cbegin();
    }
    constexpr const_iterator cend() const noexcept {
      return _vars.cend();
    }
    constexpr reverse_iterator rbegin() noexcept {
      return _vars.rbegin();
    }
    constexpr const_reverse_iterator rbegin() const noexcept {
      return _vars.rbegin();
    }
    constexpr reverse_iterator rend() noexcept {
      return _vars.rend();
    }
    constexpr const_reverse_iterator rend() const noexcept {
      return _vars.rend();
    }
    constexpr const_reverse_iterator crbegin() const noexcept {
      return _vars.crbegin();
    }
    constexpr const_reverse_iterator crend() const noexcept {
      return _vars.crend();
    }
    constexpr iterator insert(const_iterator pos, var&& val) {
      return _vars.insert(pos, std::forward<var&&>(val));
    }
    constexpr iterator insert(const_iterator pos, const var& val) {
      return _vars.insert(pos, val);
    }
    constexpr iterator insert(const_iterator pos, size_t count, const var& val) {
      return _vars.insert(pos, count, val);
    }
    constexpr iterator insert(const_iterator pos, std::initializer_list<var> ls) {
      return _vars.insert(pos, ls);
    }
    constexpr iterator insert(const_iterator pos, iterator first, iterator last) {
      return _vars.insert(pos, first, last);
    }
    constexpr var& emplace_back(const var& val) {
      return _vars.emplace_back(val);
    }
    template <class... Args>
    constexpr iterator emplace(const_iterator pos, Args&&... args) {
      return _vars.emplace(pos, args...);
    }
    constexpr void clear() noexcept {
      _vars.clear();
    }
    VAR_INLINE constexpr bool empty() const noexcept {
      return _vars.empty();
    }
    constexpr var* data() noexcept {
      return _vars.data();
    }
    constexpr const var* data() const noexcept {
      return _vars.data();
    }
    constexpr iterator erase(const_iterator first, const_iterator last) {
      return _vars.erase(first, last);
    }
    constexpr iterator erase(const_iterator pos) {
      return _vars.erase(pos);
    }
    constexpr void reverse(size_t newcapacity) {
      _vars.reserve(newcapacity);
    }
    constexpr void resize(size_t newsize) {
      _vars.resize(newsize);
    }
    constexpr void shrink_to_fit() {
      _vars.shrink_to_fit();
    }
    constexpr list& operator = (const list& other) {
      _vars = other._vars;
      return *this;
    }
    constexpr list& operator = (list&& other) noexcept {
      _vars = std::move(other._vars);
      return *this;
    }

    // extend two lists
    list operator + (const list& other) const {
      list res = *this;
      for (auto& val : other) {
        res.emplace_back(val);
      }
      return res;
    }
    template <class _Tp>
    list operator + (_Tp&& val) const
      requires (!std::is_same_v<registry::remove_cvref_t<_Tp>, list > )
    {
      list res = *this;
      res.emplace_back(std::forward<_Tp>(val));
      return res;
    }
    list& operator += (const list& other) {
      for (auto& val : other) {
        this->emplace_back(val);
      }
      return *this;
    }
    list& operator += (const var& val) {
      this->emplace_back(val);
      return *this;
    }
    list operator * (size_t n) const {
      list res = *this;
      if (n == 0) {
        res.clear();
        return res;
      }
      for (size_t i = 1; i < n; i++) {
        res += *this;
      }
      return res;
    }
    list& operator *= (size_t n) {
      if (n == 0) {
        clear();
        return *this;
      }
      list res = *this;
      for (size_t i = 1; i < n; i++) {
        *this += res;
      }
      return *this;
    }
    [[nodiscard]] constexpr var& operator[] (size_t idx) {
      return _vars[idx];
    }
    [[nodiscard]] constexpr const var& operator[] (size_t idx) const {
      return _vars[idx];
    }

    friend std::ostream& operator << (std::ostream& os, const list& ls) {
      return ls.__out__(os);
    }
  };

  var::var(std::initializer_list<var> init_ls)
    : _holder(new var_holder<list>(init_ls)) {}
};

#ifdef __VAR_TYPES__
#undef __VAR_TYPES__
#endif

#ifdef VAR_INLINE
#undef VAR_INLINE
#endif

#endif