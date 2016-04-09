#ifndef LABO1_MIXEDINVERSE_H
#define LABO1_MIXEDINVERSE_H

#include <vector>
#include <cmath>

#include "UniformGenerator.h"
#include "Slice.h"

class MixedInverse : public RandomValueGenerator<double> {
private:
    UniformRealGenerator<double>* generator;

    std::vector<double> F_parts; // "bouts" de la fonction de répartition; F_parts[i] = F_parts[i-1] + pks[i]
public:

    MixedInverse(const std::vector<double>& xs, const std::vector<double>& ys, const std::seed_seq& seed) noexcept(false)
            : RandomValueGenerator<double>(xs, ys) {

        generator = new UniformRealGenerator<double>(0, 1);
        generator->setSeed(seed);

        // création de tranches
        double A = 0; // aire totale
        for (const Slice<double>& s : slices) {
            A += s.A_k;
        }

        // création des pk
        std::vector<double> pks;
        pks.reserve(xs.size()-1);
        for (size_t i = 0; i < xs.size()-1; ++i) {
            pks.push_back(slices[i].A_k/A);
        }

        // préparation des parties de F
        F_parts.resize(xs.size());
        F_parts[0] = 0; // F_0 : premiere partie de la fonction de répartition -> 0 avant xs[0]

        for (size_t i = 1; i < F_parts.size(); ++i) {
            F_parts[i] = F_parts[i-1] + pks[i-1];
        }
    }

    double generate() const {

        // On commence par sélectionner une intervalle en fonction du p_k associé à la tranche liée à cette intervalle.
        // K représente l'indice de l'intervalle sélectionné.
        size_t K = generateK();

        // Ensuite, on applique la méthode des fonctions inverses.
        Slice<double> s = slices[K];

        double x1 = s.p1.x, x2 = s.p2.x;
        double y1 = s.p1.y, y2 = s.p2.y;

        double U = generator->next();

        // Si y1 = y2, alors on est dans le cas d'une uniforme. Sinon, on inverse la fonction de repartition associée
        // à la fonction f_k.
        if (y1 == y2) {
            return x1 + U*(x2 - x1);
        } else {
            double m = (y2 - y1)/(x2 - x1);
            return x1 + (sqrt( (y2*y2 - y1*y1) * U + y1*y1 ) - y1) / m;
        }
    }

    ~MixedInverse() {
        delete generator;
    }

private:
    // permet de trouve dans quel intervalle k on tombe en fonction de la probablilité p_k de la tranche liée à
    // cette intervalle
    size_t generateK() const {
        size_t j = 1;
        double U = generator->next();

        // on cherche l'indice de l'intervalle dans lequel on est tombé
        while (true) {
            if (U <= F_parts[j]) {
                return j-1;
            }
            ++j;
        }
    }
};

#endif //LABO1_MIXEDINVERSE_H