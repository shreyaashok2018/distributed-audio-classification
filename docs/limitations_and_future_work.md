## Current Implementation and Future Extension

### Implemented components

- I²S audio acquisition on the ESP32.
- Fixed-duration recording windows.
- 28-feature extraction.
- UART feature-vector transmission.
- RP2040 UART parsing and standardisation.
- Quantised neural-network inference.
- Classification output and execution-time diagnostics.
- Python-based dataset collection and model evaluation.

### Planned work: adaptive sampling

The intended extension is to allow the RP2040 to influence the ESP32's sampling behaviour based on classification results or other signal-level indicators.

This requires a control message from the RP2040 to the ESP32, a defined sampling-rate control protocol, and corresponding changes to the ESP32 acquisition configuration. This feedback loop is not implemented in the current firmware.

## Known Limitations

- The microphone's I²S sample format requires validation against the configured 16-bit read path.
- Feature distributions and preprocessing consistency require further verification.
- Classification performance must be validated on independently recorded audio.
- End-to-end latency, power consumption, and long-duration stability have not yet been fully characterised.
- Adaptive sampling remains future work.