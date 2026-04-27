# GeoTaskShield Batch Experiment Report

| Scenario | Workers | Tasks | Privacy | Algorithm | Completion | Avg Distance | Reward | Privacy Loss | Fairness | Timeout |
|---|---:|---:|---|---|---:|---:|---:|---:|---:|---:|
| workers-50-grid-nearest | 50 | 20 | Grid Privacy | Nearest Greedy | 100.00% | 9.51 | 1154.38 | 3.94 | 0.29 | 0.00% |
| workers-100-grid-nearest | 100 | 50 | Grid Privacy | Nearest Greedy | 100.00% | 6.80 | 2953.94 | 3.65 | 0.35 | 0.00% |
| epsilon-0.5-laplace-score | 100 | 50 | Laplace Noise Privacy | Score Greedy | 100.00% | 6.54 | 2953.94 | 3.30 | 0.37 | 0.00% |
| epsilon-2.0-laplace-score | 100 | 50 | Laplace Noise Privacy | Score Greedy | 100.00% | 5.43 | 2953.94 | 0.82 | 0.43 | 0.00% |
| k-3-hungarian | 100 | 50 | K-Anonymity Privacy | Hungarian | 100.00% | 7.12 | 2953.94 | 3.23 | 0.39 | 0.00% |
| k-8-hungarian | 100 | 50 | K-Anonymity Privacy | Hungarian | 100.00% | 7.07 | 2953.94 | 4.38 | 0.42 | 0.00% |
| grid-5-score | 100 | 50 | Grid Privacy | Score Greedy | 100.00% | 5.70 | 2953.94 | 1.90 | 0.38 | 0.00% |
| grid-20-score | 100 | 50 | Grid Privacy | Score Greedy | 100.00% | 11.41 | 2953.94 | 7.66 | 0.27 | 0.00% |

## Summary

Best privacy-utility ratio: epsilon-2.0-laplace-score (Laplace Noise Privacy + Score Greedy) at 0.55.
