#ifndef __VAR_H__
#define __VAR_H__

#include "var/config.h"
#include <functional>
#include <typeindex>
#include <memory>
#include <span>

#define __VAR_INNER__TYPES__ \
	Var::list, std::string, const Var::none_t


#ifdef REGISTRY
#define __VAR_TYPES__ __VAR_FUNDAMENTAL_TYPES__, __VAR_INNER__TYPES__, REGISTRY
#else
#define __VAR_TYPES__ __VAR_FUNDAMENTAL_TYPES__, __VAR_INNER__TYPES__
#endif

#if defined(_MSC_VER)
# ifndef VAR_INLINE
#  define VAR_INLINE inline
# endif
#elif defined(__GNUC__) || defined(__clang__)
# include <cxxabi.h>
# ifndef VAR_INLINE
#   define VAR_INLINE inline __attribute__((always_inline))
# endif
#endif

#ifndef VAR_NODISCARD
#define VAR_NODISCARD [[nodiscard]]
#endif

namespace registry {

  template <size_t N>
  using registered_types_visitor_t = types_visitor<N, __VAR_TYPES__>::type;

  template <size_t N>
  using registered_opbins_visitor_t = types_visitor<N, __VAR_OPBIN_TYPES__>::type;

  template <size_t N>
  using registered_opunas_visitor_t = types_visitor<N, __VAR_OPUNA_TYPES__>::type;

  static constexpr size_t types_size = types_count<__VAR_TYPES__>;
  static constexpr size_t opbins_size = types_count<__VAR_OPBIN_TYPES__>;
  static constexpr size_t opunas_size = types_count<__VAR_OPUNA_TYPES__>;
}

namespace Var {
  using namespace registry;

  class var;
  class list;

  using val = const var&;

  // Get the type name of a variable by template
  template <class _Tp>
  VAR_NODISCARD
    std::string getTypeNameByTemplate() {
#if defined(_MSC_VER)
    if constexpr (std::is_same_v<std::string, _Tp>) {
      return "std::string";
    }
    else if constexpr (std::is_same_v<None_t, _Tp>) {
      return "None_t";
    }
    else {
      std::string name = typeid(_Tp).name();
      if (name.find("class ") == 0) {
        return name.substr(6);
      }
      else if (name.find("struct ") == 0) {
        return name.substr(7);
      }
      return name;
    }
#elif defined(__GNUC__)
    if constexpr (std::is_same_v<std::string, _Tp>) {
      return "std::string";
    }
    else if constexpr (std::is_same_v<None_t, _Tp>) {
      return "None_t";
    }
    else {
      return abi::__cxa_demangle(typeid(_Tp).name(), nullptr, nullptr, nullptr);
    }
#endif
  }

  // Abstract base class of var_value
  // var_value �ĳ������
  struct var_base {
    virtual std::ostream& __out__(std::ostream& o) const = 0;
    VAR_NODISCARD virtual VAR_INLINE constexpr size_t       __typesize__() const noexcept = 0;
    VAR_NODISCARD virtual VAR_INLINE const std::string      __typename__() const noexcept = 0;
    VAR_NODISCARD virtual VAR_INLINE std::type_index        __index__() const noexcept = 0;
    VAR_NODISCARD virtual VAR_INLINE const std::type_info& __info__() const noexcept = 0;
    VAR_NODISCARD virtual VAR_INLINE bool __is__(void* other) const noexcept = 0;
    VAR_NODISCARD virtual VAR_INLINE void* __addr__() const noexcept = 0;
    VAR_NODISCARD virtual VAR_INLINE var_base* __clone__() const = 0;

    virtual constexpr operator bool() const noexcept = 0;

    virtual ~var_base() = default;
  };

  template <class _Tp>
  struct var_ref : var_base {
    using value_type = registry::remove_cvref_t<_Tp>&;
    using inner_type = std::remove_reference_t<_Tp>;

    inner_type* _data;

    template <class U>
    var_ref(U&& val)
      requires (!std::is_rvalue_reference_v<U&&>) : _data(&val) {}

