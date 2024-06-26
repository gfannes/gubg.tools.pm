#ifndef HEADER_org_run_LSP_hpp_ALREADY_INCLUDED
#define HEADER_org_run_LSP_hpp_ALREADY_INCLUDED

#include <org/Options.hpp>
#include <org/types.hpp>
#include <org/tree/Parser.hpp>

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
        bool read_(std::size_t &, const std::string &, const nlohmann::json &);
        bool read_(const nlohmann::json *&, const std::string &, const nlohmann::json &);

        struct Pos
        {
            Ix line = 0;
            Ix col = 0;
        };
        bool read_(Pos &, const nlohmann::json &);

        const Options &options_;

        gubg::Logger log_;

        tree::Parser parser_;

        mutable std::string tmp_str_;
    };

}} // namespace org::run

#endif
