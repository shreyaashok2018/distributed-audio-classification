# Experiments and Results

## 1. Objective

The experiments evaluate whether a compact set of audio features extracted on an ESP32 can support four-class audio classification, and whether the resulting neural network can be deployed using int8 TensorFlow Lite inference on an RP2040.

The target classes are `clap`, `music`, `noise`, and `speech`.

Two model approaches were evaluated:

1. Random Forest as a classical machine-learning baseline.
2. A neural network converted to TensorFlow Lite and quantised for embedded inference.

## 2. Experimental Setup

### Dataset

| Parameter | Value |
|---|---|
| Total examples | 800 |
| Classes | 4 |
| Examples per class | 200 |
| Features per example | 28 |
| Training examples | 640 |
| Test examples | 160 |
| Test support per class | 40 |

The examples were collected in the same location over multiple sessions. Music examples included different types of music and languages. Clapping and speech examples varied in loudness and other recording characteristics.

The available evaluation uses 160 test examples, with equal support across all four classes.

### Models

**Random Forest**

A Random Forest classifier was trained on the extracted feature vectors. It provides a non-neural baseline for comparing the discriminative value of the 28-dimensional representation.

**Neural network**

A neural network was trained using standardised features. The model was then converted to TensorFlow Lite and quantised to int8 for embedded deployment.

The reported neural-network results include the floating-point Keras model and the quantised TFLite model.

## 3. Results

### 3.1 Overall accuracy

| Model | Test accuracy |
|---|---:|
| Random Forest | 88.13% |
| Keras neural network | 83.75% |
| Quantised TFLite neural network | 85.00% |

The Random Forest achieved the highest accuracy among the three reported models. The quantised TFLite model scored 1.25 percentage points higher than the floating-point Keras model on this test set.

The small difference between the Keras and quantised results should not be interpreted as evidence that quantisation improves generalisation. It reflects performance on this particular evaluation split.

### 3.2 Random Forest

| Class | Precision | Recall | F1-score |
|---|---:|---:|---:|
| Clap | 0.88 | 0.72 | 0.79 |
| Music | 1.00 | 0.85 | 0.92 |
| Noise | 0.72 | 0.95 | 0.82 |
| Speech | 1.00 | 1.00 | 1.00 |
| **Macro average** | **0.90** | **0.88** | **0.88** |

- Accuracy: **88.13%**
- Test examples: 160

Speech was classified correctly across the reported test examples. Music also had high precision and recall. Noise had high recall but lower precision, indicating that some examples predicted as noise belonged to other classes. Clap recall was lower than its precision, indicating that some claps were missed.

### 3.3 Keras neural network

| Class | Precision | Recall | F1-score |
|---|---:|---:|---:|
| Clap | 0.75 | 0.60 | 0.67 |
| Music | 1.00 | 0.90 | 0.95 |
| Noise | 0.65 | 0.85 | 0.74 |
| Speech | 1.00 | 1.00 | 1.00 |
| **Macro average** | **0.85** | **0.84** | **0.84** |

- Accuracy: **83.75%**
- Test examples: 160

The neural network performed strongly on music and speech, while clap and noise remained more difficult to distinguish.

### 3.4 Quantised TensorFlow Lite model

| Class | Precision | Recall | F1-score |
|---|---:|---:|---:|
| Clap | 0.76 | 0.65 | 0.70 |
| Music | 1.00 | 0.90 | 0.95 |
| Noise | 0.68 | 0.85 | 0.76 |
| Speech | 1.00 | 1.00 | 1.00 |
| **Macro average** | **0.86** | **0.85** | **0.85** |

- Accuracy: **85.00%**
- Test examples: 160

The quantised model retained comparable performance to the Keras model on the reported split. The results support the feasibility of using a compact quantised neural network for the embedded inference stage, subject to further validation.

## 4. Embedded Inference

The int8 model was integrated into the RP2040 firmware using the `MicroTFLite` library. The RP2040 receives the 28 features over UART, standardises them using the exported training parameters, and runs inference.

Earlier firmware diagnostics reported inference execution times of approximately 0.8–0.9 ms per feature vector. These measurements are preliminary and should be repeated with diagnostic printing reduced or disabled, using the final deployed firmware.

The exported int8 model was approximately 8.2 kB. The RP2040 firmware allocates a 16 KiB tensor arena.

Desktop test accuracy and embedded inference latency measure different aspects of the system: the former evaluates predictive performance on a test set, while the latter measures execution time on the target hardware.

## 5. Interpretation

The reported results suggest that the extracted features contain useful information for distinguishing the four sound categories.

Three observations stand out:

1. **Speech was the strongest class** across all three reported models, with precision, recall, and F1-score of 1.00 on the test split.
2. **Music was also classified reliably**, with an F1-score of 0.92 for Random Forest and 0.95 for both neural-network variants.
3. **Clap and noise remain the main areas for improvement**, with lower recall or precision depending on the model.

The Random Forest performed better than the floating-point neural network on overall accuracy in this experiment. This suggests that a classical model is a useful baseline and should remain under consideration when comparing accuracy, model size, memory requirements, and inference cost.

## 6. Limitations

These results should be interpreted in the context of the current dataset and test procedure.

- The dataset is relatively small, with 200 examples per class.
- Recordings were collected in the same location, so performance across different rooms and acoustic environments has not been established.
- The dataset includes variation in music type, language, and sound loudness, but does not establish coverage of all speakers, sources, or background conditions.
- If related recording windows from the same session occur in both training and test partitions, the evaluation may be optimistic.
- A single held-out test split does not fully characterise performance variability.
- The reported results do not establish robustness to microphone variation, changing microphone distance, reverberation, or simultaneous sound sources.

## 7. Future Experiments

The next experiments should prioritise:

1. **Session-held-out evaluation:** Train on some collection sessions and evaluate on completely separate sessions.
2. **Environmental generalisation:** Collect recordings in additional rooms and acoustic conditions.
3. **Confusion analysis:** Inspect which classes are confused, especially clap versus noise.
4. **Feature validation:** Compare feature distributions between collection sessions and check that live features match the training pipeline.
5. **Deployment validation:** Compare desktop TFLite predictions with RP2040 predictions on identical feature vectors.
6. **Resource profiling:** Measure inference latency, peak memory use, and power consumption on the final hardware.
7. **Adaptive sampling:** Once the classification pipeline is stable, implement and evaluate a feedback mechanism that changes the ESP32 sampling behaviour based on system state.

## 8. Conclusion

The current prototype demonstrates a complete feature-based audio-classification workflow, from microphone acquisition and on-device feature extraction to quantised inference on a second microcontroller.

The best reported test accuracy is 88.13% using Random Forest. The quantised neural network achieved 85.00% test accuracy and has been integrated into the embedded inference workflow.

Further work is needed to establish generalisation across independent recording sessions and environments, validate the live acquisition path, and complete the planned adaptive-sampling functionality.

