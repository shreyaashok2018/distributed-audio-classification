# Feature Extraction

## 1. Overview

The system extracts a fixed-length, 28-dimensional feature vector from each audio recording window on the ESP32. The implementation uses the custom `ESP32Feature28` library and performs spectral analysis and Mel-frequency cepstral coefficient (MFCC) extraction on-device.

The purpose is to reduce the audio signal to a compact representation that can be transmitted to a second microcontroller for classification, avoiding the need to transfer the complete audio waveform.

## 2. Audio Input

The ESP32 reads digital audio from an INMP441 MEMS microphone using I²S.

| Parameter | Value |
|---|---|
| Nominal sampling rate | 16 kHz |
| Recording window | 2 seconds |
| Samples per window | 32,000 |
| Input buffer | 512 16-bit samples |
| Extractor | `Feature28Extractor` |

The firmware accumulates incoming samples and feeds them incrementally to the feature extractor. Once the recording window is complete, the extractor computes the feature vector and resets its accumulation state before the next window.

## 3. Feature Vector

The output contains two spectral features, followed by the means and standard deviations of 13 MFCC coefficients.

| Feature indices | Description | Number |
|---|---|---:|
| `F[0]` | Peak frequency (Hz) | 1 |
| `F[1]` | Spectral centroid (Hz) | 1 |
| `F[2]`–`F[14]` | MFCC coefficient means | 13 |
| `F[15]`–`F[27]` | MFCC coefficient standard deviations | 13 |
| **Total** | | **28** |

### 3.1 Peak frequency

The extractor accumulates power spectra from FFT blocks and uses the resulting spectrum to estimate a dominant peak frequency. The implementation uses a peak-selection procedure that considers candidate peaks within a configured frequency band.

Peak frequency provides information about the dominant spectral components of the recording.

### 3.2 Spectral centroid

The spectral centroid represents the power-weighted average frequency of the spectrum. It provides an indication of where spectral energy is concentrated.

A higher centroid generally corresponds to a spectrum with more energy at higher frequencies, although it does not uniquely identify a sound category.

### 3.3 Mel-frequency cepstral coefficients

MFCCs summarise the spectral envelope using a perceptually motivated frequency scale. In the implementation, each MFCC frame undergoes the following operations:

1. Apply a Hann window.
2. Compute the FFT.
3. Calculate the power spectrum.
4. Apply a triangular mel filterbank.
5. Compute log mel energies.
6. Apply a discrete cosine transform (DCT) to obtain MFCC coefficients.

The extractor accumulates the 13 coefficients across the frames in a recording window.

For coefficient \(k\), the mean is

\[
\mu_k=\frac{1}{N}\sum_{t=1}^{N}c_{t,k}
\]

and the standard deviation is

\[
\sigma_k=\sqrt{\frac{1}{N}\sum_{t=1}^{N}c_{t,k}^{2}-\mu_k^{2}}
\]

where \(c_{t,k}\) is coefficient \(k\) for frame \(t\), and \(N\) is the number of processed MFCC frames.

The resulting 26 statistics capture aspects of the average spectral structure and its variation over the recording window.

## 4. Feature Aggregation

The extractor maintains separate accumulators for FFT-based spectral analysis and MFCC statistics. It processes incoming samples incrementally, avoiding the need to store an entire two-second recording as a single waveform buffer.

The final feature vector is generated only when both spectral and MFCC processing have produced valid results.

The current firmware checks the return value of `compute()`. On success, it outputs the 28 features in a consistent order. On failure, it reports the error and resets the extractor.

## 5. Preprocessing for Classification

The extracted features are not passed directly to the neural network. During training, a scaler is fitted to the training data. The resulting means and scales are exported for use by the RP2040.

For each feature:

\[
z_i=\frac{x_i-\mu_i}{s_i}
\]

where \(x_i\) is the raw feature, \(\mu_i\) is its training-set mean, and \(s_i\) is its training-set scale.

The RP2040 applies this transformation before inference. The scaler must correspond to the model's training run; changing the dataset or retraining the model requires regenerating the preprocessing parameters.

The neural-network deployment additionally uses int8 quantisation for embedded inference.

## 6. Implementation Considerations

- Feature ordering must remain identical during collection, training, preprocessing export, and inference.
- The same feature-extraction implementation and parameters should be used to create training examples and process live audio.
- The microphone's sample representation must be validated against the ESP32's I²S configuration.
- Peak-frequency and MFCC parameters are defined in the `ESP32Feature28` library and should be documented alongside the library version used for an experiment.
- The 28 features are statistical summaries, not a lossless representation of the original waveform.

