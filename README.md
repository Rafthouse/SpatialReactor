# Spatial Reactor

![Version](https://img.shields.io/badge/version-1.0.0-blue)
![JUCE](https://img.shields.io/badge/JUCE-8.0.13-green)
![VST3](https://img.shields.io/badge/format-VST3-orange)
![License](https://img.shields.io/badge/license-proprietary-lightgrey)

**Psychoacoustic spatial width processor** — аудіоплагін для розширення стереобази з інтелектуальним аналізом джерела.

---

## ⬇️ Скачати

► [**Spatial Reactor v1.0.0 (VST3, x64)**](https://github.com/Rafthouse/SpatialReactor/releases/download/v1.0.0/SpatialReactor_v1.0.0.zip)

Розпакуйте, папку `Spatial Reactor.vst3` скопіюйте в:
`C:\Users\%USERNAME%\AppData\Local\Programs\Common\VST3\`

```bash
# 1. Клонуй репозиторій з JUCE
git clone --recursive https://github.com/Rafthouse/SpatialReactor.git

# 2. Збери CMake
cmake -B Build -G "Visual Studio 17 2022" -A x64 -DJUCE_DIR=path/to/JUCE
cmake --build Build --config Release --target SpatialReactor_VST3
```

Готовий `.vst3` з'явиться в `Build/SpatialReactor_artefacts/Release/VST3/`.

Скопіюй папку `Spatial Reactor.vst3` в:
```
C:\Users\%USERNAME%\AppData\Local\Programs\Common\VST3\
```

---

## 🎛️ Огляд

**Spatial Reactor** — це не просто ще один widening-плагін. Він аналізує вхідний сигнал у реальному часі та адаптує обробку під тип джерела:

| Профіль | Опис |
|---|---|
| **Auto** | Автоматичне визначення типу сигналу |
| **Vocal** | Обережне розширення, акцент на присутності |
| **Instrument** | Помірна ширина, збереження атаки |
| **Master** | Прозоре розширення для готового міксу |
| **Ambient** | Агресивне розширення для фонових текстур |

### Основний ланцюг обробки

```
L/R in → M/S Matrix → Source Classifier → MacroMapper
         ↓                    ↓                ↓
         Width (Decorrelator) ← Width Target ←─┘
         Texture (Silk/Dust/Rust) ← Texture Target
         Air (Exciter + EQ) ← Air Target
         Center Lock (опціонально)
         Correlation Governor
         M/S Inverse → L/R out
```

---

## 🎨 Дизайн

**Ukrainian Bauhaus Industrial** — панель у стилі київського НДІ акустики 1970-х, де Bauhaus переміг маркетологів.

| Елемент | Деталі |
|---|---|
| Панель | 700×960, темний графіт #2C2E31 |
| Акцент | Кобальтовий синій #2D6BFF |
| Шрифти | Inter (UI), Pirata One (підпис) |
| Ручки | Матові кола, 72 насічки, без fake-3D |
| Reactor Core | Концентричні кола + конструктивістські промені |
| Correlation | 20-сегментний LED-бар з кольоровими зонами |
| Texture | Silk / Dust / Rust — три режими сатурації |

---

## 🔧 Технічні деталі

- **Фреймворк:** JUCE 8.0.13
- **Формат:** VST3 (x64)
- **DSP:** M/S обробка, гібридний декорелятор, Texture Engine з oversampling (x2/x4/x8)
- **GUI:** Кастомні SVG-віджети, програмний шум, конструктивістські акценти Malevich/Yermylov
- **Стабільність:** 10/10 стрес-тестів (100k processBlock, 1000 editor cycles, mono/stereo)

---

## 📜 Ліцензія

Proprietary. © 2026 Mykyta Shchur

---

*"Зроблено в Києві альтернативної реальності, де Баухаус переміг маркетологів"*
