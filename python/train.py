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

# creating train and test datasets

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

print("Class order:", encoder.classes_)

# 3. Building a small neural network.
model = tf.keras.Sequential([
    tf.keras.layers.Input(shape=(28,)),
    tf.keras.layers.Dense(32, activation="relu"),
    tf.keras.layers.Dense(16, activation="relu"),
    tf.keras.layers.Dense(4, activation="softmax")
])

model.compile(
    optimizer="adam",
    loss="sparse_categorical_crossentropy",
    metrics=["accuracy"]
)

# 4. Train, retaining the best validation-performing version.
early_stop = tf.keras.callbacks.EarlyStopping(
    monitor="val_loss",
    patience=10,
    restore_best_weights=True
)

model.fit(
    X_train_scaled,
    y_train_encoded,
    epochs=100,
    batch_size=16,
    validation_split=0.2,
    callbacks=[early_stop],
    verbose=1
)

# 5. Evaluate on the same test split as the Random Forest.
probabilities = model.predict(X_test_scaled, verbose=0)
predictions = np.argmax(probabilities, axis=1)

print("Neural network accuracy:",
      accuracy_score(y_test_encoded, predictions))
print(classification_report(
    y_test_encoded,
    predictions,
    target_names=encoder.classes_
))

# 6. Save the trained model and preprocessing parameters.
model.save("sound_classifier.keras")
np.savez(
    "sound_preprocessing.npz",
    mean=scaler.mean_,
    scale=scaler.scale_,
    classes=encoder.classes_
)


# Convert the trained Keras model to TensorFlow Lite
converter = tf.lite.TFLiteConverter.from_keras_model(model)
tflite_model = converter.convert()

with open("sound_classifier.tflite", "wb") as f:
    f.write(tflite_model)

print("TFLite model size:", len(tflite_model), "bytes")

# Quantising tensorflow lite to int8
converter.optimizations = [tf.lite.Optimize.DEFAULT]

def representative_dataset():
    for i in range(min(200, len(X_train_scaled))):
        yield [X_train_scaled[i:i+1].astype(np.float32)]
converter.representative_dataset = representative_dataset
converter.target_spec.supported_ops = [
    tf.lite.OpsSet.TFLITE_BUILTINS_INT8
]
converter.inference_input_type = tf.int8
converter.inference_output_type = tf.int8

quantized_model = converter.convert()

with open("sound_classifier_int8.tflite", "wb") as f:
    f.write(quantized_model)      