.. _text_layers:

Editable Text Layers
====================

Scope
-----

- Create a ``TextLayer_*bit``
- Style/format via high-level range helpers (``*_range``, ``*_text``, ``*_all``)
- Use lower-level run APIs (``style_run(index)``, ``paragraph_run(index)``, ``set_style_run_*``)
- Read attributes from layer-level and run-level APIs
- Switch orientation between horizontal and vertical text
- Switch between box text and point text (``TextLayer`` creation defaults to box text)
- Set character direction (LTR / RTL)

C++
---

Create a layer:

.. code-block:: cpp

	using namespace NAMESPACE_PSAPI;

	LayeredFile<bpp8_t> doc = { Enum::ColorMode::RGB, 1200u, 1800u };
	auto layer = TextLayer<bpp8_t>::create(
	    "Caption",
	    "Hello\nWorld",
	    "ArialMT",
	    36.0,
	    {1.0, 0.0, 0.0, 0.0}, // [A, R, G, B]
	    120.0, 160.0,
	    600.0, 260.0
	);
	doc.add_layer(layer);

Style/format (high-level first)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Use these first:

- Character styles: ``style_all()``, ``style_text(needle, occurrence=0)``, ``style_range(start, end)``
- Paragraph styles: ``paragraph_all()``, ``paragraph_text(needle, occurrence=0)``, ``paragraph_range(start, end)``
- ``occurrence`` behavior for ``*_text(...)``: ``0`` (default) = all matches, ``1`` = first match, ``2`` = second match, etc.

Note: ``start/end`` are UTF-16 code-unit indices.

.. code-block:: cpp

	// High-level range helpers
	layer->style_all().set_font_size(36.0).set_fill_color({1.0, 0.0, 0.0, 0.0});
	layer->style_text("World").set_underline(true); // occurrence omitted -> all matches
	layer->style_text("World", 1).set_font("Arial-BoldMT").set_stroke_flag(true).set_outline_width(2.0); // first match
	layer->style_range(0, 5).set_underline(true);
	layer->style_all().set_character_direction(TextLayerEnum::CharacterDirection::RightToLeft); // RTL
	layer->style_all().set_character_direction(TextLayerEnum::CharacterDirection::LeftToRight); // LTR
	layer->paragraph_all().set_justification(TextLayerEnum::Justification::Center);

	// Orientation: horizontal <-> vertical
	layer->set_orientation(TextLayerEnum::WritingDirection::Vertical);
	layer->set_orientation(TextLayerEnum::WritingDirection::Horizontal);

	// Text frame type: box <-> point (create() defaults to box text)
	layer->convert_to_point_text();            // box -> point
	layer->convert_to_box_text(600.0, 260.0); // point -> box

Lower-level API (run-level control)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

When you need exact run control:

- Discover runs with ``style_run_lengths()`` / ``paragraph_run_lengths()``
- Edit one run with ``style_run(index)`` / ``paragraph_run(index)``
- Or use direct setters: ``set_style_run_*``, ``set_paragraph_run_*``

.. code-block:: cpp

	layer->split_style_run(0, 6);
	layer->style_run(1).set_faux_bold(true);
	layer->set_style_run_font_size(1, 42.0);

Get attributes:

.. code-block:: cpp

	auto text = layer->text();
	auto [x, y] = layer->position();
	auto box_w = layer->box_width();
	auto box_h = layer->box_height();
	auto orientation = layer->orientation();

	if (auto lengths = layer->style_run_lengths(); lengths.has_value()) {
	    for (size_t i = 0; i < lengths->size(); ++i) {
	        auto font_idx = layer->style_run_font(i);
	        auto size = layer->style_run_font_size(i);
	        auto fill = layer->style_run_fill_color(i);
	        auto char_dir = layer->style_run_character_direction(i);
	    }
	}
	if (auto p_lengths = layer->paragraph_run_lengths(); p_lengths.has_value()) {
	    for (size_t i = 0; i < p_lengths->size(); ++i) {
	        auto just = layer->paragraph_run_justification(i);
	    }
	}

