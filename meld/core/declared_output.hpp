#ifndef meld_core_declared_output_hpp
#define meld_core_declared_output_hpp

#include "meld/concurrency.hpp"
#include "meld/core/consumer.hpp"
#include "meld/core/fwd.hpp"
#include "meld/core/message.hpp"
#include "meld/core/node_options.hpp"
#include "meld/core/registrar.hpp"
#include "meld/model/level_id.hpp"
#include "meld/model/product_store.hpp"
#include "meld/model/qualified_name.hpp"

#include "oneapi/tbb/flow_graph.h"

#include <cstddef>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>

namespace meld {
  namespace detail {
    using output_function_t = std::function<void(product_store const&)>;
  }
  class declared_output : public consumer {
  public:
    declared_output(qualified_name name,
                    std::size_t concurrency,
                    std::vector<std::string> predicates,
                    tbb::flow::graph& g,
                    detail::output_function_t&& ft);

    tbb::flow::receiver<message>& port() noexcept;

  private:
    tbb::flow::function_node<message> node_;
  };

  using declared_output_ptr = std::unique_ptr<declared_output>;
  using declared_outputs = std::map<std::string, declared_output_ptr>;

  class output_creator : public node_options<output_creator> {
    using node_options_t = node_options<output_creator>;

  public:
    output_creator(registrar<declared_outputs> reg,
                   configuration const* config,
                   std::string name,
                   tbb::flow::graph& g,
                   detail::output_function_t&& f,
                   concurrency c) :
      node_options_t{config},
      name_{config ? config->get<std::string>("module_label") : "", std::move(name)},
      graph_{g},
      ft_{std::move(f)},
      concurrency_{c},
      reg_{std::move(reg)}
    {
      reg_.set([this] { return create(); });
    }

  private:
    declared_output_ptr create()
    {
      return std::make_unique<declared_output>(std::move(name_),
                                               concurrency_.value,
                                               node_options_t::release_predicates(),
                                               graph_,
                                               std::move(ft_));
    }

    qualified_name name_;
    tbb::flow::graph& graph_;
    detail::output_function_t ft_;
    concurrency concurrency_;
    registrar<declared_outputs> reg_;
  };
}

#endif // meld_core_declared_output_hpp
