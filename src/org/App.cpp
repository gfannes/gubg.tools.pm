#include <org/App.hpp>
#include <org/tree/Parser.hpp>

#include <gubg/file/Filesystem.hpp>
#include <gubg/mss.hpp>

namespace org {

    bool App::run()
    {
        MSS_BEGIN(bool);

        MSS(!options_.filepath.empty(), log_.error() << "Expected filepath to be set" << std::endl);

        std::string content;
        MSS(gubg::file::read(content, options_.filepath));

        tree::Parser parser;
        MSS(parser.parse(content));
        std::cout << parser.root << std::endl;

        MSS_END();
    }

} // namespace org
