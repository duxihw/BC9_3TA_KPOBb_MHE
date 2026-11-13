#pragma once
// Запись таблиц в CSV (разделитель — запятая, десятичная точка, первая
// строка — заголовок). Такой формат без настроек читают pandas и Excel/LibreOffice.

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <string>
#include <type_traits>
#include <vector>

namespace bench {

class CsvWriter {
public:
    CsvWriter(const std::filesystem::path& path, const std::vector<std::string>& header);

    // Записать строку из произвольного набора значений:
    //   csv.row("skiplist", 1000, 12.5);
    template <class... Ts>
    void row(const Ts&... values) {
        if (sizeof...(Ts) != columns_) throw_column_mismatch(sizeof...(Ts));
        std::size_t i = 0;
        ((write_cell(values), separator(++i)), ...);
    }

    const std::filesystem::path& path() const { return path_; }

private:
    void write_cell(const std::string& s);
    void write_cell(const char* s) { write_cell(std::string(s)); }
    void write_cell(double v);
    void write_cell(long long v);
    void write_cell(unsigned long long v);
    template <class T>
        requires std::is_integral_v<T>
    void write_cell(T v) {
        if constexpr (std::is_signed_v<T>)
            write_cell(static_cast<long long>(v));
        else
            write_cell(static_cast<unsigned long long>(v));
    }

    void separator(std::size_t written);
    [[noreturn]] void throw_column_mismatch(std::size_t got) const;

    std::filesystem::path path_;
    std::ofstream out_;
    std::size_t columns_;
};

}  // namespace bench
