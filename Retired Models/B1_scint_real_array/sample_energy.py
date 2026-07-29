import numpy as np
from scipy.integrate import cumulative_trapezoid

def flux(E, a, b):
    return ((1.27 + a) * 1e4) * (E ** (-2.75 + b))

E_points = np.linspace(20, 350, 1000)

# random parameters
a = np.random.uniform(-0.09, 0.09)
b = np.random.uniform(-0.02, 0.02)

flux_values = flux(E_points, a, b)
flux_integral = np.trapz(flux_values, E_points)
pdf = flux_values / flux_integral
cdf = cumulative_trapezoid(pdf, E_points, initial=0)

u = np.random.rand()
E = round(np.interp(u, cdf, E_points), 6)

print(E)
