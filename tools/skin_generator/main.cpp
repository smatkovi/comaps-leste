#include "generator.hpp"

#include <iostream>
#include "base/logging.hpp"

#include <QApplication>
#include <QtCore/QFile>
#include <QtCore/QHash>
#include <QtCore/QString>

#include <QtCore/QCommandLineParser>

int main(int argc, char * argv[])
{
  // Used to lock the hash seed, so the order of XML attributes is always the same.
  QHashSeed::setDeterministicGlobalSeed();
  try
  {
    QApplication app(argc, argv);

    QCommandLineParser parser;
    parser.addHelpOption();
    QCommandLineOption const symbolsDirOpt("symbolsDir", "directory with svg symbol files", "dir");
    QCommandLineOption const symbolWidthOpt("symbolWidth", "width of the rendered symbol", "px", "24");
    QCommandLineOption const symbolHeightOpt("symbolHeight", "height of the rendered symbol", "px", "24");
    QCommandLineOption const skinNameOpt("skinName", "prefix for the skin and skinImage file name", "path");
    QCommandLineOption const skinSuffixOpt("skinSuffix", "suffix for skinName<suffix>.skn and symbols<suffix>.png",
                                           "suffix", "");
    QCommandLineOption const maxSizeOpt("maxSize", "max width/height of output textures", "px", "4096");
    parser.addOptions({symbolsDirOpt, symbolWidthOpt, symbolHeightOpt, skinNameOpt, skinSuffixOpt, maxSizeOpt});
    parser.process(app);

    std::string const symbolsDir = parser.value(symbolsDirOpt).toStdString();
    std::string const skinName = parser.value(skinNameOpt).toStdString();
    std::string const skinSuffix = parser.value(skinSuffixOpt).toStdString();

    tools::SkinGenerator gen;

    std::vector<QSize> symbolSizes;
    symbolSizes.emplace_back(QSize(parser.value(symbolWidthOpt).toInt(), parser.value(symbolHeightOpt).toInt()));

    std::vector<std::string> suffixes;
    suffixes.push_back(skinSuffix);

    gen.ProcessSymbols(symbolsDir, skinName, symbolSizes, suffixes);

    if (!gen.RenderPages(parser.value(maxSizeOpt).toInt()))
    {
      LOG(LINFO, ("Error: The texture is overflown."));
      return 1;
    }

    QString newSkin(skinName.c_str());
    newSkin.replace("basic", "symbols");
    auto const filename = newSkin.toStdString() + skinSuffix + ".sdf";
    if (!gen.WriteToFileNewStyle(filename))
    {
      std::cerr << "Could not write file" << filename << std::endl;
      return -1;
    }

    std::cout << "Done" << std::endl;
    return 0;
  }
  catch (std::exception const & e)
  {
    std::cerr << "Exception " << e.what() << std::endl;
    return -1;
  }
}
