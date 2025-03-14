#include "meld/core/cached_product_stores.hpp"
#include "meld/core/framework_graph.hpp"
#include "meld/model/level_id.hpp"
#include "meld/model/product_store.hpp"
#include "test/products_for_output.hpp"

#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/spdlog.h"

#include <algorithm>
#include <array>
#include <string>
#include <vector>

using namespace meld;

namespace {

  // void log_record(char const* event,
  //                 int spill_id,
  //                 int apa_id,
  //                 void const* product,
  //                 std::size_t size,
  //                 void const* orig)
  // {
  // }
  struct Waveform {
    // We should be set to the number of samples on a wire.
    std::array<double, 8 * 1024> samples;
  };

  struct Waveforms {
    std::vector<Waveform> waveforms;
    int spill_id;
    int apa_id;

    std::size_t size() const { return waveforms.size(); }
    Waveforms(std::size_t n, double val, int spill_id, int apa_id) :
      waveforms(n, {val}), spill_id(spill_id), apa_id(apa_id)
    {
      spdlog::info(
        "wsctor\t{}\t{}\t{}\t{}", spill_id, apa_id, static_cast<void*>(this), waveforms.size());
    }

    Waveforms(Waveforms const& other) :
      waveforms(other.waveforms), spill_id(other.spill_id), apa_id(other.apa_id)
    {
      spdlog::info(
        "wscopy\t{}\t{}\t{}\t{}", spill_id, apa_id, static_cast<void*>(this), waveforms.size());
    }

    Waveforms(Waveforms&& other) :
      waveforms(std::move(other.waveforms)), spill_id(other.spill_id), apa_id(other.apa_id)
    {
      spdlog::info(
        "wsmove\t{}\t{}\t{}\t{}", spill_id, apa_id, static_cast<void*>(this), waveforms.size());
    }

    // instrument copy assignment and move assignment
    Waveforms& operator=(Waveforms const& other)
    {
      waveforms = other.waveforms;
      spill_id = other.spill_id;
      apa_id = other.apa_id;
      spdlog::info(
        "wscopy=\t{}\t{}\t{}\t{}", spill_id, apa_id, static_cast<void*>(this), waveforms.size());
      return *this;
    }

    Waveforms& operator=(Waveforms&& other)
    {
      waveforms = std::move(other.waveforms);
      spill_id = other.spill_id;
      apa_id = other.apa_id;
      spdlog::info(
        "wsmove=\t{}\t{}\t{}\t{}", spill_id, apa_id, static_cast<void*>(this), waveforms.size());
      return *this;
    }

    ~Waveforms()
    {
      spdlog::info(
        "wsdtor\t{}\t{}\t{}\t{}", spill_id, apa_id, static_cast<void*>(this), waveforms.size());
    };
  };

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
    explicit WaveformGenerator(WGI const& maxsize) : maxsize_{maxsize.size}
    {
      spdlog::info("Constructed a new WaveformGenerator with maxsize = {}", maxsize.size);
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
    auto op(std::size_t made_so_far, std::size_t chunksize) const
    {
      // How many waveforms should go into this chunk?
      std::size_t const newsize = std::min(chunksize, maxsize_ - made_so_far);
      auto result =
        std::make_pair(made_so_far + newsize, Waveforms{newsize, 1.0 * made_so_far, 0, 0});
      spdlog::info(
        "op: made_so_far = {}, chunksize = {}, ws.size() = {}", made_so_far, chunksize, newsize);
      return result;
    }

  private:
    std::size_t const maxsize_; // total number of waveforms to make for the unfold
  }; // class WaveformGenerator
} // namespace

int main()
{
  // Setup logging.
  auto tsvlog = spdlog::basic_logger_mt("tsvlog", "unfold_transform_fold.tsv");
  spdlog::set_default_logger(tsvlog);
  spdlog::set_pattern("%v"); // output only the message, no metadata.
  spdlog::info("time\tthread\tevent\tnode\tmessage\tdata");
  spdlog::set_pattern("%H:%M:%S.%f\t%t\t%v");

  // Constants that configure the work load.
  //constexpr auto wires_per_spill = 150 * 10 * 1000ul;
  constexpr auto wires_per_spill = 10 * 1000ul;
  constexpr auto number_of_spills = 2u;

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
      spdlog::info("Adding WGI to spill {} with count = {}", store->id()->hash(), next_size);
      store->add_product<WGI>("wgen", {next_size});
      ++counter;
    }
    return store;
  };

  // Create the graph. The source tells us what data we will process.
  spdlog::info("Creating the graph");
  framework_graph g{source};

  // g.with<X> is "registration code" as implemented in meld.
  // Each call to g.with<> creates a new CHOF in the graph.

  // Add the unfold node to the graph. We do not yet know how to provide the chunksize
  // to the constructor of the WaveformGenerator, so we will use the default value.
  spdlog::info("Adding the unfold node");
  auto const chunksize = 10 * 1000ULL; // this could be read from a configuration file
  // auto unfold_op = [](WaveformGenerator& wg, std::size_t running_value) {
  //   return wg.op(running_value, chunksize);
  // };
  g.with<WaveformGenerator>(
     &WaveformGenerator::predicate,
     [](WaveformGenerator const& wg, std::size_t running_value) {
       return wg.op(running_value, chunksize);
     },
     concurrency::unlimited)
    .unfold("wgen")        // the type of node to create
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
