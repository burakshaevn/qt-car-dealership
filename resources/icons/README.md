# Icons Layout

- `light/` - active icon set for light mode (current default).
- `dark/` - icon set for dark mode.
- `common/` - optional shared icons that are identical for both themes.

Current app paths remain backward-compatible via `resources.qrc` aliases, for example `:/logo.svg`.
At the moment these aliases point to `icons/light/*`.

Naming convention:
- use `lower_snake_case.svg` for all icon filenames and aliases.