    operator bool() const noexcept override {
      if constexpr (std::is_convertible_v<value_type, bool>) {
        return static_cast<bool>(*_data);
      }
      else {
        return true;
      }
    }

    std::ostream& __out__(std::ostream& os) const override {
      if constexpr (registry::ops::has_operator_Left_shift<std::ostream, inner_type>::value) {
        os << *_data;
      }
      return os;
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
    VAR_NODISCARD VAR_INLINE void* __addr__() const noexcept override {
      return (void*)_data;
    }
    VAR_INLINE bool __is__(void* other) const noexcept override {
      return (void*)_data == other;
    }
    VAR_NODISCARD VAR_INLINE var_base* __clone__() const noexcept override {
      return new var_ref<_Tp>{ *_data };
    }
  };

  template <class U>
  var_ref(U&&) -> var_ref<std::remove_reference_t<U>&>;

  template <class _Tp>
  struct var_value : var_base {

    using value_type = _Tp;

    std::unique_ptr<value_type> _data;

    var_value() = default;

    template <class U>
    var_value(U&& val)
      requires (
    !std::is_same_v<value_type, var> &&
      !std::is_same_v<value_type, list> &&
      !std::is_array_v<std::remove_reference_t<U>>&&
      std::is_copy_assignable_v <value_type>&&
      std::is_copy_constructible_v<value_type>
      ) : _data(new value_type(val)) {}

    template <size_t N, class U>
    var_value(U(&val)[N])
      requires (!std::is_same_v<char, registry::remove_cvref_t<U>>) : _data(new value_type(std::span<U>(val))) {}

    template <size_t N>
    var_value(const char(&val)[N]) noexcept
      : _data((value_type*)(new value_type(val))) {}

    template <size_t N>
    var_value(char(&val)[N]) noexcept
      : _data((value_type*)(new value_type(val))) {}

    template <class U>
    var_value(U&& val) requires (std::is_same_v<value_type, var>)
      : _data(new value_type(val))
    {
      throw std::runtime_error("var_value constructor: var is not allowed.");
    }

    var_value(const list& ls)
      : _data(new value_type(ls)) {}
    var_value(list&& ls)
      : _data(new value_type(std::move(ls))) {}

    ~var_value() = default;

