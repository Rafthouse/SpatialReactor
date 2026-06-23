# PROMPT ДЛЯ АГЕНТА: ПЕРЕЗБІРКА GUI SPATIAL REACTOR ЗА ЗАТВЕРДЖЕНИМ ДИЗАЙНОМ V6

Ти працюєш у чинному репозиторії **Spatial Reactor** на **JUCE 7/8, C++17, CMake**.

## Головне завдання

Не створюй нову дизайнерську інтерпретацію і не роби «щось схоже». Потрібно **імплементувати затверджений дизайн v6 максимально близько до піксельної відповідності**, як нативний JUCE GUI, і зібрати робочий VST3.

Джерела правди, які я додаю до завдання:

- `spatial_reactor_ukrainian_bauhaus_v6.html` — геометрія, стилі, взаємодія та логіка анімації.
- `spatial_reactor_ukrainian_bauhaus_v6.png` — еталонний вигляд при розмірі 900×560.

Скопіюй їх у репозиторій до `DesignReference/` і **не редагуй**. Вони є read-only референсом.

## Критично важливо

1. **Не використовуй WebView і не запускай HTML усередині плагіна.**
2. Реалізація має бути нативною: JUCE Components + JUCE Graphics + вбудований SVG через `BinaryData`.
3. **Не переписуй DSP і не змінюй звук**, окрім мінімальної інтеграції відсутнього параметра, якщо це об’єктивно потрібно.
4. Не видаляй чинні параметри APVTS, не змінюй їхні ID і не ламай старі пресети.
5. Не роби пам’яттєвих алокацій, блокувань або GUI-викликів в `processBlock()`.
6. Не використовуй OpenGL. Для цього GUI він не потрібний і лише додає ризиків.
7. Не заявляй, що робота завершена, доки не буде:
   - успішної Release-збірки VST3;
   - скриншота реального JUCE-інтерфейсу 900×560;
   - візуального порівняння з PNG-референсом;
   - тесту багаторазового відкриття/закриття редактора без падіння.

---

# 1. Спочатку зроби аудит

Перед змінами:

1. Знайди поточний `PluginEditor`, APVTS layout і GUI-компоненти.
2. Збери поточний проєкт як baseline.
3. Зафіксуй список чинних параметрів та їх ID.
4. Перевір, що DSP-тести або мінімальний standalone/test host працюють до зміни GUI.
5. Створи окрему гілку або checkpoint, наприклад `gui/spatial-reactor-v6`.

Не видаляй робочі DSP-класи через те, що старий GUI погано організований.

---

# 2. Точний розмір і геометрія

Редактор фіксований:

```cpp
static constexpr int kEditorWidth  = 900;
static constexpr int kEditorHeight = 560;
```

Використай:

```cpp
setSize (900, 560);
setResizable (false, false);
```

JUCE працює в логічних координатах, тому HiDPI масштабування виконує host/OS. Не додавай власне responsive-масштабування на першому етапі.

## Основні прямокутники

Усі bounds винести в `Source/GUI/GuiLayout.h` як іменовані `constexpr juce::Rectangle<int>` або функції.

| Елемент | X | Y | W | H |
|---|---:|---:|---:|---:|
| Editor | 0 | 0 | 900 | 560 |
| Inner frame | 13 | 13 | 874 | 534 |
| Header | 0 | 0 | 900 | 76 |
| Main content region | 28 | 94 | 844 | 374 |
| Reactor module | 28 | 94 | 483 | 374 |
| Controls module | 529 | 94 | 343 | 374 |
| Footer | 0 | 484 | 900 | 76 |

## Reactor module

- Core circle bounds: приблизно `x=69, y=161, w=260, h=260`.
- **Єдиний центр для всіх кіл і Amount knob:** приблизно `(199, 291)`.
- Amount knob: `92×92`, точно по центру Reactor Core.
- Readout strip: приблизно `x=359, y=132, w=132, h=318`.
- Жодне концентричне коло не може мати окремий або «візуально підправлений» центр. Усі радіуси обчислюються від `coreBounds.getCentre().toFloat()`.

## Controls module

- Три центри ручок приблизно:
  - Density: `(594, 165)`
  - Texture: `(701, 165)`
  - Air: `(807, 165)`
