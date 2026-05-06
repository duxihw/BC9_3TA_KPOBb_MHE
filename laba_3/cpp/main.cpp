#include <iostream>
#include <fstream>
#include <algorithm>
#include <cmath>
#include <ios>
#include <iomanip>
#include <filesystem>

float recurent_mean(float const *psi, float const *pdf, float const dv, unsigned size);
float close_mean(float const *psi, float const *pdf, float const dv, unsigned size);
float kehen_mean(float const *psi, float const *pdf, float const dv, unsigned size);
float fma_mean(float const *psi, float const *pdf, float const dv, unsigned size);
double d_mean(float const *psi, float const *pdf, float const dv, unsigned size);

float* linspace(float start, float end, int size) {
    float* f = new float[size--];
    for (int i = 0; i <= size; ++i) {
        f[i] = start + (end - start) * (1.f * i / size);
    }
    return f;
}

float* Maxwell_distribution(float T, float start, float stop, unsigned size) {
    float *distr = new float[size--];
    for (unsigned i = 0; i <= size; i++)
    {
        float v = start + (stop - start) * (1.f * i / size);
        distr[i] = exp(0 - v*v / T) / sqrt(T*M_PI);
    }
    return distr;
}

float recurent_mean(float const *psi, float const *pdf, float const dv, unsigned size) {
    if (size <= 2) {
        float sum = 0.f;
        for (unsigned i = 0; i != size; i++)
        {
            sum += psi[i] * pdf[i] * dv;
        }
        return sum;
    }

    return recurent_mean(psi, pdf, dv, size / 2) + recurent_mean(psi + size / 2, pdf + size / 2, dv, size - size / 2);
}

float close_mean(float const *psi, float const *pdf, float const dv, unsigned size)
{
    unsigned size_x = 1;
    for ( ; size_x < size; size_x <<= 1) {}

    float *x = new float[size_x] ();
    for (unsigned i = 0; i != size; i++)
        x[i] = psi[i] * pdf[i] * dv;
    
    for (unsigned step = 1; step != size_x; step <<= 1)
    {
        for (unsigned i = 0; i < size_x - step; i += step << 1)
        {
            x[i] = x[i] + x[i + step];
        }
        
    }

    float sum = x[0];
    delete[] x;
    return sum;
}

float kehen_mean(float const *psi, float const *pdf, float const dv, unsigned size) {
    float sum = 0.f;
    float c = 0.f;
    for (unsigned i = 0; i != size; i++)
    {
        float y = psi[i] * pdf[i] * dv - c;
        float t = sum + y;
        c = (t - sum) - y;
        sum = t;
    }
    return sum;
}

float fma_mean(float const *psi, float const *pdf, float const dv, unsigned size) {
    float sum = 0.f;
    float c = 0.f;
    for (unsigned i = 0; i != size; i++)
    {
        float y = fma(psi[i], pdf[i] * dv, -c);
        float t = sum + y;
        c = (t - sum) - y;
        sum = t;
    }
    return sum;
}

double d_mean(float const *psi, float const *pdf, float const dv, unsigned size) {
    double sum = 0.;
    double c = 0.;
    for (unsigned i = 0; i != size; i++)
    {
        double y = fma(psi[i], pdf[i] * dv, -c);
        double t = sum + y;
        c = (t - sum) - y;
        sum = t;
    }
    return sum;
}


void write_experiment_results() {

    std::string data_dir = "../../data/laba_3";
    std::filesystem::create_directories(data_dir);

    std::string filepath = data_dir + "/summation_results.csv";
    std::ofstream out(filepath);
    
    if (!out.is_open()) {
        std::cerr << "Не удалось открыть файл для записи: " << filepath << std::endl;
        return;
    }

    out << std::fixed << std::setprecision(7) << std::scientific;

    out << "Method,T,N_points,Variable,Calculated,Analytical,Abs_Error,Rel_Error_%\n";

    auto record_experiment = [&](const std::string& method, float T, unsigned size, 
                                 const std::string& var_name, double calc, double anal) {
        double abs_err = std::abs(calc - anal);
        double rel_err = (anal != 0.0) ? (calc / anal - 1.0) * 100.0 : 0.0;
        if (rel_err < 0) rel_err = -rel_err;
        
        out << method << "," << T << "," << size << "," << var_name << ","
            << calc << "," << anal << "," << abs_err << "," << rel_err << "\n";
    };

    auto execute_test = [&](float T, unsigned size) {
        float bound = 5.0f * std::sqrt(T); 
        float dv = (2.0f * bound) / (size - 1);

        float* psi = Maxwell_distribution(T, -bound, bound, size);
        float* pdf_abs_v = linspace(-bound, bound, size);
        float* pdf_v2 = new float[size];

        for (unsigned i = 0; i < size; i++) {
            if (pdf_abs_v[i] < 0) pdf_abs_v[i] = -pdf_abs_v[i];
            pdf_v2[i] = pdf_abs_v[i] * pdf_abs_v[i];
        }

        double anal_abs_v = std::sqrt(T / M_PI);
        double anal_v2 = T / 2.0;

        // 1. Recurrent Mean
        record_experiment("Recurrent", T, size, "|v|", recurent_mean(psi, pdf_abs_v, dv, size), anal_abs_v);
        record_experiment("Recurrent", T, size, "v^2", recurent_mean(psi, pdf_v2, dv, size), anal_v2);

        // 2. Close Mean
        record_experiment("Close", T, size, "|v|", close_mean(psi, pdf_abs_v, dv, size), anal_abs_v);
        record_experiment("Close", T, size, "v^2", close_mean(psi, pdf_v2, dv, size), anal_v2);

        // 3. Kehen Mean (Kahan)
        record_experiment("Kahan", T, size, "|v|", kehen_mean(psi, pdf_abs_v, dv, size), anal_abs_v);
        record_experiment("Kahan", T, size, "v^2", kehen_mean(psi, pdf_v2, dv, size), anal_v2);

        // 4. FMA Mean
        record_experiment("FMA", T, size, "|v|", fma_mean(psi, pdf_abs_v, dv, size), anal_abs_v);
        record_experiment("FMA", T, size, "v^2", fma_mean(psi, pdf_v2, dv, size), anal_v2);

        // 5. Double Mean
        record_experiment("Double", T, size, "|v|", d_mean(psi, pdf_abs_v, dv, size), anal_abs_v);
        record_experiment("Double", T, size, "v^2", d_mean(psi, pdf_v2, dv, size), anal_v2);

        delete[] psi;
        delete[] pdf_abs_v;
        delete[] pdf_v2;
    };


    float T_values[4] = {0.1f, 1.0f, 100.0f, 10000.0f};
    unsigned size = 350;

    for (float T : T_values) {
        execute_test(T, size);
    }

    unsigned sizes_for_T1[3] = {10, 1000, 10000};
    
    for (unsigned s : sizes_for_T1) {
        execute_test(1.0f, s);
    }

    out.close();
    std::cout << "Данные успешно записаны в файл: " << filepath << std::endl;
}

int main() {

    write_experiment_results();

    return 0;
}