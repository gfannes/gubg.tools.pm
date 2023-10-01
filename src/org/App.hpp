#ifndef HEADER_org_App_hpp_ALREAD_INCLUDED
#define HEADER_org_App_hpp_ALREAD_INCLUDED

#include <org/Options.hpp>
#include <org/tree/Node.hpp>

#include <gubg/Logger.hpp>

#include <nlohmann/json.hpp>

#include <string>

namespace org {

    class App
    {
    public:
        App(const Options &options)
            : options_(options), log_({.filename = options.log_filepath}) {}

        bool run();

    private:
        bool load_env_vars_();
        bool run_normal_();
        bool run_lsp_();
        bool read_json_message_(nlohmann::json &) const;
        bool read_(std::string &, const std::string &, const nlohmann::json &);
        bool read_(int &, const std::string &, const nlohmann::json &);

        const Options &options_;

        gubg::Logger log_;

        tree::Node root_;

        mutable std::string tmp_str_;
    };

} // namespace org

#endif
