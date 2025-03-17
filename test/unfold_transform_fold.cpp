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

  void log_record(char const* event,
                  std::size_t spill_id,
                  std::size_t apa_id,
                  void const* active,
                  std::size_t data,
                  void const* orig)
  {
    spdlog::info("{}\t{}\t{}\t{}\t{}\t{}", event, spill_id, apa_id, active, data, orig);
  }

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
      log_record("wsctor", spill_id, apa_id, this, n, nullptr);
    }

    Waveforms(Waveforms const& other) :
      waveforms(other.waveforms), spill_id(other.spill_id), apa_id(other.apa_id)
    {
      log_record("wscopy", spill_id, apa_id, this, waveforms.size(), &other);
    }

    Waveforms(Waveforms&& other) :
      waveforms(std::move(other.waveforms)), spill_id(other.spill_id), apa_id(other.apa_id)
    {
      log_record("wsmove", spill_id, apa_id, this, waveforms.size(), &other);
    }

    // instrument copy assignment and move assignment
    Waveforms& operator=(Waveforms const& other)
    {
      waveforms = other.waveforms;
      spill_id = other.spill_id;
      apa_id = other.apa_id;
      log_record("wscopy=", spill_id, apa_id, this, waveforms.size(), &other);
      return *this;
    }

    Waveforms& operator=(Waveforms&& other)
    {
      waveforms = std::move(other.waveforms);
      spill_id = other.spill_id;
      apa_id = other.apa_id;
      log_record("wsmove=", spill_id, apa_id, this, waveforms.size(), &other);
      return *this;
    }

    ~Waveforms() { log_record("wsdtor", spill_id, apa_id, this, waveforms.size(), nullptr); };
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
      log_record("wgc_ctor", 0, 0, this, 0, nullptr);
    }

    // The initial value of the count of how many waveforms we have made so far.
    std::size_t initial_value() const { return 0; }

    // When we have made at least as many waveforms as we have been asked to make
    // for this unfold, we are done. The predicate answers the question "should
    // we continue unfolding?"
    bool predicate(std::size_t made_so_far) const
    {
      bool const result = made_so_far < maxsize_;
      log_record("pred", 0, 0, this, made_so_far, nullptr);
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
      log_record("op", 0, 0, this, newsize, nullptr);
      return result;
    }

  private:
    std::size_t const maxsize_; // total number of waveforms to make for the unfold
  }; // class WaveformGenerator

  // This function is used to transform an input Waveforms object into an
  // output Waveforms object. The output is a clamped version of the input.
  Waveforms clampWaveforms(Waveforms const& input)
  {
    Waveforms result(input);
    for (Waveform& wf : result.waveforms) {
      for (double& x : wf.samples) {
        x = std::clamp(x, -10.0, 10.0);
      }
    }
    return result;
  }

} // namespace

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
  constexpr auto wires_per_spill = 350 * 1000ul;
  constexpr auto number_of_spills = 10u;

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
      log_record("add_wgi", store->id()->hash(), 0, &store, next_size, nullptr);
      store->add_product<WGI>("wgen", {next_size});
      ++counter;
    }
    return store;
  };

  // Create the graph. The source tells us what data we will process.
  log_record("create_graph", 0, 0, nullptr, 0, nullptr);
  framework_graph g{source};

  // g.with<X> is "registration code" as implemented in meld.
  // Each call to g.with<> creates a new CHOF in the graph.

  // Add the unfold node to the graph. We do not yet know how to provide the chunksize
  // to the constructor of the WaveformGenerator, so we will use the default value.
  log_record("add_unfold", 0, 0, nullptr, 0, nullptr);
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

  // Add the transform node to the graph.
  log_record("add_transform", 0, 0, nullptr, 0, nullptr);
  g.with(clampWaveforms, concurrency::unlimited)
    .transform("waves_in_apa") // the type of node to create, and the label of the input
    .to("clamped_waves");      // label the chunks we create as "clamped_waves"

  log_record("add_output", 0, 0, nullptr, 0, nullptr);
  g.make<test::products_for_output>().output_with(&test::products_for_output::save,
                                                  concurrency::serial);

  // Execute the graph.
  log_record("execute_graph", 0, 0, nullptr, 0, nullptr);
  g.execute("unfold_transform_fold");
  log_record("done", 0, 0, nullptr, 0, nullptr);
}
