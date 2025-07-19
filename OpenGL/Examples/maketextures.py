import numpy as np
from PIL import Image
import random

class Config:
    def __init__(self,
                 width=800,
                 height=800,
                 iterations=5_000_000,
                 transforms=35,
                 zoom=1.1,
                 skip=100,
                 gamma=2.2):
        self.width = width
        self.height = height
        self.iterations = iterations
        self.transforms = transforms
        self.zoom = zoom
        self.skip = skip
        self.gamma = gamma

class FlameFractal:
    def __init__(self, config: Config):
        self.config = config
        self.VARIATIONS = [
            self.variation_linear,
            self.variation_sinusoidal,
            self.variation_spherical,
            self.variation_swirl,
        ]

    @staticmethod
    def variation_linear(x, y):
        return x, y

    @staticmethod
    def variation_sinusoidal(x, y):
        return np.sin(x), np.sin(y)

    @staticmethod
    def variation_spherical(x, y):
        r2 = x * x + y * y + 1e-6
        return x / r2, y / r2

    @staticmethod
    def variation_swirl(x, y):
        r2 = x * x + y * y
        sinr2 = np.sin(r2)
        cosr2 = np.cos(r2)
        return x * sinr2 - y * cosr2, x * cosr2 + y * sinr2

    @staticmethod
    def random_color():
        return np.array([random.random(), random.random(), random.random()])

    def generate_transform(self):
        a, b, c = np.random.uniform(-1, 1, 3)
        d, e, f = np.random.uniform(-1, 1, 3)
        variation = random.choice(self.VARIATIONS)
        weight = np.random.uniform(0.5, 1.5)
        color = self.random_color()
        return (a, b, c, d, e, f, variation, weight, color)

    def generate(self):
        cfg = self.config
        density = np.zeros((cfg.height, cfg.width), dtype=np.float64)
        color_accum = np.zeros((cfg.height, cfg.width, 3), dtype=np.float64)

        transforms = [self.generate_transform() for _ in range(cfg.transforms)]

        x, y = 0.0, 0.0

        for i in range(cfg.iterations):
            t = random.choice(transforms)
            a, b, c, d, e, f, var_func, weight, t_color = t

            # Affine transform
            x1 = a * x + b * y + c
            y1 = d * x + e * y + f

            # Variation
            x2, y2 = var_func(x1, y1)

            x, y = x2 * weight, y2 * weight

            # Convert to image space
            ix = int((x * cfg.zoom + 1) * cfg.width / 2)
            iy = int((y * cfg.zoom + 1) * cfg.height / 2)

            if 0 <= ix < cfg.width and 0 <= iy < cfg.height:
                if i > cfg.skip:
                    density[iy, ix] += 1
                    color_accum[iy, ix] += t_color

        max_density = np.max(density)
        safe_density = np.log(density + 1) / np.log(max_density + 1 + 1e-6)

        with np.errstate(invalid='ignore'):
            norm_color = np.where(
                density[..., None] > 0,
                color_accum / (density[..., None] + 1e-6),
                0
            )

        # Apply gamma correction
        norm_color = np.clip(norm_color, 0, 1)
        norm_color = norm_color ** (1 / cfg.gamma)

        rgb = (norm_color * safe_density[..., None]) * 255
        rgb = np.clip(rgb, 0, 255).astype(np.uint8)

        return rgb


def main():
    fixed_width = 800
    fixed_height = 800
    fixed_skip = 100

    # List of parameter sets to vary
    param_sets = [
        {"iterations": 1_000_000, "transforms": 20, "zoom": 1.0, "gamma": 2.2},
        {"iterations": 2_000_000, "transforms": 30, "zoom": 1.2, "gamma": 2.0},
        {"iterations": 5_000_000, "transforms": 35, "zoom": 1.1, "gamma": 2.5},
        {"iterations": 3_000_000, "transforms": 40, "zoom": 0.9, "gamma": 1.8},
        {"iterations": 4_000_000, "transforms": 45, "zoom": 1.5, "gamma": 2.3},
        {"iterations": 6_000_000, "transforms": 50, "zoom": 1.3, "gamma": 2.1},
    ]

    for i, params in enumerate(param_sets):
        config = Config(
            width=fixed_width,
            height=fixed_height,
            skip=fixed_skip,
            iterations=params["iterations"],
            transforms=params["transforms"],
            zoom=params["zoom"],
            gamma=params["gamma"]
        )
        fractal = FlameFractal(config)
        texture = fractal.generate()
        filename = f"flame_texture_var{i+1}.png"
        Image.fromarray(texture, mode='RGB').save(filename)
        print(f"Saved {filename} with params: {params}")

if __name__ == "__main__":
    main()