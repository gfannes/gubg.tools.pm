#ifndef HEADER_org_App_hpp_ALREAD_INCLUDED
#define HEADER_org_App_hpp_ALREAD_INCLUDED

#include <org/Options.hpp>
#include <org/tree/Node.hpp>

#include <gubg/Logger.hpp>

#include <string>

namespace org { namespace run {

    class Normal
    {
    public:
        Normal(const Options &options)
            : options_(options), log_({.filename = options.log_filepath}) {}

        bool run();

    private:
        bool load_env_vars_();
        bool parse_(const std::string &fp);
        bool write_(const std::string &fp);

        const Options &options_;

        gubg::Logger log_;

        tree::Node root_;

        mutable std::string tmp_str_;
    };

}} // namespace org::run

#endif
