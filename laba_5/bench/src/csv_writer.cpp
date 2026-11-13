#include "bench/csv_writer.hpp"

#include <iomanip>
#include <locale>
#include <stdexcept>

namespace bench {

CsvWriter::CsvWriter(const std::filesystem::path& path, const std::vector<std::string>& header)
    : path_(path), columns_(header.size()) {
    if (path.has_parent_path()) std::filesystem::create_directories(path.parent_path());
    out_.open(path);
    if (!out_) throw std::runtime_error("cannot open " + path.string());
    out_.imbue(std::locale::classic());  // всегда десятичная точка
    for (std::size_t i = 0; i < header.size(); ++i) {
        write_cell(header[i]);
        separator(i + 1);
    }
}

void CsvWriter::write_cell(const std::string& s) {
    // Экранирование по RFC 4180: поле с запятой/кавычкой берётся в кавычки.
    if (s.find_first_of(",\"\n") == std::string::npos) {
        out_ << s;
        return;
    }
    out_ << '"';
    for (char c : s) {
        if (c == '"') out_ << '"';
        out_ << c;
    }
    out_ << '"';
}

void CsvWriter::write_cell(double v) { out_ << std::setprecision(8) << v; }
void CsvWriter::write_cell(long long v) { out_ << v; }
void CsvWriter::write_cell(unsigned long long v) { out_ << v; }

void CsvWriter::separator(std::size_t written) {
    if (written < columns_) {
        out_ << ',';
    } else {
        out_ << '\n';
        out_.flush();  // данные не потеряются, если бенчмарк прервут
    }
}

void CsvWriter::throw_column_mismatch(std::size_t got) const {
    throw std::logic_error(path_.string() + ": expected " + std::to_string(columns_) +
                           " columns, got " + std::to_string(got));
}

}  // namespace bench