    std::ostream& __out__(std::ostream& os) const override {
      return os << *_data;
    }
    operator bool() const noexcept override {
      if constexpr (std::is_convertible_v<value_type, bool>) {
        return static_cast<bool>(*_data);
      }
      else {
        return true;
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
    VAR_NODISCARD VAR_INLINE void* __addr__() const noexcept override {
      return _data.get();
    }
    VAR_INLINE bool __is__(void* other) const noexcept override {
      return (void*)_data.get() == other;
    }
    VAR_NODISCARD VAR_INLINE var_base* __clone__() const noexcept override {
      return new var_value<_Tp>(*_data);
    }
  };

  template <class U>
  var_value(U&&) -> var_value<std::conditional_t<std::is_array_v<registry::remove_cvref_t<U>>, std::conditional_t<std::is_same_v<char, std::remove_all_extents_t<registry::remove_cvref_t<U>>>, std::string, list>, registry::remove_cvref_t<U>>>;

  class var {
  public:
    struct opbin_space;
    struct opuna_space;


    var_base* _holder;
    static opbin_space _opbin_space;
    static opuna_space _opuna_space;

    void set_holder(var_base* v) {
      if (_holder != nullptr) {
        delete _holder;
      }
      _holder = v;
    }

    template <class _Tp>
    VAR_NODISCARD static var make_value(_Tp&& val)
      requires (registry::is_convertible_to_value<registry::remove_cvref_t<_Tp>>::value)
    {
      var res;
      res.set_holder(new var_value(val));
      return res;
    }

    template <class _Tp>
    VAR_NODISCARD static var make_ref(_Tp&& val)
      requires (!std::is_rvalue_reference_v<decltype(val)>)
    {
      var res;
      res.set_holder(new var_ref(val));
      return res;
    }

    struct opbin_space {

      using calculator_func = var(*)(var_base&, var_base&);
      using opbin_list = std::unordered_map<std::type_index, calculator_func>;
      using opbin_map = std::unordered_map<std::type_index, opbin_list>;

      opbin_map _map[opbins_size];

      opbin_space() {
#ifndef VAR_USE_NO_OPERATORS
        set_op_space(std::make_index_sequence<opbins_size>());
#endif
      }

      template <class func, class A, class B>
      static var cal_function(var_base& a, var_base& b) {
        if constexpr (std::is_copy_constructible_v<A>) {
          var_value<A>* ptra = dynamic_cast<var_value<A>*>(&a);

          if constexpr (std::is_copy_constructible_v<B>) {
            var_value<B>* ptrb = dynamic_cast<var_value<B>*>(&b);

            if constexpr (std::is_reference_v<decltype(func()(*(ptra->_data), *(ptrb->_data)))>) {
              return var::make_ref(func()(*(ptra->_data), *(ptrb->_data)));
            }
            else {
              return var::make_value(func()(*(ptra->_data), *(ptrb->_data)));
            }
          }
          else {
            var_ref<B&>* ptrb = dynamic_cast<var_ref<B&>*>(&b);

            if constexpr (std::is_reference_v<decltype(func()(*(ptra->_data), *(ptrb->_data)))>) {
              return var::make_ref(func()(*(ptra->_data), *(ptrb->_data)));
            }
            else {
              return var::make_value(func()(*(ptra->_data), *(ptrb->_data)));
            }
          }
        }
        else {
          var_ref<A&>* ptra = dynamic_cast<var_ref<A&>*>(&a);

          if constexpr (std::is_copy_constructible_v<B>) {
            var_value<B>* ptrb = dynamic_cast<var_value<B>*>(&b);

            if constexpr (std::is_reference_v<decltype(func()(*(ptra->_data), *(ptrb->_data)))>) {
              return var::make_ref(func()(*(ptra->_data), *(ptrb->_data)));
            }
            else {
              return var::make_value(func()(*(ptra->_data), *(ptrb->_data)));
            }
          }
          else {
            var_ref<B&>* ptrb = dynamic_cast<var_ref<B&>*>(&b);

            if constexpr (std::is_reference_v<decltype(func()(*(ptra->_data), *(ptrb->_data)))>) {
              return var::make_ref(func()(*(ptra->_data), *(ptrb->_data)));
            }
            else {
              return var::make_value(func()(*(ptra->_data), *(ptrb->_data)));
            }
          }
        }
      }

      bool has_type(const std::type_index& a) const {
        return _map[0].find(a) != _map[0].end();
      }

      template <size_t OP>
      bool has_opbin(const std::type_index& a, const std::type_index& b) {
        if (has_type(a) && has_type(b) && nullptr != _map[OP][a][b]) {
          return true;
        }
        return false;
      }
      template <size_t OP>
      void set_opbin(calculator_func func, const std::type_index& a, const std::type_index& b) {
        _map[OP][a][b] = func;
      }

      template <size_t OP>
      calculator_func get_opbin(const std::type_index& a, const std::type_index& b) {
        if (this->has_opbin<OP>(a, b)) {
          return _map[OP][a][b];
        }
        return nullptr;
      }

      template <size_t N, class A, class B>
      void set_op_single() {
        using op = registered_opbins_visitor_t<N>;
        if constexpr (std::is_invocable_v<op, A, B>) {
          this->set_opbin<N>(cal_function<op, A, B>, std::type_index(typeid(A)), std::type_index(typeid(B)));
        }
        else {
          this->set_opbin<N>(nullptr, std::type_index(typeid(A)), std::type_index(typeid(B)));
        }
      };

      template <size_t N, size_t A, size_t... Bs>
      void set_op_list(std::index_sequence<Bs...>) {
        (this->set_op_single<N, registered_types_visitor_t<A>,
          registered_types_visitor_t<Bs>>(), ...);
      }

      template <size_t OP, size_t... As>
      void set_op_map(std::index_sequence<As...>) {
        (this->set_op_list<OP, As>(std::make_index_sequence<types_size>()), ...);
      }

      template <size_t... OPs>
      void set_op_space(std::index_sequence<OPs...>) {
        (this->set_op_map<OPs>(std::make_index_sequence<types_size>()), ...);
      }
    };

    struct opuna_space {
      using calculator_func = var(*)(var_base&);
      using opuna_map = std::unordered_map<std::type_index, calculator_func>;

      opuna_map _map[opunas_size];

      opuna_space() {
#ifndef VAR_USE_NO_OPERATORS
        this->set_op_space(std::make_index_sequence<opunas_size>());
#endif
      }

      template <class _Func, class _Param>
      static var cal_function(var_base& _param) {
        if constexpr (std::is_copy_constructible_v<_Param>) {
          var_value<_Param>* parg = dynamic_cast<var_value<_Param>*>(&_param);

          if constexpr (std::is_reference_v<decltype(_Func()(*(parg->_data)))>) {
            return var::make_ref(_Func()(*(parg->_data)));
          }
          else {
            return var::make_value(_Func()(*(parg->_data)));
          }
        }
        else {
          var_ref<_Param&>* parg = dynamic_cast<var_ref<_Param&>*>(&_param);

          if constexpr (std::is_reference_v<decltype(_Func()(*(parg->_data)))>) {
            return var::make_ref(_Func()(*(parg->_data)));
          }
          else {
            return var::make_value(_Func()(*(parg->_data)));
          }
        }
      }

      bool has_type(const std::type_index& a) const {
        return _map[0].find(a) != _map[0].end();
      }

      template <size_t OP>
      bool has_opuna(const std::type_index& a) {
        if (has_type(a) && nullptr != _map[OP][a]) {
          return true;
        }
        return false;
      }
      template <size_t OP>
      void set_opuna(calculator_func func, const std::type_index& a) {
        _map[OP][a] = func;
      }

      template <size_t OP>
      calculator_func get_opuna(const std::type_index& a) {
        if (this->has_opuna<OP>(a)) {
          return _map[OP][a];
        }
        return nullptr;
      }

      template <size_t N, class A>
      void set_op_single() {
        using op = registered_opunas_visitor_t<N>;
        if constexpr (std::is_invocable_v<op, A>) {
          this->set_opuna<N>(cal_function<op, A>, std::type_index(typeid(A)));
        }
        else {
          this->set_opuna<N>(nullptr, std::type_index(typeid(A)));
        }
      }

      template <size_t OP, size_t... As>
      void set_op_map(std::index_sequence<As...>) {
        (this->set_op_single<OP, registry::registered_types_visitor_t<As>>(), ...);
      }

      template <size_t... OPs>
      void set_op_space(std::index_sequence<OPs...>) {
        (this->set_op_map<OPs>(std::make_index_sequence<types_size>()), ...);
      }
    };

  public:

    var(std::initializer_list<var> init_ls);
    template <class _Tp>
    var(_Tp&& val)
      requires (
    !std::is_same_v<registry::remove_cvref_t<_Tp>, var>&& registry::is_convertible_to_value<registry::remove_cvref_t<_Tp>>::value
      ) : _holder(new var_value(std::forward<_Tp>(val))) {}
    template <class _Tp>
    var(_Tp&& val)
      requires (!std::is_same_v<registry::remove_cvref_t<_Tp>, var> && !std::is_copy_constructible_v<registry::remove_cvref_t<_Tp>> &&
    !registry::is_convertible_to_value<registry::remove_cvref_t<_Tp>>::value &&
      !std::is_rvalue_reference_v<decltype(val)>
      ) : _holder(new var_ref(std::forward<_Tp>(val))) {}
    var() noexcept : var(None) {};
    var(var&& val) noexcept : _holder(val._holder) { val._holder = nullptr; }
    var(const var& val) : _holder(val._holder->__clone__()) {}
    ~var() { delete _holder;  _holder = nullptr; }

    const var& operator=(const var& val) {
      delete _holder;
      _holder = val._holder->__clone__();
      return *this;
    }
    const var& operator=(std::initializer_list<var> v) {
      delete _holder;
      _holder = var{ v }._holder->__clone__();
      return *this;
    }
    VAR_INLINE size_t TypeSize() const noexcept {
      return this->_holder->__typesize__();
    }
    VAR_INLINE const std::string TypeName() const {
      return this->_holder->__typename__();
    }
    VAR_INLINE std::type_index TypeIndex() const noexcept {
      return this->_holder->__index__();
    }
    VAR_INLINE const std::type_info& TypeInfo() const noexcept {
      return this->_holder->__info__();
    }
    VAR_NODISCARD VAR_INLINE var_base* Address() noexcept {
      return _holder;
    }
    VAR_NODISCARD VAR_INLINE const var_base* Address() const noexcept {
      return _holder;
    }


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

    operator bool() const {
      return bool(*_holder);
    }

    bool is(const var& other) const {
      return _holder->__is__(other._holder->__addr__());
    }

    friend std::ostream& operator<<(std::ostream& os, const var& v) {
      return v._holder->__out__(os);
    }

    template <class _Tp>
    static _Tp var_vcast(const var& val) {
      if (typeid(_Tp) != val.TypeInfo()) {
        throw std::runtime_error("var_cast:\nbad cast.");
      }
      return *(dynamic_cast<var_value<_Tp>*>(val._holder)->_data);
    }

    template <class _Tp>
    static _Tp var_rcast(const var& val) {
      if (typeid(_Tp) != val.TypeInfo()) {
        throw std::runtime_error("var_cast:\nbad cast.");
      }
      return *(dynamic_cast<var_ref<_Tp>*>(val._holder)->_data);
    }


    template <size_t OP>
    var assign_opbin(const var& other) const {
      var res;
      auto func = _opbin_space.get_opbin<OP>(this->TypeIndex(), other.TypeIndex());
      if (func == nullptr) {
        throw std::runtime_error("get_op: ops not found!\n");
      }
      res = func(*(this->_holder), *other._holder);
      return res;
    }

    template <size_t OP>
    var assign_opuna() const {
      var res;
      auto func = _opuna_space.get_opuna<OP>(this->TypeIndex());
      if (func == nullptr) {
        throw std::runtime_error("get_op: ops not found!\n");
      }
      res = func(*(this->_holder));
      return res;
    }

    template <class _Tp>
    var operator + (_Tp&& other) const {
      return assign_opbin<OP_PLUS>(other);
    }
    template <class _Tp>
    var operator - (_Tp&& other) const {
      return assign_opbin<OP_MINUS>(other);
    }
    template <class _Tp>
    var operator * (_Tp&& other) const {
      return assign_opbin<OP_MUL>(other);
    }
    template <class _Tp>
    var operator / (_Tp&& other) const {
      return assign_opbin<OP_DIV>(other);
    }
    template <class _Tp>
    var operator % (_Tp&& other) const {
      return assign_opbin<OP_MOD>(other);
    }
    template <class _Tp>
      requires (!std::is_same_v<_Tp, None_t>)
    var operator == (_Tp&& other) const {
      return assign_opbin<OP_EQ>(other);
    }
    template <class _Tp>
      requires (!std::is_same_v<_Tp, None_t>)
    var operator != (_Tp&& other) const {
      return assign_opbin<OP_NEQ>(other);
    }
    template <class _Tp>
    var operator > (_Tp&& other) const {
      return assign_opbin<OP_GREATER>(other);
    }
    template <class _Tp>
    var operator < (_Tp&& other) const {
      return assign_opbin<OP_LESS>(other);
    }
    template <class _Tp>
    var operator >= (_Tp&& other) const {
      return assign_opbin<OP_GEQ>(other);
    }
    template <class _Tp>
    var operator <= (_Tp&& other) const {
      return assign_opbin<OP_LEQ>(other);
    }
    template <class _Tp>
    var operator && (_Tp&& other) const {
      return assign_opbin<OP_LAND>(other);
    }
    template <class _Tp>
    var operator || (_Tp&& other) const {
      return assign_opbin<OP_LOR>(other);
    }
    template <class _Tp>
    var operator & (_Tp&& other) const {
      return assign_opbin<OP_BAND>(other);
    }
    template <class _Tp>
    var operator | (_Tp&& other) const {
      return assign_opbin<OP_BOR>(other);
    }
    template <class _Tp>
    var operator ^ (_Tp&& other) const {
      return assign_opbin<OP_BXOR>(other);
    }
    template <class _Tp>
    var operator << (_Tp&& other) const {
      return assign_opbin<OP_LSHIFT>(other);
    }
    template <class _Tp>
    var operator >> (_Tp&& other) const {
      return assign_opbin<OP_RSHIFT>(other);
    }
    template <class _Tp>
    var operator [] (_Tp&& other) const {
      return assign_opbin<OP_BRACKET>(other);
    }

    var operator !() const {
      return assign_opuna<OP_LNOT>();
    }
    var operator ~() const {
      return assign_opuna<OP_BNOT>();
    }
    var& operator ++() {
      assign_opuna<OP_PREINCREMENT>();
      return *this;
    }
    var operator ++(int) {
      var tmp = *this;
      assign_opuna<OP_POSTINCREMENT>();
      return tmp;
    }
    var& operator --() {
      assign_opuna<OP_PREDECREMENT>();
      return *this;
    }
    var operator --(int) {
      var tmp = *this;
      assign_opuna<OP_POSTDECREMENT>();
      return tmp;
    }
    var operator +() const {
      return assign_opuna<OP_UPLUS>();
    }
    var operator -() const {
      return assign_opuna<OP_UMINUS>();
    }
    var operator *() const {
      return assign_opuna<OP_DEFERENCE>();
    }
    var& operator +=(const var& other) {
      *this = *this + other;
      return *this;
    }
    var& operator -=(const var& other) {
      *this = *this - other;
      return *this;
    }
    var& operator *=(const var& other) {
      *this = *this * other;
      return *this;
    }
    var& operator /=(const var& other) {
      *this = *this / other;
      return *this;
    }
    var& operator %=(const var& other) {
      *this = *this % other;
      return *this;
    }
    var& operator &=(const var& other) {
      *this = *this & other;
      return *this;
    }
    var& operator |=(const var& other) {
      *this = *this | other;
      return *this;
    }
    var& operator ^=(const var& other) {
      *this = *this ^ other;
      return *this;
    }
    var& operator <<=(const var& other) {
      *this = *this << other;
      return *this;
    }
    var& operator >>=(const var& other) {
      *this = *this >> other;
      return *this;
    }
  };

  [[nodiscard]] const std::string type(const var& val) {
    return val.TypeName();
  }

  var::opbin_space var::_opbin_space;
  var::opuna_space var::_opuna_space;

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
    template <class _Tp>
      requires (!std::is_same_v<registry::remove_cvref_t<_Tp>, var>)
    list(std::span<_Tp> val) noexcept : _vars(val.begin(), val.end()) {}
    list() noexcept : _vars() {}
    list(const list& other) noexcept : _vars(other._vars) {}
    list(list&& other) noexcept : _vars(other._vars) {}
    ~list() noexcept {}

    operator bool() const noexcept {
      return !_vars.empty();
    }


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
      requires (!std::is_same_v<registry::remove_cvref_t<_Tp>, list >)
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
    : _holder(new var_value<list>(init_ls)) {}
};

#ifdef __VAR_TYPES__
#undef __VAR_TYPES__
#endif

#ifdef VAR_INLINE
#undef VAR_INLINE
#endif

#ifdef REGISTRY
#undef REGISTRY
#endif

#endif