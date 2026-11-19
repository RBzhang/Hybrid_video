#!/usr/bin/env python3
"""Host-side *theoretical* frequency table; does not connect to the board."""
from fractions import Fraction

FRACN = (0, 393, 786, 1180, 1573, 1966, 2359, 2753, 3146, 3539)

print("tag | FRACN | target_Hz | nominal_Hz | quantization_error_Hz")
for tag, fracn in enumerate(FRACN):
    target = 10_000_000 + 24_000 * tag
    nominal = Fraction(10_000_000) + Fraction(500_000 * fracn, 8192)
    print(
        f"{tag:3d} | {fracn:5d} | {target:9d} | "
        f"{float(nominal):14.3f} | {float(nominal - target):+9.3f}"
    )
assert all(0 <= fracn <= 8191 for fracn in FRACN)
assert max(abs(Fraction(10_000_000) + Fraction(500_000 * n, 8192) -
               (10_000_000 + i * 24_000))
           for i, n in enumerate(FRACN)) <= Fraction(500_000, 2 * 8192)
