# Cars Images Layout

- `resources/cars/<Model Name>/<color>.png`

`image_url` in DB stays model-relative (for example: `Mercedes-AMG GT 43/white.png`).
The app resolves both layouts for compatibility:
- `resources/cars/<image_url>` (new)
- `resources/<image_url>` (legacy)
