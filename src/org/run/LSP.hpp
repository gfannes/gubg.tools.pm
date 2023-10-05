#ifndef HEADER_org_run_LSP_hpp_ALREAD_INCLUDED
#define HEADER_org_run_LSP_hpp_ALREAD_INCLUDED

#include <org/Options.hpp>

#include <nlohmann/json.hpp>

namespace org { namespace run {

    class LSP
    {
    public:
        LSP(const Options &);

        bool run();

    private:
        bool read_json_message_(nlohmann::json &) const;
        bool read_(std::string &, const std::string &, const nlohmann::json &);
        bool read_(int &, const std::string &, const nlohmann::json &);

        const Options &options_;

        gubg::Logger log_;

        mutable std::string tmp_str_;
    };

}} // namespace org::run

#endif
