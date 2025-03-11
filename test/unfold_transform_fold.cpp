#include "meld/core/cached_product_stores.hpp"
#include "meld/core/framework_graph.hpp"
#include "meld/model/level_id.hpp"
#include "meld/model/product_store.hpp"
#include "test/products_for_output.hpp"

#include "spdlog/spdlog.h"

#include <algorithm>
#include <string>
#include <vector>

using namespace meld;

namespace {
  struct Waveform {
    double val;
  };

  using Waveforms = std::vector<Waveform>;

  // This is the data product that our unfold node will receive from each spill.
  struct WaveformGeneratorInput {
    std::size_t size;
  };

  using WGI = WaveformGeneratorInput;

  // This is a class that provides unfold operation and a predicate to control
  // when the unfold is stopped.
  // Note that this class knows nothing about "spills" or "APAs". It does not
  // know where the values of maxsize and chunksize come from.
  class WaveformGenerator {
  public:
    // Create a WaveformGenerator that will generate waveforms exactly maxsize
    // waveforms. They will be spread across vectors each of size no more than
    //  chunksize.
    explicit WaveformGenerator(WGI const& maxsize, std::size_t chunksize = 3UL) :
      maxsize_{maxsize.size}, chunksize_{chunksize}
    {
      spdlog::info("Constructed a new WaveformGenerator with maxsize = {} and chunksize = {}",
                   maxsize.size,
                   chunksize);
    }

    // The initial value of the count of how many waveforms we have made so far.
    std::size_t initial_value() const { return 0; }

    // When we have made at least as many waveforms as we have been asked to make
    // for this unfold, we are done. The predicate answers the question "should
    // we continue unfolding?"
    bool predicate(std::size_t made_so_far) const
    {
      bool const result = made_so_far < maxsize_;
      spdlog::info(
        "predicate: made_so_far = {}, maxsize_ = {}, result = {}", made_so_far, maxsize_, result);
      return result;
    }

    // Generate the next chunk of waveforms, and update the count of how many
    // waveforms we have made so far.
    auto op(std::size_t made_so_far) const
    {
      // How many waveforms should go into this chunk?
      std::size_t const newsize = std::min(chunksize_, maxsize_ - made_so_far);
      Waveforms ws(newsize, {1.0 * made_so_far});
      spdlog::info("op: made_so_far = {}, chunksize_ = {}, ws.size() = {}",
                   made_so_far,
                   chunksize_,
                   ws.size());
      return std::make_pair(made_so_far + ws.size(), ws);
    }

  private:
    std::size_t const maxsize_;   // total number of waveforms to make for the unfold
    std::size_t const chunksize_; // number of waveforms in each chunk we make
  };
} // namespace

int main()
{
  // Create some levels of the data set categories hierarchy.
  // We may or may not want to create pre-generated data set categories like this.
  // Each data set category gets an index number in the hierarchy.
  constexpr auto index_limit = 2u;
  std::vector<level_id_ptr> levels;
  levels.reserve(index_limit + 1u);
  levels.push_back(level_id::base_ptr());
  // TODO: Put runs and subruns into the hierarchy.
  for (unsigned i = 0u; i != index_limit; ++i) {
    levels.push_back(level_id::base().make_child(i, "spill"));
  }

  // Create the lambda that will be used to intialize the graph.
  // The range [it, e) will be used to iterate over the levels.
  auto it = cbegin(levels);
  auto const e = cend(levels);
  std::size_t counter = 1;

  auto source [[maybe_unused]] =
    [it, e, counter](cached_product_stores& cached_stores) mutable -> product_store_ptr {
    // If the range is empty, return nullptr. This tells the framework_graph there is nothing
    // to process.
    if (it == e) {
      return nullptr;
    }
    // Capture a rerefence to the first id, and increment the iterator.
    auto const& id = *it++;

    auto store = cached_stores.get_store(id);
    if (store->id()->level_name() == "spill") {
      // Put the WGI product into the spill, so that our CHOF can find it.
      auto next_size = counter * 10;
      spdlog::info("Adding WGI to spill {} with count = {}", store->id()->hash(), next_size);
      store->add_product<WGI>("wgen", {next_size});
      ++counter;
    }
    return store;
  };

  // Create the graph. The source tells us what data we will process.
  spdlog::info("Creating the graph");
  framework_graph g{source};

  // Add the unfold node to the graph. We do not yet know how to provide the chunksize
  // to the constructor of the WaveformGenerator, so we will use the default value.
  spdlog::info("Adding the unfold node");
  g.with<WaveformGenerator>(
     &WaveformGenerator::predicate, &WaveformGenerator::op, concurrency::unlimited)
    .split("wgen")         // create an unfold node
    .into("waves_in_apa")  // label the chunks we create as "waves_in_apa"
    .within_family("APA"); // put the chunks into a data set category called "APA"

  spdlog::info("Adding the output node");
  g.make<test::products_for_output>().output_with(&test::products_for_output::save,
                                                  concurrency::serial);

  // Execute the graph.
  spdlog::info("Executing the graph");
  g.execute("unford_transform_fold");
  spdlog::info("Done");
}
