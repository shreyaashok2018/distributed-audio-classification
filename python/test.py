
import pandas as pd
file = "audio_features.csv"
df = pd.read_csv(file)

# Remove columns that aren't model inputs
X = df.drop(columns=["timestamp", "label"])

# The answer the model should learn to predict
y = df["label"]

print("Features:", X.shape)
print("Labels:", y.shape)
print("Feature columns:", X.columns.tolist())

from sklearn.model_selection import train_test_split

X_train, X_test, y_train, y_test = train_test_split(
    X,
    y,
    test_size=0.2,
    random_state=42,
    stratify=y
)

print("Training:", X_train.shape)
print("Testing:", X_test.shape)

import numpy as np
import tensorflow as tf
from sklearn.preprocessing import StandardScaler, LabelEncoder
from sklearn.metrics import accuracy_score, classification_report

# 1. Normalize features so their scales are comparable.
scaler = StandardScaler()
X_train_scaled = scaler.fit_transform(X_train).astype(np.float32)
X_test_scaled = scaler.transform(X_test).astype(np.float32)

# 2. Convert text labels into integer class IDs.
encoder = LabelEncoder()
y_train_encoded = encoder.fit_transform(y_train)
y_test_encoded = encoder.transform(y_test)

import numpy as np
import tensorflow as tf
from sklearn.metrics import accuracy_score, classification_report

interpreter = tf.lite.Interpreter(
    model_path="sound_classifier_int8.tflite"
)
interpreter.allocate_tensors()

input_info = interpreter.get_input_details()[0]
output_info = interpreter.get_output_details()[0]

scale, zero_point = input_info["quantization"]

# Quantise the standardised test features to int8.
X_test_int8 = np.clip(
    np.round(X_test_scaled / scale + zero_point),
    -128, 127
).astype(np.int8)

predictions = []

for sample in X_test_int8:
    interpreter.set_tensor(
        input_info["index"], sample.reshape(1, 28)
    )
    interpreter.invoke()

    output = interpreter.get_tensor(output_info["index"])
    predictions.append(np.argmax(output[0]))

print("Quantised TFLite accuracy:",
      accuracy_score(y_test_encoded, predictions))

print(classification_report(
    y_test_encoded,
    predictions,
    target_names=encoder.classes_
))
