<?php

$plugin = array(
  'title' => t('Frame: Stack'),
  'category' => t('Builders'),
  'icon' => 'envelope.png',
  // 'builder' => TRUE,
  // 'builder tab title' => 'Stack this mother effers!',
  // 'get child' => 'panels_frame_stack_get_sublayout',
  // 'get children' => 'panels_frame_stack_get_sublayouts',
  'regions function' => 'panels_frame_stack_region',
  'theme' => 'panels_frame_stack',
);

// function panels_frame_stack_fake_layout($name, $plugin) {
//   return array(
//     'name' => $name,
//     'category' => 'Frame: Stack',
//     'title' => ucfirst($name),
//     'description' => $name . ' is a "Stack" instance',
//     'builder' => FALSE,
//     'builder tabe title' => NULL,
//   ) + $plugin;
// }

function panels_frame_stack_fake_frames() {
  return array(
    'first' => array(
      'layout' => 'twocol',
    ),
    'second' => array(
      'layout' => 'threecol_25_50_25',
    ),
  );
}

// function panels_frame_stack_get_sublayout($plugin, $layout_name, $sublayout_name) {
//   // @todo this should load from Ctools Export/CRUD API
//   return panels_frame_stack_fake_layout('bob', $plugin);
// }
//
// function panels_frame_stack_get_sublayouts($plugin, $layout_name) {
//   // @todo this should load from Ctools Export/CRUD API
//   $sub_layouts = array(
//     'bob' => panels_frame_stack_fake_layout('bob', $plugin),
//     'steve' => panels_frame_stack_fake_layout('steve', $plugin),
//     'bill' => panels_frame_stack_fake_layout('bill', $plugin),
//   );
//   return $sub_layouts;
// }

/**
 * Regions callback.
 * Collect the regions from the compiled layouts.
 */
function panels_frame_stack_region(&$display, $settings, $base_layout) {
  $regions = array();

  // @todo replace with export_ui objects
  foreach (panels_frame_stack_fake_frames() as $name => $frame) {
    // Avoid recursion. @todo add form validation to prevent this from occurring on the front-end.
    if ($display->layout == $frame['layout']) {
      watchdog('panels_frame', '@frame cannot be embedded within itself!', array('@frame' => $frame['layout']), WATHDOG_WARNING);
      continue;
    }

    $frame['plugin'] = panels_get_layout($frame['layout']);

    foreach (panels_get_regions($frame['plugin'], $display) as $region => $label) {
      $frame['regions'][$region]['region'] = $region;
      $regions[$name . '_' . $region] = $label;
    }

    $display->panels_frame[$name] = $frame;
  }

  return $regions;
}

/**
 * @todo render each individual sub-layout; map content to appropriate regions.
 */
function template_preprocess_panels_frame_stack(&$vars) {
  dpm($vars,'vars');
  $original_renderer = $vars['renderer'];

  foreach ($vars['display']->panels_frame as $name => $frame) {
    $temp_display = clone $vars['display'];
    unset($temp_display->panels_frame);
    $temp_display->layout = $frame['layout'];

    $renderer = panels_get_renderer_handler($vars['display']->renderer, $temp_display);
    foreach ($frame['regions'] as $region => $data) {
      $renderer->rendered['regions'][$region] = $original_renderer->rendered['regions'][$name . '_' . $region];
    }

    // hook_panels_pre_render
    // $renderer->add_meta();
    // Determine renderer theme
    // $renderer->prefix
    // theme('layout')
    // $renderer->suffix
    // hook_panels_post_render
    // @todo but how will this work with the Editor/IPE Renderers?

    // Clone Display
    //  - curate content
    //  - curate panels
    //  - clear css_id (css_id will be applicable to wrapper div)
    //  - update layout
    //  - curate layout_settings
    //  - curate panel_settings
    //  - set skip_cache TRUE
    //  - remove panels_frame
    // Re-create Renderer
    //  - $renderer = panels_get_renderer_handler($engine);
    //  - curate/copy over prepared panes and regions
    //  - curate/copy over rendered panes and regions
    //  - Patch Panels to check for $renderer->rendered[regions] and $renderer->rendered[panes] before trying to render
    //  - Worry about using the Pane title as the Page title. Maybe.
    $vars['frames'][$name] = $output;
  }
}

function theme_panels_frame_stack($vars) {
  $output = '';
  foreach ($vars['layout']['regions'] as $name => $label) {
    $output .= '<div class="xxregion-' . $name . '">' . $vars['content'][$name] . '</div>';
  }

  return $output;
}