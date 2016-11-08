#ifndef RANDOM_HPP
#define RANDOM_HPP

#include <cstdlib>
#include <ctime>

namespace webcppd {

    class random {
    public:

        random() : now(time(0)) {
            srand(this->now);
        }

        template<class T>
        T create(T left, T right) {
            return static_cast<T> (this->create()*(right - left) + left);
        }
    private:
        time_t now;

        double create() {
            return static_cast<double> (rand() / (RAND_MAX + 1.0));
        }
    };
}

#endif /* RANDOM_HPP */

