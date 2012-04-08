SPECS
--------------
Sub-layouts should be rendered via their own renderers.
  - Curated $display object will be passed to the renderer's init() method.
  - All prep_run should occur in the Frame, and corresponding prepared data mapped to renderer.
    - $prepared[panes] = pids
    - $prepared[regions] = region styles/settings/pids



CAUTION
--------------
Display's css_id should not propagate to layouts.
Cloning Displays is memory intensive. Be sure to unset() after use.