#pragma once

#include <SDL3/SDL.h>

// Définition basique de Matrix4 (à ajuster si nécessaire)
struct Matrix4 {
    float m[4][4]; // Matrice 4x4 simple
    Matrix4() {
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j)
                m[i][j] = (i == j) ? 1.0f : 0.0f; // Identité par défaut
    }
};

class Spiral {
public:
    void render(const Matrix4& projection, const Matrix4& view) const;
};
