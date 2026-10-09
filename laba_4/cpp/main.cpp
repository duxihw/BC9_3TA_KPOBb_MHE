#include <random>
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>

class RandomVariable {
public:
    virtual unsigned roll() = 0;
    virtual ~RandomVariable() = default;
};

class Dice : public RandomVariable {
public:
    Dice(unsigned max, unsigned seed):
        max(max), dstr(1, max), reng(seed) { }

    unsigned roll() override {
        return dstr(reng);
    }

    unsigned max_value() const { return max; }

private:
    unsigned max;
    std::uniform_int_distribution<unsigned> dstr;
    std::default_random_engine reng;
};

class ThreeDicePool : public RandomVariable {
public:
    ThreeDicePool(RandomVariable &d1, RandomVariable &d2, RandomVariable &d3):
        d1(d1), d2(d2), d3(d3) { }

    unsigned roll() override {
        return d1.roll() + d2.roll() + d3.roll();
    }

private:
    RandomVariable &d1, &d2, &d3;
};

double expected_value(RandomVariable &rv, unsigned number_of_rolls = 1) {
    if (number_of_rolls == 0) return 0;
    auto accum = 0llu;
    for (unsigned cnt = 0; cnt != number_of_rolls; ++cnt)
        accum += rv.roll();
    return static_cast<double>(accum) / number_of_rolls;
}

class PenaltyDice : public RandomVariable {
public:
    PenaltyDice(RandomVariable &d): d(d) { }

    unsigned roll() override {
        unsigned r1 = d.roll();
        unsigned r2 = d.roll();
        return r1 < r2 ? r1 : r2;
    }

protected:
    RandomVariable &d;
};

class BonusDice : public RandomVariable {
public:
    BonusDice(RandomVariable &d): d(d) { }

    unsigned roll() override {
        unsigned r1 = d.roll();
        unsigned r2 = d.roll();
        return r1 > r2 ? r1 : r2;
    }

protected:
    RandomVariable &d;
};

double value_probability(unsigned value, RandomVariable &d, unsigned number_of_rolls = 1) {
    if (number_of_rolls == 0) return 0;
    auto hits = 0llu;
    for (unsigned cnt = 0; cnt != number_of_rolls; ++cnt)
        if (d.roll() == value)
            ++hits;
    return static_cast<double>(hits) / number_of_rolls;
}

void save_histogram_csv(const std::string &filepath, RandomVariable &d,
                         unsigned min_value, unsigned max_value, unsigned number_of_rolls) {
    std::ofstream out(filepath);
    out << "value,probability\n";
    for (unsigned v = min_value; v <= max_value; ++v) {
        double p = value_probability(v, d, number_of_rolls);
        out << v << ',' << p << '\n';
    }
}


class DoubleDice : public PenaltyDice, public BonusDice {
public:
    DoubleDice(RandomVariable &d): PenaltyDice(d), BonusDice(d) { }
};

/* без множественного наследования */
class DoubleDiceComposite {
public:
    DoubleDiceComposite(RandomVariable &d): penalty(d), bonus(d) { }

    PenaltyDice &penalty_dice() { return penalty; }
    BonusDice   &bonus_dice()   { return bonus; }

private:
    PenaltyDice penalty;
    BonusDice   bonus;
};

int main() {
    const unsigned N = 200000;

    std::string data_dir = "../../data/laba_4";
    std::filesystem::create_directories(data_dir);

    std::ofstream ev_out(data_dir + "/expected_values.csv");
    ev_out << "name,expected_value\n";

    /* Задание 1 */
    Dice d100(100, 1);
    ev_out << "\"Dice(100)\"," << expected_value(d100, N) << '\n';

    Dice a(6, 11), b(6, 22), c(6, 33);
    ThreeDicePool tdp(a, b, c);
    ev_out << "\"ThreeDicePool(6,6,6)\"," << expected_value(tdp, N) << '\n';

    /* Задание 2 */
    Dice base100(100, 42);
    PenaltyDice pen100(base100);
    BonusDice   bon100(base100);

    save_histogram_csv(data_dir + "/dice100_normal.csv", base100, 1, 100, N);
    save_histogram_csv(data_dir + "/dice100_penalty.csv", pen100, 1, 100, N);
    save_histogram_csv(data_dir + "/dice100_bonus.csv", bon100, 1, 100, N);

    Dice t1(6, 101), t2(6, 202), t3(6, 303);
    ThreeDicePool tdp2(t1, t2, t3);
    PenaltyDice pen_tdp(tdp2);
    BonusDice   bon_tdp(tdp2);

    save_histogram_csv(data_dir + "/tdp_normal.csv", tdp2, 3, 18, N);
    save_histogram_csv(data_dir + "/tdp_penalty.csv", pen_tdp, 3, 18, N);
    save_histogram_csv(data_dir + "/tdp_bonus.csv", bon_tdp, 3, 18, N);

    /* Задание 3 */
    Dice base_dd(100, 777);
    DoubleDice dd(base_dd);

    ev_out << "\"DoubleDice(penalty part)\"," << expected_value(static_cast<PenaltyDice&>(dd), N) << '\n';
    ev_out << "\"DoubleDice(bonus part)\"," << expected_value(static_cast<BonusDice&>(dd), N) << '\n';

    save_histogram_csv(data_dir + "/doubledice_penalty.csv", static_cast<PenaltyDice&>(dd), 1, 100, N);
    save_histogram_csv(data_dir + "/doubledice_bonus.csv", static_cast<BonusDice&>(dd), 1, 100, N);

    Dice base_dd2(100, 888);
    DoubleDiceComposite ddc(base_dd2);

    ev_out << "\"DoubleDiceComposite(penalty)\"," << expected_value(ddc.penalty_dice(), N) << '\n';
    ev_out << "\"DoubleDiceComposite(bonus)\"," << expected_value(ddc.bonus_dice(), N) << '\n';

    ev_out.close();

    std::cout << "Данные успешно записаны в: " << data_dir << std::endl;

    return 0;
}
