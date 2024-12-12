#pragma comment(lib, "bcrypt.lib")
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <iomanip>
#include <chrono>
#include "toml.hpp"
#include "unzip.h"


namespace fs = std::filesystem;
using namespace std;

// Класс для работы с логи
class Logger {
public:
    Logger(const string& logFilePath) {
        logFile.open(logFilePath, ios::app); //ios::app: файл открывается для дозаписи. Старые данные не удаляются.
        if (!logFile.is_open()) {
            throw runtime_error("Ошибка при открытии лог-файла!");
        }
    }

    ~Logger() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }

    void log(const string& action) {
        time_t now = chrono::system_clock::to_time_t(chrono::system_clock::now());
        tm localTime;
        localtime_s(&localTime, &now);

        //char buf[100] = { 0 };
        logFile << "\"" << put_time(&localTime, "%Y-%m-%d %H:%M:%S") << "\",\"" << action << "\"" << std::endl;
        //logFile << "\"" << buf << "\",\"" << action << "\"" << endl;
    }

private:
    ofstream logFile;
};

// Основной класс для эмулятора shell
class ShellEmulator {
public:
    ShellEmulator(const string& configPath) : logger(toml::get<string>(toml::parse(configPath)["logfile"])) {
        // Чтение конфигурации
        auto config = toml::parse(configPath);
        hostname = toml::get<string>(config["hostname"]);
        zipPath = toml::get<string>(config["virtual_fs"]);
        logPath = toml::get<string>(config["logfile"]);

        // Разархивируем виртуальную файловую систему во временную папку
        //fs::path tempDir = fs::temp_directory_path() / fs::unique_path();
        /*fs::create_directory(tempDir);
        unzip(zipPath, tempDir.string());*/

        char tempPath[L_tmpnam];
        tmpnam_s(tempPath);
        fs::path tempDir = fs::temp_directory_path() / tempPath;
        fs::create_directory(tempDir);
        unzip(zipPath, tempDir.string());

        currentPath = tempDir;
    }

    ~ShellEmulator() {
        fs::remove_all(currentPath); // Чистим временную папку
    }

    void run() {
        string command;
        while (true) {
            prompt();
            getline(cin, command);
            executeCommand(command);
        }
    }

    void executeCommand(const string& command) {
        istringstream iss(command); //Превращаем строку в поток
        string cmd;
        iss >> cmd;

        if (cmd == "exit") {
            logger.log("exit");
            exit(0);
        }
        else if (cmd == "ls") {
            ls();
        }
        else if (cmd == "cd") {
            string path;
            iss >> path;
            cd(path);
        }
        else if (cmd == "rmdir") {
            string dir;
            iss >> dir;
            rmdir(dir);
        }
        else if (cmd == "uniq") {
            string file;
            iss >> file;
            uniq(file);
        }
        else {
            cout << "Unknown command: " << cmd << endl;
        }
    }

private:
    string hostname;
    string zipPath;
    string logPath;
    fs::path currentPath;
    Logger logger;

    void prompt() const {
        cout << hostname << ":" << currentPath.string() << "> ";
    }

    void ls() {
        for (const auto& entry : fs::directory_iterator(currentPath)) {
            cout << entry.path().filename().string() << endl;
        }
        logger.log("ls");
    }

    void cd(const string& path) {
        fs::path newPath = currentPath / path;
        if (fs::exists(newPath) && fs::is_directory(newPath)) {
            currentPath = fs::canonical(newPath);
            logger.log("cd " + newPath.string());
        }
        else {
            cout << "Directory not found: " << path << endl;
        }
    }

    void rmdir(const string& dir) {
        fs::path dirPath = currentPath / dir;
        if (fs::exists(dirPath) && fs::is_directory(dirPath)) {
            fs::remove_all(dirPath);
            logger.log("rmdir " + dirPath.string());
            cout << "Removed directory: " << dir << endl;
        }
        else {
            cout << "Directory not found: " << dir << endl;
        }
    }

    void uniq(const string& file) {
        fs::path filePath = currentPath / file;
        if (fs::exists(filePath) && fs::is_regular_file(filePath)) {
            ifstream infile(filePath);
            ofstream outfile(filePath.string() + ".uniq");
            string line;
            string prevLine;
            while (std::getline(infile, line)) {
                if (line != prevLine) {
                    outfile << line << endl;
                    prevLine = line;
                }
            }
            infile.close();
            outfile.close();
            logger.log("uniq " + filePath.string());
            cout << "Created unique file: " << file << ".uniq" << endl;
        }
        else {
            cout << "File not found: " << file << endl;
        }
    }

    void unzip(const string& zipPath, const string& extractTo) {
        unzFile zipFile = unzOpen(zipPath.c_str());
        if (zipFile == nullptr) {
            cerr << "Не удалось открыть ZIP-файл: " << zipPath << endl;
            return;
        }

        if (unzGoToFirstFile(zipFile) != UNZ_OK) {
            cerr << "Ошибка при переходе к первому файлу в ZIP: " << zipPath << endl;
            unzClose(zipFile);
            return;
        }

        do {
            char filename[256];
            unz_file_info fileInfo;
            if (unzGetCurrentFileInfo(zipFile, &fileInfo, filename, sizeof(filename), nullptr, 0, nullptr, 0) != UNZ_OK) {
                cerr << "Ошибка при получении информации о файле из ZIP: " << zipPath << endl;
                continue;
            }

            const fs::path filePath = fs::path(extractTo) / filename;

            if (filename[strlen(filename) - 1] == '/') {
                fs::create_directories(filePath);
            }
            else {
                fs::create_directories(filePath.parent_path());
                ofstream outFile(filePath, ios::binary);

                if (unzOpenCurrentFile(zipFile) != UNZ_OK) {
                    cerr << "Не удалось открыть файл из ZIP: " << filename << endl;
                    continue;
                }

                char buffer[8192];
                int bytesRead;
                while ((bytesRead = unzReadCurrentFile(zipFile, buffer, sizeof(buffer))) > 0) {
                    outFile.write(buffer, bytesRead);
                }

                outFile.close();
                unzCloseCurrentFile(zipFile);
            }
        } while (unzGoToNextFile(zipFile) == UNZ_OK);

        unzClose(zipFile);
    }
};

// Основная функция для запуска эмулятора
int main(int argc, char* argv[]) {
    setlocale(LC_ALL, "Russian");

    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <config_file>" << endl;
        return 1;
    }

    string configPath = argv[1];
    try {
        ShellEmulator emulator(configPath);
        emulator.run();
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
