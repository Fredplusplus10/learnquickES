# Third-party components of the LearnQuick voice

## Supertonic 2

- Source: <https://huggingface.co/Supertone/supertonic-2>, commit `75e6727618a02f323c720cba9478152d4bc16ca4`. Copyright (c) 2026 Supertone Inc.
- The model (the four `.onnx` files, `tts.json`, `unicode_indexer.json` and the voice styles `F1`, `F3`, `M1`, `M2`, `M4`, `M5`) is distributed unmodified under the BigScience Open RAIL-M License. Its full text is `LICENSE`, next to this file.
- The model is subject to the use-based restrictions in paragraph 5 and Attachment A of that license, and anyone who uses it through LearnQuick must comply with them.
- The Spanish audio LearnQuick plays is machine generated: by this model, or by the operating system's voices when it cannot run.
- LearnQuick runs the model with its own C++ code, which follows Supertone's MIT-licensed Python package (`supertonic` 1.3.1).

## ONNX Runtime 1.23.2

- Source: <https://github.com/microsoft/onnxruntime>, Copyright (c) Microsoft Corporation, MIT License.
- Its license (`LICENSE`) and the notices of the components it includes (`ThirdPartyNotices.txt`) are in the `onnxruntime` folder that ships with the app.

## Pronunciation

Before the voice reads a phrase, the app writes numbers as words and respells names a Spanish voice misreads ("Ohio" → "Ojáio"), with its own rules.