Important when editing an existing PSD
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

If you modify text on a PSD you read from disk, call ``invalidate_text_cache()`` on the
``LayeredFile`` before writing. This triggers Photoshop's text-update prompt on open.
Without this, text can appear invisible until Photoshop refreshes text internals.

.. code-block:: cpp

	doc.invalidate_text_cache();
	doc.write("edited.psd");

Python
------

Create a layer:

.. code-block:: python

	import photoshopapi as psapi

	doc = psapi.LayeredFile_8bit(psapi.enum.ColorMode.rgb, 1200, 1800)
	layer = psapi.TextLayer_8bit(
	    layer_name="Caption",
	    text="Hello\nWorld",
	    font="ArialMT",                   # PostScript font name
	    font_size=36.0,
	    fill_color=[1.0, 0.0, 0.0, 0.0], # [A, R, G, B]
	    position_x=120.0,
	    position_y=160.0,
	    box_width=600.0,
	    box_height=260.0,
	)
	doc.add_layer(layer)

Style/format (high-level first)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Same high-level API semantics as C++ above (``occurrence=0`` means all matches).

.. code-block:: python

	# Character styling
	layer.style_all().set_font_size(36.0).set_fill_color([1.0, 0.0, 0.0, 0.0])
	layer.style_text("World", 1).set_font("Arial-BoldMT").set_stroke_flag(True).set_outline_width(2.0)
	layer.style_range(0, 5).set_underline(True)
	layer.style_all().set_character_direction(psapi.enum.CharacterDirection.RightToLeft)  # RTL
	layer.style_all().set_character_direction(psapi.enum.CharacterDirection.LeftToRight)   # LTR

	# Paragraph styling
	layer.paragraph_all().set_justification(psapi.enum.Justification.Center)

	# Orientation: horizontal <-> vertical
	layer.set_orientation(psapi.enum.WritingDirection.Vertical)
	layer.set_orientation(psapi.enum.WritingDirection.Horizontal)

	# Text frame type: box <-> point (TextLayer_8bit(...) defaults to box text)
	layer.convert_to_point_text()             # box -> point
	layer.convert_to_box_text(600.0, 260.0)   # point -> box

Lower-level API (run-level control)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Same run-control API as C++ above (``style_run_lengths``, ``paragraph_run_lengths``,
``style_run(index)``, ``paragraph_run(index)``, and direct ``set_*`` run setters).

.. code-block:: python

	layer.split_style_run(0, 6)               # "Hello " | "World"
	layer.style_run(1).set_faux_bold(True)
	layer.style_run(1).set_fill_color([1.0, 1.0, 0.0, 0.0])

	# equivalent direct setter
	layer.set_style_run_font_size(1, 42.0)

Get attributes
^^^^^^^^^^^^^^

High-level layer reads:

.. code-block:: python

	text = layer.text
	x, y = layer.position()
	box_w = layer.box_width()
	box_h = layer.box_height()
	orientation = layer.orientation()

Run-level reads:

.. code-block:: python

	for i, run_len in enumerate(layer.style_run_lengths() or []):
	    font_idx = layer.style_run_font(i)
	    font_ps = layer.font_postscript_name(font_idx) if font_idx is not None else None
	    size = layer.style_run_font_size(i)
	    fill = layer.style_run_fill_color(i)
	    stroke = layer.style_run_stroke_color(i)
	    underline = layer.style_run_underline(i)
	    char_dir = layer.style_run_character_direction(i)

	for i, run_len in enumerate(layer.paragraph_run_lengths() or []):
	    just = layer.paragraph_run_justification(i)

Proxy-style run reads:

.. code-block:: python

	run = layer.style_run(0)
	size = run.font_size
	fill = run.fill_color
	outline_w = run.outline_width

When editing an existing PSD, call:

.. code-block:: python

	doc.invalidate_text_cache()
	doc.write("edited.psd")
