#include "meld/core/cached_product_stores.hpp"
#include "meld/core/framework_graph.hpp"
#include "meld/model/level_id.hpp"
#include "meld/model/product_store.hpp"
#include "test/products_for_output.hpp"

#include <atomic>
#include <string>
#include <vector>

using namespace meld;

namespace {
  struct Waveform {
    double val;
  }; 

  using Waveforms = std::vector<Waveform>;

  struct WaveformGeneratorInput {
    int size;
  };

  using WGI = WaveformGeneratorInput;

  class WaveformGenerator {
  public:
    explicit WaveformGenerator(unsigned int maxsize, unsigned int chunksize) : maxsize_{maxsize}, chunksize_{chunksize} {}
    unsigned int initial_value() const { return 0; }
    bool predicate(unsigned int made_so_far) const { return made_so_far >= maxsize_; }
    auto op(unsigned int made_so_far) const { 
      Waveforms ws(chunksize_, 1.0*made_so_far);
      return std::make_pair(made_so_far+chunksize_, ws); 
    };

  private:
    unsigned int maxsize_;
    unsigned int chunksize_;

  };


int main()
{
  constexpr auto index_limit = 2u;
  std::vector<level_id_ptr> levels;
  levels.reserve(index_limit + 1u);
  levels.push_back(level_id::base_ptr());
  for (unsigned i = 0u; i != index_limit; ++i) {
    levels.push_back(level_id::base().make_child(i, "event"));
  }

  auto it = cbegin(levels);
  auto const e = cend(levels);
  framework_graph g{[it, e](cached_product_stores& cached_stores) mutable -> product_store_ptr {
    if (it == e) {
      return nullptr;
    }
    auto const& id = *it++;

    auto store = cached_stores.get_store(id);
    if (store->id()->level_name() == "event") {
      store->add_product<unsigned>("maxsize_number", 10u * (id->number() + 1));
      store->add_product<numbers_t>("ten_numbers", numbers_t(10, id->number() + 1));
    }
    return store;
  }};

  g.with<WaveformGenerator>(&WaveformGenerator::predicate, &WaveformGenerator::unfold, concurrency::unlimited)
    .split("maxsize_number")
    .into("new_number")
    .within_family("lower1");
  g.with(add, concurrency::unlimited).reduce("new_number").for_each("event").to("sum1");
  g.with(check_sum, concurrency::unlimited).observe("sum1");

  g.with<iterate_through>(
     &iterate_through::predicate, &iterate_through::unfold, concurrency::unlimited)
    .split("ten_numbers")
    .into("each_number")
    .within_family("lower2");
  g.with(add_numbers, concurrency::unlimited).reduce("each_number").for_each("event").to("sum2");
  g.with(check_sum_same, concurrency::unlimited).observe("sum2");

  g.make<test::products_for_output>().output_with(&test::products_for_output::save,
                                                  concurrency::serial);

  g.execute("splitter_t");

}
