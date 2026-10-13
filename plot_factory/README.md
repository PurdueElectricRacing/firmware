## Plot Factory
Automations for generating plots from raw data.

```python
python3 plot_factory/run_plot_factory.py
```

The default data folder is `plot_factory/data/`. You can select another folder with
`python3 plot_factory/run_plot_factory.py /path/to/data`.

The output folder is `plot_factory/output/`.

Plot configs are discovered recursively under `plot_factory/configs/`, grouped
into `thermals/`, `vehicle_dynamics/`, `powertrain/`, and `sensors/`.
Output folders mirror the config folders: `configs/sensors/shockpots_vs_time.toml`
generates `output/sensors/shockpots_vs_time.png`. Additional nested folders are
supported, and root-level configs still write directly to `output/`.

`configs/derived_signals/` contains shared signal definitions and is excluded
from plot discovery. Existing output files are not automatically removed when
configs move.
