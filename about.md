# dashback

dashback automatically solves Geometry Dash levels using a variety of
interchangeable algorithms, and logs per-attempt metrics (runtime, deaths,
success rate) so the algorithms can be compared.

Open a level and the mod takes over, retrying with the selected algorithm until
it solves the level or hits the attempt cap. Results are written to `metrics.csv`
in the mod's save directory.

See the [README](https://github.com/rhelgason/dashback) for the architecture and
how to add your own algorithm.
