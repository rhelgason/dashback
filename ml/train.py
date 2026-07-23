#!/usr/bin/env python3
"""Train an imitation-learning policy from dashback trajectory CSVs and export it
in the weight format the in-game `policy` algorithm loads.

Usage:
    python train.py --data data --out model.txt [--hidden 32] [--epochs 40]

Then copy model.txt to <mod save dir>/policy/model.txt and select the `policy`
algorithm in-game. See README.md.
"""
import argparse
import glob
import os
import sys

import numpy as np

# Non-input metadata columns present in the CSV besides the feature columns.
META_COLS = {"action", "solved", "session", "level_id"}


def load_data(data_dir):
    files = sorted(glob.glob(os.path.join(data_dir, "*.csv")))
    if not files:
        sys.exit(f"no CSVs found in {data_dir!r} — record some runs first (see README)")

    feature_cols = None
    X, y = [], []
    for path in files:
        with open(path) as fh:
            header = fh.readline().strip().split(",")
        cols = [c for c in header if c not in META_COLS]
        if feature_cols is None:
            feature_cols = cols
        elif cols != feature_cols:
            sys.exit(f"feature columns in {path} don't match {files[0]} — retrain per schema")
        arr = np.genfromtxt(path, delimiter=",", names=True)
        feats = np.stack([arr[c] for c in feature_cols], axis=1).astype(np.float32)
        X.append(feats)
        y.append(arr["action"].astype(np.float32))
    return np.concatenate(X), np.concatenate(y), feature_cols


def train(X, y, hidden, epochs, lr=1e-3, seed=0):
    """Tiny 2-hidden-layer MLP trained with numpy (no torch dependency)."""
    rng = np.random.default_rng(seed)
    n, f = X.shape

    def he(shape):
        return rng.standard_normal(shape).astype(np.float32) * np.sqrt(2.0 / shape[1])

    W1, b1 = he((hidden, f)), np.zeros(hidden, np.float32)
    W2, b2 = he((hidden, hidden)), np.zeros(hidden, np.float32)
    W3, b3 = he((1, hidden)), np.zeros(1, np.float32)

    # class weighting (holds are usually the minority)
    pos = max(1.0, float(y.sum()))
    w_pos = (len(y) - pos) / pos

    for ep in range(epochs):
        idx = rng.permutation(n)
        for s in range(0, n, 512):
            b = idx[s:s + 512]
            xb, yb = X[b], y[b]
            z1 = xb @ W1.T + b1; a1 = np.maximum(z1, 0)
            z2 = a1 @ W2.T + b2; a2 = np.maximum(z2, 0)
            z3 = a2 @ W3.T + b3; p = 1 / (1 + np.exp(-z3[:, 0]))
            wt = np.where(yb > 0.5, w_pos, 1.0)
            g = ((p - yb) * wt / len(b))[:, None]
            gW3 = g.T @ a2; gb3 = g.sum(0)
            d2 = (g @ W3) * (z2 > 0)
            gW2 = d2.T @ a1; gb2 = d2.sum(0)
            d1 = (d2 @ W2) * (z1 > 0)
            gW1 = d1.T @ xb; gb1 = d1.sum(0)
            for p_, gp in ((W1, gW1), (b1, gb1), (W2, gW2), (b2, gb2), (W3, gW3), (b3, gb3)):
                p_ -= lr * gp
        z1 = np.maximum(X @ W1.T + b1, 0)
        z2 = np.maximum(z1 @ W2.T + b2, 0)
        p = 1 / (1 + np.exp(-(z2 @ W3.T + b3)[:, 0]))
        acc = ((p > 0.5) == (y > 0.5)).mean()
        print(f"epoch {ep + 1}/{epochs}  train_acc={acc:.3f}")
    return W1, b1, W2, b2, W3[0], b3[0]


def export(path, W1, b1, W2, b2, W3, b3):
    f, h = W1.shape[1], W1.shape[0]
    with open(path, "w") as out:
        out.write("DASHBACK_MLP 1\n")
        out.write(f"{f} {h}\n")
        for arr in (W1.ravel(), b1, W2.ravel(), b2, W3):
            out.write(" ".join(f"{v:.6f}" for v in arr) + "\n")
        out.write(f"{b3:.6f}\n")
    print(f"wrote {path}  (F={f}, H={h})")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--data", default="data")
    ap.add_argument("--out", default="model.txt")
    ap.add_argument("--hidden", type=int, default=32)
    ap.add_argument("--epochs", type=int, default=40)
    args = ap.parse_args()

    X, y, cols = load_data(args.data)
    print(f"loaded {len(X)} frames, {len(cols)} features, {int(y.sum())} holds")
    export(args.out, *train(X, y, args.hidden, args.epochs))


if __name__ == "__main__":
    main()
