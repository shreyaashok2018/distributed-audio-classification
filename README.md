# Distributed Audio Classification on Resource-Constrained Devices

An embedded TinyML project exploring distributed audio sensing and classification using an ESP32, an INMP441 MEMS microphone, and an RP2040-based shrike lite microcontroller.

## Overview

This project implements an edge audio-classification pipeline in which audio acquisition and feature extraction run on an ESP32, while a separate RP2040 performs inference using a lightweight, quantised neural network.

The system is designed to explore distributed processing across resource-constrained microcontrollers, reducing the need to transmit raw audio to a more powerful computer for every classification.

Four audio categories are considered:
- Clap
- Music
- Noise
- Speech

## System Architecture

**Audio input → Feature extraction → UART communication → Embedded inference**

1. **ESP32 — Audio acquisition:** Captures audio from an INMP441 I²S MEMS microphone at a nominal sampling rate of 16 kHz.
2. **Feature extraction:** Computes 28 features: peak frequency, spectral centroid, 13 MFCC means, and 13 MFCC standard deviations.
3. **UART communication:** Transmits the extracted feature vector to the RP2040.
4. **RP2040 — Classification:** Runs an int8-quantised TensorFlow Lite model and produces class scores.
5. **Python tooling:** Supports dataset collection, model training, quantisation, evaluation, and preprocessing export.

## Repository Structure

- `esp32/`: Microphone acquisition and feature extraction firmware.
- `rp2040/`: Embedded inference firmware.
- `python/`: Dataset collection, model training, evaluation, and preprocessing scripts.
- `dataset/`: Extracted feature dataset.
- `models/`: Trained models and preprocessing artefacts.
- `headers/`: Model and preprocessing headers for embedded deployment.
- `docs/`: Detailed architecture, experimental methodology, results, and limitations.

## Models and Preliminary Results

The models were evaluated on a test split of 160 samples, with 40 samples per class.

| Model | Test accuracy |
|---|---:|
| Random Forest | 88.13% |
| Keras neural network | 83.75% |
| Quantised TFLite neural network | 85.00% |

The Random Forest achieved the highest accuracy in this evaluation. The quantised neural network performed slightly better than its floating-point counterpart on the same test split.

These results are preliminary and should be interpreted in the context of the dataset, feature-extraction pipeline, and evaluation methodology. Generalisation to independently recorded audio remains to be established.

See [`docs/results.md`](docs/results.md) for per-class metrics and [`docs/experiments.md`](docs/experiments.md) for the evaluation workflow.

## Reproducing the Experiments

1. Install the Python dependencies required by the scripts.
2. Collect labelled feature samples using `python/collect_dataset.py`.
3. Train and evaluate the Random Forest using `python/train.ipynb`.
4. Train the neural network and generate the quantised model using `python/train.py`.
5. Validate quantised inference using `python/test.py`.
6. Export preprocessing parameters using `python/preprocessing.py`.
7. Use the generated headers with the ESP32 and RP2040 firmware.

Refer to the individual scripts for their exact dependencies, configuration, and input/output formats.

## Current Status

The prototype includes microphone acquisition, feature extraction, inter-device communication, dataset collection, model training, and an embedded inference implementation.

Current work focuses on validating the consistency of audio capture and feature extraction, improving the robustness of classification, and establishing reliable performance on recordings not represented in the training data.

## Future Work

- Improve dataset diversity and recording-session-aware evaluation.
- Investigate model optimisation for constrained hardware.
- Implement adaptive sampling, allowing the inference device to influence the audio acquisition rate.
- Evaluate end-to-end latency, memory use, power consumption, and classification robustness.

## Acknowledgements

Developed as an exploration of embedded systems, signal processing, and TinyML.
