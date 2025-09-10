#pragma once
#include <cmath>
#include <algorithm>
#include <utility>

namespace Color {

struct RGB { int r{0}, g{0}, b{0}; };
struct XYZ { double X{0}, Y{0}, Z{0}; };
struct Lab { double L{0}, a{0}, b{0}; };
struct HSV { double h{0}, s{0}, v{0}; };

struct ConvertFlags { bool outOfGamut = false; }; // сигнал: цвет был обрезан

// === constants (D65 white point) ===
static constexpr double Xn = 95.047;
static constexpr double Yn = 100.000;
static constexpr double Zn = 108.883;

// --- helpers ---
inline double clamp01(double x) { return std::clamp(x, 0.0, 1.0); }
inline int clamp255(double x01) { return (int)std::round(std::clamp(x01, 0.0, 1.0) * 255.0); }

// sRGB gamma
inline double srgb_to_linear(double u) {
    return (u <= 0.04045) ? (u / 12.92) : std::pow((u + 0.055) / 1.055, 2.4);
}
inline double linear_to_srgb(double u) {
    return (u <= 0.0031308) ? (12.92 * u) : (1.055 * std::pow(u, 1.0 / 2.4) - 0.055);
}

// ---------- HSV <-> RGB ----------
inline HSV RGB_to_HSV(const RGB& rgb) {
    double r = rgb.r / 255.0, g = rgb.g / 255.0, b = rgb.b / 255.0;
    double cmax = std::max({ r, g, b });
    double cmin = std::min({ r, g, b });
    double delta = cmax - cmin;

    double H = 0.0;
    if (delta > 1e-12) {
        if (cmax == r)      H = 60.0 * std::fmod(((g - b) / delta), 6.0);
        else if (cmax == g) H = 60.0 * (((b - r) / delta) + 2.0);
        else                H = 60.0 * (((r - g) / delta) + 4.0);
    }
    if (H < 0) H += 360.0;

    double S = (cmax <= 1e-12) ? 0.0 : (delta / cmax);
    double V = cmax;
    return { H, S, V };
}

inline RGB HSV_to_RGB(const HSV& hsv) {
    double H = std::fmod(hsv.h, 360.0); if (H < 0) H += 360.0;
    double S = clamp01(hsv.s);
    double V = clamp01(hsv.v);

    double C = V * S;
    double X = C * (1 - std::fabs(std::fmod(H / 60.0, 2.0) - 1));
    double m = V - C;

    double r1 = 0, g1 = 0, b1 = 0;
    if      (H <  60) { r1 = C; g1 = X; b1 = 0; }
    else if (H < 120) { r1 = X; g1 = C; b1 = 0; }
    else if (H < 180) { r1 = 0; g1 = C; b1 = X; }
    else if (H < 240) { r1 = 0; g1 = X; b1 = C; }
    else if (H < 300) { r1 = X; g1 = 0; b1 = C; }
    else              { r1 = C; g1 = 0; b1 = X; }

    return { clamp255(r1 + m), clamp255(g1 + m), clamp255(b1 + m) };
}

// ---------- RGB <-> XYZ (sRGB, D65) ----------
inline XYZ RGB_to_XYZ(const RGB& rgb) {
    double r = srgb_to_linear(rgb.r / 255.0);
    double g = srgb_to_linear(rgb.g / 255.0);
    double b = srgb_to_linear(rgb.b / 255.0);

    double X = 100.0 * (0.412453 * r + 0.357580 * g + 0.180423 * b);
    double Y = 100.0 * (0.212671 * r + 0.715160 * g + 0.072169 * b);
    double Z = 100.0 * (0.019334 * r + 0.119193 * g + 0.950227 * b);
    return { X, Y, Z };
}

inline std::pair<RGB, ConvertFlags> XYZ_to_RGB(const XYZ& xyz) {
    double r_lin =  3.2406 * (xyz.X / 100.0) + (-1.5372) * (xyz.Y / 100.0) + (-0.4986) * (xyz.Z / 100.0);
    double g_lin = -0.9689 * (xyz.X / 100.0) +  1.8758 * (xyz.Y / 100.0) +  0.0415  * (xyz.Z / 100.0);
    double b_lin =  0.0557 * (xyz.X / 100.0) + (-0.2040) * (xyz.Y / 100.0) +  1.0570 * (xyz.Z / 100.0);

    ConvertFlags f;
    constexpr double EPS = 1e-6;

    auto to8 = [&](double v_lin) -> int {
        if (v_lin < -EPS || v_lin > 1.0 + EPS) {
            f.outOfGamut = true;
        }
        // обрезаем и переводим в sRGB
        double v_srgb = linear_to_srgb(std::clamp(v_lin, 0.0, 1.0));
        return (int)std::round(v_srgb * 255.0);
    };

    return { RGB{ to8(r_lin), to8(g_lin), to8(b_lin) }, f };
}


// ---------- XYZ <-> Lab ----------
inline double f_lab(double t) {
    return (t >= 0.008856) ? std::cbrt(t) : (7.787 * t + 16.0 / 116.0);
}
inline double f_inv_lab(double t) {
    double t3 = t * t * t;
    return (t3 >= 0.008856) ? t3 : (t - 16.0 / 116.0) / 7.787;
}

inline Lab XYZ_to_Lab(const XYZ& xyz) {
    double xr = xyz.X / Xn, yr = xyz.Y / Yn, zr = xyz.Z / Zn;
    double fx = f_lab(xr), fy = f_lab(yr), fz = f_lab(zr);
    double L = 116.0 * fy - 16.0;
    double a = 500.0 * (fx - fy);
    double b = 200.0 * (fy - fz);
    return { L, a, b };
}

inline XYZ Lab_to_XYZ(const Lab& lab) {
    double fy = (lab.L + 16.0) / 116.0;
    double fx = fy + lab.a / 500.0;
    double fz = fy - lab.b / 200.0;
    double xr = f_inv_lab(fx);
    double yr = f_inv_lab(fy);
    double zr = f_inv_lab(fz);
    return { xr * Xn, yr * Yn, zr * Zn };
}

}
