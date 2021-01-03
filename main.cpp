#include "modules/include/FileSystemWatcher.h"

#include <bitset>
#include <chrono>
#include <filesystem>
#include <memory>

class Listen
{
  public:
    Listen(std::filesystem::path path)
    {
        _watcher = std::make_unique<FileSystemWatcher>(
            path, std::chrono::milliseconds(1),
            std::bind(&Listen::listenerFunction, this,
                      std::placeholders::_1));
    }
    void listenerFunction(std::vector<EventPtr> events)
    {
        for (const auto &event : events) {
            std::bitset<16> typeBits(event->type);
            std::cout << event->relativePath << " с типом: " << typeBits
                      << std::endl;
        }
    }

  private:
    std::unique_ptr<FileSystemWatcher> _watcher;
};

int main(int argc, char *argv[])
{
    if (argc < 2) {
        std::cout << "требуется один входной параметр" << std::endl;
    }

    const auto path = std::filesystem::path(argv[1]);

    std::cout << "наблюдаемый путь '" << path.string() << "'" << std::endl;

    auto listenerInstance = Listen(path);

    std::cout << "Выход ... любая кнопка" << std::endl;
    std::cin.ignore();

    return 0;
}
