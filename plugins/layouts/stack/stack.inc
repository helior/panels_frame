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
  // panels_frame_stack_synthesized_display_renderer($vars);
  panels_frame_stack_distribute_layout_content($vars);
}

function theme_panels_frame_stack($vars) {
  $output = '';

  $output .= '<div class="panels-frame">';
  foreach ($vars['frames'] as $name => $content) {
    $output .= '<div class="frame-' . $name . '">' . $content . '</div>';
  }
  $output .= '</div>';

  // $output .= '<hr />';
  //
  // foreach ($vars['content'] as $content) {
  //   $output .= $content;
  // }

  return $output;
}

function panels_frame_stack_distribute_layout_content(&$vars) {
  // Back-up original renderer prefix for the encompassing layout render
  $renderer_prefix = $vars['renderer']->prefix;
  $renderer_layout = $vars['renderer']->plugins['layout'];

  foreach ($vars['display']->panels_frame as $name => $frame) {
    $content = $settings = array();

    // Distribute content
    foreach (array_keys($frame['regions']) as $region) {
      $key = $name . '_' . $region;
      $content[$region] = $vars['content'][$key];
    }

    // Choose theme
    if ($vars['renderer']->admin && !empty($frame['plugin']['admin theme'])) {
      $theme = $frame['plugin']['admin theme'];
    }
    else {
      $theme = $frame['plugin']['theme'];
    }

    $vars['frames'][$name] = '';
    $vars['renderer']->prefix = '';
    $vars['renderer']->plugins['layout'] = $frame['plugin'];

    $vars['renderer']->add_meta();
    $vars['frames'][$name] = $vars['renderer']->prefix . theme($theme, array('content' => $content, 'display' => $vars['display'], 'layout' => $frame['plugin'], 'renderer' => $vars['renderer']));
  }

  $vars['renderer']->prefix = $renderer_prefix;
  $vars['renderer']->plugins['layout'] = $renderer_layout;
}

/**
 * Create clones of original display objects to make Panels believe that they
 * are rendering original layouts. This breaks using the editor renderer since
 * it renders each layout with an hard-coded ID.
 */
function panels_frame_stack_synthesized_display_renderer(&$vars) {
  // Clone Display
  //  -+ curate content
  //  -+ curate panels
  //  -+ clear css_id (css_id will be applicable to wrapper div)
  //  -+ update layout
  //  - curate layout_settings
  //  - curate panel_settings
  //  -+ set skip_cache TRUE
  //  -+ remove panels_frame
  // Re-create Renderer
  //  -+ $renderer = panels_get_renderer_handler($engine);
  //  -+ curate/copy over prepared panes and regions
  //  -+ curate/copy over rendered panes and regions
  //  - Patch Panels to check for $renderer->rendered[regions] and $renderer->rendered[panes] before trying to render
  //  - Worry about using the Pane title as the Page title. Maybe.

  $renderer = $vars['renderer'];

  foreach ($vars['display']->panels_frame as $name => $frame) {
    $panels = $content = $prepared = $rendered = array();

    // Deep copy of the original display
    $display = unserialize(serialize($vars['display']));

    // Frame metadata isn't necessary in nested displays
    unset($display->panels_frame);

    // Let the display know the /real/ layout
    $display->layout = $frame['layout'];

    // Prevent the css ID to propogate to nested displays.
    $display->css_id = NULL;

    // Skip cache at this scope since the parent display can handle itself.
    $display->skip_cache = TRUE;

    // Loop through each region of the nested layout and map content values
    // accordingly.
    foreach (array_keys($frame['regions']) as $region) {
      $pseudo_region = $name . '_' . $region;

      // Map display regions back to display in their "original" form.
      if (isset($display->panels[$pseudo_region])) {
        $panels[$region] = $display->panels[$pseudo_region];

        // Map each piece of content within the regions back to their "original" form.
        foreach ($panels[$region] as $pid) {
          $content[$pid] = $display->content[$pid];

          // Also update the pane's reference to their assigned region.
          $content[$pid]->panel = $region;
        }
      }

      // Map renderer's prepared/rendered values
      $rendered['regions'][$region] = $renderer->rendered['regions'][$pseudo_region];
      $prepared['regions'][$region] = $renderer->prepared['regions'][$pseudo_region];

      foreach ($prepared['regions'][$region]['pids'] as $pid) {
        $rendered['panes'][$pid] = $renderer->rendered['panes'][$pid];
        $prepared['panes'][$pid] = $renderer->prepared['panes'][$pid];
        $prepared['panes'][$pid]->panel = $region;
      }
    }

    $display->panels = $panels;
    $display->content = $content;

    $new_renderer = panels_get_renderer_handler($renderer->plugin['name'], $display);
    $new_renderer->prepared = $prepared;
    $new_renderer->rendered = $rendered;
    $new_renderer->prep_run = TRUE;
    $vars['frames'][$name] = panels_render_display($display, $new_renderer);
  }
}