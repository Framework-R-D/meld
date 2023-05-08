#ifndef meld_model_product_store_hpp
#define meld_model_product_store_hpp

#include "meld/model/fwd.hpp"
#include "meld/model/handle.hpp"
#include "meld/model/level_id.hpp"
#include "meld/model/products.hpp"

#include <array>
#include <cstddef>
#include <map>
#include <memory>
#include <string>
#include <type_traits>

namespace meld {

  class product_store : public std::enable_shared_from_this<product_store> {
  public:
    static product_store_ptr base();

    product_store_const_ptr store_for_product(std::string const& product_name) const;

    auto begin() const noexcept { return products_.begin(); }
    auto end() const noexcept { return products_.end(); }

    std::string const& level_name() const noexcept;
    std::string_view source() const noexcept;
    product_store_const_ptr parent(std::string const& level_name) const noexcept;
    product_store_const_ptr parent() const noexcept;
    product_store_ptr make_flush() const;
    product_store_ptr make_continuation(std::string_view source) const;
    product_store_ptr make_continuation(level_id_ptr id, std::string_view source) const;
    product_store_ptr make_child(std::size_t new_level_number,
                                 std::string const& new_level_name,
                                 std::string_view source,
                                 products new_products);
    product_store_ptr make_child(std::size_t new_level_number,
                                 std::string const& new_level_name,
                                 std::string_view source = {},
                                 stage st = stage::process);
    level_id_ptr const& id() const noexcept;
    bool is_flush() const noexcept;

    // Product interface
    bool contains_product(std::string const& key) const;

    template <typename T>
    T const& get_product(std::string const& key) const;

    template <typename T>
    handle<T> get_handle(std::string const& key) const;

    // Thread-unsafe operations
    template <typename T>
    void add_product(std::string const& key, T&& t);

    template <typename T>
    void add_product(std::string const& key, std::shared_ptr<product<T>>&& t);

  private:
    explicit product_store(level_id_ptr id = level_id::base_ptr(),
                           std::string_view source = {},
                           stage processing_stage = stage::process);
    explicit product_store(product_store_const_ptr parent,
                           std::size_t new_level_number,
                           std::string const& new_level_name,
                           std::string_view source,
                           products new_products);
    explicit product_store(product_store_const_ptr parent,
                           std::size_t new_level_number,
                           std::string const& new_level_name,
                           std::string_view source,
                           stage processing_stage);

    product_store_const_ptr parent_{nullptr};
    products products_{};
    level_id_ptr id_;
    std::string_view source_;
    stage stage_;
  };

  template <typename T>
  void add_to(product_store& store, T const& t, std::array<std::string, 1u> const& name)
  {
    store.add_product(name[0], t);
  }

  template <typename... Ts>
  void add_to(product_store& store,
              std::tuple<Ts...> const& ts,
              std::array<std::string, sizeof...(Ts)> const& names)
  {
    [&store, &names]<std::size_t... Is>(auto const& ts, std::index_sequence<Is...>) {
      (store.add_product(names[Is], std::get<Is>(ts)), ...);
    }(ts, std::index_sequence_for<Ts...>{});
  }

  product_store_ptr const& more_derived(product_store_ptr const& a, product_store_ptr const& b);

  template <std::size_t I, typename Tuple, typename Element>
  Element const& get_most_derived(Tuple const& tup, Element const& element)
  {
    constexpr auto N = std::tuple_size_v<Tuple>;
    if constexpr (I == N - 1) {
      return more_derived(element, std::get<I>(tup));
    }
    else {
      return get_most_derived<I + 1>(tup, more_derived(element, std::get<I>(tup)));
    }
  }

  template <typename Tuple>
  auto const& most_derived(Tuple const& tup)
  {
    constexpr auto N = std::tuple_size_v<Tuple>;
    static_assert(N > 0ull);
    if constexpr (N == 1ull) {
      return std::get<0>(tup);
    }
    else {
      return get_most_derived<1ull>(tup, std::get<0>(tup));
    }
  }

  // Implementation details
  template <typename T>
  void product_store::add_product(std::string const& key, T&& t)
  {
    add_product(key, std::make_shared<product<std::remove_cvref_t<T>>>(std::forward<T>(t)));
  }

  template <typename T>
  void product_store::add_product(std::string const& key, std::shared_ptr<product<T>>&& t)
  {
    products_.add(key, std::move(t));
  }

  // template <typename T>
  // void
  // product_store::add_product(labeled_data<T>&& ldata)
  // {
  //   add_product(ldata.label, std::forward<T>(ldata.data));
  // }

  template <typename T>
  [[nodiscard]] handle<T> product_store::get_handle(std::string const& key) const
  {
    return handle<T>{products_.get<T>(key), *id_};
  }

  template <typename T>
  [[nodiscard]] T const& product_store::get_product(std::string const& key) const
  {
    return *get_handle<T>(key);
  }
}

#endif /* meld_model_product_store_hpp */
