# Icons

- `ui/` — monochrome line icons (24×24, stroke). They are drawn in black and
  recoloured at runtime by `ThemeManager::tintedIcon()` / `bindIcon()` with a
  palette token, so a single set serves every theme.
- `light/`, `dark/` — theme-specific artwork (logo, wordmark, stylesheet
  glyphs such as the combo-box chevron). `ThemeManager::icon()` looks here first
  and falls back to `ui/`.
- `common/` — glyphs identical in all themes (e.g. the check-box tick).

Naming: `lower_snake_case.svg`. Every file is registered in `resources/resources.qrc`.