- Кожна мала ручка: `66×66`.
- Texture mode segmented control: приблизно `x=545, y=300, w=311, h=58`.
- Trust Governor region: приблизно `x=545, y=370, w=311, h=84`.
- SAFE/FREE toggle: `128×50`, праворуч у Trust Governor region.

Дрібні координати уточнюй за HTML і PNG. Допуск для центрів активних елементів: **не більше 2 px**.

---

# 3. Дизайн-система

Винеси кольори до `Source/GUI/GuiTheme.h`.

```cpp
kBackground      = 0xff17191c;
kPanel           = 0xff2c2e31;
kPanelSecondary  = 0xff242629;
kAluminium       = 0xffa8adb3;
kAluminiumDim    = 0xff666b71;
kInk             = 0xffe8e6e3;
kMuted           = 0xff95999f;
kAccent          = 0xff2d6bff;
kDeep            = 0xff101114;
kDanger          = 0xffd85a49;
```

Додаткові alpha-кольори відтворити з HTML:

- Accent soft: `rgba(45,107,255,0.18)`
- Main line: `rgba(168,173,179,0.32)`
- Module background: майже чорний з alpha приблизно 0.18

Візуальна мова: **Ukrainian Bauhaus Industrial / Eastern Bloc Hi-End**. Не додавати лампи, радіаційні піктограми, гранж, псевдоіржу, неонові sci-fi панелі або «вінтаж заради вінтажу».

Шрифти першої версії:

- Sans: `Segoe UI` на Windows, fallback на системний sans.
- Mono: `Consolas` на Windows, fallback на системний monospace.
- Для letter spacing використовуй `juce::GlyphArrangement`, а не звичайний `drawText`, де потрібна точна розрядка.

Не завантажуй шрифти з файлової системи під час роботи плагіна.

---

# 4. Гібридна SVG/JUCE реалізація

Створи:

```text
/Resources
  spatial_reactor_faceplate_v6.svg

/Source/GUI
  GuiLayout.h
  GuiTheme.h
  FaceplateComponent.h/.cpp
  ReactorCoreComponent.h/.cpp
  SpatialRotaryKnob.h/.cpp
  SegmentedSelector.h/.cpp
  TrustToggle.h/.cpp
  CorrelationMeter.h/.cpp
  TrackedLabel.h/.cpp
```

## SVG faceplate

У SVG винеси лише статичну геометрію:

- зовнішній graphite faceplate;
- inner frame;
- module borders;
- header/footer separators;
- logo geometry;
- corner cobalt marks;
- screws;
- статичні осі, шкали та декоративні лінії, якщо вони не анімуються.

Активні компоненти, текстові readouts, ручки, кнопки, correlation meter, concentric rings і glow малювати нативно JUCE поверх SVG.

SVG повинен бути вбудований у binary data. Наприклад:

```cmake
juce_add_binary_data(SpatialReactorAssets
    SOURCES
        Resources/spatial_reactor_faceplate_v6.svg)

target_link_libraries(SpatialReactor
    PRIVATE
        SpatialReactorAssets)
```

SVG парсити **один раз** у конструкторі або ресурсному кеші. Не парсити SVG у `paint()`.

Якщо SVG не завантажився, GUI має показати безпечний fallback, а не падати.

---

# 5. Reactor Core, кільця і glow

Це головний візуальний елемент. Реалізуй у `ReactorCoreComponent`.

Нормалізоване значення:

```cpp
const float t = juce::jlimit (0.0f, 1.0f, amountNormalized);
```

Всі кільця мають спільний центр Amount knob.

Діаметри з HTML:

```cpp
r1 = 212.0f * (0.76f + t * 0.20f);
r2 = 170.0f * (0.74f + t * 0.30f);
r3 = 126.0f * (0.72f + t * 0.42f);
r4 =  80.0f * (0.72f + t * 0.50f);
```

Використай `drawEllipse()` з тонкою 1 px лінією. Друге й четверте кільця мають cobalt accent. Інші — aluminium dim із відповідними alpha.

## Glow

Glow не має бути постійною синьою плямою. На нулі він практично відсутній і плавно наростає разом з Amount.

