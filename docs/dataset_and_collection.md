# Dataset and Data Collection

## 1. Dataset Overview

The dataset contains labelled feature vectors for four sound categories:

- `clap`
- `music`
- `noise`
- `speech`

Each example contains 28 numerical features extracted from a two-second audio recording window. The dataset is stored in `dataset/audio_features.csv` and is used to train and evaluate the classification models.

The dataset used in the reported experiments contains 799 examples, with ~200 examples per class.

| Property | Value |
|---|---|
| Total examples | 800 |
| Number of classes | 4 |
| Examples per class | 200 |
| Features per example | 28 |
| Recording window | 2 seconds |
| Nominal sampling rate | 16 kHz |
| Data format | CSV |

The CSV also contains a timestamp and a class label. These fields identify each example and its target category; they are not model input features.

## 2. Collection Hardware and Software

Audio features are collected using an INMP441 microphone connected to an ESP32.

The ESP32 firmware:

1. Reads audio through I²S.
2. Accumulates a two-second recording window.
3. Computes the 28 features using `ESP32Feature28`.
4. Prints the comma-separated feature vector over USB serial.

The Python collection utility, `python/collect_dataset.py`, is used to collect and label the feature vectors and save them to CSV.

The collection workflow stores extracted features rather than raw audio recordings.

## 3. Recording Conditions

Recordings were collected in the same location across multiple sessions.

The sound categories were varied as follows:

| Class | Collection variation |
|---|---|
| Clap | Claps produced at different loudness levels and with different characteristics |
| Music | Multiple types of music, including music in different languages |
| Noise | Recordings labelled as noise |
| Speech | Speech produced at different loudness levels and with different characteristics |

Music type and language are relevant contextual variables because they introduce variation in the acoustic content of the music class. Similarly, varying speech and clap loudness helps test whether the extracted features remain useful across different recording intensities.

These variations increase the diversity of examples, but they do not by themselves establish generalisation to unseen speakers, sound sources, rooms, or recording devices.

## 4. Dataset Schema

The CSV contains the following columns:

- `timestamp`
- `label`
- `peak_hz`
- `centroid_hz`
- `mfcc_mean_1` through `mfcc_mean_13`
- `mfcc_std_1` through `mfcc_std_13`

The 28 feature columns correspond to the output ordering defined by `Feature28Extractor`.

For model training, `label` is the target variable. The timestamp is metadata and should not be included as a predictive feature.

## 5. Data Preparation

The dataset is loaded into a Pandas DataFrame. The feature matrix is constructed from the 28 numerical feature columns, and the labels are extracted separately.

Before training, the data should be checked for:

- Missing or non-numeric feature values.
- Unexpected class labels.
- Duplicate rows or repeated recordings.
- Incorrect feature ordering.
- Inconsistent extraction settings between collection sessions.

For the neural network, feature standardisation is fitted using the training partition and then applied to the test partition. The fitted parameters are exported for embedded inference.

The Random Forest baseline is trained on the numerical feature matrix without requiring standardisation.

## 6. Train/Test Evaluation

The reported model evaluation uses 639 examples for training and 160 for testing, with 40 test examples per class.

This is an 80/20 split of the 800-example dataset.

## 7. Reproducibility

For reproducible experiments, record:

- The version of the ESP32 feature-extraction library.
- The microphone and I²S configuration.
- The collection session and label for each example, where available.
- The dataset version used for each model.
- The train/test split and random seed.
- The scaler parameters and class ordering associated with each neural-network model.

Future dataset versions should retain the original feature schema or explicitly document any changes.

