#ifndef HEADER_pit_report_hpp_ALREADY_INCLUDED
#define HEADER_pit_report_hpp_ALREADY_INCLUDED

#include <pit/reporter/Stdout.hpp>
#include <pit/reporter/Tsv.hpp>
#include <pit/Model.hpp>
#include <pit/Options.hpp>

namespace pit { 

    template <typename Reporter>
    bool report_(const Model &model, const Options &options)
    {
        MSS_BEGIN(bool);

        Reporter reporter;
        MSS(reporter.process(options));

        auto start_node = model.root();
        if (options.uri)
        {
            const auto &uri = *options.uri;
            std::string error;
            auto ptr = model.resolve(uri, *start_node, &error);
            MSS(!!ptr, std::cout << "Error: could not find node " << uri << ": " << error << std::endl);
            start_node = ptr;
        }

        MSS(reporter.process(model, start_node));

        MSS_END();
    }

    inline bool report(const Model &model, const Options &options)
    {
        MSS_BEGIN(bool);

        if (options.output_fn)
            MSS(report_<reporter::Tsv>(model, options));
        else
            MSS(report_<reporter::Stdout>(model, options));

        MSS_END();
    }

} 

#endif