Відтворити поведінку HTML:

```cpp
const float glowAlpha = 0.52f * t;
const float glowScale = 0.76f + t * 0.38f;
const float glowDiameter = 170.0f * glowScale;
```

Намалюй кілька концентричних radial-gradient шарів із м’яким спадом alpha. Не використовуй дорогий realtime blur. Можна кешувати glow у `juce::Image` і перебудовувати лише коли значення Amount змінилося суттєво.

При Amount = 0 glow не повинен бути помітним. При 100% glow лишається делікатним, не перетворюється на неонову лампу.

Repaint тільки потрібної області.

---

# 6. Ручки

Усі ручки є custom JUCE Slider-компонентами або класами, сумісними з `AudioProcessorValueTreeState::SliderAttachment`.

Кут:

```cpp
startAngle = juce::degreesToRadians (-132.0f);
endAngle   = juce::degreesToRadians ( 132.0f);
```

Вигляд:

- темний круг;
- тонкий aluminium border;
- внутрішні кільця;
- cobalt/ivory indicator;
- короткі tick marks навколо;
- без stock JUCE LookAndFeel.

Розміри:

- Amount: `92×92`.
- Density/Texture/Air: `66×66`.

Поведінка:

- vertical drag;
- mouse wheel;
- double-click повертає default;
- display value оновлюється без мерехтіння;
- автоматизація DAW працює через APVTS attachment.

---

# 7. Параметри та прив’язки

Спочатку проаудитуй чинні ID. Очікувані параметри:

```text
amount
density
texture
air
trustMode
profile
```

Прив’язка:

- Amount knob → `amount`
- Density knob → `density`
- Texture knob → `texture`
- Air knob → `air`
- SAFE/FREE → `trustMode`, де `true = SAFE`, `false = FREE`
- AUTO/VOCAL/INST/MASTER/AMBIENT → `profile`

Дизайн також має selector `SILK / DUST / RUST`.

Якщо окремий choice-параметр уже існує, використай його. Якщо його немає:

1. Додай `textureMode` як `AudioParameterChoice` із трьома значеннями.
2. Default: `SILK`.
3. Не змінюй ID або семантику старого `texture`.
4. Реалізуй сумісне завантаження старих state/preset без `textureMode`.
5. Підключи `textureMode` до чинного `TextureEngine` без алокацій і без створення oversampler у `processBlock()`.

Усі attachments мають бути RAII-членами редактора або компонента. Порядок полів повинен гарантувати, що attachments знищуються раніше за контролери/APVTS, до яких вони звертаються.

Не використовуй `MessageManager::callAsync([this] {...})` із сирим `this`. Якщо async справді потрібен, застосуй `juce::Component::SafePointer` або `WeakReference`.

---

# 8. Сегментовані контролери

Створи custom `SegmentedSelector`, а не п’ять окремих stock TextButton.

## Profile

```text
AUTO | VOCAL | INST | MASTER | AMBIENT
```

- Header, праворуч.
- Висота 28 px.
- Тонкий border і vertical separators.
- Active segment: accent-soft background + 2 px cobalt underline.

## Texture mode

```text
SILK | DUST | RUST
```

- Повна ширина відповідного блоку.
- Та сама active-state мова.

Клавіатурна доступність бажана, але не змінюй геометрію.

---

# 9. SAFE/FREE toggle

Custom двосегментний control:

- `128×50`.
- Темна база.
- Active half має accent-soft fill і cobalt lower border.
- Indicator плавно переміщується між половинами за 120–180 ms.
- Не використовуй timer для простої анімації, якщо достатньо короткого component animator або interpolation у low-frequency timer.

SAFE = `trustMode == true`.

---

# 10. Readouts і meters

## Readout strip

Виводить:

- WIDTH
- PROFILE
- MODE
- FIELD bars

`WIDTH` для першої візуальної версії може відображатися як:

```cpp
widthDisplay = 1.0f + amountNormalized;
```

Якщо в DSP уже є реальне mapped width value, використовуй його замість декоративної формули.

`PROFILE` і `MODE` завжди показують реальний активний параметр.

## Correlation meter

