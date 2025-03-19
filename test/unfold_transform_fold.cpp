#include "meld/core/cached_product_stores.hpp"
#include "meld/core/framework_graph.hpp"
#include "meld/model/level_id.hpp"
#include "meld/model/product_store.hpp"
#include "test/log_record.hpp"
#include "test/products_for_output.hpp"
#include "test/unfold_transform_fold_support.hpp"

#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/spdlog.h"

#include <algorithm>
#include <array>
#include <string>
#include <vector>

using namespace meld;

int main()
{
  // Setup logging.
  auto tsvlog = spdlog::basic_logger_mt("tsvlog", "unfold_transform_fold.tsv");
  spdlog::set_default_logger(tsvlog);
  spdlog::set_pattern("%v"); // output only the message, no metadata.
  spdlog::info("time\tthread\tevent\tspill\tapa\tactive\tdata\tother");
  spdlog::set_pattern("%H:%M:%S.%f\t%t\t%v");

  // Constants that configure the work load.
  //constexpr auto wires_per_spill = 150 * 10 * 1000ul;
  constexpr auto wires_per_spill = 1000ul; // 350 * 1000ul;
  constexpr auto number_of_spills = 2ul;

  // Create some levels of the data set categories hierarchy.
  // We may or may not want to create pre-generated data set categories like this.
  // Each data set category gets an index number in the hierarchy.

  // Create the "index" of records we will process.
  // There will be one job record and then number_of_spills spill records.
  std::vector<level_id_ptr> levels;
  levels.reserve(number_of_spills + 1u);
  levels.push_back(level_id::base_ptr());
  // TODO: Put runs and subruns into the hierarchy.
  std::generate_n(std::back_inserter(levels), number_of_spills, [i = 0u]() mutable {
    return level_id::base().make_child(i++, "spill");
  });

  // Create the lambda that will be used to intialize the graph.
  // The range [it, e) will be used to iterate over the levels.
  auto it = cbegin(levels);
  auto const e = cend(levels);
  std::size_t counter = 1;

  auto source =
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
      auto next_size = wires_per_spill;
      demo::log_record("add_wgi", store->id()->number(), 0, &store, next_size, nullptr);
      store->add_product<demo::WGI>("wgen", {next_size});
      ++counter;
    }
    return store;
  };

  // Create the graph. The source tells us what data we will process.
  demo::log_record("create_graph", 0, 0, nullptr, 0, nullptr);
  framework_graph g{source};

  // g.with<X> is "registration code" as implemented in meld.
  // Each call to g.with<> creates a new CHOF in the graph.

  // Add the unfold node to the graph. We do not yet know how to provide the chunksize
  // to the constructor of the WaveformGenerator, so we will use the default value.
  demo::log_record("add_unfold", 0, 0, nullptr, 0, nullptr);
  auto const chunksize = 256LL; // this could be read from a configuration file
  // auto unfold_op = [](WaveformGenerator& wg, std::size_t running_value) {
  //   return wg.op(running_value, chunksize);
  // };
  g.with<demo::WaveformGenerator>(
     &demo::WaveformGenerator::predicate,
     [](demo::WaveformGenerator const& wg, std::size_t running_value) {
       return wg.op(running_value, chunksize);
     },
     concurrency::unlimited)
    .unfold("wgen")        // the type of node to create
    .into("waves_in_apa")  // label the chunks we create as "waves_in_apa"
    .within_family("APA"); // put the chunks into a data set category called "APA"

  // Add the transform node to the graph.
  demo::log_record("add_transform", 0, 0, nullptr, 0, nullptr);
  g.with(demo::clampWaveforms, concurrency::unlimited)
    .transform("waves_in_apa") // the type of node to create, and the label of the input
    .for_each("APA")
    .to("clamped_waves"); // label the chunks we create as "clamped_waves"

  // Add the fold node to the graph.
  demo::log_record("add_fold", 0, 0, nullptr, 0, nullptr);
  g.with("accum_for_spill", demo::accumulateSCW, concurrency::unlimited)
    .fold("clamped_waves"_in("APA"))
    .to("summed_waveforms")
    .for_each("spill");

  demo::log_record("add_output", 0, 0, nullptr, 0, nullptr);
  g.make<test::products_for_output>().output_with(&test::products_for_output::save,
                                                  concurrency::serial);

  // Execute the graph.
  demo::log_record("execute_graph", 0, 0, nullptr, 0, nullptr);
  g.execute("unfold_transform_fold");
  demo::log_record("done", 0, 0, nullptr, 0, nullptr);
}
