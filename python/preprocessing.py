import numpy as np

data = np.load("sound_preprocessing.npz")
mean = data["mean"]
scale = data["scale"]

assert len(mean) == 28
assert len(scale) == 28
assert np.all(scale > 0)

with open("preprocessing.h", "w") as f:
    f.write("#pragma once\n\n")
    f.write("const float feature_mean[28] = {\n")
    f.write(",\n".join(f"    {x:.9g}f" for x in mean))
    f.write("\n};\n\n")

    f.write("const float feature_scale[28] = {\n")
    f.write(",\n".join(f"    {x:.9g}f" for x in scale))
    f.write("\n};\n")

print("Created preprocessing.h")