- 32 сегменти.
- Значення `-1.00 … +1.00`.
- Негативна небезпечна зона може бути червоною.
- Значення передавати з processor до editor лише через lock-free atomic snapshot.
- GUI читає meters через Timer приблизно 24–30 Hz.
- Timer існує тільки в editor/GUI.
- Жодного GUI callback з audio thread.

Якщо correlation estimator уже є, використай його. Якщо немає, додай легкий блоковий estimator у processor і публікуй тільки `std::atomic<float>`.

## FIELD bars

Бажано пов’язати із реальною Side/high-band energy. На першому UI-pass допускається залежність від Amount, але це має бути явно позначено як тимчасовий GUI fallback, а не вигаданий DSP-meter.

---

# 11. Структура PluginEditor

`PluginEditor` повинен бути композицією компонентів, а не 1000-рядковим `paint()`.

Рекомендовано:

```cpp
FaceplateComponent m_faceplate;
ReactorCoreComponent m_reactorCore;
SpatialRotaryKnob m_amount;
SpatialRotaryKnob m_density;
SpatialRotaryKnob m_texture;
SpatialRotaryKnob m_air;
SegmentedSelector m_profileSelector;
SegmentedSelector m_textureSelector;
TrustToggle m_trustToggle;
CorrelationMeter m_correlationMeter;
```

`resized()` лише встановлює bounds із `GuiLayout.h`.

Статичні decorative components повинні мати:

```cpp
setInterceptsMouseClicks (false, false);
```

Не роби layout на основі випадкових magic numbers, розкиданих по `.cpp`.

---

# 12. Продуктивність та стабільність

Обов’язково:

- жодних файлових I/O при відкритті GUI;
- жодного SVG parse у `paint()`;
- жодних heap allocations в audio thread;
- жодних mutex між GUI та DSP;
- жодних dangling callbacks;
- editor можна відкрити/закрити мінімум 100 разів у test host;
- автоматизація параметрів не викликає crashes;
- перемикання profile/texture/safe-free не створює DSP-об’єкти на льоту в audio thread;
- при відсутньому resource GUI не падає;
- не змінюй чинну latency без DSP-причини.

Після GUI rebuild запусти наявні тести, standalone/minimal host і pluginval, якщо він доступний.

---

# 13. Візуальна прийомка

Зроби скриншот реального JUCE editor при 900×560 із тестовими значеннями:

```text
Amount  = 58%
Density = 46%
Texture = 32%
Air     = 24%
Profile = AUTO
Mode    = SILK
Trust   = SAFE
```

Порівняй зі `spatial_reactor_ukrainian_bauhaus_v6.png`.

Вимоги:

- borders/module edges: похибка не більше 1 px;
- centers of knobs/core rings: не більше 2 px;
- кольори близькі до референсу, без самовільних нових accent colors;
- concentric rings точно центровані відносно Amount;
- glow відсутній на 0%, м’яко з’являється при збільшенні;
- усі написи не обрізані;
- жоден control не виходить за module bounds;
- вигляд у host та standalone збігається.

Якщо можливо, створи автоматичний image-diff або хоча б overlay 50/50 і додай його в звіт.

---

# 14. Що має бути у фінальному звіті

Не пиши загальних фраз. Надай:

1. Повний список змінених/створених файлів.
2. Які APVTS ID використано і чи був доданий `textureMode`.
3. Чи змінювався DSP. Якщо так, конкретно що і навіщо.
4. Команди збірки.
5. Фактичний шлях до зібраного `.vst3`.
6. Результати Debug і Release build.
7. Результат тесту editor open/close.
8. Скриншот JUCE GUI.
9. Перелік відомих відмінностей від референсу, навіть дрібних.
10. Підтвердження, що HTML/WebView у runtime не використовується.

## Заборонені фінальні відмовки

Не можна завершувати відповіддю на кшталт:

- «Я створив основу, далі можна доробити»;
- «візуально приблизно відповідає»;
- «компіляцію не перевіряв»;
- «SVG доданий, але контролери поки заглушки»;
- «метри статичні, але це неважливо».

Результат першого етапу: **робочий VST3 із повністю інтегрованим GUI v6, усі видимі контролери функціонують, GUI максимально відповідає еталонному PNG, плагін не падає при відкритті в host.**
